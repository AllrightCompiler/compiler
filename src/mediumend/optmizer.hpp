#pragma once

#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

namespace mediumend {

void remove_unused_function(ir::Program *prog);
void mem2reg(ir::Function *func, CFG* cfg);

inline void run_medium(ir::Program *prog) {
  std::unordered_map<ir::Function *, CFG> func2cfg;
  for (auto &func : prog->functions){
    func2cfg[&func.second] = CFG(&func.second);
  }
  for (auto &func : prog->functions) {
    mem2reg(&func.second, &func2cfg[&func.second]);
  }
  for(auto &each : func2cfg){
    each.second.remove_unused_reg();
  }
  remove_unused_function(prog);
}

} // namespace mediumend