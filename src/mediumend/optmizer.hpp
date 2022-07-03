#pragma once

#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

namespace mediumend {

void remove_unused_function(ir::Program *prog);
void mem2reg(ir::Program *prog);
void constant_propagation(ir::Program *prog);
void mark_pure_func(ir::Program *);
void remove_uneffective_inst(ir::Program *prog);

inline void run_medium(ir::Program *prog) {
  for (auto &func : prog->functions){
    func.second.cfg = new CFG(&func.second);
    func.second.cfg->build();
    func.second.cfg->remove_unreachable_bb();
    compute_use_def_list(&func.second);
    func.second.cfg->compute_dom();
    mark_global_addr_reg(&func.second);
  }
  mem2reg(prog);
  remove_unused_function(prog);
  constant_propagation(prog);
  mark_pure_func(prog);
  remove_uneffective_inst(prog);
  // 移除无用指令后可能有的函数不会被调用，pure function / unreachable BB里的function
  remove_unused_function(prog);

  for(auto &func : prog->functions){
    func.second.cfg->clean_useless_cf();
  }
}

} // namespace mediumend