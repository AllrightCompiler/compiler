#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

#include "iostream"

namespace mediumend {

using ir::Function;
using ir::BasicBlock;

void gvn_gcm(Function *f) {
  // Global Value Numbering
  f->cfg->build();
  f->cfg->compute_dom();
  // cfg dom, domby, idom, domlevel set
  f->cfg->compute_rpo();
  // rpo set
  for (auto bb : f->cfg->rpo) {
    for (auto &insn : bb->insns) {
      TypeCase(i, ir::insns::Phi *, insn.get()) {

      }
      TypeCase(i, ir::insns::LoadImm *, insn.get()) {

      }
      TypeCase(i, ir::insns::Binary *, insn.get()) {

      }
      // TODO: more inst types
    }
  }
}

void gvn_gcm(ir::Program *prog) {
  for(auto &func : prog->functions){
    // std::cout << "func: " << func.first << std::endl;
    gvn_gcm(&func.second);
  }
}

}

