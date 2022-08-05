#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

#include <cassert>

namespace mediumend {

using ir::Function;
using ir::Reg;

const static int MIN_BR_CNT = 5;

void br2switch(Function *func) {
  func->cfg->compute_rpo();
   
}

void br2switch(ir::Program *prog) {
  for (auto &[name, func] : prog->functions) {
    br2switch(&func);
  }
}

} // namespace mediumend