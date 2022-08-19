#include "backend/armv7/if_to_cond.hpp"

#include "common/common.hpp"

#include <iostream>

namespace armv7 {

constexpr auto MAX_COND_INSTR_COUNT = 6u;

static bool can_be_cond(BasicBlock const &bb) {
  for (auto const &instr : bb.insns) {
    if (instr->cond != ExCond::Always) {
      return false;
    }
    if (instr->is<Compare>() || instr->is<PseudoCompare>() ||
        instr->is<PseudoOneDividedByReg>() || instr->is<CmpBranch>() ||
        instr->is<Switch>() || instr->is<Call>()) {
      return false;
    }
  }
  return true;
}

static std::vector<std::unique_ptr<Instruction>>
conditionalize(BasicBlock const &bb, ExCond const cond) {
  assert(!bb.insns.empty());
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

std::optional<BasicBlock *> try_get_next(BasicBlock const &bb) {
  if (bb.succ.empty()) {
    return nullptr;
  } else if (bb.succ.size() == 1u) {
    return *bb.succ.begin();
  } else {
    return {};
  }
}

void if_to_cond(Function &func) {
  for (auto &bb : func.bbs) {
    assert(!bb->insns.empty());
    auto const br = dynamic_cast<CmpBranch *>(bb->insns.back().get());
    if (br == nullptr) {
      continue;
    }
    auto const then_next = try_get_next(*br->true_target);
    auto const otherwise_next = try_get_next(*br->false_target);
    if (then_next && *then_next == br->false_target &&
        br->true_target->insns.size() <= MAX_COND_INSTR_COUNT &&
        can_be_cond(*br->true_target)) {
      auto const cond = br->cond;
      auto const true_target = br->true_target;
      bb->insns.back() = std::move(br->cmp);
      auto then_instrs = conditionalize(*true_target, cond);
      bb->insns.insert(bb->insns.cend(),
                       std::make_move_iterator(then_instrs.begin()),
                       std::make_move_iterator(std::prev(then_instrs.end())));
      BasicBlock::remove_edge(bb.get(), true_target);
      bb->insns.push_back(std::make_unique<Branch>(*then_next));
      BasicBlock::add_edge(bb.get(), *then_next);
    } else if (otherwise_next && *otherwise_next == br->true_target &&
               br->false_target->insns.size() <= MAX_COND_INSTR_COUNT &&
               can_be_cond(*br->false_target)) {
      auto const cond = br->cond;
      auto const false_target = br->false_target;
      bb->insns.back() = std::move(br->cmp);
      auto otherwise_instrs = conditionalize(*false_target, logical_not(cond));
      bb->insns.insert(
          bb->insns.cend(), std::make_move_iterator(otherwise_instrs.begin()),
          std::make_move_iterator(std::prev(otherwise_instrs.end())));
      BasicBlock::remove_edge(bb.get(), false_target);
      bb->insns.push_back(std::make_unique<Branch>(*otherwise_next));
      BasicBlock::add_edge(bb.get(), *otherwise_next);
    } else if (then_next && otherwise_next && *then_next == *otherwise_next &&
               br->true_target->insns.size() + br->false_target->insns.size() <=
                   MAX_COND_INSTR_COUNT &&
               can_be_cond(*br->true_target) &&
               can_be_cond(*br->false_target)) {
      auto const cond = br->cond;
      auto const true_target = br->true_target;
      auto const false_target = br->false_target;
      bb->insns.back() = std::move(br->cmp);
      auto then_instrs = conditionalize(*true_target, cond);
      bb->insns.insert(bb->insns.cend(),
                       std::make_move_iterator(then_instrs.begin()),
                       std::make_move_iterator(std::prev(
                           then_instrs.end(), *then_next != nullptr)));
      BasicBlock::remove_edge(bb.get(), true_target);
      auto otherwise_instrs = conditionalize(*false_target, logical_not(cond));
      bb->insns.insert(
          bb->insns.cend(), std::make_move_iterator(otherwise_instrs.begin()),
          std::make_move_iterator(
              std::prev(otherwise_instrs.end(), *otherwise_next != nullptr)));
      BasicBlock::remove_edge(bb.get(), false_target);
      if (*then_next != nullptr) {
        bb->insns.push_back(std::make_unique<Branch>(*then_next));
        BasicBlock::add_edge(bb.get(), *then_next);
      }
    } else {
      continue;
    }
    debug(std::cerr) << "conditionalize " << bb->label << '\n';
  }
}

} // namespace armv7
