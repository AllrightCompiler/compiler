#include "backend/armv7/program.hpp"
#include "backend/armv7/arch.hpp"
#include "backend/armv7/instruction.hpp"

#include "common/common.hpp"
#include "common/ir.hpp"

#include <cassert>
#include <map>
#include <unordered_map>

namespace armv7 {

inline bool is_power_of_2(int x) { return (x & (x - 1)) == 0; }

template <typename Container = std::list<std::unique_ptr<Instruction>>>
void emit_load_imm(Container &cont, typename Container::iterator it, Reg dst,
                   int imm) {
  if (is_imm8m(imm))
    cont.emplace(it, new Move{dst, Operand2::from(imm)});
  else if (is_imm8m(~imm))
    cont.emplace(it, new Move{dst, Operand2::from(~imm), true});
  else {
    uint32_t x = uint32_t(imm);
    auto lo = x & 0xffff, hi = x >> 16;
    cont.emplace(it, new MovW(dst, lo));
    if (hi > 0)
      cont.emplace(it, new MovT(dst, hi));
  }
}

void emit_load_imm(BasicBlock *bb, Reg dst, int imm) {
  emit_load_imm(bb->insns, bb->insns.end(), dst, imm);
}

class ProgramTranslator {
  armv7::Program &dst;
  const ir::Program &src;

  struct CompareInfo {
    ExCond cond;
    Reg lhs, rhs;
  };
  struct LogicalNotInfo {
    bool flip;
    Reg src;
  };

  std::unordered_map<ir::BasicBlock *, armv7::BasicBlock *> bb_map;
  std::map<Reg, CompareInfo> cmp_info;
  std::map<Reg, LogicalNotInfo> lnot_info;

  void translate_function(armv7::Function &dst_fn, const ir::Function &src_fn) {
    bb_map.clear();
    cmp_info.clear();
    lnot_info.clear();

    // 新的入口块，用于prologue
    auto entry_bb = new armv7::BasicBlock;
    entry_bb->label = ".entry." + dst_fn.name;
    dst_fn.bbs.emplace_back(entry_bb);

    auto &params = src_fn.sig.param_types;
    int nr_params = params.size();
    int nr_gp_params, nr_fp_params;
    nr_gp_params = nr_fp_params = 0;
    std::vector<int> stack_params;
    for (int i = 0; i < nr_params; ++i) {
      auto type = ir_to_machine_reg_type(params[i].base_type);
      Reg dst = Reg{type, -(i + 1)};
      if (type != Fp) {
        if (nr_gp_params++ < NR_ARG_GPRS) {
          entry_bb->push(new Move{dst, Operand2::from(Reg{type, r0 + i})});
        } else {
          stack_params.push_back(i);
        }
      } else {
        if (nr_fp_params++ < NR_ARG_FPRS) {
          entry_bb->push(new Move{dst, Operand2::from(Reg{type, s0 + i})});
        } else {
          stack_params.push_back(i);
        }
      }
    }

    for (auto &ir_bb : src_fn.bbs) {
      auto bb = new armv7::BasicBlock;
      bb_map[ir_bb.get()] = bb;
      dst_fn.bbs.emplace_back(bb);
    }
    dst_fn.regs_used = src_fn.nr_regs;

    auto prev_entry = bb_map[src_fn.bbs.begin()->get()];
    BasicBlock::add_edge(entry_bb, prev_entry);

    int i = 1;
    auto dst_it = std::next(dst_fn.bbs.begin()); // skip entry
    for (auto src_it = src_fn.bbs.begin(); src_it != src_fn.bbs.end();
         ++src_it, ++dst_it, ++i) {
      BasicBlock *next_bb = nullptr;
      if (i + 1 < dst_fn.bbs.size())
        next_bb = std::next(dst_it)->get();

      auto bb = dst_it->get();
      bb->label = dst.new_label();
      for (auto &ir_insn : (*src_it)->insns)
        translate_instruction(dst_fn, bb, ir_insn.get(), next_bb);
    }

    int params_size = 0;
    for (int i : stack_params) {
      auto obj = new StackObject;
      obj->size = 4; // 参数的size总是4
      obj->offset = params_size;
      params_size += obj->size;
      dst_fn.param_objs.push_back(obj);
      dst_fn.stack_objects.emplace_back(obj);

      Reg dst = Reg::from(params[i].base_type, -(i + 1));
      // entry_bb->push(new LoadStack{dst, obj, 0});
      dst_fn.defer_stack_param_load(dst, obj);
    }
  }

  void translate_instruction(armv7::Function &fn, BasicBlock *bb,
                             ir::Instruction *ins, BasicBlock *next_bb) {
    namespace ii = ir::insns;
    TypeCase(alloca, ii::Alloca *, ins) {
      auto obj = new StackObject;
      obj->size = alloca->type.size();
      fn.normal_objs.push_back(obj);
      fn.stack_objects.emplace_back(obj);

      bb->push(new LoadStackAddr{Reg::from(alloca->dst), obj, 0});
    }
    else TypeCase(load, ii::Load *, ins) {
      bb->push(new Load{Reg::from(load->dst), Reg::from(load->addr), 0});
    }
    else TypeCase(load, ii::LoadAddr *, ins) {
      bb->push(new LoadAddr{Reg::from(load->dst), load->var_name});
    }
    else TypeCase(load, ii::LoadImm *, ins) {
      Reg dst = Reg::from(load->dst);
      if (!dst.is_float()) {
        emit_load_imm(bb, dst, load->imm.iv);
      } else {
        float imm = load->imm.fv;
        if (false && is_vmov_f32_imm(imm)) { // TODO
          bb->push(new Move(dst, Operand2::from(imm)));
        } else {
          Reg t = fn.new_reg(General);
          emit_load_imm(bb, t, *reinterpret_cast<int *>(&imm));
          bb->push(new Move(dst, Operand2::from(t)));
        }
      }
    }
    else TypeCase(store, ii::Store *, ins) {
      Reg val = Reg::from(store->val);
      Reg addr = Reg::from(store->addr);
      bb->push(new Store{val, addr, 0});
    }
    else TypeCase(gep, ii::GetElementPtr *, ins) {
      Reg index_reg = Reg::from(gep->indices[0]);
      int nr_indices = gep->indices.size();
      for (int i = 1; i < nr_indices; ++i) {
        Reg t = fn.new_reg(General);
        Reg s = Reg::from(gep->indices[i]);
        int dim = gep->type.dims[i];
        if (is_power_of_2(dim)) {
          bb->push(new FullRType{
              FullRType::Op::Add, t, s,
              Operand2::from(LSL, index_reg, __builtin_ctz(dim))});
        } else {
          Reg imm_reg = fn.new_reg(General);
          emit_load_imm(bb, imm_reg, dim);
          bb->push(new FusedMul{t, index_reg, imm_reg, s});
        }
        index_reg = t;
      }
      for (int i = nr_indices; i < static_cast<int>(gep->type.dims.size());
           ++i) {
        Reg t = fn.new_reg(General);
        int dim = gep->type.dims[i];
        if (is_power_of_2(dim)) {
          bb->push(
              new Move{t, Operand2::from(LSL, index_reg, __builtin_ctz(dim))});
        } else {
          Reg imm_reg = fn.new_reg(General);
          emit_load_imm(bb, imm_reg, dim);
          bb->push(new RType{RType::Op::Mul, t, index_reg, imm_reg});
        }
        index_reg = t;
      }
      Reg dst = Reg::from(gep->dst);
      Reg base = Reg::from(gep->base);
      bb->push(new FullRType{FullRType::Op::Add, dst, base,
                             Operand2::from(LSL, index_reg, 2)});
    }
    else TypeCase(cvt, ii::Convert *, ins) {
      auto const dst = Reg::from(cvt->dst);
      auto const src = Reg::from(cvt->src);
      auto const temp = fn.new_reg(RegType::Fp);
      if (cvt->dst.type != cvt->src.type) {
        if (cvt->dst.type == ScalarType::Float) { // int -> float
          bb->push(new Move(temp, Operand2::from(src)));
          bb->push(new Convert(dst, temp, ConvertType::Int2Float));
        } else { // float -> int
          bb->push(new Convert(temp, src, ConvertType::Float2Int));
          bb->push(new Move(dst, Operand2::from(temp)));
        }
      }
    }
    else TypeCase(unary, ii::Unary *, ins) {
      Reg dst = Reg::from(unary->dst);
      Reg src = Reg::from(unary->src);
      switch (unary->op) {
      case UnaryOp::Sub:
        if (!dst.is_float()) {
          bb->push(new IType{IType::Op::RevSub, dst, src, 0});
        } else {
          // TODO: emit vneg
        }
        break;
      case UnaryOp::Not: {
        if (!dst.is_float() && !src.is_float()) {
          // bb->push(new Move{dst, Operand2::from(0)});
          // bb->push(new Compare{src, Operand2::from(0)});
          // bb->push(ExCond::Eq, new Move{dst, Operand2::from(1)});
          // alternative: clz dst, src; lsr dst, dst, #5
          bb->push(new CountLeadingZero{dst, src});
          bb->push(new Move{dst, Operand2::from(LSR, dst, 5)});
        } else {
          auto cmp = std::make_unique<Compare>(src, Operand2::from(0));
          bb->push(ExCond::Eq, new PseudoCompare(cmp.release(), dst));
        }

        if (cmp_info.count(src)) {
          auto &cmp = cmp_info[src];
          cmp_info[dst].cond = logical_not(cmp.cond);
          cmp_info[dst].lhs = cmp.lhs;
          cmp_info[dst].rhs = cmp.rhs;
        } else if (lnot_info.count(src)) {
          lnot_info[dst].flip = !lnot_info[src].flip;
          lnot_info[dst].src = lnot_info[src].src;
        } else {
          lnot_info[dst].flip = true;
          lnot_info[dst].src = src;
        }
        break;
      }
      default:
        break;
      }
    }
    else TypeCase(binary, ii::Binary *, ins) {
      Reg dst = Reg::from(binary->dst);
      Reg s1 = Reg::from(binary->src1);
      Reg s2 = Reg::from(binary->src2);
      switch (binary->op) {
      case BinaryOp::Add:
      case BinaryOp::Sub:
      case BinaryOp::Mul:
      case BinaryOp::Div:
        bb->push(new RType{RType::from(binary->op), dst, s1, s2});
        break;
      case BinaryOp::Mod: {
        // TODO: 确认负数取模的行为
        Reg t = fn.new_reg(General);
        bb->push(new RType{RType::Op::Div, t, s1, s2});
        bb->push(new FusedMul{dst, t, s2, s1, true});
        break;
      }
      case BinaryOp::Eq:
      case BinaryOp::Neq:
      case BinaryOp::Geq:
      case BinaryOp::Gt:
      case BinaryOp::Leq:
      case BinaryOp::Lt: {
        auto cond = from(binary->op);
        auto inner = new Compare{s1, Operand2::from(s2)};
        bb->push(cond, new PseudoCompare{inner, dst});

        auto &cmp = cmp_info[dst];
        cmp.cond = cond;
        cmp.lhs = s1;
        cmp.rhs = s2;
        break;
      }
      default:
        __builtin_unreachable();
      }
    }
    else TypeCase(call, ii::Call *, ins) {
      // NOTE: 需要用栈传的参数现在是push入栈的，这要求调用后的sp调整量完全匹配
      // 另一种方式是先减sp，然后生成StoreStack
      int nr_args, nr_gp_args, nr_fp_args;
      std::vector<int> reg_args, stack_args;
      nr_gp_args = nr_fp_args = 0;
      nr_args = call->args.size();

      for (int i = 0; i < nr_args; ++i) {
        Reg arg_reg = Reg::from(call->args[i]);
        if (!arg_reg.is_float()) {
          if (nr_gp_args++ < NR_ARG_GPRS)
            reg_args.push_back(i);
          else
            stack_args.push_back(i);
        } else {
          if (nr_fp_args++ < NR_ARG_FPRS)
            reg_args.push_back(i);
          else
            stack_args.push_back(i);
        }
      }
      for (auto it = stack_args.rbegin(); it != stack_args.rend(); ++it) {
        Reg arg_reg = Reg::from(call->args[*it]);
        bb->push(new Push{{arg_reg}});
      }
      for (auto i : reg_args) {
        Reg arg_reg = Reg::from(call->args[i]);
        if (!arg_reg.is_float()) {
          bb->push(
              new Move{Reg{arg_reg.type, r0 + i}, Operand2::from(arg_reg)});
        } else {
          // TODO: move or explicit vmov?
          bb->push(new Move{Reg{Fp, s0 + i}, Operand2::from(arg_reg)});
        }
      }
      bb->push(new Call{call->func, nr_gp_args, nr_fp_args});
      // 这里钻了个空子，这个语法中函数的返回值只能是int或float
      // 如果接收返回值的寄存器类型是String表明实际上无返回值
      if (call->dst.type != String)
        bb->push(
            new Move{Reg::from(call->dst),
                     Operand2::from(Reg::from(call->dst.type, 0))}); // r0或s0
      int sp_adjustment = 4 * stack_args.size(); // NOTE: 所有标量类型都是4字节
      if (sp_adjustment != 0)
        bb->push(new AdjustSp{sp_adjustment});
    }
    else TypeCase(ret, ii::Return *, ins) {
      // NOTE: 此指令不一定是bx lr，可能实际是到exit_bb的跳转
      if (ret->val)
        bb->push(
            new Move{Reg::from(ret->val->type, 0),
                     Operand2::from(Reg::from(ret->val.value()))}); // r0或s0
      bb->push(new Return{});
    }
    else TypeCase(jump, ii::Jump *, ins) {
      auto target = bb_map[jump->target];
      BasicBlock::add_edge(bb, target);
      if (target != next_bb)
        bb->push(new Branch{target});
    }
    else TypeCase(br, ii::Branch *, ins) {
      Reg val = Reg::from(br->val);
      auto true_target = bb_map[br->true_target];
      auto false_target = bb_map[br->false_target];
      BasicBlock::add_edge(bb, true_target);
      BasicBlock::add_edge(bb, false_target);

      auto emit_branch = [bb, next_bb, true_target, false_target](ExCond cond) {
        if (next_bb == false_target)
          bb->push(cond, new Branch{true_target});
        else if (next_bb == true_target)
          bb->push(logical_not(cond), new Branch{false_target});
        else {
          // TODO: 根据距离决定条件跳转目标
          bb->push(cond, new Branch{true_target});
          bb->push(new Branch{false_target});
        }
      };

      if (cmp_info.count(val)) { // 显式二元比较
        auto &cmp = cmp_info[val];
        bb->push(new Compare{cmp.lhs, Operand2::from(cmp.rhs)});
        emit_branch(cmp.cond);
      } else { // 隐式与0比较
        bool flip_cond = false;
        if (lnot_info.count(val)) {
          flip_cond = lnot_info[val].flip;
          val = lnot_info[val].src;
        }

        bool use_cbz_cbnz = false;
        if (!val.is_float() && use_cbz_cbnz) {
          // emit cbz/cbnz
          auto emit_cond_branch = [flip_cond, bb](Reg val, BasicBlock *target,
                                                  bool flip) {
            if (flip_cond ^ flip)
              bb->push(new RegBranch{RegBranch::Cbz, val, target});
            else
              bb->push(new RegBranch{RegBranch::Cbnz, val, target});
          };

          if (next_bb == false_target)
            emit_cond_branch(val, true_target, false);
          else if (next_bb == true_target)
            emit_cond_branch(val, false_target, true);
          else {
            emit_cond_branch(val, true_target, false);
            bb->push(new Branch{false_target});
          }
        } else {
          // cmp rd, #0 / vcmp.f32 sd, #0
          auto cond = flip_cond ? ExCond::Eq : ExCond::Ne;
          bb->push(new Compare{val, Operand2::from(0)});
          emit_branch(cond);
        }
      }
    }
  }

public:
  ProgramTranslator(armv7::Program &dst, const ir::Program &src)
      : dst{dst}, src{src} {}
  void translate() {
    auto &asm_funcs = dst.functions;
    for (auto &[name, ir_func] : src.functions) {
      asm_funcs[name].name = name;
      translate_function(asm_funcs[name], ir_func);
    }
  }
};

std::unique_ptr<Program> translate(const ir::Program &ir_program) {
  auto prog = new Program;
  ProgramTranslator translator{*prog, ir_program};
  translator.translate();
  return std::unique_ptr<Program>(prog);
}

void Function::emit(std::ostream &os) {
  os << name << ":\n";
  for (auto &bb : bbs) {
    // if (bb->insns.empty())
    //   continue;

    os << bb->label << ':';
    if (bb->insns.empty()) {
      next_instruction(os);
      os << "nop";
    }

    for (auto &insn : bb->insns) {
      next_instruction(os);
      insn->emit(os);
    }
    os << "\n\n";
  }
}

void Program::emit(std::ostream &os) {
  os << ".cpu cortex-a72\n";
  os << ".arm\n";
  os << ".global main\n";
  os << ".section .text\n\n";

  for (auto &[_, f] : functions)
    f.emit(os);
}

void emit_global_array(std::ostream &os, const std::shared_ptr<Var> &var) {
  int size = var->type.size();
  if (!var->arr_val) {
    os << "    .space " << size << ", 0\n";
    return;
  }

  int nr_elems = size / 4;
  int cur_index = 0;
  for (auto &[index, val] : *var->arr_val) {
    if (index > cur_index)
      os << "    .space " << (index - cur_index) * 4 << ", 0\n";
    if (val.type != Float)
      os << "    .word " << val.iv << '\n';
    else
      os << "    .float " << val.fv << '\n';
    cur_index = index + 1;
  }
  if (nr_elems > cur_index)
    os << "    .space " << (nr_elems - cur_index) * 4 << ", 0\n";
}

void emit_global_var(std::ostream &os, const std::string &name,
                     const std::shared_ptr<Var> &var) {
  os << ".align\n";
  os << name << ":\n";
  if (var->type.is_array()) {
    emit_global_array(os, var);
    return;
  }

  if (var->val) {
    auto init = var->val.value();
    if (init.type != Float)
      os << "    .word " << init.iv << '\n';
    else
      os << "    .float " << init.fv << '\n';
  } else {
    os << "    .space 4, 0\n";
  }
}

void emit_global(std::ostream &os, const ir::Program &ir_program) {
  bool gen_rodata = false;
  bool gen_data = false;
  bool gen_bss = false;

  auto &str_tab = ir_program.string_table;
  auto &glob_vars = ir_program.global_vars;

  if (!ir_program.string_table.empty())
    gen_rodata = true;
  for (auto &[_, var] : glob_vars) {
    if (var->type.is_const)
      gen_rodata = true;
    if (!var->val && !var->arr_val)
      gen_bss = true;
    else
      gen_data = true;
  }

  if (gen_rodata) {
    os << ".section .rodata\n";
    for (size_t i = 0; i < str_tab.size(); ++i) {
      os << ".str." << i << ":\n";
      os << "    .asciz " << str_tab[i] << '\n';
    }

    for (auto &[name, var] : glob_vars)
      if (var->type.is_const)
        emit_global_var(os, name, var);
    os << "\n\n";
  }
  if (gen_data) {
    os << ".section .data\n";
    for (auto &[name, var] : glob_vars)
      if (!var->type.is_const && (var->val || var->arr_val))
        emit_global_var(os, name, var);
    os << "\n\n";
  }
  if (gen_bss) {
    os << ".section .bss\n";
    for (auto &[name, var] : glob_vars)
      if (!var->type.is_const && (!var->val && !var->arr_val))
        emit_global_var(os, name, var);
    os << "\n\n";
  }
}

int round_up_to_imm8m(int x) {
  if (!x)
    return x;
  int bits = 8 * sizeof(x);
  int hi = bits - 1 - __builtin_clz(x);
  int lo = __builtin_ctz(x);
  if (hi & 1) {
    if (hi - lo + 1 <= 7)
      return x;
  } else {
    if (hi - lo + 1 <= 8)
      return x;
  }

  int a;
  if (hi & 1)
    a = 1 << (hi - 6);
  else
    a = 1 << (hi - 7);
  int m = ~(a - 1);
  return (x + a - 1) & m;
}

void assign_offsets(const std::vector<StackObject *> &objs) {
  int offset = 0;
  for (auto obj : objs) {
    obj->offset = offset;
    offset += obj->size;
  }
}

// 计算栈对象相对sp的偏移，如果全部StackStore的对象偏移都能用ldr和str的立即数表示返回true
// 否则将StackStore替换为LoadStackAddr + Store
bool Function::check_and_resolve_stack_store() {
  bool ok = true;
  assign_offsets(normal_objs);

  int sp_offset = 0; // sp相对normal objects基址的偏移
  for (auto &bb : bbs) {
    auto &insns = bb->insns;
    for (auto it = insns.begin(); it != insns.end();) {
      auto ins = it->get();
      TypeCase(store, StoreStack *, ins) {
        int offset = store->offset + store->base->offset - sp_offset;
        if (!is_valid_ldst_offset(offset)) {
          auto next = std::next(it);

          Reg tmp = new_reg(General);
          insns.emplace(next,
                        new LoadStackAddr{tmp, store->base, store->offset});
          insns.emplace(next, new Store{store->src, tmp, 0});

          ok = false;
          insns.erase(it);
          it = next;
          continue;
        }
      }
      else TypeCase(push, Push *, ins) {
        sp_offset -= push->srcs.size() * 4;
      }
      else TypeCase(pop, Pop *, ins) {
        sp_offset += pop->dsts.size() * 4;
      }
      else TypeCase(spa, AdjustSp *, ins) {
        sp_offset += spa->offset;
      }
      ++it;
    }
  }
  return ok;
}

void Function::defer_stack_param_load(Reg r, StackObject *obj) {
  for (auto &bb : bbs) {
    auto &insns = bb->insns;
    for (auto it = insns.begin(); it != insns.end(); ++it) {
      auto ins = it->get();
      auto use = ins->use();
      auto ptrs = ins->reg_ptrs();
      if (use.count(r)) {
        Reg tmp = new_reg(r.type);
        ins->replace_reg(r, tmp);
        insns.emplace(it, new LoadStack{tmp, obj, 0});
      }
    }
  }
}

void Function::emit_prologue_epilogue() {
  bool gpr_used[NR_GPRS], fpr_used[NR_FPRS];
  std::fill_n(gpr_used, NR_GPRS, false);
  std::fill_n(fpr_used, NR_FPRS, false);

  for (auto &bb : bbs) {
    for (auto &insn : bb->insns) {
      auto def = insn->def();
      for (Reg r : def) {
        if (!r.is_float())
          gpr_used[r.id] = true;
        else
          fpr_used[r.id] = true;
      }
    }
  }

  bool save_lr = gpr_used[lr];
  std::vector<Reg> saved_gprs, saved_fprs;
  for (int i = 0; i < NR_GPRS; ++i)
    if (GPRS_ATTR[i] == NonVolatile && gpr_used[i] && i != lr)
      saved_gprs.push_back(Reg{General, i});
  if (save_lr)
    saved_gprs.push_back(Reg{General, lr});
  for (int i = NR_VOLATILE_FPRS; i < NR_FPRS; ++i)
    if (fpr_used[i])
      saved_fprs.push_back(Reg{Fp, i});

  auto entry = bbs.begin()->get();
  auto exit = new BasicBlock;
  exit->label = ".exit." + name;

  int stack_obj_size = 0;
  for (auto obj : normal_objs)
    stack_obj_size += obj->size;
  stack_obj_size = round_up_to_imm8m(stack_obj_size);

  // emit prologue
  auto &prologue = entry->insns;
  // 每次都插入在头部，先生成后面的
  if (stack_obj_size)
    prologue.emplace(prologue.begin(), new AdjustSp{-stack_obj_size});
  if (!saved_fprs.empty())
    prologue.emplace(prologue.begin(), new Push{saved_fprs});
  if (!saved_gprs.empty())
    prologue.emplace(prologue.begin(), new Push{saved_gprs});

  // emit epilogue
  auto &epilogue = exit->insns;
  if (stack_obj_size)
    epilogue.emplace(epilogue.end(), new AdjustSp{stack_obj_size});
  if (!saved_fprs.empty())
    epilogue.emplace(epilogue.end(), new Pop{saved_fprs});
  if (!saved_gprs.empty()) {
    if (save_lr)
      saved_gprs.back().id = pc;
    epilogue.emplace(epilogue.end(), new Pop{saved_gprs});
  }
  if (!save_lr)
    epilogue.emplace(epilogue.end(), new Return);

  auto last = std::prev(bbs.end())->get();
  bool trivial_return =
      !stack_obj_size && saved_fprs.empty() && saved_gprs.empty();
  for (auto &bb : bbs) {
    auto &insns = bb->insns;
    for (auto it = insns.begin(); it != insns.end(); ++it) {
      auto ins = it->get();
      TypeCase(ret, Return *, ins) {
        if (bb.get() == last && std::next(it) == insns.end()) {
          it = insns.erase(it);
          break;
        }

        if (!trivial_return)
          it->reset(new Branch{exit});
      }
    }
  }
  bbs.emplace_back(exit);

  int frame_size = stack_obj_size + 4 * (saved_gprs.size() + saved_fprs.size());
  resolve_stack_ops(frame_size);
}

void Function::resolve_stack_ops(int frame_size) {
  assign_offsets(normal_objs);
  assign_offsets(param_objs);

  // 获得StackObject相对于fp的偏移
  auto get_obj_offset = [this, frame_size](const StackObject *obj) {
    if (std::find(param_objs.begin(), param_objs.end(), obj) !=
        param_objs.end()) {
      return obj->offset;
    } else {
      return obj->offset - frame_size;
    }
  };

  Reg reg_sp{General, sp};
  int sp_offset = 0; // sp相对fp的偏移
  for (auto &bb : bbs) {
    auto &insns = bb->insns;
    for (auto it = insns.begin(); it != insns.end(); ++it) {
      auto ins = it->get();
      TypeCase(load_addr, LoadStackAddr *, ins) {
        Reg dst = load_addr->dst;
        int offset =
            load_addr->offset + get_obj_offset(load_addr->base) - sp_offset;
        int imm = std::abs(offset);
        bool sub = offset < 0;

        // TODO: 如果Thumb-2可用，这里可以是imm12吗?
        // if (is_imm8m(imm) || is_imm12(imm)) {
        if (is_imm8m(imm)) {
          auto op = sub ? IType::Sub : IType::Add;
          it->reset(new IType{op, dst, reg_sp, imm});
        } else {
          auto op = sub ? RType::Sub : RType::Add;
          emit_load_imm(insns, it, dst, imm);
          it->reset(new RType{op, dst, reg_sp, dst});
        }
      }
      else TypeCase(load, LoadStack *, ins) {
        Reg dst = load->dst;
        int offset = load->offset + get_obj_offset(load->base) - sp_offset;
        // TODO: 目标为浮点寄存器时可能无法生成有效的立即数偏移
        // 也需要在寄存器分配循环中做检查
        if (dst.is_float()) {
          assert(is_valid_ldst_offset(offset));
          it->reset(new Load{dst, reg_sp, offset});
        } else {
          if (is_valid_ldst_offset(offset)) {
            it->reset(new Load{dst, reg_sp, offset});
          } else {
            // Rd = loadimm #offset
            // add Rd, sp, Rd
            // ldr Rd, [Rd]

            // TODO:
            // Rd = loadimm #offset
            // ldr Rd, [sp, Rd]
            emit_load_imm(insns, it, dst, offset);
            insns.emplace(it, new RType{RType::Add, dst, reg_sp, dst});
            it->reset(new Load{dst, dst, 0});
          }
        }
      }
      else TypeCase(store, StoreStack *, ins) {
        int offset = store->offset + get_obj_offset(store->base) - sp_offset;
        assert(is_valid_ldst_offset(offset));
        it->reset(new Store{store->src, reg_sp, offset});
      }
      else TypeCase(sp_adjust, AdjustSp *, ins) {
        int offset = sp_adjust->offset;
        int imm = std::abs(offset);

        if (is_imm8m(imm)) {
          auto op = offset < 0 ? IType::Sub : IType::Add;
          it->reset(new IType{op, reg_sp, reg_sp, imm});
        } else {
          // NOTE: 假设非imm8m的sp调整只会出现在函数调用之后
          // (如果prologue和epilogue中sp的调整量都向上对齐到imm8m)
          // 这时r1~r3是报废的，可以使用
          auto op = offset < 0 ? RType::Sub : RType::Add;
          Reg tmp{General, r2};
          emit_load_imm(insns, it, tmp, imm);
          it->reset(new RType{op, reg_sp, reg_sp, tmp});
        }
        sp_offset += offset;
      }
      else TypeCase(push, Push *, ins) {
        sp_offset -= push->srcs.size() * 4;
      }
      else TypeCase(pop, Pop *, ins) {
        sp_offset += pop->dsts.size() * 4;
      }
    }
  }
}

void Function::replace_pseudo_insns() {
  for (auto &bb : bbs) {
    auto &insns = bb->insns;
    for (auto it = insns.begin(); it != insns.end(); ++it) {
      TypeCase(pcmp, PseudoCompare *, it->get()) {
        auto cond = pcmp->cond;
        auto dst = pcmp->dst;

        auto cmov_true = new Move{dst, Operand2::from(1)};
        cmov_true->cond = cond;
        auto cmov_false = new Move{dst, Operand2::from(0)};
        cmov_false->cond = logical_not(cond);

        insns.emplace(it, pcmp->cmp.release());
        insns.emplace(it, cmov_false);
        it->reset(cmov_true);
      }
    }
  }
}

} // namespace armv7
