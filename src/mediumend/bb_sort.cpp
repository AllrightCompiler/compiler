#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

#include <cassert>
#include <stack>

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using std::list;
using std::unique_ptr;

// void sort_basicblock(Function *func){
//   list<unique_ptr<BasicBlock>> new_bbs;
//   std::stack<BasicBlock *> bb_queue;
//   bb_queue.push(func->bbs.front().get());
//   func->clear_visit();
//   while(bb_queue.size()){
//     auto bb = bb_queue.top();
//     bb_queue.pop();
//     if(bb->visit) continue;
//     new_bbs.emplace_back(bb);
//     bb->visit = true;
//     for(auto suc : bb->succ){
//       bb_queue.push(suc);
//     }
//   }
//   for(auto &bb : func->bbs){
//     std::ignore = bb.release();
//   }
//   func->bbs = std::move(new_bbs);
// }

struct Chain {
  std::vector<BasicBlock *> bbs;
  int nr_edges;

  Chain() {}
  Chain(BasicBlock *bb) : bbs{bb} {}
};

int count_edge(const Chain &c1, const Chain &c2) {
  int n = 0;
  for (auto b1 : c1.bbs)
    for (auto b2 : c2.bbs)
      if (b1->succ.count(b2))
        ++n;
  return n;
}

// sort basic blocks according to Pettis-Hansen Heuristic
void sort_basicblock(Function *f) {
  std::vector<Chain> chains;
  auto entry = f->bbs.front().get();
  for (auto &bb_ptr : f->bbs)
    chains.emplace_back(bb_ptr.release());
  f->bbs.clear();

  auto &freqs = f->branch_freqs;
  bool changed = true;
  while (changed) {
    changed = false;
    std::vector<Chain>::iterator head, tail;
    double max_freq = 0;
    for (auto it1 = chains.begin(); it1 != chains.end(); ++it1) {
      for (auto it2 = chains.begin(); it2 != chains.end(); ++it2) {
        if (it1 == it2)
          continue;

        auto e = std::make_pair(it1->bbs.back(), it2->bbs.front());
        if (freqs.count(e)) {
          auto freq = freqs.at(e);
          if (max_freq < freq) {
            max_freq = freq;
            head = it1, tail = it2;
          }
        }
      }
    }
    if (max_freq > 0) {
      // merge basic blocks of `tail` into `head`
      for (auto bb : tail->bbs)
        head->bbs.push_back(bb);
      chains.erase(tail);
      changed = true;
    }
  }

  auto entry_chain =
      std::find_if(chains.begin(), chains.end(), [entry](const Chain &ch) {
        return ch.bbs.front() == entry;
      });
  for (auto bb : entry_chain->bbs)
    f->bbs.emplace_back(bb);
  chains.erase(entry_chain);

  for (auto it1 = chains.begin(); it1 != chains.end(); ++it1) {
    it1->nr_edges = 0;
    for (auto it2 = chains.begin(); it2 != chains.end(); ++it2)
      if (it1 != it2)
        it1->nr_edges += count_edge(*it1, *it2);
  }
  std::sort(chains.begin(), chains.end(), [](const Chain &c1, const Chain &c2) {
    return c1.nr_edges > c2.nr_edges;
  });
  for (auto &c : chains)
    for (auto bb : c.bbs)
      f->bbs.emplace_back(bb);
}

void sort_basicblock(ir::Program *prog) {
  for (auto &[_, f] : prog->functions) {
    sort_basicblock(&f);
  }
}

} // namespace mediumend
