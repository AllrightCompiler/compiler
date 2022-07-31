#include "backend/armv7/merge_instr.hpp"

#include <functional>

namespace armv7 {

// def -> bool
// reg, def, use -> instr
static void merge_instr(
    Function &func,
    std::function<bool(Instruction const &)> const &check_def_instr,
    std::function<std::unique_ptr<Instruction>(Reg, Instruction const &,
                                               Instruction const &)> const
        &transform_use_instr) {
  func.build_def_use();
  for (auto const &[r, occurs] : func.reg_def) {
    if (occurs.size() != 1) {
      continue;
    }
    auto &def = *occurs.cbegin();
    if (def.instr->get()->cond != ExCond::Always) {
      continue;
    }
    if (!check_def_instr(**def.instr)) {
      continue;
    }
    bool do_transform = true;
    int max_use_instr_index = -1;
    std::vector<std::pair<OccurPoint const *, std::unique_ptr<Instruction>>>
        new_instrs;
    auto const &uses = func.reg_use[r];
    for (auto const &use : uses) {
      auto new_instr = transform_use_instr(r, **def.instr, **use.instr);
      if (!new_instr) {
        goto continue_;
      }
      new_instr->cond = use.instr->get()->cond;
      new_instrs.emplace_back(&use, std::move(new_instr));
      if (use.bb == def.bb) {
        max_use_instr_index = std::max(use.index, max_use_instr_index);
      }
    }
    for (auto dep_r : def.instr->get()->use()) {
      for (auto const &dep_def : func.reg_def[dep_r]) {
        if (dep_def.bb == def.bb && def.index <= dep_def.index &&
            dep_def.index <= max_use_instr_index) {
          goto continue_;
        }
      }
    }
    for (auto &[use, new_instr] : new_instrs) {
      func.erase_def_use(*use, **use->instr);
      *use->instr = std::move(new_instr);
      func.insert_def_use(*use, **use->instr);
    }
  continue_:
    continue;
  }
}

} // namespace armv7
