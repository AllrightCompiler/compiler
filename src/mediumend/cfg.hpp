#pragma once

#include "common/ir.hpp"

namespace mediumend {

using ir::BasicBlock;
using std::unordered_map;
using std::unordered_set;
using std::vector;

void compute_use_def_list(ir::Program *);

void mark_global_addr_reg(ir::Program *);

class CFG {
  ir::Function *func;

public:
  CFG() {}
  CFG(ir::Function *func);
  // edge in cfg
  // unordered_map<BasicBlock *, unordered_set<BasicBlock *>> rdom, rdomby;
  // unordered_map<BasicBlock *, BasicBlock *> ridom;
  vector<BasicBlock *> rpo; // Reverse PostOrder

  void build();

  void remove_unreachable_bb();

  void compute_dom();

  void compute_rdom();

  void compute_rpo();

  unordered_map<BasicBlock *, unordered_set<BasicBlock *>> compute_df();

  unordered_map<BasicBlock *, unordered_set<BasicBlock *>> compute_rdf();

private:
  void compute_dom_level(BasicBlock *bb, int dom_level);
};

} // namespace mediumend
