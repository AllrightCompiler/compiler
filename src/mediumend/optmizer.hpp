#pragma once

#include "common/ir.hpp"

namespace mediumend {

void remove_unused_function(ir::Program *prog);
void mem2reg(ir::Function *func);

inline void run_medium(ir::Program *prog) {
  for (auto &func : prog->functions) {
    mem2reg(&func.second);
  }
  remove_unused_function(prog);
}

} // namespace mediumend