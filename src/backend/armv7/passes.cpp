#include "backend/armv7/passes.hpp"
#include "backend/armv7/ColoringRegAllocator.hpp"
#include "backend/armv7/arch.hpp"
#include "backend/armv7/instruction.hpp"
#include "backend/armv7/merge_instr.hpp"

#include "common/common.hpp"

#include <iostream>
#include <iterator>

namespace armv7 {

void backend_passes(Program &p) {
  ColoringRegAllocator reg_allocator;

  // 具有f.some_pass()形式的步骤必须进行
  for (auto &[_, f] : p.functions) {
    fold_constants(f);

    merge_shift_with_binary_op(f);
    merge_add_with_load_or_store(f);

    remove_unused(f);

    f.resolve_phi();

    f.do_reg_alloc(reg_allocator, false); // fp reg
    f.do_reg_alloc(reg_allocator);

    remove_useless(f);

    f.emit_prologue_epilogue();

    sanitize_cfg(f);

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
    for (auto instr = bb->insns.begin(); instr != bb->insns.end(); ++instr) {
      auto &insn = *instr;
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
          int imm;
          Reg other;
          if (auto const iter = constants.find(r_ins->s1);
              iter != constants.end()) {
            imm = iter->second.iv;
            other = r_ins->s2;
          } else if (auto const iter = constants.find(r_ins->s2);
                     iter != constants.end()) {
            imm = iter->second.iv;
            other = r_ins->s1;
          } else {
            continue;
          }
          if (is_power_of_2(imm)) {
            if (imm == 0) {
              insn = std::make_unique<Move>(r_ins->dst, Operand2::from(0));
            } else if (imm == 1) {
              insn = std::make_unique<Move>(r_ins->dst, Operand2::from(other));
            } else {
              insn = std::make_unique<Move>(
                  r_ins->dst,
                  Operand2::from(ShiftType::LSL, other,
                                 static_cast<int>(std::log2<unsigned>(imm))));
            }
          } else if (imm == -1) {
            insn = std::make_unique<IType>(IType::RevSub, r_ins->dst, other, 0);
          } else if (is_power_of_2(imm - 1)) {
            insn = std::make_unique<FullRType>(
                FullRType::Add, r_ins->dst, other,
                Operand2::from(ShiftType::LSL, other,
                               static_cast<int>(std::log2<unsigned>(imm - 1))));
          } else if (is_power_of_2(imm + 1)) {
            insn = std::make_unique<FullRType>(
                FullRType::RevSub, r_ins->dst, other,
                Operand2::from(ShiftType::LSL, other,
                               static_cast<int>(std::log2<unsigned>(imm + 1))));
          }
        } else if (op == RType::Div) {
          if (auto const iter = constants.find(r_ins->s2);
              iter != constants.end()) {
            int imm = iter->second.iv;
            insn =
                std::make_unique<PseudoDivConstant>(r_ins->dst, r_ins->s1, imm);
          } else if (auto const iter = constants.find(r_ins->s1);
                     iter != constants.end()) {
            int imm = iter->second.iv;
            if (imm == 0) {
              insn = std::make_unique<Move>(r_ins->dst, Operand2::from(0));
            } else if (imm == 1) {
              insn = std::make_unique<PseudoOneDividedByReg>(r_ins->dst,
                                                             r_ins->s2);
            }
          }
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
      else TypeCase(br, CmpBranch *, ins) {
        inline_compare_constant(br->cmp.get());
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

      if (ins->is<SpRelative>() || ins->is<Store>() ||
          ins->is<ComplexStore>() || ins->is<Call>() || ins->is<Terminator>())
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
        if (mov->is_reg_mov() && mov->use().count(mov->dst) &&
            mov->cond == ExCond::Always)
          remove = true;
      }
      TypeCase(i_type, IType *, it->get()) {
        if (i_type->op == IType::Add || i_type->op == IType::Sub) {
          if (i_type->dst == i_type->s1 && i_type->imm == 0)
            remove = true;
        }
      }

      if (remove)
        it = insns.erase(it);
      else
        ++it;
    }
  }
}

void post_order_dfs(BasicBlock *bb, std::unordered_set<BasicBlock *> &visited,
                    std::vector<BasicBlock *> &order) {
  if (visited.count(bb))
    return;
  visited.insert(bb);
  for (auto next : bb->succ)
    post_order_dfs(next, visited, order);
  order.push_back(bb);
}

std::vector<BasicBlock *> Function::compute_post_order() const {
  std::vector<BasicBlock *> order;
  if (bbs.empty())
    return order;

  std::unordered_set<BasicBlock *> visited;
  post_order_dfs(bbs.front().get(), visited, order);
  return order;
}

// inline std::unique_ptr<Instruction> &get_terminator(BasicBlock *bb) {
//   assert(!bb->insns.empty());
//   auto &insn = bb->insns.back();
//   assert(insn->is<Terminator>());
//   return insn;
// }

bool clean_control_flow(Function &f) {
  if (f.bbs.empty())
    return false;

  std::unordered_set<BasicBlock *> visited;
  std::vector<BasicBlock *> order;
  auto entry = f.bbs.front().get();
  post_order_dfs(entry, visited, order);

  bool changed = false;
  auto get_terminator = [](BasicBlock *bb) -> std::unique_ptr<Instruction> & {
    assert(!bb->insns.empty());
    auto &insn = bb->insns.back();
    // assert(insn->is<Terminator>());
    return insn;
  };
  for (auto bb_iter = order.rbegin(); bb_iter != order.rend(); ++bb_iter) {
    auto bb = *bb_iter;
    if (!visited.count(bb)) // skip newly produced unreachable basic blocks
      continue;

    auto &insn = get_terminator(bb);
    TypeCase(br, CmpBranch *, insn.get()) {
      if (br->true_target == br->false_target) { // case 1
        insn.reset(new Branch{br->true_target});
        changed = true;
      }
    }
    TypeCase(br, Branch *, insn.get()) {
      auto target = br->target;
      if (bb->insns.size() == 1 && bb != entry) { // case 2: remove empty `bb`
        // replace transfers to `bb` with transfers to `target`
        for (auto pred : bb->pred) {
          auto transfer = get_terminator(pred).get();
          TypeCase(cmp_br, CmpBranch *, transfer) {
            if (cmp_br->true_target == bb)
              cmp_br->true_target = target;
            if (cmp_br->false_target == bb)
              cmp_br->false_target = target;
          }
          else TypeCase(jump, Branch *, transfer) {
            assert(jump->target == bb);
            jump->target = target;
          }
          pred->succ.erase(bb);
          pred->succ.insert(target);
        }
        target->pred.erase(bb);
        target->pred.merge(bb->pred);
        bb->pred.clear();
        bb->succ.clear();

        visited.erase(bb);
        changed = true;
        continue;
      }
      if (target->pred.size() == 1) { // case 3
        // merge `target` into `bb`
        for (auto next : target->succ) {
          next->pred.erase(target);
          next->pred.insert(bb);
        }
        bb->succ.erase(target);
        bb->succ.merge(target->succ);
        target->succ.clear();
        target->pred.clear();

        bb->insns.pop_back();
        bb->insns.splice(bb->insns.end(), target->insns);

        visited.erase(target);
        changed = true;
        continue;
      }
      if (target->insns.size() == 1) {
        auto &last = get_terminator(target);
        TypeCase(cmp_br, CmpBranch *, last.get()) { // case 4
                                                    // hoist a branch
          auto true_target = cmp_br->true_target;
          auto false_target = cmp_br->false_target;
          BasicBlock::add_edge(bb, true_target);
          BasicBlock::add_edge(bb, false_target);
          BasicBlock::remove_edge(bb, target);

          auto cmp_cloned = new Compare{*cmp_br->cmp};
          auto cmp_br_cloned =
              new CmpBranch{cmp_cloned, true_target, false_target};
          cmp_br_cloned->cond = cmp_br->cond;
          insn.reset(cmp_br_cloned);
          changed = true;
        }
      }
    }
  }

  // remove unreachable basic blocks
  for (auto it = f.bbs.begin(); it != f.bbs.end();) {
    if (!visited.count(it->get()))
      it = f.bbs.erase(it);
    else
      ++it;
  }
  return changed;
}

void sanitize_cfg(Function &f) {
  bool changed;
  do {
    changed = clean_control_flow(f);
  } while (changed);
}

} // namespace armv7
