#include "backend/armv7/program.hpp"

namespace armv7 {

void Function::do_liveness_analysis(RegFilter filter) {
  for (auto &bb : bbs) {
    bb->live_use.clear();
    bb->def.clear();

    for (auto &insn : bb->insns) {
      auto def = insn->def();
      auto use = insn->use();

      for (auto &u : use) {
        if (filter(u) && !bb->def.count(u))
          bb->live_use.insert(u);
      }
      for (auto &d : def) {
        if (filter(d))
          bb->def.insert(d);
      }
    }

    bb->live_in = bb->live_use;
    bb->live_out.clear();
  }

  bool changed = true;
  while (changed) {
    changed = false;
    for (auto &bb : bbs) {
      std::set<Reg> new_out;
      for (auto succ : bb->succ)
        new_out.insert(succ->live_in.begin(), succ->live_in.end());
      
      if (bb->live_out != new_out) {
        bb->live_out = std::move(new_out);
        changed = true;

        auto new_in = bb->live_use;
        for (auto &e : bb->live_out)
          if (!bb->def.count(e))
            new_in.insert(e);
        
        bb->live_in = std::move(new_in);
      }
    }
  }
}

} // namespace armv7
