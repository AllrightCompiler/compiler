#pragma once

#include "common/Display.hpp"
#include "common/common.hpp"

#include "frontend/symbol_table.hpp"

#include <ostream>

namespace ir {

using std::optional;
using std::string;
using std::unique_ptr;
using std::vector;
using std::list;

struct Reg {
  int type;
  int id;

  Reg() {}
  Reg(int type_, int id_) : type{type_}, id{id_} {}
};

struct Storage {
  bool global;
  Reg reg; // reg的type字段无效，此reg表示变量的地址。全局变量的此字段无效。
};

struct Instruction : Display {
  virtual void print(std::ostream &, unsigned) const override;
  virtual ~Instruction();
};

struct BasicBlock {
  string label;
  list<unique_ptr<Instruction>> insns;

  void push(Instruction *insn) { insns.emplace_back(insn); }
};

struct FunctionSignature {
  optional<int> ret_type;
  vector<Type> param_types;
};

struct Function {
  string name;
  FunctionSignature sig;

  list<unique_ptr<BasicBlock>> bbs;
};

struct LibFunction {
  FunctionSignature sig;
};

struct Program {
  std::unordered_map<string, Function> functions;
  std::unordered_map<string, LibFunction> lib_funcs;
  frontend::VarTable global_vars;
  vector<string> string_table;
};

std::ostream &operator<<(std::ostream &, const Program &);

// instructions

namespace insns {

struct Terminator : Instruction {};

// 写目的寄存器的指令
struct Output : Instruction {
  Reg dst;

  Output() {}
  Output(Reg r) : dst{r} {}
};

// 栈空间分配
struct Alloca : Output {
  Type type;

  Alloca(Reg dst, Type tp) : type{std::move(tp)}, Output{dst} {}
};

// dst = mem[src]
struct Load : Output {
  Reg addr;

  Load(Reg dst, Reg addr) : addr{addr}, Output{dst} {}
};

// 将全局变量地址加载到dst寄存器
struct LoadAddr : Output {
  string var_name;

  LoadAddr(Reg dst, string name) : var_name{std::move(name)}, Output{dst} {}
};

struct LoadImm : Output {
  ConstValue imm;

  LoadImm(Reg dst, ConstValue immediate) : imm{immediate}, Output{dst} {}
};

struct Store : Instruction {
  Reg addr, val;

  Store(Reg addr, Reg val) : addr{addr}, val{val} {}
};

struct GetElementPtr : Output {
  Type type;
  Reg base;
  vector<Reg> indices;

  GetElementPtr(Reg dst, Type tp, Reg base, vector<Reg> indexes)
      : type{std::move(tp)}, indices{std::move(indexes)}, base{base},
        Output{dst} {}
};

struct Convert : Output {
  Reg src;

  Convert(Reg to, Reg from) : src{from}, Output{to} {}
};

struct Call : Output {
  string func;
  vector<Reg> args;

  Call(Reg dst, string callee, vector<Reg> arg_regs)
      : func{std::move(callee)}, args{std::move(arg_regs)}, Output{dst} {}
};

struct Unary : Output {
  UnaryOp op;
  Reg src;

  Unary(Reg dst, UnaryOp op, Reg src) : op{op}, src{src}, Output{dst} {}
};

struct Binary : Output {
  BinaryOp op;
  Reg src1, src2;

  Binary(Reg dst, BinaryOp op, Reg src1, Reg src2)
      : op{op}, src1{src1}, src2{src2}, Output{dst} {}
};

struct Phi : Output{
  Phi(Reg dst) : Output(dst) {}
};

struct Return : Terminator {
  std::optional<Reg> val;

  Return(std::optional<Reg> ret_val) : val{ret_val} {}
};

struct Jump : Terminator {
  BasicBlock *target;

  Jump(BasicBlock *dest) : target{dest} {}
};

struct Branch : Terminator {
  BasicBlock *true_target, *false_target;
  Reg val;

  Branch(Reg src, BasicBlock *true_dst, BasicBlock *false_dst)
      : val{src}, true_target{true_dst}, false_target{false_dst} {}
};

} // namespace insns

} // namespace ir
