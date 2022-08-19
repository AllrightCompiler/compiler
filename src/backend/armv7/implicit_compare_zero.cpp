#include "backend/armv7/implicit_compare_zero.hpp"

#include "common/common.hpp"

#include <iostream>

namespace armv7 {

static bool is_compare_zero(std::unique_ptr<Instruction> const &instr) {
  TypeCase(cmp, Compare const *, instr.get()) {
    if (cmp->s2.is_imm8m()) {
      return cmp->s2.get<int>() == 0 && !cmp->neg;
    }
  }
  return false;
}

static bool involve_cpsr(Instruction const &instr) {
  if (instr.cond != ExCond::Always) {
    return true;
  }
  if (instr.update_cpsr) {
    return true;
  }
  return false;
}

static bool can_update_cpsr(Instruction const &instr) {
  TypeCase(r_instr, RType const *, &instr) {
    return r_instr->op != RType::SMMul && !r_instr->dst.is_float();
  }
  TypeCase(i_instr, IType const *, &instr) { return !i_instr->dst.is_float(); }
  TypeCase(fr_instr, FullRType const *, &instr) {
    return !fr_instr->dst.is_float();
  }
  TypeCase(mov, Move const *, &instr) {
    return !mov->dst.is_float() && !mov->is_transfer_vmov();
  }
  TypeCase(fused_mul, FusedMul const *, &instr) {
    return fused_mul->op != FusedMul::SMAdd;
  }
  return false;
}

void implicit_compare_zero(Function &func) {
  for (auto &bb : func.bbs) {
    auto cmp = bb->insns.begin();
    while (cmp = std::find_if(cmp, bb->insns.end(), is_compare_zero),
           cmp != bb->insns.end()) {
      debug(std::cerr) << "find compare 0: " << **cmp << '\n';
      auto instr = cmp;
      while (instr != bb->insns.begin()) {
        instr = std::prev(instr);
        if (involve_cpsr(**instr)) {
          goto continue_;
        }
        if (instr->get()->def().count(static_cast<Compare *>(cmp->get())->s1)) {
          break;
        }
      }
      if (can_update_cpsr(**instr)) {
        instr->get()->update_cpsr = true;
        debug(std::cerr) << "implicit compare 0: " << **cmp << '\n';
        cmp = bb->insns.erase(cmp);
        continue;
      }
    continue_:
      cmp = std::next(cmp);
    }
  }
}

} // namespace armv7
