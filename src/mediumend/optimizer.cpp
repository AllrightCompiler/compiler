#include "mediumend/optimizer.hpp"

namespace mediumend {

// add pass here
const std::map<std::string, Pass> PASS_MAP = {
  {"remove_unused_function", remove_unused_function},
  {"mem2reg", mem2reg},
  {"constant_propagation", constant_propagation},
  {"mark_pure_func", mark_pure_func},
  {"remove_uneffective_inst", remove_uneffective_inst},
  {"clean_useless_cf", clean_useless_cf},
  {"main_global_var_to_local", main_global_var_to_local},
  {"gvn_gcm", gvn_gcm},
  {"function_inline", function_inline},
  {"operator_strength_reduction", operator_strength_reduction},
  {"split_critical_edges", split_critical_edges},
  {"operator_strength_promotion", operator_strength_promotion},
  {"array_mem2reg", array_mem2reg},  // 在array ssa之前必须进行一次gvn_gcm
  {"array_ssa_destruction", array_ssa_destruction},
  {"remove_useless_loop", remove_useless_loop},
  {"gvn_no_cfg", gvn_no_cfg},
  {"gvn_cfg", gvn_cfg},
  {"loop_fusion", loop_fusion},
  {"loop_unroll", loop_unroll},
  {"duplicate_load_store_elimination", duplicate_load_store_elimination},
  {"remove_zero_global_def", remove_zero_global_def},
  {"sort_basicblock", sort_basicblock},
  {"gep_destruction", gep_destruction},
  {"remove_recursive_tail_call", remove_recursive_tail_call},
  {"loop_parallel", loop_parallel},
  {"value_range_analysis", value_range_analysis},
  {"br2switch", br2switch},
  {"loop_interchange", loop_interchange},
  {"algebra_simpilifacation", algebra_simpilifacation},
  {"estimate_exec_freq", estimate_exec_freq},
  {"if_combine", if_combine},
  {"loop_compute", loop_compute},
  {"recursive_memory", recursive_memory},
};

// define default passes here
std::vector<Pass> passes = {
  clean_useless_cf,
  main_global_var_to_local,
  mem2reg,
  mark_pure_func,
  remove_uneffective_inst,
  remove_unused_function,
  main_global_var_to_local,
  remove_uneffective_inst, // important! must done before mark_pure_func
  mark_pure_func,
  remove_uneffective_inst,
  remove_unused_function,
  
  gvn_no_cfg,
  recursive_memory,
  array_mem2reg,
    gvn_no_cfg,
    clean_useless_cf,
    loop_fusion,
    gvn_no_cfg,
    duplicate_load_store_elimination,
    if_combine,
  array_ssa_destruction,

  loop_interchange,

  remove_recursive_tail_call,

  gvn_cfg,

  loop_parallel,

  loop_compute,

  function_inline,
  remove_unused_function,
  main_global_var_to_local,
  mem2reg,

  loop_unroll,

  array_mem2reg,
    gvn_cfg,
    duplicate_load_store_elimination,
  array_ssa_destruction,

  value_range_analysis,
  
  algebra_simpilifacation,
  
  gep_destruction,

  gvn_cfg,
  gvn_cfg,
  loop_unroll,

  gvn_cfg,
  remove_useless_loop,
  gvn_cfg,


  remove_useless_loop,
  array_mem2reg,
    gvn_cfg,
    duplicate_load_store_elimination,
  array_ssa_destruction,

  // operator_strength_reduction,
  gvn_no_cfg,

  gvn_cfg,

  estimate_exec_freq,
  sort_basicblock,
};

// without modify cfg
void gvn_no_cfg(ir::Program *prog) {
  remove_uneffective_inst(prog);
  gvn_gcm(prog);
  remove_uneffective_inst(prog);
  algebra_simpilifacation(prog);
  remove_unused_function(prog);
  operator_strength_promotion(prog);
}

// modify cfg
void gvn_cfg(ir::Program *prog) {
  remove_uneffective_inst(prog);
  gvn_gcm(prog);
  remove_uneffective_inst(prog);
  algebra_simpilifacation(prog);
  remove_unused_function(prog);
  operator_strength_promotion(prog);
  constant_propagation(prog);
  clean_useless_cf(prog);
}

void ir_validation(ir::Program *prog) {
  auto validate = [=](bool v) {
    if (!v) std::cerr << (*prog) << std::endl;
    assert(v);
  };
  for (auto &[_, func] : prog->functions) {
    for (auto &bb : func.bbs) {
      validate(bb->func != nullptr && bb->func == &func);
      for (auto &insn : bb->insns) {
        validate(insn->bb != nullptr && insn->bb == bb.get());
      }
      for (auto succ : bb->succ) {
        validate(succ->prev.count(bb.get()));
      }
      for (auto prev : bb->prev) {
        validate(prev->succ.count(bb.get()));
      }
      for (auto &insn : bb->insns) {
        TypeCase(phi, ir::insns::Phi *, insn.get()) {
          validate(phi->incoming.size() == bb->prev.size());
          for (auto pair : phi->incoming) {
            validate(bb->prev.count(pair.first));
          }
        } else break;
      }
    }
  }
}

}
