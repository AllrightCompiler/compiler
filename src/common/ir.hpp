#pragma once

#include "common/common.hpp"

#include "frontend/symbol_table.hpp"

#include <functional>
#include <ostream>

namespace mediumend {
class CFG;
}

namespace ir {

using std::list;
using std::optional;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

struct Reg {
  ScalarType type;
  int id;

  Reg() {}
  Reg(ScalarType type, int id) : type{type}, id{id} {}

  bool operator<(const Reg &b) const { return id < b.id; }
  bool operator==(const Reg &b) const { return id == b.id; }
  bool operator!=(const Reg &b) const { return !this->operator==(b); }
};
} // namespace ir

namespace std {
template <> class hash<ir::Reg> {
public:
  size_t operator()(const ir::Reg &r) const { return hash<int>()(r.id); }
};
} // namespace std

namespace ir {
struct Storage {
  bool global;
  Reg reg; // reg的type字段无效，此reg表示变量的地址。全局变量的此字段无效。
};

struct BasicBlock;
struct Function;

struct Instruction {
  BasicBlock *bb;

  template <typename T> bool is() const {
    return dynamic_cast<const T *>(this) != nullptr;
  }

  virtual ~Instruction() = default;

  virtual void emit(std::ostream &os) const {}
  virtual void add_use_def() {}
  virtual void remove_use_def() {}
  virtual void change_use(Reg, Reg) {}
  virtual std::vector<Reg *> reg_ptrs() { return {}; }
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
  Loop *loop = nullptr;
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

  mediumend::CFG *cfg = nullptr;
  int pure = -1;

  unordered_map<Reg, unordered_set<Instruction *>> use_list;
  unordered_map<Reg, Instruction *> def_list;
  unordered_set<Reg> global_addr;

  list<unique_ptr<BasicBlock>> bbs;
  Reg new_reg(ScalarType t) { return ir::Reg{t, ++nr_regs}; }
  ~Function();
  bool has_param(Reg r) { return r.id <= sig.param_types.size(); }
  bool is_pure() const { return pure == 1; }
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

// 指令输出相关
std::ostream &operator<<(std::ostream &os, const Program &);
void emit_global_var(std::ostream &os, const std::string &name, Var *var);
void set_print_context(const Program &);
const Program *get_print_context();

inline std::string reg_name(Reg r) { return "%" + std::to_string(r.id); }

inline std::string var_name(std::string name) { return "@" + name; }

inline std::string label_name(std::string name) { return "%" + name; }

inline std::string type_string(ScalarType t) {
  if (t == Int)
    return "i32";
  if (t == Float)
    return "float";
  return "?";
}

inline std::string type_string(const std::optional<ScalarType> &t) {
  if (!t)
    return "void";
  return type_string(t.value());
}

inline std::string type_string(const Type &t) {
  auto s = type_string(t.base_type);
  if (!t.is_array())
    return s;

  bool pointer = false;
  int i = 0;
  if (t.dims[0] == 0) {
    pointer = true;
    i = 1;
  }
  if (t.is_array()) {
    if (t.dims.size() == 1 && pointer) { // scalar pointer
      s = type_string(t.base_type);
    } else { // really array
      s = "";
      for (; i < t.nr_dims() - 1; ++i)
        s += "[" + std::to_string(t.dims[i]) + " x ";
      s += "[" + std::to_string(t.dims[i]) + " x " + type_string(t.base_type) +
           "]";
      i = pointer ? 1 : 0;
      for (; i < t.nr_dims() - 1; ++i)
        s += "]";
    }
  }
  if (pointer)
    s += "*";
  return s;
}

// instructions

namespace insns {

struct Terminator : Instruction {};

// 写目的寄存器的指令
struct Output : Instruction {
  Reg dst;

  Output() {}
  Output(Reg r) : dst{r} {}

  std::ostream &write_reg(std::ostream &os) const;

  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&dst}; }
};

// 栈空间分配
struct Alloca : Output {
  Type type;

  Alloca(Reg dst, Type tp) : type{std::move(tp)}, Output{dst} {}

  virtual void emit(std::ostream &os) const override;
};

// dst = mem[src]
struct Load : Output {
  Reg addr;

  Load(Reg dst, Reg addr) : addr{addr}, Output{dst} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&dst, &addr}; }
};

// 将全局变量地址加载到dst寄存器
struct LoadAddr : Output {
  string var_name;

  LoadAddr(Reg dst, string name) : var_name{std::move(name)}, Output{dst} {}

  virtual void emit(std::ostream &os) const override;
};

struct LoadImm : Output {
  ConstValue imm;

  LoadImm(Reg dst, ConstValue immediate) : imm{immediate}, Output{dst} {}

  virtual void emit(std::ostream &os) const override;
};

struct Store : Instruction {
  Reg addr, val;

  Store(Reg addr, Reg val) : addr{addr}, val{val} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&addr, &val}; }
};

// NOTE: 在LLVM阶段之前，type标注的是base对应的变量的类型
// LLVM阶段后，type即指令模板中的<ty>
struct GetElementPtr : Output {
  Type type;
  Reg base;
  vector<Reg> indices;
  bool omit_first_index; // 如果为true，则在输出为LLVM IR时需要首位添0

  GetElementPtr(Reg dst, Type tp, Reg base, vector<Reg> indexes)
      : type{std::move(tp)}, indices{std::move(indexes)}, base{base},
        Output{dst}, omit_first_index(false) {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override {
    std::vector<Reg *> ptrs{&dst, &base};
    for (Reg &r : indices)
      ptrs.push_back(&r);
    return ptrs;
  }
};

struct Convert : Output {
  Reg src;

  Convert(Reg to, Reg from) : src{from}, Output{to} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
};

struct Call : Output {
  string func;
  vector<Reg> args;

  Call(Reg dst, string callee, vector<Reg> arg_regs)
      : func{std::move(callee)}, args{std::move(arg_regs)}, Output{dst} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override {
    std::vector<Reg *> ptrs{&dst};
    for (Reg &r : args)
      ptrs.push_back(&r);
    return ptrs;
  }
};

struct Unary : Output {
  UnaryOp op;
  Reg src;

  Unary(Reg dst, UnaryOp op, Reg src) : op{op}, src{src}, Output{dst} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
};

struct Binary : Output {
  BinaryOp op;
  Reg src1, src2;

  Binary(Reg dst, BinaryOp op, Reg src1, Reg src2)
      : op{op}, src1{src1}, src2{src2}, Output{dst} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override {
    return {&dst, &src1, &src2};
  }
};

struct Phi : Output {
  std::unordered_map<BasicBlock *, Reg> incoming;

  Phi(Reg dst) : Output{dst} {}

  Phi(Reg dst, vector<BasicBlock *> bbs, vector<Reg> regs) : Output{dst} {
    for (size_t i = 0; i < bbs.size(); i++) {
      incoming[bbs[i]] = regs[i];
    }
  };

  void remove_prev(BasicBlock *bb);

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override {
    std::vector<Reg *> ptrs{&dst};
    for (auto &[_, r] : incoming)
      ptrs.push_back(&r);
    return ptrs;
  }
};

struct Return : Terminator {
  std::optional<Reg> val;

  Return(std::optional<Reg> ret_val) : val{ret_val} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override {
    if (val.has_value())
      return {&val.value()};
    return {};
  }
};

struct Jump : Terminator {
  BasicBlock *target;

  Jump(BasicBlock *dest) : target{dest} {}

  virtual void emit(std::ostream &os) const override;
};

struct Branch : Terminator {
  BasicBlock *true_target, *false_target;
  Reg val;

  Branch(Reg src, BasicBlock *true_dst, BasicBlock *false_dst)
      : val{src}, true_target{true_dst}, false_target{false_dst} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&val}; }
};

} // namespace insns

} // namespace ir
