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
  {"gvn_no_cfg", gvn_no_cfg},
  {"gvn_cfg", gvn_cfg},
  {"loop_fusion", loop_fusion},
  {"loop_unroll", loop_unroll},
  {"duplicate_load_store_elimination", duplicate_load_store_elimination},
  {"remove_zero_global_def", remove_zero_global_def},
  {"sort_basicblock", sort_basicblock},
  {"gep_destruction", gep_destruction},
  {"remove_recursive_tail_call", remove_recursive_tail_call},
};

// define default passes here
std::vector<funcptr> passes = {
  main_global_var_to_local,
  mem2reg,
  remove_uneffective_inst,
  remove_unused_function,
  main_global_var_to_local,
  remove_uneffective_inst, // important! must done before mark_pure_func
  mark_pure_func,

  gvn_no_cfg,

  array_mem2reg,
    gvn_no_cfg,
    clean_useless_cf, // for loop fusion
    loop_fusion,
    gvn_no_cfg,
    duplicate_load_store_elimination,
  array_ssa_destruction,

  loop_unroll,

  gvn_cfg,
  gvn_cfg,

  function_inline,

  array_mem2reg,
    gvn_cfg,
    duplicate_load_store_elimination,
  array_ssa_destruction,

  main_global_var_to_local,
  mem2reg,

  loop_unroll,

  gvn_cfg,
  loop_unroll,

  gvn_cfg,
  remove_useless_loop,
  gvn_cfg,

  
  gep_destruction,
  gvn_no_cfg,

  operator_strength_reduction,
  gvn_cfg,

  sort_basicblock,
};

// without modify cfg
void gvn_no_cfg(ir::Program *prog) {
  gvn_gcm(prog);
  remove_uneffective_inst(prog);
  remove_unused_function(prog);
  operator_strength_promotion(prog);
}

// modify cfg
void gvn_cfg(ir::Program *prog) {
  gvn_gcm(prog);
  remove_uneffective_inst(prog);
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
