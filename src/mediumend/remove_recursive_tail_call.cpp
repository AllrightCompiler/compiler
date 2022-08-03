#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Reg;

using std::unordered_set;

void remove_recursive_tail_call(Function *func) {
  unordered_set<BasicBlock *> ret_bbs;
  for (auto &bb : func->bbs) {
    for (auto iter = bb->insns.begin(); iter != bb->insns.end(); iter++) {
      TypeCase(call, ir::insns::Call *, iter->get()) {
        if (call->func == func->name) {
          auto next = std::next(iter);
          TypeCase(ret, ir::insns::Return *, next->get()) {
            auto dst = call->dst;
            if(ret->val.has_value()){
              if(dst != ret->val.value()){
                return;
              }
            }
            for(int i = 0; i < func->sig.param_types.size(); i++){
              auto &each = func->sig.param_types[i];
              if(each.is_array()){
                if(call->args[i].id != i + 1){
                  return;
                }
              }
            }
            ret_bbs.insert(bb.get());
          } else {
            return;
          }
        }
      }
    }
  }

  if(ret_bbs.empty()){
    return;
  }

  auto old_entry = func->bbs.front().get();
  auto new_bb = new BasicBlock();
  func->bbs.emplace_front(new_bb);
  
  new_bb->label = "B0";
  new_bb->func = func;
  new_bb->succ.insert(old_entry);
  new_bb->push_front(new ir::insns::Jump(old_entry));
  old_entry->prev.insert(new_bb);
  old_entry->label = "CUR_ENTRY";
  
  vector<ir::insns::Phi *> params_phi;
  for(int i = 0; i < func->sig.param_types.size(); i++){
    auto &param = func->sig.param_types[i];
    if(param.is_array()){
      continue;
    }
    auto reg = func->new_reg(param.base_type);
    auto old_reg = Reg(param.base_type, i + 1);
    auto phi = new ir::insns::Phi(reg);
    params_phi.push_back(phi);
    old_entry->push_front(phi);
    copy_propagation(func->use_list, old_reg, reg);
    phi->incoming[new_bb] = old_reg;
  }
  for(auto bb : ret_bbs){
    bb->pop_back();
    auto call = dynamic_cast<ir::insns::Call *>(bb->insns.back().get());
    assert(call);
    int offset = 0;
    for(int i = 0; i < call->args.size(); i++){
      if(func->sig.param_types[i].is_array()){
        offset++;
        continue;
      }
      auto &param = call->args[i];
      params_phi[i - offset]->incoming[bb] = param;
    }
    bb->pop_back();
    bb->push_back(new ir::insns::Jump(old_entry));
    bb->succ.insert(old_entry);
    old_entry->prev.insert(bb);
  }
  for(auto phi : params_phi){
    phi->add_use_def();
  }
}

void remove_recursive_tail_call(ir::Program *prog) {
  for(auto &[name, func] : prog->functions){
    remove_recursive_tail_call(&func);
  }
}

} // namespace mediumend