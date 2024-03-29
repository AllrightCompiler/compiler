#pragma once

#include "common/ir.hpp"
#include "common/utils.hpp"
#include "mediumend/cfg.hpp"
#include <iostream>
#include <fstream>

namespace mediumend {

using Pass = void (*)(ir::Program *);

// IMPORTANT: if add new pass, modify PASS_MAP in optimizer.cpp
extern const std::map<std::string, Pass> PASS_MAP;
// define default passes in optimizer.cpp
extern std::vector<Pass> passes;

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
void gvn_cfg(ir::Program *prog);
void gvn_no_cfg(ir::Program *prog);
void loop_fusion(ir::Program *prog);
void loop_unroll(ir::Program *prog);
void duplicate_load_store_elimination(ir::Program *prog);
void remove_zero_global_def(ir::Program *prog);
void sort_basicblock(ir::Program *prog);
void gep_destruction(ir::Program *prog);
void remove_recursive_tail_call(ir::Program *prog);
void loop_parallel(ir::Program *prog);
void value_range_analysis(ir::Program *prog);
void br2switch(ir::Program *prog);
void loop_interchange(ir::Program *prog);
void algebra_simpilifacation(ir::Program *prog);
void estimate_exec_freq(ir::Program *prog);
void loop_compute(ir::Program *prog);
void if_combine(ir::Program *prog);
void hoist_load_store(ir::Program *prog);
void recursive_memory(ir::Program *prog);

void copy_propagation(unordered_map<ir::Reg, std::unordered_set<ir::Instruction *> > &use_list, ir::Reg dst, ir::Reg src);
ConstValue const_compute(ir::Instruction *inst, const ConstValue &oprand);
ConstValue const_compute(ir::Instruction *inst, const ConstValue &op1, const ConstValue &op2);
void ir_validation(ir::Program *prog);
bool in_array_ssa();

inline void run_medium(ir::Program *prog, bool disable_gep_des) {
  for (auto &[_, f] : prog->functions){
    auto cfg = new CFG{&f};
    cfg->build();
    cfg->remove_unreachable_bb();
    f.cfg = std::unique_ptr<CFG>(cfg);
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

  for (int i = 0; i < passes.size(); i++) {
    if (disable_gep_des && passes[i] == loop_parallel) continue;
    passes[i](prog);
  }
}

} // namespace mediumend
