#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::Function;
using ir::Loop;
using ir::Instruction;

bool hoist_load_store(Function *func){
  func->loop_analysis();
  bool ret = false;
  unordered_map<Loop *, unordered_set<BasicBlock *>> loop_bbs;
  unordered_map<Loop *, BasicBlock *> loop_out;
  for (auto &loop : func->loops) {
    vector<BasicBlock *> stack;
    stack.push_back(loop->header);
    while (stack.size()) {
      auto bb = stack.back();
      stack.pop_back();
      if (loop_bbs[loop.get()].count(bb)) {
        continue;
      }
      loop_bbs[loop.get()].insert(bb);
      for (auto &succ : bb->succ) {
        if (succ->loop == loop.get()) {
          stack.push_back(succ);
        }
      }
    }
  }
  for (auto &loop : loop_bbs) {
    BasicBlock * out = nullptr;
    bool check = true;
    for(auto bb : loop.second){
      for(auto succ : bb->succ){
        if(!loop.second.count(succ)){
          if(succ->loop && succ->loop->outer == loop.first){
            continue;
          }
          if(out && out != succ){
            check = false;
          } else {
            out = succ;
          }
        }
      }
    }
    if(check){
      loop_out[loop.first] = out;
    }
  }
  unordered_set<Instruction *> movable;
  vector<Instruction *> insts;
  for (auto &loop : loop_bbs) {
    auto head = loop.first->header;
    if(!loop_out.count(loop.first)){
      continue;
    }
    auto out = loop_out[loop.first];
    for (auto &bb : loop.second) {
      for(auto &inst : bb->insns){
        TypeCase(store, ir::insns::Store *, inst.get()){
          auto addr = store->addr;
          bool move_able = true;
          if(!func->has_param(addr) && loop.second.count(func->def_list.at(addr)->bb)){
            move_able = false;
          }
          if(move_able){
            movable.insert(store);
            insts.push_back(store);
          }
        }
      }
    }
    if(movable.size()){
      ret = true;
      BasicBlock *new_bb = new BasicBlock;
      new_bb->label = head->label + "_out";
      new_bb->func = func;
      for (auto &bb : loop.second) {
        if(bb->succ.count(out)) {
          bb->change_succ(out, new_bb);
          out->prev.insert(new_bb);
        }
        for(auto iter = bb->insns.begin(); iter != bb->insns.end();){
          if(movable.count(iter->get())){
            iter->release();
            iter = bb->insns.erase(iter);
          } else {
            iter++;
          }
        }
      }
      for(auto inst : insts){
        new_bb->push_back(inst);
      }
      new_bb->push_back(new ir::insns::Jump(out));
      func->bbs.emplace_back(new_bb);
    }
  }
  return ret;
}

void hoist_load_store(ir::Program *prog){
  for(auto &func : prog->functions){
    while(hoist_load_store(&func.second)) { }
  }
}


}