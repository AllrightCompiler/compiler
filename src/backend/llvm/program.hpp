#pragma once

#include "common/ir.hpp"

namespace llvm {

using Instruction = ir::Instruction;

using std::list;
using std::map;
using std::string;
using std::unique_ptr;
using std::vector;

struct BasicBlock {
  string label;
  list<unique_ptr<Instruction>> insns;

  void push(Instruction *insn) { insns.emplace_back(insn); }
};

struct Function {
  string name;
  ir::FunctionSignature sig;
  int nr_regs;

  ir::Reg new_reg(ScalarType t) { return ir::Reg{t, ++nr_regs}; }

  list<unique_ptr<BasicBlock>> bbs;
};

struct Program {
  vector<string> lib_func_decls;
  map<string, ir::LibFunction> lib_funcs;
  map<string, Function> functions;
  frontend::VarTable global_vars;
  vector<string> string_table;

  Program();

  const ir::FunctionSignature &get_signature(const string &name) const {
    if (lib_funcs.count(name))
      return lib_funcs.at(name).sig;
    return functions.at(name).sig;
  }

  void emit(std::ostream &os) const;
  void emit_header(std::ostream &os) const;
  void emit_basic_block(std::ostream &os, const BasicBlock &bb) const;
  void emit_function(std::ostream &os, const Function &f) const;
  bool emit_special_instruction(std::ostream &os, const Instruction &ins) const;
};

std::unique_ptr<llvm::Program> translate(const ir::Program &ir_program);
void emit_global(std::ostream &os, const ir::Program &ir_program);

} // namespace llvm
