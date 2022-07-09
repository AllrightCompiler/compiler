#pragma once

#include "common/Display.hpp"
#include "common/common.hpp"

#include "frontend/symbol_table.hpp"

#include <ostream>

namespace mediumend{
class CFG;
}

namespace ir {

using std::optional;
using std::string;
using std::unique_ptr;
using std::vector;
using std::list;
using std::unordered_map;
using std::unordered_set;

struct Reg {
  ScalarType type;
  int id;

  Reg() {}
  Reg(ScalarType type_, int id_) : type{type_}, id{id_} {}
  bool operator== (const Reg &b)const{
      return id == b.id && type == b.type;
  }
};
}

namespace std{
template<>
class hash<ir::Reg> {
public:
	size_t operator()(const ir::Reg& r) const{ return hash<int>()(r.id); }
};
}

namespace ir{
struct Storage {
  bool global;
  Reg reg; // reg的type字段无效，此reg表示变量的地址。全局变量的此字段无效。
};

struct BasicBlock;
struct Function;

struct Instruction : Display {
  BasicBlock *bb;
  virtual void print(std::ostream &, unsigned) const override;
  virtual ~Instruction();
  virtual void add_use_def() {};
  virtual void remove_use_def() {};
  virtual void change_use(Reg , Reg ) {};
};

class Loop {
public:
    Loop *outer;
    BasicBlock *header;
    int level;
    Loop(BasicBlock *head) : header(head), outer(nullptr), level(-1) {}
};

struct BasicBlock {
  Function *func;
  string label;
  list<unique_ptr<Instruction>> insns;
  // edge in cfg
  unordered_set<BasicBlock *> prev, succ;
  // edge of dom tree
  unordered_set<BasicBlock *> dom;
  // edge of dom tree (reverse)
  BasicBlock *idom;
  // dom by (recursive)
  unordered_set<BasicBlock *> domby;
  Loop *loop;
  bool visit;
  int domlevel;

  // modify use-def
  void push_back(Instruction *insn);
  // modify use-def
  void push_front(Instruction *insn);
  // not modify use-def
  void insert_after_phi(Instruction *insn);
  // not modify use-def
  bool remove(Instruction *insn);
  // modify use-def
  std::vector<Instruction *> remove_if(std::function<bool(Instruction *)> f);
  int get_loop_level() const { return !loop ? 0 : loop->level; }
  void rpo_dfs(vector<BasicBlock *> &rpo);
  void loop_dfs();
  void clear_visit() { visit = false; }
};

void calc_loop_level(Loop *loop);

struct FunctionSignature {
  optional<ScalarType> ret_type;
  vector<Type> param_types;
};

struct Function {
  string name;
  FunctionSignature sig;
  int nr_regs;

  mediumend::CFG* cfg = nullptr;
  int pure = -1;

  unordered_map<Reg, list<Instruction *>> use_list;
  unordered_map<Reg, Instruction *> def_list;
  unordered_set<Reg> global_addr;

  list<unique_ptr<BasicBlock>> bbs;
  Reg new_reg(::ScalarType t) {
    return ir::Reg{t, ++nr_regs};
  }
  ~Function();
  bool has_param(Reg r){ return r.id <= sig.param_types.size(); }
  void clear_visit();
  void clear_graph();
  void clear_dom();
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
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
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
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
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
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
};

struct GetElementPtr : Output {
  Type type;
  Reg base;
  vector<Reg> indices;

  GetElementPtr(Reg dst, Type tp, Reg base, vector<Reg> indexes)
      : type{std::move(tp)}, indices{std::move(indexes)}, base{base},
        Output{dst} {}
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
};

struct Convert : Output {
  Reg src;

  Convert(Reg to, Reg from) : src{from}, Output{to} {}
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;

};

struct Call : Output {
  string func;
  vector<Reg> args;

  Call(Reg dst, string callee, vector<Reg> arg_regs)
      : func{std::move(callee)}, args{std::move(arg_regs)}, Output{dst} {}
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
};

struct Unary : Output {
  UnaryOp op;
  Reg src;

  Unary(Reg dst, UnaryOp op, Reg src) : op{op}, src{src}, Output{dst} {}
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
};

struct Binary : Output {
  BinaryOp op;
  Reg src1, src2;

  Binary(Reg dst, BinaryOp op, Reg src1, Reg src2)
      : op{op}, src1{src1}, src2{src2}, Output{dst} {}
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
};

struct Phi : Output {
  unordered_map<BasicBlock *, Reg> incoming;

  Phi(Reg dst) : Output{dst} {}

  Phi(Reg dst, vector<BasicBlock *> bbs, vector<Reg> regs): Output{dst} {
    for (int i = 0; i < bbs.size(); i++) {
      incoming[bbs[i]] = regs[i];
    }
  };
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
};

struct Return : Terminator {
  std::optional<Reg> val;

  Return(std::optional<Reg> ret_val) : val{ret_val} {}
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
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
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
};

} // namespace insns

} // namespace ir

