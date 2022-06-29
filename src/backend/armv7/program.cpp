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

void emit_load_imm(BasicBlock *bb, Reg dst, int imm) {
  if (is_imm8m(imm))
    bb->push(new Move{dst, Operand2::from(imm)});
  else if (is_imm8m(~imm))
    bb->push(new Move{dst, Operand2::from(~imm), true});
  else {
    uint32_t x = uint32_t(imm);
    auto lo = x & 0xffff, hi = x >> 16;
    bb->push(new MovW(dst, lo));
    if (hi > 0)
      bb->push(new MovT(dst, hi));
  }
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

    auto entry_bb = new armv7::BasicBlock;
    auto &params = src_fn.sig.param_types;
    int nr_params = params.size();
    int nr_gp_params, nr_fp_params;
    nr_gp_params = nr_fp_params = 0;
    std::vector<int> stack_params;
    for (int i = 0; i < nr_params; ++i) {
      auto type = params[i].base_type;
      Reg dst = Reg{type, -(i + 1)};
      if (type != Float) {
        if (nr_gp_params++ < NR_GPRS) {
          entry_bb->push(new Move{dst, Operand2::from(Reg{type, r0 + i})});
        } else {
          stack_params.push_back(i);
        }
      } else {
        if (nr_fp_params++ < NR_FPRS) {
          entry_bb->push(new Move{dst, Operand2::from(Reg{type, s0 + i})});
        } else {
          stack_params.push_back(i);
        }
      }
    }

    int params_size = 0;
    for (int i : stack_params) {
      auto obj = new StackObject;
      obj->size = 4; // 参数的size总是4
      obj->offset = params_size;
      params_size += obj->size;
      dst_fn.param_objs.insert(obj);
      dst_fn.stack_objects.emplace_back(obj);

      Reg dst = Reg{params[i].base_type, -(i + 1)};
      entry_bb->push(new LoadStack{dst, obj, 0});
    }
    entry_bb->label = ".entry";
    dst_fn.bbs.emplace_back(entry_bb);

    for (auto &ir_bb : src_fn.bbs) {
      auto bb = new armv7::BasicBlock;
      bb_map[ir_bb.get()] = bb;
      dst_fn.bbs.emplace_back(bb);
    }
    dst_fn.regs_allocated = src_fn.regs_used;

    int i = 1;
    auto dst_it = std::next(dst_fn.bbs.begin()); // skip entry
    for (auto src_it = src_fn.bbs.begin(); src_it != src_fn.bbs.end();
         ++src_it, ++dst_it, ++i) {
      BasicBlock *next_bb = nullptr;
      if (i + 1 < dst_fn.bbs.size())
        next_bb = std::next(dst_it)->get();

      auto bb = dst_it->get();
      bb->label = ".L" + std::to_string(i);
      for (auto &ir_insn : (*src_it)->insns)
        translate_instruction(dst_fn, bb, ir_insn.get(), next_bb);
    }
  }

  void translate_instruction(armv7::Function &fn, BasicBlock *bb,
                             ir::Instruction *ins, BasicBlock *next_bb) {
    namespace ii = ir::insns;
    TypeCase(alloca, ii::Alloca *, ins) {
      auto obj = new StackObject;
      obj->size = alloca->type.size();
      fn.normal_objs.insert(obj);
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
      // TODO: 对于某些浮点立即数，可以生成vmov.f32
      Reg dst = Reg::from(load->dst);
      if (!dst.is_float()) {
        emit_load_imm(bb, dst, load->imm.iv);
      } else {
        float imm = load->imm.fv;
        if (is_vmov_f32_imm(imm))
          ; // TODO
        else {
          Reg t = fn.new_reg(Int);
          emit_load_imm(bb, t, *reinterpret_cast<int *>(&imm));
          // TODO: emit vmov
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
        Reg t = fn.new_reg(Int);
        Reg s = Reg::from(gep->indices[i]);
        int dim = gep->type.dims[i];
        if (is_power_of_2(dim)) {
          bb->push(new FullRType{
              FullRType::Op::Add, t, s,
              Operand2::from(LSL, index_reg, __builtin_ctz(dim))});
        } else {
          Reg imm_reg = fn.new_reg(Int);
          emit_load_imm(bb, imm_reg, dim);
          bb->push(new FusedMul{t, index_reg, imm_reg, s});
        }
        index_reg = t;
      }
      Reg dst = Reg::from(gep->dst);
      Reg base = Reg::from(gep->base);
      bb->push(new FullRType{FullRType::Op::Add, dst, base,
                             Operand2::from(LSL, index_reg, 2)});
    }
    else TypeCase(cvt, ii::Convert *, ins) {
      // TODO: emit vmov + vcvt
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
        // 根据语法特性，!expr将不会参与常规计算，暂不生成实际代码而只生成标记
        // if (!dst.is_float()) {
        //   // bb->push(new Move{dst, Operand2::from(0)});
        //   // bb->push(new Compare{src, Operand2::from(0)});
        //   // bb->push(ExCond::Eq, new Move{dst, Operand2::from(1)});
        //   // alternative: clz dst, src; lsr dst, dst, #5
        //   bb->push(new CountLeadingZero{dst, src});
        //   bb->push(new Move{dst, Operand2::from(LSR, dst, 5)});
        // } else {
        //   // TODO: emit vcmp vmrs etc.
        // }
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
        Reg t = fn.new_reg(Int);
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
        // 暂不生成实际代码，只生成标记（要求虚拟寄存器单赋值）
        auto &cmp = cmp_info[dst];
        cmp.cond = from(binary->op);
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
          bb->push(new Move{Reg{arg_reg.type, r0 + nr_gp_args},
                            Operand2::from(arg_reg)});
        } else {
          // TODO: move or explicit vmov?
          bb->push(
              new Move{Reg{Float, s0 + nr_fp_args}, Operand2::from(arg_reg)});
        }
      }
      bb->push(new Call{call->func});
      // 这里钻了个空子，这个语法中函数的返回值只能是int或float
      // 如果接收返回值的寄存器类型是String表明实际上无返回值
      if (call->dst.type != String)
        bb->push(new Move{Reg::from(call->dst),
                          Operand2::from(Reg{call->dst.type, 0})}); // r0或s0
      int sp_adjustment = 4 * stack_args.size(); // NOTE: 所有标量类型都是4字节
      if (sp_adjustment != 0)
        bb->push(new AdjustSp{sp_adjustment});
    }
    else TypeCase(ret, ii::Return *, ins) {
      if (ret->val)
        bb->push(
            new Move{Reg{ret->val->type, 0},
                     Operand2::from(Reg::from(ret->val.value()))}); // r0或s0
      bb->push(new Return{});
    }
    else TypeCase(jump, ii::Jump *, ins) {
      auto target = bb_map[jump->target];
      if (target != next_bb)
        bb->push(new Branch{target});
    }
    else TypeCase(br, ii::Branch *, ins) {
      Reg val = Reg::from(br->val);
      auto true_target = bb_map[br->true_target];
      auto false_target = bb_map[br->false_target];

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

        bool use_cbz_cbnz = true;
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
  return std::unique_ptr<Program>(prog);
}

void Program::emit(std::ostream &os) {}

} // namespace armv7
