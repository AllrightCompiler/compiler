#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

#include <cassert>
#include <queue>

namespace mediumend {

using ir::Function;
using ir::Reg;
using std::list;
using std::unique_ptr;

void sort_basicblock(Function *func){
  list<unique_ptr<BasicBlock>> new_bbs;
  std::queue<BasicBlock *> bb_queue;
  bb_queue.push(func->bbs.front().get());
  func->clear_visit();
  while(bb_queue.size()){
    auto bb = bb_queue.front();
    bb_queue.pop();
    if(bb->visit) continue;
    new_bbs.emplace_back(bb);
    bb->visit = true;
    for(auto suc : bb->succ){
      bb_queue.push(suc);
    }
  }
  for(auto &bb : func->bbs){
    std::ignore = bb.release();
  }
  func->bbs = std::move(new_bbs);
}

void sort_basicblock(ir::Program *prog){
  for(auto &[name, func] : prog->functions){
    sort_basicblock(&func);
  }
}

}