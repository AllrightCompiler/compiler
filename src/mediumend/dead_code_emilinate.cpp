#include "common/ir.hpp"

namespace mediumend {

using std::vector;
using std::unordered_map;
using std::unordered_set;


vector<ir::Reg> get_inst_use_reg(ir::Instruction *inst){
  vector<ir::Reg> regs;
  TypeCase(unary, ir::insns::Unary *, inst){
    regs.push_back(unary->src);
  }
  TypeCase(binary, ir::insns::Binary *, inst){
    regs.push_back(binary->src1);
    regs.push_back(binary->src2);
  }
  TypeCase(load, ir::insns::Load *, inst){
    regs.push_back(load->addr);
  }
  TypeCase(store, ir::insns::Store *, inst){
    regs.push_back(store->addr);
    regs.push_back(store->val);
  }
  TypeCase(call, ir::insns::Call *, inst){
    for(auto &r : call->args){
      regs.push_back(r);
    }
  }
  TypeCase(phi, ir::insns::Phi *, inst){
    for(auto &r : phi->incoming){
      regs.push_back(r.second);
    }
  }
  TypeCase(convey, ir::insns::Convert *, inst){
    regs.push_back(convey->src);
  }
  TypeCase(ret, ir::insns::Return *, inst){
    if(ret->val.has_value()){
      regs.push_back(ret->val.value());
    }
  }
  TypeCase(branch, ir::insns::Branch *, inst){
    regs.push_back(branch->val);
  }
  TypeCase(ptr, ir::insns::GetElementPtr *, inst){
    regs.push_back(ptr->base);
    for(auto &r : ptr->indices){
      regs.push_back(r);
    }
  }
  return regs;
}

void detect_pure_function(ir::Program *prog, ir::Function *func) {
  if(func->pure != -1){
    return;
  }
  for (auto &bb : func->bbs) {
    for (auto &inst : bb->insns) {
      if(auto x = dynamic_cast<ir::insns::Store *>(inst.get())){
        func->pure = 0;
        return;
      }
      TypeCase(call, ir::insns::Call *, inst.get()) {
        if(prog->functions[call->func].pure == -1){
          detect_pure_function(prog, &prog->functions[call->func]);
        }
        if(prog->functions[call->func].pure == 0){
          func->pure = 0;
          return;
        }
      }
    }
  }
  func->pure = 1;
}

void mark_pure_func(ir::Program *prog){
  prog->functions["main"].pure = 0;
  for(auto &func : prog->functions){
    detect_pure_function(prog, &func.second);
  }
}

void remove_unused_function(ir::Program *prog){
  std::unordered_set<std::string> used;
  std::vector<std::string> stack;
  stack.emplace_back("main");
  while(!stack.empty()){
    auto func_name = stack.back();
    stack.pop_back();
    used.insert(func_name);
    auto &func = prog->functions[func_name];
    for(auto &bb : func.bbs){
      for(auto &ins : bb->insns){
        if(auto call = dynamic_cast<ir::insns::Call *>(ins.get())){
          if(!used.count(call->func)){
            stack.emplace_back(call->func);
          }
        }
      }
    }
  }
  for(auto iter = prog->functions.begin(); iter != prog->functions.end();){
    if(!used.count(iter->first)){
      iter = prog->functions.erase(iter);
    }else{
      iter++;
    }
  }
}

void remove_uneffective_inst(ir::Program *prog){
  for(auto &each : prog->functions){
    ir::Function *func = &each.second;
    auto &bbs = func->bbs;
    vector<ir::Instruction *> stack;
    unordered_set<ir::Instruction *> remove_inst_set; 
    for (auto &bb : bbs) {
      auto &insns = bb->insns;
      for (auto &inst : insns) {
        TypeCase(output, ir::insns::Output *, inst.get()){
          if(auto call = dynamic_cast<ir::insns::Call *>(output)){
            if(prog->functions[call->func].pure == 0){
              continue;
            }
          }
          auto &reg = output->dst;
          if (reg.id && func->use_list[reg].size() == 0) {
            stack.push_back(output);
          }
        }
      }
    }
    while(!stack.empty()){
      auto inst = stack.back();
      stack.pop_back();
      auto regs = get_inst_use_reg(inst);
      inst->remove_use_def(func->use_list, func->def_list);
      for(auto reg : regs){
        if(func->use_list[reg].size() == 0){
          auto inst = func->def_list[reg];
          if(auto call = dynamic_cast<ir::insns::Call *>(inst)){
            continue;
          }
          stack.push_back(inst);
        }
      }
      remove_inst_set.insert(inst);
    }
    for(auto &bb : bbs){
      auto &insns = bb->insns;
      for(auto iter = insns.begin(); iter != insns.end();){
        if(remove_inst_set.find(iter->get()) != remove_inst_set.end()){
          iter = insns.erase(iter);
        }else{
          iter++;
        }
      }
    }
  }
}


}