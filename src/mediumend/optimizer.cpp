#include "mediumend/optimizer.hpp"

namespace mediumend {

// add pass here
const std::map<std::string, funcptr> PASS_MAP = {
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
  {"clean_hodgepodge", clean_hodgepodge},
  {"loop_fusion", loop_fusion},
  {"loop_unroll", loop_unroll},
  {"duplicate_load_store_elimination", duplicate_load_store_elimination},
  {"remove_zero_global_def", remove_zero_global_def},
  {"sort_basicblock", sort_basicblock},
  {"gep_destruction", gep_destruction},
};

// define default passes here
std::vector<funcptr> passes = {
  main_global_var_to_local,
  mem2reg,
  remove_unused_function,
  main_global_var_to_local,
  remove_uneffective_inst, // important! must done before mark_pure_func
  mark_pure_func,
  
  gvn_gcm,
  clean_hodgepodge,
  

  
  array_mem2reg,
  gvn_gcm,
  clean_hodgepodge,

  clean_useless_cf,
  remove_zero_global_def,
  loop_fusion,

  gvn_gcm,
  clean_hodgepodge,
  duplicate_load_store_elimination,
  array_ssa_destruction,

  loop_unroll,


  function_inline,

  array_mem2reg,
  gvn_gcm,
  clean_hodgepodge,
  clean_useless_cf,
  gvn_gcm,
  duplicate_load_store_elimination,
  array_ssa_destruction,


  gvn_gcm,
  clean_hodgepodge,
  constant_propagation,
  clean_useless_cf,

  remove_uneffective_inst,
  remove_useless_loop,
  clean_hodgepodge,
  constant_propagation,
  clean_useless_cf,

  main_global_var_to_local,
  mem2reg,

  gep_destruction,
  gvn_gcm,
  clean_hodgepodge,

  operator_strength_reduction,
  gvn_gcm,
  clean_hodgepodge,
  constant_propagation,
  clean_useless_cf,

  operator_strength_promotion,
  sort_basicblock,
};

void clean_hodgepodge(ir::Program *prog) {
  remove_uneffective_inst(prog);
  remove_unused_function(prog);
}

}
