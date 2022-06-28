#include "mediumend/mem2reg.hpp"
#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

namespace mediumend {

using std::vector;
using ir::Function;
using ir::BasicBlock;

void remove_unreachable_bb(Function *func, CFG *cfg){
  auto entry = func->bbs.front().get();
  std::unordered_set<BasicBlock*> remove_set;
  vector<BasicBlock*> to_remove;

  for(auto prev : cfg->prev){
    if(prev.second.size() == 0 && prev.first != entry){
      to_remove.push_back(prev.first);
    }
  }
  while(to_remove.size()){
    auto bb = to_remove.back();
    to_remove.pop_back();
    for(auto & succ: cfg->succ[bb]){
      cfg->prev[succ].erase(bb);
      if(cfg->prev[succ].size() == 0){
        to_remove.push_back(succ);
      }
    }
    remove_set.insert(bb);
  }
  for(auto iter = func->bbs.begin(); iter != func->bbs.end();){
    if(remove_set.find(iter->get()) != remove_set.end()){
      iter = func->bbs.erase(iter);
    }else{
      iter++;
    }
  }
}

void mem2reg(Function *func){
  CFG cfg(func);
  remove_unreachable_bb(func, &cfg);
}

}