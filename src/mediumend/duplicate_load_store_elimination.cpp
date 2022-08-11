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
            } else {
              if(def->bb == memdef->bb && memdef->store_val == def->store_val){
                if(func->def_list.count(memdef->store_val)){
                  auto load = dynamic_cast<ir::insns::MemUse *>(func->def_list.at(memdef->store_val));
                  if(load && def->uses_before_def.count(load->dst)){
                    if(load->load_src == memdef->store_dst){
                      copy_propagation(func->use_list, memdef->dst, def->dst);
                      memdef->remove_use_def();
                      removable_inst.insert(memdef);
                    }
                  }
                }
              }
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
  unordered_set<Instruction *> stack;
  func.clear_visit();
  unordered_map<Reg, Reg> reg2base;
  unordered_map<std::string, Reg> name2reg;
  vector<BasicBlock *> dom_stack;
  func.cfg->compute_dom();
  dom_stack.push_back(func.bbs.front().get());
  while(dom_stack.size()) {
    auto bb = dom_stack.back();
    dom_stack.pop_back();
    for(auto &ins : bb->insns){
      auto inst = ins.get();
      TypeCase(loadaddr, ir::insns::LoadAddr *, inst) {
        if (prog->global_vars.at(loadaddr->var_name)->arr_val.get() &&
            prog->global_vars.at(loadaddr->var_name)->arr_val.get()->size() &&
            !none_zero_inst.count(loadaddr)) {
          stack.insert(loadaddr);
        }
        if(prog->global_vars.at(loadaddr->var_name)->val.has_value()){
          stack.insert(loadaddr);
        }
        if(!name2reg.count(loadaddr->var_name)){
          reg2base[loadaddr->dst] = loadaddr->dst;
          name2reg[loadaddr->var_name] = loadaddr->dst;
        }
        reg2base[loadaddr->dst] = name2reg.at(loadaddr->var_name);
      }
      TypeCase(memdef, ir::insns::MemDef *, inst) {
        auto def_val = func.def_list.at(memdef->store_val);
        TypeCase(loadimm, ir::insns::LoadImm *, def_val) {
          if (loadimm->imm != 0) {
            stack.insert(memdef);
          }
        }
        else {
          stack.insert(inst);
        }
        reg2base[memdef->dst] = reg2base.at(memdef->dep);
      }
      TypeCase(alloca, ir::insns::Alloca *, inst) {
        stack.insert(inst);
        reg2base[alloca->dst] = alloca->dst;
      }
      TypeCase(phi, ir::insns::Phi *, inst){
        if(phi->array_ssa){
          for(auto each : phi->incoming){
            if(reg2base.count(each.second)){
              reg2base[phi->dst] = reg2base.at(each.second);
              break;
            }
          }
        }
      }
    }
    for(auto each : bb->dom){
      dom_stack.push_back(each);
    }
  }
  while (stack.size()) {
    bool changed = false;
    auto iter = stack.begin();
    auto inst = *iter;
    stack.erase(iter);
    if (none_zero_inst.count(inst)) {
      continue;
    }
    none_zero_inst.insert(inst);
    TypeCase(output, ir::insns::Output *, inst){
      auto uses = func.use_list[output->dst];
      stack.merge(uses);
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