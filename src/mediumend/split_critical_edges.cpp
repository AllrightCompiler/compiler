#include "common/ir.hpp"
#include "mediumend/optmizer.hpp"

#include <cassert>

namespace mediumend {

using namespace ir;
using namespace ir::insns;

void split_critical_edges(Function &f) {
  std::unordered_map<BasicBlock *, std::list<Phi *>> phi_info;
  std::unordered_set<BasicBlock *>
      incoming_bbs; // 只含与phi函数有关，且有多(>1)条出边的基本块
  std::list<std::pair<BasicBlock *, BasicBlock *>> new_bbs; // (bb_in, bb_mid)

  for (auto &bb : f.bbs) {
    for (auto &insn : bb->insns) {
      TypeCase(phi, Phi *, insn.get()) {
        phi_info[bb.get()].emplace_back(phi);
        for (auto &[bb_in, r] : phi->incoming)
          if (bb_in->succ.size() > 1)
            incoming_bbs.insert(bb_in);
      }
    }
  }

  for (auto &bb_ptr : f.bbs) {
    auto bb = bb_ptr.get();
    auto preds = bb->prev;
    for (auto bb_in : preds) {
      if (!incoming_bbs.count(bb_in))
        continue;

      auto bb_mid = new BasicBlock;
      // TODO: 哪些字段继承自bb_in ?
      bb_mid->func = bb_in->func;
      bb_mid->loop = bb_in->loop;
      bb_mid->label = "BS" + std::to_string(new_bbs.size());
      new_bbs.push_back({bb_in, bb_mid});

      // 将CFG中 (bb_in -> bb) 更改为 (bb_in -> bb_mid) & (bb_mid -> bb)
      BasicBlock::remove_edge(bb_in, bb);
      BasicBlock::add_edge(bb_in, bb_mid);
      BasicBlock::add_edge(bb_mid, bb);

      // bb_in至少有2条出边，其中一定含有条件分支指令
      auto last_br = dynamic_cast<Branch *>(bb_in->insns.back().get());
      assert(last_br != nullptr);
      if (last_br->true_target == bb)
        last_br->true_target = bb_mid;
      else
        last_br->false_target = bb_mid;

      bb_mid->insns.emplace_back(new Jump{bb});

      if (!phi_info.count(bb))
        continue;
      for (auto phi : phi_info.at(bb)) {
        auto &incoming = phi->incoming;
        auto it = incoming.find(bb_in);
        if (it != incoming.end()) {
          Reg r = it->second;
          incoming.erase(it);
          incoming[bb_mid] = r;
        }
      }
    }
  }

  for (auto bb_iter = f.bbs.begin(); bb_iter != f.bbs.end();) {
    auto bb = bb_iter->get();
    ++bb_iter;

    for (auto it = new_bbs.begin(); it != new_bbs.end();) {
      if (it->first == bb) {
        f.bbs.emplace(bb_iter, it->second);
        it = new_bbs.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void split_critical_edges(Program *prog) {
  for (auto &[_, f] : prog->functions) {
    split_critical_edges(f);
  }
}

} // namespace mediumend
