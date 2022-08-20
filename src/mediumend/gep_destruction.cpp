#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::Reg;

void gep_destruction(ir::Function *func) {
  for (auto &bb : func->bbs) {
    for (auto it = bb->insns.begin(); it != bb->insns.end();) {
      TypeCase(gep, ir::insns::GetElementPtr *, it->get()) {
        std::vector<ir::Instruction *> insts_to_insert;
        Reg index_reg = gep->base;
        int n_indices = gep->indices.size();
        for (int i = 0; i < n_indices; i++) {
          Reg t = func->new_reg(String);
          insts_to_insert.push_back(new ir::insns::GetElementPtr(
              t, Type(gep->type.base_type, {gep->type.dims.begin() + i, gep->type.dims.end()}), index_reg, {gep->indices[i]}));
          index_reg = t;
        }
        copy_propagation(func->use_list, gep->dst, index_reg);
        auto pos = bb->remove_at(it);
        for (auto i : insts_to_insert) {
          bb->insert_at(pos, i);
        }
        it = pos;
      }
      else it++;
    }
  }
}

void gep_destruction(ir::Program *prog) {
  for (auto &func : prog->functions) {
    gep_destruction(&func.second);
  }
}

} // namespace mediumend