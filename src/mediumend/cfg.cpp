#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

#include <cassert>

namespace mediumend {

CFG::CFG(ir::Function *func) {
  for (auto & bb : func->bbs) {
    this->succ[bb.get()] = {};
    this->pred[bb.get()] = {};
  }
  for (auto &bb : func->bbs) {
    if(bb->insns.empty()) continue;
    auto &ins = bb->insns.back();
    ir::insns::Jump *jmp = dynamic_cast<ir::insns::Jump *>(ins.get());
    ir::insns::Branch *brh = dynamic_cast<ir::insns::Branch *>(ins.get());
    ir::insns::Return *ret = dynamic_cast<ir::insns::Return *>(ins.get());
    auto &succ = this->succ[bb.get()];
    if (jmp) {
      succ.insert(jmp->target);
      pred[jmp->target].insert(bb.get());
    } else if (brh) {
      succ.insert(brh->true_target);
      succ.insert(brh->false_target);
      pred[brh->true_target].insert(bb.get());
      pred[brh->false_target].insert(bb.get());
    } else if (ret) {
    } else {
      assert(false);
    }
  }
}

} // namespace mediumend