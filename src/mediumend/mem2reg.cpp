#include "mediumend/mem2reg.hpp"
#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

namespace mediumend {

using std::vector;
using ir::Function;
using ir::BasicBlock;

void remove_unreachable_bb(Function *func, CFG *cfg){
  auto entry = func->bbs[0].get();
  vector<BasicBlock*> to_remove;
  for(auto & prev: cfg->pred){
    if(prev.second.size() == 0 && prev.first != entry){
      to_remove.push_back(prev.first);
    }
  }
  while(to_remove.size()){
    auto bb = to_remove.back();
    to_remove.pop_back();
    for(auto & succ: cfg->succ[bb]){
      cfg->pred[succ].erase(bb);
      if(cfg->pred[succ].size() == 0){
        to_remove.push_back(succ);
      }
    }
    for(auto iter = func->bbs.begin(); iter != func->bbs.end();++iter){
      if(iter->get() == bb){
        func->bbs.erase(iter);
        break;
      }
    }
  }
}

void mem2reg(Function *func){
  CFG cfg(func);
  remove_unreachable_bb(func, &cfg);
}

}