#include "backend/armv7/program.hpp"
#include "backend/armv7/arch.hpp"
#include "backend/armv7/instruction.hpp"

#include "common/common.hpp"
#include "common/ir.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <unordered_map>

namespace armv7 {

void emit_load_imm(BasicBlock *bb, Reg dst, int imm) {
  emit_load_imm(bb->insns, bb->insns.end(), dst, imm);
}

void Function::emit_imm(
    std::list<std::unique_ptr<Instruction>> &insns,
    const std::list<std::unique_ptr<Instruction>>::iterator &it, Reg dst,
    int imm) {
  emit_load_imm(insns, it, dst, imm);
  if (dst.is_virt())
    reg_val[dst] = imm;
}

void Function::emit_imm(BasicBlock *bb, Reg dst, int imm) {
  emit_load_imm(bb, dst, imm);
  if (dst.is_virt())
    reg_val[dst] = imm;
}

std::list<std::unique_ptr<Instruction>>::iterator BasicBlock::sequence_end() {
  // auto it = insns.begin();
  // for (; it != insns.end(); ++it) {
  //   if ((*it)->is<Branch>()) {
  //     // 如果这是一个典型条件跳转 (e.g. cmp r0, #0; bne Lx)
  //     // 将指令插入到cmp之前
  //     if ((*it)->cond != ExCond::Always && it != insns.begin() &&
  //         (*std::prev(it))->is<Compare>())
  //       --it;
  //     break;
  //   }
  // }
  // return it;
  if (insns.empty())
    return insns.end();
  auto it = std::prev(insns.end());
  return (*it)->is<Terminator>() ? it : insns.end();
}

void BasicBlock::insert_at_end(Instruction *insn) {
  insns.emplace(sequence_end(), insn);
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
      auto type = machine_reg_type(params[i]);
      Reg dst = Reg{type, -(i + 1)};
      if (type != Fp) {
        if (nr_gp_params < NR_ARG_GPRS) {
          entry_bb->push(
              new Move{dst, Operand2::from(Reg{type, r0 + nr_gp_params++})});
        } else {
          stack_params.push_back(i);
        }
      } else {
        if (nr_fp_params < NR_ARG_FPRS) {
          entry_bb->push(
              new Move{dst, Operand2::from(Reg{type, s0 + nr_fp_params++})});
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
    entry_bb->push(new Branch{prev_entry});

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

      Reg dst{machine_reg_type(params[i]), -(i + 1)};
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

      Reg dst = Reg::from(alloca->dst);
      fn.reg_val[dst] = obj;
      bb->push(new LoadStackAddr{dst, obj, 0});
    }
    else TypeCase(load, ii::Load *, ins) {
      bb->push(new Load{Reg::from(load->dst), Reg::from(load->addr), 0});
    }
    else TypeCase(load, ii::LoadAddr *, ins) {
      Reg dst = Reg::from(load->dst);
      fn.reg_val[dst] = load->var_name;
      bb->push(new LoadAddr{dst, load->var_name});
    }
    else TypeCase(load, ii::LoadImm *, ins) {
      Reg dst = Reg::from(load->dst);
      if (!dst.is_float()) {
        fn.emit_imm(bb, dst, load->imm.iv);
      } else {
        float imm = load->imm.fv;
        if (false && is_vmov_f32_imm(imm)) { // TODO
          bb->push(new Move(dst, Operand2::from(imm)));
        } else {
          Reg t = fn.new_reg(General);
          fn.emit_imm(bb, t, *reinterpret_cast<int *>(&imm));
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
          fn.emit_imm(bb, imm_reg, dim);
          bb->push(new FusedMul{FusedMul::Add, t, index_reg, imm_reg, s});
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
          fn.emit_imm(bb, imm_reg, dim);
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
          bb->push(new Vneg(dst, src));
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
      case BinaryOp::Mod:
        bb->push(new PseudoModulo{dst, s1, s2});
        break;
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

      int sp_adjustment =
          4 * stack_args.size(); // NOTE: 目前所有标量类型都是4字节
      // NOTE: 调用约定要求函数边界的sp按照8字节对齐
      if ((sp_adjustment & 7) != 0) {
        sp_adjustment += 4;
        bb->push(new AdjustSp{-4});
      }

      for (auto it = stack_args.rbegin(); it != stack_args.rend(); ++it) {
        Reg arg_reg = Reg::from(call->args[*it]);
        bb->push(new Push{{arg_reg}});
      }
      nr_gp_args = nr_fp_args = 0;
      for (auto i : reg_args) {
        Reg arg_reg = Reg::from(call->args[i]);
        if (!arg_reg.is_float()) {
          bb->push(new Move{Reg{arg_reg.type, r0 + nr_gp_args++},
                            Operand2::from(arg_reg)});
        } else {
          bb->push(
              new Move{Reg{Fp, s0 + nr_fp_args++}, Operand2::from(arg_reg)});
        }
      }
      bb->push(new Call{call->func, nr_gp_args, nr_fp_args, call->variadic_at});
      // 这里钻了个空子，这个语法中函数的返回值只能是int或float
      // 如果接收返回值的寄存器类型是String表明实际上无返回值
      if (call->dst.type != String)
        bb->push(
            new Move{Reg::from(call->dst),
                     Operand2::from(Reg::from(call->dst.type, 0))}); // r0或s0

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
      bb->push(new Branch{target});
    }
    else TypeCase(br, ii::Branch *, ins) {
      Reg val = Reg::from(br->val);
      auto true_target = bb_map[br->true_target];
      auto false_target = bb_map[br->false_target];
      BasicBlock::add_edge(bb, true_target);
      BasicBlock::add_edge(bb, false_target);

      // auto emit_branch = [bb, next_bb, true_target, false_target](ExCond
      // cond) {
      //   if (next_bb == false_target)
      //     bb->push(cond, new Branch{true_target});
      //   else if (next_bb == true_target)
      //     bb->push(logical_not(cond), new Branch{false_target});
      //   else {
      //     bb->push(cond, new Branch{true_target});
      //     bb->push(new Branch{false_target});
      //   }
      // };

      if (cmp_info.count(val)) { // 显式二元比较
        auto &cmp = cmp_info[val];
        // bb->push(new Compare{cmp.lhs, Operand2::from(cmp.rhs)});
        // emit_branch(cmp.cond);
        auto inner_cmp = new Compare{cmp.lhs, Operand2::from(cmp.rhs)};
        bb->push(cmp.cond, new CmpBranch{inner_cmp, true_target, false_target});
      } else { // 隐式与0比较
        bool flip_cond = false;
        if (lnot_info.count(val)) {
          flip_cond = lnot_info[val].flip;
          val = lnot_info[val].src;
        }

        // 不启用Thumb-2的CBZ/CBNZ，限制较多
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
          // bb->push(new Compare{val, Operand2::from(0)});
          // emit_branch(cond);

          // TODO: 区分vcmp.f32
          auto inner_cmp = new Compare{val, Operand2::from(0)};
          bb->push(cond, new CmpBranch{inner_cmp, true_target, false_target});
        }
      }
    }
    else TypeCase(phi, ii::Phi *, ins) {
      std::vector<std::pair<BasicBlock *, Reg>> srcs;
      for (auto &[ir_bb, reg] : phi->incoming)
        srcs.emplace_back(bb_map.at(ir_bb), Reg::from(reg));
      bb->push(new Phi{Reg::from(phi->dst), std::move(srcs)});
    }
    else TypeCase(sw, ii::Switch *, ins) {
      Reg val = Reg::from(sw->val);
      Reg tmp = fn.new_reg(General);
      auto default_target = bb_map.at(sw->default_target);
      BasicBlock::add_edge(bb, default_target);

      std::vector<std::pair<int, BasicBlock *>> targets;
      for (auto &[v, ir_bb] : sw->targets) {
        auto target = bb_map.at(ir_bb);
        targets.push_back({v, target});
        BasicBlock::add_edge(bb, target);
      }
      assert(!targets.empty());
      bb->push(new Switch{val, tmp, default_target, std::move(targets)});
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
  os << ".section .text\n";
  os << ".align\n";
  os << name << ":\n";
  for (auto &bb : bbs) {
    os << bb->label << ':';
    // if (bb->insns.empty()) {
    //   next_instruction(os);
    //   os << "nop";
    // }

    for (auto &insn : bb->insns) {
      next_instruction(os);
      insn->emit(os);
    }
    os << "\n\n";
  }
  emit_jump_tables(os);
}

void Function::emit_jump_tables(std::ostream &os) {
  os << ".section .rodata\n";
  for (size_t i = 0; i < jump_tables.size(); ++i) {
    auto jt_name = "JT" + std::to_string(i) + "$" + name;
    auto &sw = jump_tables[i];
    // 这里为方便起见，进行了稠密化处理
    int first = std::min(sw->targets.front().first, 0);
    int last = std::max(sw->targets.back().first, 0);
    int len = last - first + 1;
    std::vector<BasicBlock *> dense_targets(len, sw->default_target);
    for (auto [v, target] : sw->targets)
      dense_targets[v - first] = target;

    for (int j = first; j <= last; ++j) {
      if (j == 0)
        os << jt_name << ":\n";
      os << "    .word " << dense_targets[j]->label << '\n';
    }
    os << '\n';
  }
}

Program::Program() : labels_used{0} {
  auto p = [this](const char *s) { builtin_code.emplace_back(s); };
  p("__builtin_array_init:");
  p("    add r1, r0, r1, lsl #2");
  p("    mov r2, #0");
  p("1:");
  p("    str r2, [r0]");
  p("    add r0, r0, #4");
  p("    cmp r0, r1");
  p("    blt 1b");
  p("    bx  lr");
}

void Program::emit(std::ostream &os) {
  os << ".cpu cortex-a72\n";
  os << ".arm\n";
  os << ".fpu vfp\n";
  os << ".global main\n";
  os << ".section .text\n\n";

  for (auto &line : builtin_code)
    os << line << '\n';
  os << '\n';

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
    os << "    .word " << val.iv << '\n';
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
    os << "    .word " << init.iv << '\n';
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
  if (!(hi & 1)) {
    if (hi - lo + 1 <= 7)
      return x;
  } else {
    if (hi - lo + 1 <= 8)
      return x;
  }

  int a;
  if (!(hi & 1))
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

  // NOTE: 调用约定要求函数边界的sp按照8字节对齐
  int frame_size = stack_obj_size + 4 * (saved_gprs.size() + saved_fprs.size());
  if ((frame_size & 7) != 0) {
    frame_size += 4;
    saved_gprs.insert(saved_gprs.begin(), Reg{General, r3});
  }

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

  bool trivial_return =
      !stack_obj_size && saved_fprs.empty() && saved_gprs.empty();
  for (auto &bb_ptr : bbs) {
    auto bb = bb_ptr.get();
    auto &insns = bb->insns;
    if (!insns.empty()) {
      auto it = std::prev(insns.end());
      TypeCase(ret, Return *, it->get()) {
        if (!trivial_return) {
          BasicBlock::add_edge(bb, exit);
          it->reset(new Branch{exit});
        }
      }
    }
  }
  bbs.emplace_back(exit);

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
            // ldr Rd, [sp, Rd]
            emit_load_imm(insns, it, dst, offset);
            it->reset(new ComplexLoad{dst, reg_sp, dst});
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
  for (auto bb_iter = bbs.begin(); bb_iter != bbs.end();) {
    auto bb = bb_iter->get();
    auto &insns = bb->insns;

    ++bb_iter;
    auto next_bb = (bb_iter == bbs.end()) ? nullptr : bb_iter->get();

    for (auto it = insns.begin(); it != insns.end();) {
      bool remove = false;
      TypeCase(pcmp, PseudoCompare *, it->get()) {
        auto cond = pcmp->cond;
        auto dst = pcmp->dst;

        auto cmov_true = new Move{dst, Operand2::from(1)};
        cmov_true->cond = cond;
        auto cmov_false = new Move{dst, Operand2::from(0)};

        insns.emplace(it, cmov_false);
        insns.emplace(it, pcmp->cmp.release());
        it->reset(cmov_true);
      }
      TypeCase(br, CmpBranch *, it->get()) {
        auto cond = br->cond;
        auto true_target = br->true_target;
        auto false_target = br->false_target;

        insns.emplace(it, br->cmp.release());
        if (next_bb == false_target) {
          auto b_true = new Branch{true_target};
          b_true->cond = cond;
          it->reset(b_true);
        } else if (next_bb == true_target) {
          auto b_false = new Branch{false_target};
          b_false->cond = logical_not(cond);
          it->reset(b_false);
        } else {
          auto b_true = new Branch{true_target};
          b_true->cond = cond;
          insns.emplace(it, b_true);
          it->reset(new Branch{false_target});
        }
      }
      TypeCase(br, Branch *, it->get()) {
        if (br->target == next_bb && br->cond == ExCond::Always)
          remove = true;
      }
      TypeCase(div, PseudoOneDividedByReg *, it->get()) {
        insns.insert(
            it, std::make_unique<IType>(IType::Add, div->dst, div->src, 1));
        insns.insert(it,
                     std::make_unique<Compare>(div->dst, Operand2::from(3)));
        *it = std::make_unique<Move>(div->dst, Operand2::from(0));
        it->get()->cond = ExCond::Cs;
      }
      TypeCase(sw, Switch *, it->get()) {
        int lb = sw->targets.front().first;
        int ub = sw->targets.back().first;
        Reg val = sw->val, tmp = sw->tmp;

        if (is_imm8m(lb))
          insns.emplace(it, new Compare{val, Operand2::from(lb)});
        else {
          emit_load_imm(insns, it, tmp, lb);
          insns.emplace(it, new Compare{val, Operand2::from(tmp)});
        }
        auto lbr = new Branch{sw->default_target};
        lbr->cond = ExCond::Lt;
        insns.emplace(it, lbr);

        if (is_imm8m(ub))
          insns.emplace(it, new Compare{val, Operand2::from(ub)});
        else {
          emit_load_imm(insns, it, tmp, ub);
          insns.emplace(it, new Compare{val, Operand2::from(tmp)});
        }
        auto ubr = new Branch{sw->default_target};
        ubr->cond = ExCond::Gt;
        insns.emplace(it, ubr);

        auto jt_name = "JT" + std::to_string(jump_tables.size()) + "$" + name;
        insns.emplace(it, new LoadAddr{tmp, jt_name});

        jump_tables.emplace_back(dynamic_cast<Switch *>(it->release()));
        it->reset(new ComplexLoad{Reg{General, pc}, tmp, val, LSL, 2});
      }

      if (remove)
        it = insns.erase(it);
      else
        ++it;
    }
  }
}

void Function::resolve_phi() {
  // de-SSA (phi resolution)
  // 假定当前是Conventional SSA，如为T-SSA需要额外转换
  // 保守的实现:
  // 每个move被拆成两步，引入新的临时变量以消除顺序依赖，达到phi函数的并行求值效果
  std::unordered_map<BasicBlock *, std::vector<std::pair<Reg, Reg>>> par_copies;
  for (auto &bb : bbs) {
    auto &insns = bb->insns;
    for (auto it = insns.begin(); it != insns.end();) {
      TypeCase(phi, Phi *, it->get()) {
        for (auto &[bb, src] : phi->srcs)
          par_copies[bb].push_back({phi->dst, src});
        it = insns.erase(it);
      }
      else {
        ++it;
      }
    }
  }

  for (auto &[bb, pcopy] : par_copies) {
    while (!std::all_of(pcopy.begin(), pcopy.end(),
                        [](const auto &p) { return p.first == p.second; })) {
      std::set<Reg> live_in;
      for (auto [_, src] : pcopy)
        live_in.insert(src);

      Move *mov = nullptr;
      auto it = std::find_if(pcopy.begin(), pcopy.end(), [&](const auto &p) {
        return !live_in.count(p.first);
      });
      if (it != pcopy.end()) {
        auto [dst, src] = *it;
        mov = new Move{dst, Operand2::from(src)};
        pcopy.erase(it);
      } else {
        auto it = std::find_if(pcopy.begin(), pcopy.end(), [](const auto &p) {
          return p.first != p.second;
        });
        Reg tmp = new_reg(it->first.type);
        mov = new Move{tmp, Operand2::from(it->second)};
        it->second = tmp;
      }

      bb->insert_at_end(mov);
      phi_moves.insert(mov);
    }
  }
}

} // namespace armv7
