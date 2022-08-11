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

  std::set<BasicBlock *> pred, succ;              // CFG
  std::set<Reg> def, live_use, live_in, live_out; // for liveness analysis

  void push(Instruction *insn) { insns.emplace_back(insn); }
  void push(ExCond cond, Instruction *insn) {
    insn->cond = cond;
    insns.emplace_back(insn);
  }

  std::list<std::unique_ptr<Instruction>>::iterator sequence_end();
  void insert_at_end(Instruction *insn);

  static void add_edge(BasicBlock *from, BasicBlock *to) {
    from->succ.insert(to);
    to->pred.insert(from);
  }
  static void remove_edge(BasicBlock *from, BasicBlock *to) {
    from->succ.erase(to);
    to->pred.erase(from);
  }
};

struct OccurPoint {
  BasicBlock *bb;
  std::list<std::unique_ptr<Instruction>>::iterator instr;
  int index;

  friend bool operator<(OccurPoint const &lhs, OccurPoint const &rhs) {
    return lhs.bb == rhs.bb ? lhs.index < rhs.index : lhs.bb < rhs.bb;
  }
};

using RegFilter = std::function<bool(const Reg &)>;

// 单赋值虚拟寄存器的一些特殊取值
enum RegValueType {
  Imm = 0,
  GlobalName = 1,
  StackAddr = 2,
};
using RegValue = std::variant<int, std::string, StackObject *>;

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

  std::map<Reg, RegValue> reg_val; // 记录一些单赋值虚拟寄存器的取值
  std::set<Move *>
      phi_moves; // 由phi指令产生的mov，这些mov相关的寄存器不应被合并

  int regs_used; // 分配的虚拟寄存器总数

  std::unordered_map<Reg, std::set<OccurPoint>> reg_def, reg_use;

  Reg new_reg(Reg::Type type) { return Reg{type, -(++regs_used)}; }
  void push(BasicBlock *bb) { bbs.emplace_back(bb); }

  void emit_imm(std::list<std::unique_ptr<Instruction>> &insns,
                const std::list<std::unique_ptr<Instruction>>::iterator &it,
                Reg dst, int imm);
  void emit_imm(BasicBlock *, Reg dst, int imm);

  void do_liveness_analysis(RegFilter filter = [](const Reg &) {
    return true;
  });
  void insert_def_use(OccurPoint const &pos, Instruction &instr);
  void erase_def_use(OccurPoint const &pos, Instruction &instr);
  void build_def_use();

  bool check_and_resolve_stack_store();
  void defer_stack_param_load(Reg r, StackObject *obj);
  void resolve_phi();

  template <typename RegAllocator>
  void do_reg_alloc(RegAllocator &allocator, bool is_gp_pass = true) {
    allocator.do_reg_alloc(*this, is_gp_pass);
  }

  std::vector<BasicBlock *> compute_post_order() const;

  // post-register allocation passes
  void emit_prologue_epilogue();
  void resolve_stack_ops(int frame_size);
  void replace_pseudo_insns();

  void emit(std::ostream &os);
};

struct Program {
  std::vector<std::string> builtin_code;
  std::unordered_map<std::string, Function> functions;

  int labels_used;
  std::string new_label() { return ".L" + std::to_string(labels_used++); }

  Program();

  void emit(std::ostream &os);
};

std::unique_ptr<Program> translate(const ir::Program &ir_program);
void emit_global(std::ostream &os, const ir::Program &ir_program);

template <typename Container = std::list<std::unique_ptr<Instruction>>>
auto emit_load_imm(Container &cont, typename Container::iterator it, Reg dst,
                   int imm) -> typename Container::iterator {
  if (is_imm8m(imm))
    return cont.emplace(it, new Move{dst, Operand2::from(imm)});
  else if (is_imm8m(~imm))
    return cont.emplace(it, new Move{dst, Operand2::from(~imm), true});
  else {
    uint32_t x = uint32_t(imm);
    auto lo = x & 0xffff, hi = x >> 16;
    auto ret = cont.emplace(it, new MovW(dst, lo));
    if (hi > 0)
      cont.emplace(it, new MovT(dst, hi));
    return ret;
  }
}

} // namespace armv7
