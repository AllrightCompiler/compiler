#pragma once

#include "common/ir.hpp"

namespace llvm {

using Reg = ir::Reg;
using Instruction = ir::Instruction;
using Output = ir::insns::Output;

struct BasicBlock;

struct SimplePhi : Output {
  std::map<BasicBlock *, Reg> srcs;

  SimplePhi(Reg dst) : Output{dst} {}

  void emit(std::ostream &os) const override;
  std::vector<Reg *> reg_ptrs() override {
    std::vector<Reg *> ptrs{&dst};
    for (auto &[bb, _] : srcs)
      ptrs.push_back(&srcs[bb]);
    return ptrs;
  }
};

struct SimpleJump : ir::insns::Terminator {
  BasicBlock *target;

  SimpleJump(BasicBlock *target) : target{target} {}

  void emit(std::ostream &os) const override;
};

struct SimpleBranch : ir::insns::Terminator {
  BasicBlock *true_target, *false_target;
  Reg val;

  SimpleBranch(Reg src, BasicBlock *true_dst, BasicBlock *false_dst)
      : val{src}, true_target{true_dst}, false_target{false_dst} {}

  void emit(std::ostream &os) const override;
  std::vector<Reg *> reg_ptrs() override { return {&val}; }
};

// llvm bitcast，用于指针转换
struct PtrCast : Output {
  Reg src;
  Type src_type, dst_type;

  PtrCast(Reg dst, Reg src, Type dst_type, Type src_type)
      : Output{dst}, src{src}, dst_type{std::move(dst_type)},
        src_type{std::move(src_type)} {}

  void emit(std::ostream &os) const override;
  std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
};

// zext a to b / trunc a to b
struct IntCast : Output {
  enum Op {
    Zext,
    Trunc,
  };

  Op op;
  Reg src;

  IntCast(Reg dst, Op op, Reg src) : op{op}, Output{dst}, src{src} {}

  void emit(std::ostream &os) const override;
  std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
};

struct ZeroCmp : Output {
  enum Op {
    Eq,
    Ne,
  } op;
  Reg src;

  ZeroCmp(Reg dst, Op op, Reg src) : op{op}, Output{dst}, src{src} {}

  void emit(std::ostream &os) const override;
  std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
};

} // namespace llvm
