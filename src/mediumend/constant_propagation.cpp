#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optmizer.hpp"

#include <cassert>

namespace mediumend {

using ir::Reg;

void constant_propagation(ir::Program *prog) {
  for(auto &each : prog->functions){
    ir::Function *func = &each.second;
    ir::BasicBlock *entry = func->bbs.front().get();
    std::unordered_map<Reg, ConstValue> const_map;
    unordered_set<ir::BasicBlock *> stack;
    unordered_set<ir::BasicBlock *> remove_list;
    unordered_set<ir::Instruction *> remove_inst_list;
    for (auto &bb : func->bbs) {
      for (auto &ins : bb->insns) {
        TypeCase(loadimm, ir::insns::LoadImm *, ins.get()) {
          const_map[loadimm->dst] = loadimm->imm;
          stack.insert(bb.get());
          stack.insert(bb->succ.begin(), bb->succ.end());
        }
      }
    }
    while(stack.size()) {
      auto s_iter = stack.begin();
      auto bb = *s_iter;
      stack.erase(s_iter);
      if(remove_list.count(bb)){
        continue;
      }
      if(bb->prev.empty() && bb != entry){
        for(auto &suc : bb->succ){
          suc->prev.erase(bb);
        }
        stack.insert(bb->succ.begin(), bb->succ.end());
        bb->succ.clear();
        remove_list.insert(bb);
        for(auto &inst : bb->insns){
          inst->remove_use_def();
        }
        continue;
      }
      bool add_res = false;
      for (auto &ins : bb->insns) {
        if(remove_inst_list.count(ins.get())){
          continue;
        }
        TypeCase(output, ir::insns::Output *, ins.get()){
          if(func->use_list[output->dst].empty()){
            TypeCase(call, ir::insns::Call *, output){
              auto func_iter = prog->functions.find(call->func);
              if(func_iter == prog->functions.end() || !func_iter->second.pure){
                continue;
              }
            }
            auto reg_use = get_inst_use_reg(output);
            for(auto &use : reg_use){
              auto it = func->def_list.find(use);
              if(it != func->def_list.end()){
                stack.insert(it->second->bb);
              }
            }
            output->remove_use_def();
            remove_inst_list.insert(output);
            continue;
          }
          if(const_map.count(output->dst)){
            continue;
          }
          TypeCase(unary, ir::insns::Unary *, ins.get()) {
            if (const_map.count(unary->src)) {
              Reg reg = unary->dst;
              for(auto &uses : func->use_list[unary->src]){
                stack.insert(uses->bb);
              }
              unary->remove_use_def();
              ConstValue new_val = const_compute(unary, const_map[unary->src]);
              ins.reset(new ir::insns::LoadImm(reg, new_val));
              const_map[reg] = new_val;
              ins->bb = bb;
              ins->add_use_def();
              add_res = true;
            }
            continue;
          }
          TypeCase(binary, ir::insns::Binary *, ins.get()) {
            if (const_map.count(binary->src1) && const_map.count(binary->src2)) {
              Reg reg = binary->dst;
              for(auto &uses : func->use_list[binary->src1]){
                stack.insert(uses->bb);
              }
              for(auto &uses : func->use_list[binary->src2]){
                stack.insert(uses->bb);
              }
              binary->remove_use_def();
              ConstValue new_val = const_compute(binary, const_map[binary->src1], const_map[binary->src2]);
              auto new_ins = new ir::insns::LoadImm(reg, new_val);
              ins.reset(new_ins);
              const_map[reg] = new_val;
              ins->bb = bb;
              ins->add_use_def();
              add_res = true;
            }
            continue;
          }
          TypeCase(convey, ir::insns::Convert *, ins.get()){
            if(const_map.find(convey->src) != const_map.end()){
              Reg reg = convey->dst;
              for(auto &uses : func->use_list[convey->src]){
                stack.insert(uses->bb);
              }
              convey->remove_use_def();
              ConstValue new_val = const_compute(convey, const_map[convey->src]);
              auto new_ins = new ir::insns::LoadImm(reg, new_val);
              ins.reset(new_ins);
              ins->bb = bb;
              ins->add_use_def();
              const_map[reg] = new_val;
              add_res = true;
            }
            continue;
          }
          TypeCase(phi, ir::insns::Phi *, ins.get()){
            ConstValue val;
            bool set_val = false;
            bool phi_const = true;
            for(auto iter = phi->incoming.begin(); iter != phi->incoming.end();){
              if(!bb->prev.count(iter->first)){
                iter = phi->incoming.erase(iter);
              } else {
                if(!set_val){
                  set_val = true;
                  val = const_map[iter->second];
                } else {
                  if(!(val == const_map[iter->second])){
                    phi_const = false;
                  }
                }
                iter++;
              }
            }
            if(phi_const){
              Reg reg = phi->dst;
              for(auto &in : phi->incoming){
                for(auto &uses : func->use_list[in.second]){
                  stack.insert(uses->bb);
                }
              }
              ins->remove_use_def();
              ins.reset(new ir::insns::LoadImm(phi->dst, val));
              ins->bb = bb;
              ins->add_use_def();
              add_res = true;
              const_map[reg] = val;
            }
          }
        }
        TypeCase(br, ir::insns::Branch *, ins.get()) {
          if (const_map.find(br->val) != const_map.end()) {
            for(auto &uses : func->use_list[br->val]){
              stack.insert(uses->bb);
            }
            br->remove_use_def();
            ir::BasicBlock *target;
            if(const_map[br->val].iv) {
              target = br->true_target;
              bb->succ.erase(br->false_target);
              br->false_target->prev.erase(br->false_target);
              stack.insert(br->false_target);
            } else {
              target = br->false_target;
              bb->succ.erase(br->true_target);
              br->true_target->prev.erase(bb);
              stack.insert(br->true_target);
            }
            auto new_ins = new ir::insns::Jump(target);
            ins.reset(new_ins);
            ins->bb = bb;
            ins->add_use_def();
          }
          continue;
        }
      }
      if(add_res){
        stack.insert(bb->succ.begin(), bb->succ.end());
      }
    }
    for(auto iter = func->bbs.begin(); iter != func->bbs.end();){
      if(remove_list.count(iter->get())){
        iter = func->bbs.erase(iter);
      } else {
        for(auto inst_iter = iter->get()->insns.begin(); inst_iter != iter->get()->insns.end();){
          if(remove_inst_list.count(inst_iter->get())){
            inst_iter = iter->get()->insns.erase(inst_iter);
          } else {
            inst_iter++;
          }
        }
        iter++;
      }
    }
  }
}

} // namespace mediumend
