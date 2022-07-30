#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::Function;
using ir::Instruction;
using ir::Reg;

using std::unordered_map;
using std::unordered_set;

// Rely on Array SSA!

void duplicate_load_store_elimination(Function *func) {
  unordered_set<Instruction *> removable_inst;
  for (auto &bb : func->bbs) {
    for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
      TypeCase(memuse, ir::insns::MemUse *, iter->get()) {
        if (func->def_list.count(memuse->dep)) {
          auto def = func->def_list.at(memuse->dep);
          TypeCase(memdef, ir::insns::MemDef *, def) {
            if (memuse->load_src == memdef->store_dst) {
              copy_propagation(func->use_list, memuse->dst, memdef->store_val);
              memuse->remove_use_def();
              removable_inst.insert(memuse);
            }
          }
        }
      }
      TypeCase(memdef, ir::insns::MemDef *, iter->get()) {
        if (func->def_list.count(memdef->dep)) {
          auto dep = func->def_list.at(memdef->dep);
          TypeCase(def, ir::insns::MemDef *, dep) {
            if (memdef->uses_before_def.size() == 0 && def->bb == memdef->bb &&
                memdef->store_dst == def->store_dst) {
              def->remove_use_def();
              removable_inst.insert(def);
            }
          }
        }
      }
      iter++;
    }
  }
  for (auto &bb : func->bbs) {
    for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
      if (removable_inst.count(iter->get())) {
        iter = bb->insns.erase(iter);
      } else {
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

void remove_zero_global_def(ir::Program *prog) {
  auto &func = prog->functions.at("main");
  unordered_set<Instruction *> none_zero_inst;
  vector<BasicBlock *> stack;
  func.clear_visit();
  stack.push_back(func.bbs.front().get());
  unordered_map<Reg, Reg> reg2base;
  unordered_map<std::string, Reg> name2reg;
  while (stack.size()) {
    bool changed = false;
    auto bb = stack.back();
    stack.pop_back();
    for (auto &inst : bb->insns) {
      if (none_zero_inst.count(inst.get())) {
        continue;
      }
      TypeCase(loadaddr, ir::insns::LoadAddr *, inst.get()) {
        if (name2reg.count(loadaddr->var_name)) {
          reg2base[loadaddr->dst] = name2reg.at(loadaddr->var_name);
          continue;
        }
        reg2base[loadaddr->dst] = loadaddr->dst;
        name2reg[loadaddr->var_name] = loadaddr->dst;
        if (prog->global_vars.at(loadaddr->var_name)->arr_val.get() &&
            prog->global_vars.at(loadaddr->var_name)->arr_val.get()->size() &&
            !none_zero_inst.count(loadaddr)) {
          none_zero_inst.insert(loadaddr);
          changed = true;
        }
      }
      TypeCase(memdef, ir::insns::MemDef *, inst.get()) {
        auto def_val = func.def_list.at(memdef->store_val);
        TypeCase(loadimm, ir::insns::LoadImm *, def_val) {
          if (loadimm->imm != 0) {
            none_zero_inst.insert(memdef);
            changed = true;
          }
        }
        else {
          none_zero_inst.insert(inst.get());
          changed = true;
        }
        reg2base[memdef->dst] = reg2base.at(memdef->dep);
        if (none_zero_inst.count(func.def_list.at(memdef->dep)) &&
            !none_zero_inst.count(memdef)) {
          none_zero_inst.insert(memdef);
          changed = true;
        }
      }
      TypeCase(phi, ir::insns::Phi *, inst.get()) {
        if (phi->array_ssa) {
          for (auto &use : phi->use()) {
            auto use_def = func.def_list.at(use);
            if (none_zero_inst.count(use_def)) {
              if(!none_zero_inst.count(phi)){
                none_zero_inst.insert(inst.get());
                changed = true;
              }
            }
            if (reg2base.count(use)) {
              reg2base[phi->dst] = reg2base.at(use);
            }
          }
        }
      }
      TypeCase(alloca, ir::insns::Alloca *, inst.get()) {
        none_zero_inst.insert(inst.get());
        reg2base[alloca->dst] = alloca->dst;
      }
    }
    if (changed || !bb->visit) {
      bb->visit = true;
      for (auto suc : bb->succ) {
        stack.push_back(suc);
      }
    }
  }
  for (auto &each : func.bbs) {
    for (auto iter = each->insns.begin(); iter != each->insns.end();) {
      if (none_zero_inst.count(iter->get())) {
        iter++;
        continue;
      }
      TypeCase(memdef, ir::insns::MemDef *, iter->get()) {
        copy_propagation(func.use_list, memdef->dst, reg2base.at(memdef->dst));
        iter->get()->remove_use_def();
        iter = each->insns.erase(iter);
        continue;
      }
      TypeCase(phi, ir::insns::Phi *, iter->get()) {
        if (phi->array_ssa) {
          copy_propagation(func.use_list, phi->dst, reg2base.at(phi->dst));
          phi->remove_use_def();
          iter = each->insns.erase(iter);
          continue;
        }
      }
      TypeCase(memuse, ir::insns::MemUse *, iter->get()) {
        auto def = func.def_list.at(memuse->dep);
        if (!none_zero_inst.count(def) && !memuse->call_use) {
          memuse->remove_use_def();
          if (memuse->dst.type == ScalarType::Int) {
            iter->reset(new ir::insns::LoadImm(memuse->dst, ConstValue(0)));
          } else {
            iter->reset(new ir::insns::LoadImm(memuse->dst, ConstValue(0.0f)));
          }
          iter->get()->bb = each.get();
          iter->get()->add_use_def();
        }
      }
      iter++;
    }
  }
}

} // namespace mediumend