#include "mediumend/optmizer.hpp"

namespace mediumend {

// add pass here
const std::map<std::string, std::function<void(ir::Program *)> > PASS_MAP = {
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
  {"operator_strength_promotion", operator_strength_promotion},
};

// define default passes here
std::vector<std::function<void(ir::Program *)> > passes = {
  main_global_var_to_local,
  mem2reg,
  remove_unused_function,
  main_global_var_to_local,
  // 纯函数可以用来做GVN和无用代码移除
  mark_pure_func,
  function_inline,
  gvn_gcm,
  // 原remove_uneffective_inst, simplification_phi, constant_propagation都被合并进了constant_propagation
  constant_propagation,
  // simplification_phi,
  // remove_uneffective_inst,
  // 移除无用指令后可能有的函数不会被调用，pure function / unreachable BB里的function
  remove_unused_function,
  clean_useless_cf,
  main_global_var_to_local,
  mem2reg,
  operator_strength_reduction,
  gvn_gcm,
  remove_uneffective_inst,
  constant_propagation,
  remove_unused_function,
  clean_useless_cf,

  operator_strength_promotion,
};

}