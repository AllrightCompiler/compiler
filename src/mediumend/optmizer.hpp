#pragma once

#include "mem2reg.hpp"

namespace mediumend {

inline void run_medium(ir::Program *prog) {
  for (auto &func : prog->functions) {
    mem2reg(&func.second);
  }
}

} // namespace mediumend