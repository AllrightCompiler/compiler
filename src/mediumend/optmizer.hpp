#pragma once

#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

namespace mediumend {

void remove_unused_function(ir::Program *prog);
void mem2reg(ir::Program *prog);
void constant_propagation(ir::Program *prog);
void mark_pure_func(ir::Program *);
void remove_uneffective_inst(ir::Program *prog);
void clean_useless_cf(ir::Program *prog);
void simplification_phi(ir::Program *prog);
void main_global_var_to_local(ir::Program *prog);
void gvn_gcm(ir::Program *prog);
void function_inline(ir::Program *prog);

ConstValue const_compute(ir::Instruction *inst, ConstValue oprand);
ConstValue const_compute(ir::Instruction *inst, ConstValue op1, ConstValue op2);

inline void run_medium(ir::Program *prog) {
  for (auto &func : prog->functions){
    func.second.cfg = new CFG(&func.second);
  }
  compute_use_def_list(prog);
  main_global_var_to_local(prog);
  mem2reg(prog);

  remove_unused_function(prog);
  main_global_var_to_local(prog);

  // 纯函数可以用来做GVN和无用代码移除
  mark_pure_func(prog);

  function_inline(prog);
  gvn_gcm(prog);
  remove_uneffective_inst(prog);

  // 下面这两步和SCCP感觉是等效的？
  constant_propagation(prog);
  simplification_phi(prog);

  remove_uneffective_inst(prog);
  // 移除无用指令后可能有的函数不会被调用，pure function / unreachable BB里的function
  remove_unused_function(prog);
  clean_useless_cf(prog);
}

} // namespace mediumend