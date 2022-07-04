#pragma once

#include "backend/armv7/instruction.hpp"
#include "common/common.hpp"

#include <functional>
#include <list>
#include <set>

namespace armv7 {

struct StackObject {
  int size, offset; // offset: 暂定为相对于某个sp的偏移
};

struct BasicBlock {
  std::string label;
  std::list<std::unique_ptr<Instruction>> insns;

  std::list<BasicBlock *> pred, succ;             // CFG
  std::set<Reg> def, live_use, live_in, live_out; // for liveness analysis

  void push(Instruction *insn) { insns.emplace_back(insn); }
  void push(ExCond cond, Instruction *insn) {
    insn->cond = cond;
    insns.emplace_back(insn);
  }

  static void add_edge(BasicBlock *from, BasicBlock *to) {
    from->succ.push_back(to);
    to->pred.push_back(from);
  }
};

using RegFilter = std::function<bool(const Reg &)>;

struct Function {
  std::string name;
  std::list<std::unique_ptr<BasicBlock>> bbs;
  std::list<std::unique_ptr<StackObject>> stack_objects;
  // BasicBlock *entry, *exit;

  // stack objects的细分类型:
  // 1. 此函数的参数，位于fp（初始sp）之上，相对fp偏移为正
  // 2. 普通栈对象，包括局部数组和spilled regs，相对fp偏移为负
  // 未计入的类型:
  // 3. 调用子函数压栈的参数，相对fp偏移为负
  std::vector<StackObject *> param_objs, normal_objs;

  int regs_allocated; // 分配的虚拟寄存器总数

  Reg new_reg(Reg::Type type) { return Reg{type, ++regs_allocated}; }
  void push(BasicBlock *bb) { bbs.emplace_back(bb); }

  void do_liveness_analysis(RegFilter filter = [](const Reg &){ return true; });
  bool check_and_resolve_stack_store();
  
  // post-register allocation passes
  void emit_prologue_epilogue();
  void resolve_stack_ops(int frame_size);
};

struct Program {
  std::unordered_map<std::string, Function> functions;

  void emit(std::ostream &os);
};

std::unique_ptr<Program> translate(const ir::Program &ir_program);

} // namespace armv7
