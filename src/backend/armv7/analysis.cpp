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

  auto post_order = compute_post_order();
  bool changed = true;
  while (changed) {
    changed = false;
    for (auto it = post_order.rbegin(); it != post_order.rend(); ++it) {
      auto bb = *it;
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

void Function::insert_def_use(OccurPoint const &pos, Instruction &instr) {
  for (auto r : instr.def()) {
    this->reg_def[r].insert(pos);
  }
  for (auto r : instr.use()) {
    this->reg_use[r].insert(pos);
  }
}
void Function::erase_def_use(OccurPoint const &pos, Instruction &instr) {
  for (auto r : instr.def()) {
    this->reg_def[r].erase(pos);
  }
  for (auto r : instr.use()) {
    this->reg_use[r].erase(pos);
  }
}
void Function::build_def_use() {
  this->reg_def.clear();
  this->reg_use.clear();
  for (auto const &bb : this->bbs) {
    for (auto [instr, index] = std::pair{bb->insns.begin(), 0};
         instr != bb->insns.end(); ++instr, ++index) {
      this->insert_def_use(
          OccurPoint{.bb = bb.get(), .instr = instr, .index = index}, **instr);
    }
  }
}

} // namespace armv7
