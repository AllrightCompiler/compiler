#pragma once

#include "backend/armv7/instruction.hpp"
#include "common/common.hpp"

#include <list>
#include <set>

namespace armv7 {

struct StackObject {
  int size, offset; // offset: 暂定为相对于某个sp的偏移
};

struct BasicBlock {
  std::string label;
  std::list<std::unique_ptr<Instruction>> insns;

  void push(Instruction *insn) { insns.emplace_back(insn); }
  void push(ExCond cond, Instruction *insn) {
    insn->cond = cond;
    insns.emplace_back(insn);
  }
};

struct Function {
  std::string name;
  std::list<std::unique_ptr<BasicBlock>> bbs;
  std::list<std::unique_ptr<StackObject>> stack_objects;

  // stack objects的细分类型:
  // 1. 此函数的参数，位于fp（初始sp）之上，相对fp偏移为正
  // 2. 普通栈对象，包括局部数组和spilled regs，相对fp偏移为负
  // 未计入的类型:
  // 3. 调用子函数压栈的参数，相对fp偏移为负
  std::set<StackObject *> param_objs, normal_objs;

  int regs_allocated; // 分配的虚拟寄存器总数

  Reg new_reg(ScalarType type) { return Reg{type, ++regs_allocated}; }
  void push(BasicBlock *bb) { bbs.emplace_back(bb); }
};

struct Program {
  std::unordered_map<std::string, Function> functions;

  void emit(std::ostream &os);
};

std::unique_ptr<Program> translate(const ir::Program &ir_program);

} // namespace armv7
