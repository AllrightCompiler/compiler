#include "backend/armv7/if_to_cond.hpp"

namespace armv7 {

constexpr auto MAX_COND_INSTR_COUNT = 5u;

static bool can_be_cond(BasicBlock const &bb) {
  for (auto const &instr : bb.insns) {
    if (instr->cond != ExCond::Always) {
      return false;
    }
    if (instr->is<Compare>() || instr->is<PseudoCompare>() ||
        instr->is<PseudoOneDividedByReg>() || instr->is<CmpBranch>() ||
        instr->is<Switch>()) {
      return false;
    }
  }
  return true;
}

static std::vector<std::unique_ptr<Instruction>>
conditionalize(BasicBlock const &bb, ExCond const cond) {
  std::vector<std::unique_ptr<Instruction>> instrs;
  instrs.reserve(bb.insns.size());
  std::transform(bb.insns.cbegin(), bb.insns.cend(), std::back_inserter(instrs),
                 [cond](std::unique_ptr<Instruction> const &instr) {
                   auto cond_instr = instr->clone();
                   cond_instr->cond = cond;
                   return cond_instr;
                 });
  return instrs;
}

BasicBlock *try_get_next(BasicBlock const &bb) {
  assert(!bb.insns.empty());
  TypeCase(jump, Branch const *, bb.insns.back().get()) {
    if (jump->cond == ExCond::Always) {
      return jump->target;
    }
  }
  return nullptr;
}

void if_to_cond(Function &func) {
  for (auto iter = func.bbs.begin(); iter != func.bbs.end(); ++iter) {
    auto &bb = *iter;
    assert(!bb->insns.empty());
    auto const br = dynamic_cast<CmpBranch *>(bb->insns.back().get());
    if (br == nullptr) {
      continue;
    }
    if (br->true_target->insns.size() > MAX_COND_INSTR_COUNT ||
        br->false_target->insns.size() > MAX_COND_INSTR_COUNT) {
      continue;
    }
    auto const then = std::next(iter);
    if (then == func.bbs.end()) {
      continue;
    }
    auto const otherwise = std::next(then);
    if (otherwise == func.bbs.end()) {
      continue;
    }
    auto true_target = br->true_target;
    auto false_target = br->false_target;
    auto cond = br->cond;
    if (then->get() != true_target) {
      std::swap(true_target, false_target);
      cond = logical_not(cond);
    }
    if (then->get() != true_target || otherwise->get() != false_target) {
      continue;
    }
    BasicBlock *then_next;
    if (auto const target = try_get_next(*true_target)) {
      then_next = target;
    }
    BasicBlock *otherwise_next;
    if (auto const target = try_get_next(*false_target)) {
      otherwise_next = target;
    }
    if (then_next != otherwise_next) {
      continue;
    }
    if (!can_be_cond(*true_target) || !can_be_cond(*false_target)) {
      continue;
    }
    bb->insns.back() = std::move(br->cmp);
    auto then_instrs = conditionalize(*true_target, cond);
    bb->insns.insert(bb->insns.cend(),
                     std::make_move_iterator(then_instrs.begin()),
                     std::make_move_iterator(std::prev(then_instrs.end())));
    auto otherwise_instrs = conditionalize(*false_target, logical_not(cond));
    bb->insns.insert(
        bb->insns.cend(), std::make_move_iterator(otherwise_instrs.begin()),
        std::make_move_iterator(std::prev(otherwise_instrs.end())));
    bb->insns.push_back(std::make_unique<Branch>(then_next));
    BasicBlock::remove_edge(bb.get(), true_target);
    BasicBlock::remove_edge(bb.get(), false_target);
    BasicBlock::add_edge(bb.get(), then_next);
  }
}

} // namespace armv7
