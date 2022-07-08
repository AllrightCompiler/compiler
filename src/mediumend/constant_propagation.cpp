#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optmizer.hpp"

#include <cassert>

namespace mediumend {

using ir::Reg;

void constant_propagation(ir::Program *prog) {
  for(auto &each : prog->functions){
    ir::Function *func = &each.second;
    std::unordered_map<Reg, ConstValue> const_map;
    for (auto &bb : func->bbs) {
      for (auto &ins : bb->insns) {
        TypeCase(loadimm, ir::insns::LoadImm *, ins.get()) {
          const_map[loadimm->dst] = loadimm->imm;
        }
      }
    }
    unordered_set<ir::BasicBlock *> stack;
    for (auto &bb : func->bbs) {
      stack.insert(bb.get());
    }
    while(stack.size()) {
      auto s_iter = stack.begin();
      auto bb = *s_iter;
      stack.erase(s_iter);
      bool add_res = false;
      for (auto &ins : bb->insns) {
        TypeCase(unary, ir::insns::Unary *, ins.get()) {
          if (const_map.find(unary->src) != const_map.end()) {
            Reg reg = unary->dst;
            unary->remove_use_def();
            ConstValue new_val = const_compute(unary, const_map[unary->src]);
            ins.reset(new ir::insns::LoadImm(reg, new_val));
            ins->add_use_def();
            add_res = true;
          }
          continue;
        }
        TypeCase(binary, ir::insns::Binary *, ins.get()) {
          if (const_map.find(binary->src1) != const_map.end() && const_map.find(binary->src2) != const_map.end()) {
            Reg reg = binary->dst;
            binary->remove_use_def();
            ConstValue new_val = const_compute(binary, const_map[binary->src1], const_map[binary->src2]);
            auto new_ins = new ir::insns::LoadImm(reg, new_val);
            ins.reset(new_ins);
            ins->add_use_def();
            const_map[new_ins->dst] = new_ins->imm;
            add_res = true;
          }
          continue;
        }
        TypeCase(convey, ir::insns::Convert *, ins.get()){
          if(const_map.find(convey->src) != const_map.end()){
            Reg reg = convey->dst;
            convey->remove_use_def();
            ConstValue new_val = const_compute(convey, const_map[convey->src]);
            auto new_ins = new ir::insns::LoadImm(reg, new_val);
            ins.reset(new_ins);
            ins->add_use_def();
            const_map[new_ins->dst] = new_ins->imm;
            add_res = true;
          }
          continue;
        }
        TypeCase(br, ir::insns::Branch *, ins.get()) {
          if (const_map.find(br->val) != const_map.end()) {
            br->remove_use_def();
            ir::BasicBlock *target;
            if(const_map[br->val].iv) {
              target = br->true_target;
            } else {
              target = br->false_target;
            }
            auto new_ins = new ir::insns::Jump(target);
            ins.reset(new_ins);
            add_res = true;
          }
          continue;
        }
      }
      if(add_res){
        for(auto suc : bb->succ){
          stack.insert(suc);
        }
      }
    }
    func->cfg->build();
    func->cfg->remove_unreachable_bb();
    for(auto &bb : func->bbs){
      for(auto &inst : bb->insns){
        if(auto phi = dynamic_cast<ir::insns::Phi *>(inst.get())){
          for(auto iter = phi->incoming.begin(); iter != phi->incoming.end();){
            if(!bb->prev.count(iter->first)){
              iter = phi->incoming.erase(iter);
            } else iter++;
          }
        }
      }
    }
  }
}

} // namespace mediumend
