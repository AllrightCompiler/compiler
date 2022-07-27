#pragma once

#include "common/ir.hpp"
#include "common/utils.hpp"
#include "mediumend/cfg.hpp"
#include <iostream>
#include <fstream>

namespace mediumend {

void remove_unused_function(ir::Program *prog);
void mem2reg(ir::Program *prog);
void constant_propagation(ir::Program *prog);
void mark_pure_func(ir::Program *);
void remove_uneffective_inst(ir::Program *prog);
void clean_useless_cf(ir::Program *prog);
void main_global_var_to_local(ir::Program *prog);
void gvn_gcm(ir::Program *prog);
void function_inline(ir::Program *prog);
void remove_unused_phi(ir::Function *func);
void operator_strength_reduction(ir::Program *prog);
void split_critical_edges(ir::Program *prog);
void operator_strength_promotion(ir::Program *prog);
void array_mem2reg(ir::Program *prog);
void array_ssa_destruction(ir::Program *prog);
void remove_useless_loop(ir::Program * prog);
void clean_hodgepodge(ir::Program *prog);

// IMPORTANT: if add new pass, modify PASS_MAP in optimizer.cpp
extern const std::map<std::string, std::function<void(ir::Program *)> > PASS_MAP;
// define default passes in optimizer.cpp
extern std::vector<std::function<void(ir::Program *)> > passes;

ConstValue const_compute(ir::Instruction *inst, const ConstValue &oprand);
ConstValue const_compute(ir::Instruction *inst, const ConstValue &op1, const ConstValue &op2);
void copy_propagation(unordered_map<ir::Reg, std::unordered_set<ir::Instruction *> > &use_list, ir::Reg dst, ir::Reg src);

inline void run_medium(ir::Program *prog) {
  for (auto &func : prog->functions){
    func.second.cfg = new CFG(&func.second);
    func.second.cfg->build();
    func.second.cfg->remove_unreachable_bb();
  }
  compute_use_def_list(prog);

  std::ifstream pass_config("pass_config.txt");
  if (pass_config.is_open()) {
    // warn(std::cerr) << "use user pass config!" << std::endl;
    passes.clear();
    std::string line;
    while (std::getline(pass_config, line)) {
      if (!line.length()) continue; // empty line
      assert(PASS_MAP.count(line));
      passes.push_back(PASS_MAP.at(line));
    }
  }

  for (auto pass : passes) {
    pass(prog);
  }
}

} // namespace mediumend
