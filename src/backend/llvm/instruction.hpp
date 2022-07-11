#pragma once

#include "common/ir.hpp"

namespace llvm {

using Instruction = ir::Instruction;

struct BasicBlock;

struct SimplePhi : ir::insns::Output {
  std::map<BasicBlock *, ir::Reg> srcs;

  SimplePhi(ir::Reg dst) : ir::insns::Output{dst} {}

  void emit(std::ostream &os) const override;
};

struct SimpleJump : ir::insns::Terminator {
  BasicBlock *target;

  SimpleJump(BasicBlock *target) : target{target} {}

  void emit(std::ostream &os) const override;
};

struct SimpleBranch : ir::insns::Terminator {
  BasicBlock *true_target, *false_target;
  ir::Reg val;

  SimpleBranch(ir::Reg src, BasicBlock *true_dst, BasicBlock *false_dst)
      : val{src}, true_target{true_dst}, false_target{false_dst} {}
  
  void emit(std::ostream &os) const override;
};

} // namespace llvm
