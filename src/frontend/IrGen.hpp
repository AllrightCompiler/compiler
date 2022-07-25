#pragma once

#include "common/ir.hpp"

#include "frontend/ast.hpp"

namespace frontend {

struct BranchTargets {
  std::vector<ir::BasicBlock **> true_list, false_list;

  std::vector<ir::BasicBlock **> &operator[](int index) {
    return index == 0 ? false_list : true_list;
  }

  void resolve(ir::BasicBlock *true_bb, ir::BasicBlock *false_bb) {
    for (auto p : true_list)
      *p = true_bb;
    for (auto p : false_list)
      *p = false_bb;
  }
};

class IrGen {
  std::unique_ptr<ir::Program> program;

  int local_regs, global_regs;
  ir::Function *cur_func;  // 在函数内部保持有效
  ir::BasicBlock *cur_bb;  // 当前基本块
  ir::BasicBlock *init_bb; // 用于初始化代码的全局基本块

  std::vector<ir::BasicBlock *> break_targets, continue_targets;

  std::unordered_map<Var *, ir::Storage> mem_vars;

  ir::Reg new_reg(int t) {
    auto &nr_regs = cur_func ? local_regs : global_regs;
    return ir::Reg{t, ++nr_regs};
  }

  ir::BasicBlock *new_bb();
  ir::Reg scalar_cast(ir::Reg src, int dst_type);

  void emit(ir::Instruction *insn) { cur_bb->push(insn); }
  BranchTargets emit_branch(ir::Reg val);

  void visit_declaration(const ast::Declaration &);
  void visit_function(const ast::Function &);
  void visit_statement(const ast::Statement &);
  void visit_initializer(const std::vector<std::unique_ptr<ast::Initializer>> &,
                         const std::shared_ptr<Var> &, ir::Reg base_reg,
                         int depth, int &index);

  void visit_if(const ast::IfElse &);
  void visit_while(const ast::While &);

  ir::Reg visit_lvalue(const ast::LValue &, bool return_addr_when_scalar);

  ir::Reg visit_arith_expr(const ast::Expression *);
  BranchTargets visit_logical_expr(const ast::Expression *);

  ir::Reg visit_arith_expr(const std::unique_ptr<ast::Expression> &p) {
    return visit_arith_expr(p.get());
  }
  BranchTargets visit_logical_expr(const std::unique_ptr<ast::Expression> &p) {
    return visit_logical_expr(p.get());
  }

public:
  IrGen();
  void visit_compile_unit(const ast::CompileUnit &);
  const std::unique_ptr<ir::Program> &get_program() const { return program; }
};

} // namespace frontend
