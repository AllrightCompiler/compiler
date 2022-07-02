#pragma once

#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

namespace mediumend {

void remove_unused_function(ir::Program *prog);
void mem2reg(ir::Function *func);
void constant_propagation(Function *func);

inline void run_medium(ir::Program *prog) {
  for (auto &func : prog->functions){
    func.second.cfg = new CFG(&func.second);
    func.second.cfg->build();
    func.second.cfg->remove_unreachable_bb();
    func.second.cfg->compute_use_def_list();
    func.second.cfg->compute_dom();
  }
  for (auto &func : prog->functions) {
    mem2reg(&func.second);
  }
  for(auto &func : prog->functions){
    func.second.cfg->remove_unused_reg();
  }
  remove_unused_function(prog);
  for(auto &func : prog->functions){
    constant_propagation(&func.second);
  }
  for(auto &func : prog->functions){
    func.second.cfg->remove_unused_reg();
  }
}

} // namespace mediumend