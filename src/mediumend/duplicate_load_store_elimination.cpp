#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::Function;
using ir::Instruction;

// Rely on Array SSA!

void duplicate_load_store_elimination(Function *func) {
  unordered_set<Instruction *> removable_inst;
  for (auto &bb : func->bbs) {
    for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
      TypeCase(memuse, ir::insns::MemUse *, iter->get()) {
        if(func->def_list.count(memuse->dep)){
          auto def = func->def_list.at(memuse->dep);
          TypeCase(memdef, ir::insns::MemDef *, def){
            if(memuse->load_src == memdef->store_dst){
              copy_propagation(func->use_list, memuse->dst, memdef->store_val);
              memuse->remove_use_def();
              removable_inst.insert(memuse);
            }
          }
        }
      }
      TypeCase(memdef, ir::insns::MemDef *, iter->get()) {
        auto dep = func->def_list.at(memdef->dep);
        TypeCase(def, ir::insns::MemDef *, dep){
          if(memdef->uses_before_def.size() == 0 && def->bb == memdef->bb && memdef->store_dst == def->store_dst){
            def->remove_use_def();
            removable_inst.insert(def);
          }
        }

      }
      iter++;
    }
  }
  for (auto &bb : func->bbs) {
    for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
      if(removable_inst.count(iter->get())){
        iter = bb->insns.erase(iter);
      }else{
        iter++;
      }
    }
  }
}

void duplicate_load_store_elimination(ir::Program *prog) {
  for (auto &each : prog->functions) {
    auto func = &each.second;
    duplicate_load_store_elimination(func);
  }
}

} // namespace mediumend