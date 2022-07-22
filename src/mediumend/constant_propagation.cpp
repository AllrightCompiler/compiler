#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optmizer.hpp"

#include <cassert>

namespace mediumend {

using ir::Reg;

void checkbb(BasicBlock* bb){
  #ifdef FRW_DEBUG
  assert(bb->label[0]);
  int a = bb->visit;
  for(auto each : bb->succ){
    assert(each);
  }
  for(auto each : bb->prev){
    assert(each);
  }
  #endif
}

void constant_propagation(ir::Program *prog) {
  for (auto &each : prog->functions) {
    ir::Function *func = &each.second;
    ir::BasicBlock *entry = func->bbs.front().get();
    std::unordered_map<Reg, ConstValue> const_map;
    unordered_set<ir::BasicBlock *> stack;
    unordered_set<ir::Instruction *> remove_inst_list;
    func->clear_visit();
    for (auto &bb : func->bbs) {
      for (auto &ins : bb->insns) {
        TypeCase(loadimm, ir::insns::LoadImm *, ins.get()) {
          const_map[loadimm->dst] = loadimm->imm;
          stack.insert(bb.get());
          stack.insert(bb->succ.begin(), bb->succ.end());
        }
        TypeCase(phi, ir::insns::Phi *, ins.get()){
          stack.insert(bb.get());
        }
        TypeCase(output, ir::insns::Output *, ins.get()){
          if(func->use_list[output->dst].empty()){
            stack.insert(bb.get());
          }
        }
      }
    }
    while (stack.size()) {
      auto s_iter = stack.begin();
      auto bb = *s_iter;
      stack.erase(s_iter);
      if (bb->visit) {
        continue;
      }
      if (bb->prev.empty() && bb != entry) {
        for (auto &suc : bb->succ) {
          suc->prev.erase(bb);
        }
        stack.insert(bb->succ.begin(), bb->succ.end());
        for(auto each : bb->succ){
          checkbb(each);
        }
        bb->succ.clear();
        bb->visit = true;;
        for (auto &inst : bb->insns) {
          inst->remove_use_def();
        }
        continue;
      }
      bool add_res = false;
      for (auto &ins : bb->insns) {
        if (remove_inst_list.count(ins.get())) {
          continue;
        }
        TypeCase(output, ir::insns::Output *, ins.get()) {
          if (func->use_list[output->dst].empty()) {
            TypeCase(call, ir::insns::Call *, output) {
              auto func_iter = prog->functions.find(call->func);
              if (func_iter == prog->functions.end() ||
                  !func_iter->second.is_pure()) {
                continue;
              }
            } else TypeCase(memdef, ir::insns::MemDef *, ins.get()){
              continue;
            }
            auto reg_use = output->use();
            for (auto &use : reg_use) {
              if(!func->has_param(use)){
                auto def = func->def_list.at(use);
                stack.insert(def->bb);
                checkbb(def->bb);
              }
            }
            output->remove_use_def();
            remove_inst_list.insert(output);
            continue;
          }
          if (const_map.count(output->dst)) {
            continue;
          }
          TypeCase(unary, ir::insns::Unary *, ins.get()) {
            if (const_map.count(unary->src)) {
              Reg reg = unary->dst;
              for (auto &uses : func->use_list[unary->dst]) {
                stack.insert(uses->bb);
                checkbb(uses->bb);
              }
              if(!func->has_param(unary->src)){
                auto def = func->def_list.at(unary->src);
                stack.insert(def->bb);
                checkbb(def->bb);
              }
              unary->remove_use_def();
              ConstValue new_val =
                  const_compute(unary, const_map.at(unary->src));
              ins->remove_use_def();
              ins.reset(new ir::insns::LoadImm(reg, new_val));
              const_map[reg] = new_val;
              ins->bb = bb;
              ins->add_use_def();
              add_res = true;
            }
            continue;
          }
          TypeCase(binary, ir::insns::Binary *, ins.get()) {
            if (const_map.count(binary->src1) &&
                const_map.count(binary->src2)) {
              Reg reg = binary->dst;
              for (auto &uses : func->use_list[binary->dst]) {
                stack.insert(uses->bb);
                checkbb(uses->bb);
              }
              if(!func->has_param(binary->src1)){
                auto def = func->def_list.at(binary->src1);
                stack.insert(def->bb);
                checkbb(def->bb);
              }
              if(!func->has_param(binary->src2)){
                auto def = func->def_list.at(binary->src2);
                stack.insert(def->bb);
                checkbb(def->bb);
              }
              ConstValue new_val =
                  const_compute(binary, const_map.at(binary->src1),
                                const_map.at(binary->src2));
              auto new_ins = new ir::insns::LoadImm(reg, new_val);
              ins->remove_use_def();
              ins.reset(new_ins);
              const_map[reg] = new_val;
              ins->bb = bb;
              ins->add_use_def();
              add_res = true;
            }
            continue;
          }
          TypeCase(convey, ir::insns::Convert *, ins.get()) {
            if (const_map.find(convey->src) != const_map.end()) {
              Reg reg = convey->dst;
              for (auto &uses : func->use_list[convey->dst]) {
                stack.insert(uses->bb);
                checkbb(uses->bb);
              }
              if(!func->has_param(convey->src)){
                auto def = func->def_list.at(convey->src);
                stack.insert(def->bb);
                checkbb(def->bb);
              }
              ConstValue new_val =
                  const_compute(convey, const_map.at(convey->src));
              auto new_ins = new ir::insns::LoadImm(reg, new_val);
              ins->remove_use_def();
              ins.reset(new_ins);
              ins->bb = bb;
              ins->add_use_def();
              const_map[reg] = new_val;
              add_res = true;
            }
            continue;
          }
          TypeCase(phi, ir::insns::Phi *, ins.get()) {
            ConstValue val;
            bool set_val = false;
            bool phi_const = true;
            bool use_same_reg = true;
            bool set_reg = false;
            Reg use_reg;
            if(func->use_list[phi->dst].size() == 1){
              if(*func->use_list[phi->dst].begin() == phi){
                auto reg_use = phi->use();
                for (auto &use : reg_use) {
                  if(!func->has_param(use) && use != phi->dst){
                    auto def = func->def_list.at(use);
                    stack.insert(def->bb);
                    checkbb(def->bb);
                  }
                }
                phi->remove_use_def();
                remove_inst_list.insert(phi);
                continue;
              }
            }
            for (auto iter = phi->incoming.begin(); iter != phi->incoming.end();) {
              if (!bb->prev.count(iter->first)) {
                func->use_list.at(iter->second).erase(phi);
                if(!func->has_param(iter->second) && func->def_list.count(iter->second)){
                  auto def = func->def_list.at(iter->second);
                  stack.insert(def->bb);
                  checkbb(def->bb);
                }
                iter = phi->incoming.erase(iter);
              } else {
                if (!set_reg) {
                  use_reg = iter->second;
                  set_reg = true;
                } else {
                  if (use_reg != iter->second) {
                    use_same_reg = false;
                  }
                }
                if (phi_const) {
                  if (!set_val) {
                    if (!const_map.count(iter->second)) {
                      phi_const = false;
                    } else {
                      val = const_map.at(iter->second);
                      set_val = true;
                    }
                  } else {
                    if (!const_map.count(iter->second) ||
                        val != const_map.at(iter->second)) {
                      phi_const = false;
                    }
                  }
                }
                iter++;
              }
            }
            phi->add_use_def();
            if (phi_const) {
              Reg reg = phi->dst;
              for (auto &in : phi->incoming) {
                if(!func->has_param(in.second)){
                  auto def = func->def_list.at(in.second);
                  stack.insert(def->bb);
                  checkbb(def->bb);
                }
              }
              for (auto &uses : func->use_list[reg]) {
                stack.insert(uses->bb);
                checkbb(uses->bb);
              }
              ins->remove_use_def();
              ins.reset(new ir::insns::LoadImm(phi->dst, val));
              ins->bb = bb;
              ins->add_use_def();
              add_res = true;
              const_map[reg] = val;
            } else {
              if (use_same_reg) {
                copy_propagation(func->use_list, phi->dst, use_reg);
                stack.insert(func->def_list.at(use_reg)->bb);
                checkbb(func->def_list.at(use_reg)->bb);
                phi->remove_use_def();
                remove_inst_list.insert(phi);
              }
            }
          }
          continue;
        }
        TypeCase(br, ir::insns::Branch *, ins.get()) {
          if (const_map.find(br->val) != const_map.end()) {
            if(!func->has_param(br->val)) {
              auto def = func->def_list.at(br->val);
              stack.insert(def->bb);
              checkbb(def->bb);
            }
            ir::BasicBlock *target;
            if (const_map.at(br->val).iv) {
              target = br->true_target;
              bb->succ.erase(br->false_target);
              br->false_target->prev.erase(bb);
              stack.insert(br->false_target);
              checkbb(br->false_target);
            } else {
              target = br->false_target;
              bb->succ.erase(br->true_target);
              br->true_target->prev.erase(bb);
              stack.insert(br->true_target);
              checkbb(br->true_target);
            }
            auto new_ins = new ir::insns::Jump(target);
            ins->remove_use_def();
            ins.reset(new_ins);
            ins->bb = bb;
            ins->add_use_def();
          }
          continue;
        }
      }
      if (add_res) {
        stack.insert(bb->succ.begin(), bb->succ.end());
        for(auto each : bb->succ){
          checkbb(each);
        }
      }
    }
    for (auto iter = func->bbs.begin(); iter != func->bbs.end();) {
      if (iter->get()->visit) {
        iter = func->bbs.erase(iter);
      } else {
        for (auto inst_iter = iter->get()->insns.begin();
             inst_iter != iter->get()->insns.end();) {
          if (remove_inst_list.count(inst_iter->get())) {
            inst_iter = iter->get()->insns.erase(inst_iter);
          } else {
            inst_iter++;
          }
        }
        iter++;
      }
    }
    remove_unused_phi(func);
    func->cfg->remove_unreachable_bb();
  }
}

} // namespace mediumend
