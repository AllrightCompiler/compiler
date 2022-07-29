#include "backend/armv7/passes.hpp"
#include "backend/armv7/ColoringRegAllocator.hpp"
#include "backend/armv7/arch.hpp"
#include "backend/armv7/instruction.hpp"

#include "common/common.hpp"

#include <iostream>
#include <iterator>

namespace armv7 {

void backend_passes(Program &p) {
  ColoringRegAllocator reg_allocator;

  for (auto &[_, f] : p.functions) {
    fold_constants(f);
    remove_unused(f);

    f.resolve_phi();

    reg_allocator.do_reg_alloc(f, false); // fp reg

    // f.emit(std::cerr);

    reg_allocator.do_reg_alloc(f);

    remove_useless(f);

    // 以下步骤必须进行
    f.emit_prologue_epilogue();
    f.replace_pseudo_insns();
  }
}

bool is_add_sub_imm(int x) { return is_imm8m(x); }

int compute_shift(ShiftType type, int r, int s) {
  switch (type) {
  case LSL:
    return r << s;
  case LSR:
    return unsigned(r) >> s;
  case ASR:
    return r >> s;
  case ROR:
    return (unsigned(r) >> s) | (r << (32 - s));
  default:
    __builtin_unreachable();
  }
}

void fold_constants(Function &f) {
  std::map<Reg, ConstValue> constants;

  for (auto &bb : f.bbs) {
    for (auto &insn : bb->insns) {
      auto ins = insn.get();

      // 匹配常数加载指令
      bool is_load_imm = false;
      TypeCase(mov, Move *, ins) {
        if (mov->src.is_imm8m()) {
          int imm = mov->src.get<int>();
          if (mov->flip)
            imm = ~imm;

          Reg dst = mov->dst;
          if (dst.is_virt())
            constants[dst] = imm;
          is_load_imm = true;
        } else if (mov->src.is_fpimm()) {
          float imm = mov->src.get<float>();
          assert(!mov->flip);
          if (mov->dst.is_virt()) {
            constants[mov->dst] = imm;
          }
          is_load_imm = true;
        }
      }
      else TypeCase(movw, MovW *, ins) {
        Reg dst = movw->dst;
        if (dst.is_virt())
          constants[dst] = movw->imm;
        is_load_imm = true;
      }
      else TypeCase(movt, MovT *, ins) {
        Reg dst = movt->dst;
        if (constants.count(dst)) {
          int lo = constants[dst].iv;
          constants[dst] = (movt->imm << 16) | lo;
          is_load_imm = true;
        }
      }
      if (is_load_imm)
        continue;

      auto eval_operand2 =
          [&constants](const Operand2 &s2) -> std::optional<int> {
        int x;
        if (s2.is_imm_shift()) {
          // e.g. r1 LSL #2 转常数
          auto sh = s2.get<RegImmShift>();
          if (!constants.count(sh.r))
            return std::nullopt;
          x = compute_shift(sh.type, constants[sh.r].iv, sh.s);
        } else if (s2.is_reg_shift()) {
          // e.g. r1 LSL r2
          auto sh = s2.get<RegRegShift>();
          if (!constants.count(sh.r1) || !constants.count(sh.r2))
            return std::nullopt;
          x = compute_shift(sh.type, constants[sh.r1].iv, constants[sh.r2].iv);
        } else {
          x = s2.get<int>();
        }
        return x;
      };

      auto inline_compare_constant = [&](Compare *cmp) {
        // TODO: 检查s1是否是常数，可以反转指令条件
        auto opt_imm = eval_operand2(cmp->s2);
        if (opt_imm) {
          int imm = opt_imm.value();
          if (is_imm8m(imm))
            cmp->s2 = Operand2::from(imm);
          else if (is_imm8m(-imm)) {
            cmp->s2 = Operand2::from(-imm);
            cmp->neg = !cmp->neg;
          }
        }
      };

      TypeCase(r_ins, RType *, ins) {
        auto op = r_ins->op;
        if (r_ins->dst.is_float()) // TODO: 浮点立即数
          continue;

        if (op == RType::Add || op == RType::Sub) {
          // TODO: 加负数 -> 减正数
          int imm;
          Reg other, s1 = r_ins->s1, s2 = r_ins->s2;
          if (constants.count(s1) && is_add_sub_imm(constants[s1].iv)) {
            other = s2, imm = constants[s1].iv;
            if (op == RType::Sub && !is_imm8m(imm)) // rsb
              continue;
          } else if (constants.count(r_ins->s2) &&
                     is_add_sub_imm(constants[s2].iv)) {
            other = s1, imm = constants[s2].iv;
          } else
            continue;

          auto new_op = IType::Add;
          if (op == RType::Sub)
            new_op = (other == s2) ? IType::RevSub : IType::Sub;
          auto new_insn = new IType{new_op, r_ins->dst, other, imm};
          insn.reset(new_insn);
        } else if (op == RType::Mul) {
          // TODO: 乘2的幂 -> 移位
        } else if (op == RType::Div) {
          // TODO: 除2的幂 -> 移位
        }
      }
      else TypeCase(i_ins, IType *, ins) {
        // 应该不用处理
      }
      else TypeCase(fr_ins, FullRType *, ins) {
        auto opt_imm = eval_operand2(fr_ins->s2);
        if (opt_imm && is_imm8m(opt_imm.value()))
          fr_ins->s2 = Operand2::from(opt_imm.value());
      }
      else TypeCase(mov, Move *, ins) {
        auto opt_imm = eval_operand2(mov->src);
        if (opt_imm && !mov->is_transfer_vmov()) {
          int imm = opt_imm.value();
          Reg dst = mov->dst;
          if (dst.is_virt())
            constants[dst] = imm;
          if (is_imm8m(imm))
            mov->src = Operand2::from(imm);
          else if (is_imm8m(~imm)) {
            mov->src = Operand2::from(~imm);
            mov->flip = !mov->flip;
          }
        }
      }
      else TypeCase(cmp, Compare *, ins) {
        inline_compare_constant(cmp);
      }
      else TypeCase(pcmp, PseudoCompare *, ins) {
        inline_compare_constant(pcmp->cmp.get());
      }
    }
  }
}

void remove_unused(Function &f) {
  f.do_liveness_analysis();
  for (auto &bb : f.bbs) {
    auto live = bb->live_out;
    for (auto it = bb->insns.rbegin(); it != bb->insns.rend();) {
      bool no_effect = true;
      auto ins = it->get();

      if (ins->is<SpRelative>() || ins->is<Compare>() || ins->is<Store>() ||
          ins->is<Branch>() || ins->is<RegBranch>() || ins->is<Call>() ||
          ins->is<Return>())
        no_effect = false;
      else {
        for (Reg d : ins->def())
          if (live.count(d))
            no_effect = false;
      }

      if (no_effect) {
        auto base = it.base();
        bb->insns.erase(std::prev(base));
        it = std::make_reverse_iterator(base);
      } else {
        for (Reg d : ins->def())
          live.erase(d);
        for (Reg u : ins->use())
          live.insert(u);
        ++it;
      }
    }
  }
}

void remove_useless(Function &f) {
  for (auto &bb : f.bbs) {
    auto &insns = bb->insns;
    for (auto it = insns.begin(); it != insns.end();) {
      bool remove = false;
      TypeCase(mov, Move *, it->get()) {
        if (mov->is_reg_mov() && mov->use().count(mov->dst))
          remove = true;
      }

      if (remove)
        it = insns.erase(it);
      else
        ++it;
    }
  }
}

} // namespace armv7
