#pragma once

#include "common/common.hpp"

#include "frontend/symbol_table.hpp"

#include <functional>
#include <ostream>

#include "mediumend/cfg.hpp"

namespace ir {

using std::list;
using std::map;
using std::optional;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

struct Reg {
  int type;
  int id;

  Reg() {}
  Reg(int type_, int id_) : type{type_}, id{id_} {}

  bool operator<(const Reg &b) const { return id < b.id; }
  bool operator==(const Reg &b) const { return id == b.id; }
  bool operator!=(const Reg &b) const { return !this->operator==(b); }
};
} // namespace ir

namespace std {
template <> class hash<ir::Reg> {
public:
  size_t operator()(const ir::Reg &r) const { return r.id; }
};
template <class T1, class T2> struct hash<tuple<T1, T2>> {
  size_t operator()(const tuple<T1, T2> &r) const {
    return hash<T1>()(get<0>(r)) * 1221821 + hash<T2>()(get<1>(r)) * 31;
  }
};
template <class T1, class T2, class T3> struct hash<tuple<T1, T2, T3>> {
  size_t operator()(const tuple<T1, T2, T3> &r) const {
    return hash<T1>()(get<0>(r)) * 264893 + hash<T2>()(get<1>(r)) * 1221821 +
           hash<T3>()(get<2>(r)) * 31;
  }
};
} // namespace std

namespace ir {
struct Storage {
  bool global;
  Reg reg; // reg的type字段无效，此reg表示变量的地址。全局变量的此字段无效。
};

struct BasicBlock;
struct Function;

enum InstType {
  ALLOCA,
  LOAD,
  LOADADDR,
  LOADIMM,
  STORE,
  GEP,
  CONVERT,
  CALL,
  UNARY,
  BINARY,
  PHI,
  MEMUSE,
  MEMDEF,
  RETURN,
  JUMP,
  BRANCH,
  SWITCH,
  OUTPUT,
  TERMINATOR,
};

struct Instruction {
  BasicBlock *bb;
  InstType type;
  Instruction(InstType type) : type(type) {}

  template <typename T> bool is() const {
    return dynamic_cast<const T *>(this) != nullptr;
  }

  virtual ~Instruction() = default;

  virtual void emit(std::ostream &os) const {}
  virtual void add_use_def() {}
  virtual void remove_use_def() {}
  virtual void change_use(Reg, Reg) {}
  virtual std::vector<Reg *> reg_ptrs() { return {}; }
  virtual unordered_set<Reg> def() const { return {}; }
  virtual unordered_set<Reg> use() const { return {}; }
};

class Loop {
public:
  Loop *outer;
  BasicBlock *header;
  int level;
  bool no_inner;
  Loop(BasicBlock *head)
      : header(head), outer(nullptr), level(-1), no_inner(true) {}
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
  unordered_set<Reg> def, live_use, live_in, live_out; // for liveness analysis

  Loop *loop = nullptr;
  bool visit;
  int domlevel;
  int rpo_num; // id in rpo, available after compute_rpo
  double freq; // estimated execution frequency

  // modify use-def
  void push_back(Instruction *insn);
  // modify use-def
  void push_front(Instruction *insn);
  // modify use-def
  void pop_front();
  // modify use-def
  void insert_at(list<unique_ptr<Instruction>>::iterator it, Instruction *insn);
  // modify use-def
  list<unique_ptr<Instruction>>::iterator
  remove_at(list<unique_ptr<Instruction>>::iterator it);
  void pop_back();
  // not modify use-def
  void insert_at_pos(int pos, Instruction *insn);
  // not modify use-def
  void insert_after_phi(Instruction *insn);
  // modify use-def
  void insert_after_inst(Instruction *prev, Instruction *insn);
  // not modify use-def
  void insert_before_ter(Instruction *insn);
  // not modify use-def
  bool remove(Instruction *insn);
  // modify use-def
  std::vector<Instruction *> remove_if(std::function<bool(Instruction *)> f);
  int get_loop_level() const { return !loop ? 0 : loop->level; }
  void rpo_dfs(vector<BasicBlock *> &rpo);
  void loop_dfs();
  void clear_visit() { visit = false; }
  static void add_edge(BasicBlock *from, BasicBlock *to) {
    from->succ.insert(to);
    to->prev.insert(from);
  }
  static void remove_edge(BasicBlock *from, BasicBlock *to) {
    from->succ.erase(to);
    to->prev.erase(from);
  }
  void change_succ(BasicBlock *old_bb, BasicBlock *new_bb);
  void change_prev(BasicBlock *old_bb, BasicBlock *new_bb);
  void remove_prev(BasicBlock *bb);
};

void calc_loop_level(Loop *loop);

struct FunctionSignature {
  optional<int> ret_type;
  vector<Type> param_types;
};

struct Function {
  string name;
  FunctionSignature sig;
  int nr_regs;

  unique_ptr<mediumend::CFG> cfg;
  int pure = -1;
  int array_ssa_pure = -1;
  int only_load_param = -1;
  bool ret_used = false;

  unordered_map<Reg, unordered_set<Instruction *>> use_list;
  unordered_map<Reg, Instruction *> def_list;
  unordered_set<Reg> global_addr;
  vector<unique_ptr<Loop>> loops;
  map<std::pair<BasicBlock *, BasicBlock *>, double> branch_probs;
  map<std::pair<BasicBlock *, BasicBlock *>, double> branch_freqs;

  list<unique_ptr<BasicBlock>> bbs;
  Reg new_reg(int t) { return ir::Reg{t, ++nr_regs}; }

  bool has_param(Reg r) { return r.id <= sig.param_types.size(); }
  bool is_pure() const { return pure == 1; }
  bool is_array_ssa_pure() const { return array_ssa_pure == 1; }
  bool is_only_load_param() const { return only_load_param == 1; }
  bool is_ret_used() const { return ret_used; }
  void clear_visit();
  void clear_graph();
  void clear_dom();
  void do_liveness_analysis();
  void loop_analysis();
};

struct LibFunction {
  FunctionSignature sig;
};

struct Program {
  std::unordered_map<string, Function> functions;
  std::unordered_map<string, LibFunction> lib_funcs;
  frontend::VarTable global_vars;
  vector<string> string_table;

  bool may_alias = false;
};

// 指令输出相关
std::ostream &operator<<(std::ostream &os, const Program &);
std::ostream &operator<<(std::ostream &os, const ConstValue &);
void emit_global_var(std::ostream &os, const std::string &name, Var *var);

std::ostream &dump_cfg(std::ostream &os, const Program &);

void set_print_context(const Program &);
const Program *get_print_context();

inline std::string reg_name(Reg r) { return "%" + std::to_string(r.id); }

inline std::string var_name(std::string name) { return "@" + name; }

inline std::string label_name(std::string name) { return "%" + name; }

inline std::string type_string(int t) {
  if (t == Int)
    return "i32";
  if (t == Float)
    return "float";
  return "?";
}

inline std::string type_string(const std::optional<int> &t) {
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

struct Terminator : Instruction {
  Terminator(InstType type = TERMINATOR) : Instruction(type) {}
};

// 写目的寄存器的指令
struct Output : Instruction {
  Reg dst;

  Output(InstType type = OUTPUT) : Instruction(type) {}
  Output(Reg r, InstType type = TERMINATOR) : dst{r}, Instruction(type) {}

  std::ostream &write_reg(std::ostream &os) const;

  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&dst}; }
  unordered_set<Reg> def() const override { return {dst}; }
};

// 栈空间分配
struct Alloca : Output {
  Type type;

  Alloca(Reg dst, Type tp) : type{std::move(tp)}, Output{dst, ALLOCA} {}

  virtual void emit(std::ostream &os) const override;
};

// dst = mem[src]
struct Load : Output {
  Reg addr;

  Load(Reg dst, Reg addr) : addr{addr}, Output{dst, LOAD} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&dst, &addr}; }
  unordered_set<Reg> use() const override { return {addr}; }
};

// 将全局变量地址加载到dst寄存器
struct LoadAddr : Output {
  string var_name;

  LoadAddr(Reg dst, string name)
      : var_name{std::move(name)}, Output{dst, LOADADDR} {}

  virtual void emit(std::ostream &os) const override;
};

struct LoadImm : Output {
  ConstValue imm;

  LoadImm(Reg dst, ConstValue immediate)
      : imm{immediate}, Output{dst, LOADIMM} {}

  virtual void emit(std::ostream &os) const override;
};

struct Store : Instruction {
  Reg addr, val;

  Store(Reg addr, Reg val) : Instruction(STORE), addr{addr}, val{val} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&addr, &val}; }
  unordered_set<Reg> use() const override { return {addr, val}; }
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
        Output{dst, GEP}, omit_first_index(false) {}

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
  unordered_set<Reg> use() const override {
    unordered_set<Reg> ret{base};
    for (auto each : indices) {
      ret.insert(each);
    }
    return ret;
  }
};

struct Convert : Output {
  Reg src;

  Convert(Reg to, Reg from) : src{from}, Output{to, CONVERT} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
  unordered_set<Reg> use() const override { return {src}; }
};

struct Call : Output {
  string func;
  vector<Reg> args;
  vector<Reg> global_use;
  int variadic_at; // 第几个参数是 `...`

  Call(Reg dst, string callee, vector<Reg> arg_regs, int variadic_at = -1)
      : func{std::move(callee)}, args{std::move(arg_regs)},
        variadic_at{variadic_at}, Output{dst, CALL} {}

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
  unordered_set<Reg> use() const override {
    unordered_set<Reg> ret;
    for (auto each : args) {
      ret.insert(each);
    }
    for (auto each : global_use) {
      ret.insert(each);
    }
    return ret;
  }
};

struct Unary : Output {
  UnaryOp op;
  Reg src;

  Unary(Reg dst, UnaryOp op, Reg src) : op{op}, src{src}, Output{dst, UNARY} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
  unordered_set<Reg> use() const override { return {src}; }
};

struct Binary : Output {
  BinaryOp op;
  Reg src1, src2;

  Binary(Reg dst, BinaryOp op, Reg src1, Reg src2)
      : op{op}, src1{src1}, src2{src2}, Output{dst, BINARY} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override {
    return {&dst, &src1, &src2};
  }
  unordered_set<Reg> use() const override { return {src1, src2}; }
};

struct Phi : Output {
  std::unordered_map<BasicBlock *, Reg> incoming;
  std::unordered_set<Reg> use_before_def;
  bool array_ssa;

  Phi(Reg dst, bool array_ssa = false)
      : Output{dst, PHI}, array_ssa(array_ssa) {}

  Phi(Reg dst, vector<BasicBlock *> bbs, vector<Reg> regs)
      : Output{dst, PHI}, array_ssa(false) {
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
  unordered_set<Reg> use() const override {
    unordered_set<Reg> ret;
    for (auto each : incoming) {
      ret.insert(each.second);
    }
    for (auto each : use_before_def) {
      ret.insert(each);
    }
    return ret;
  }
};

struct MemUse : Output {
  Reg dep;
  Reg load_src;
  bool call_use;
  MemUse(Reg dst, Reg dep, Reg load_src, bool call_use)
      : dep{dep}, load_src{load_src}, call_use(call_use), Output{dst, MEMUSE} {}
  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override {
    return {&dst, &dep, &load_src};
  }
  unordered_set<Reg> use() const override { return {dep, load_src}; }
};

struct MemDef : Output {
  Reg dep;
  Reg store_dst;
  Reg store_val;
  bool call_def;
  unordered_set<Reg> uses_before_def;
  MemDef(Reg dst, Reg dep, Reg store_dst, Reg store_val, bool call_def,
         unordered_set<Reg> uses_before_def)
      : dep(dep), store_dst{store_dst}, store_val{store_val},
        call_def(call_def),
        uses_before_def(uses_before_def), Output{dst, MEMDEF} {}
  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override {
    return {&store_dst, &store_val, &dst, &dep};
  }
  unordered_set<Reg> use() const override {
    unordered_set<Reg> ret = {store_dst, store_val, dep};
    for (auto each : uses_before_def) {
      ret.insert(each);
    }
    return ret;
  }
};

struct Return : Terminator {
  std::optional<Reg> val;

  Return(std::optional<Reg> ret_val) : Terminator(RETURN), val{ret_val} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override {
    if (val.has_value())
      return {&val.value()};
    return {};
  }
  unordered_set<Reg> use() const override {
    if (val.has_value())
      return {val.value()};
    else
      return {};
  }
};

struct Jump : Terminator {
  BasicBlock *target;

  Jump(BasicBlock *dest) : Terminator(JUMP), target{dest} {}

  virtual void emit(std::ostream &os) const override;
};

struct Branch : Terminator {
  BasicBlock *true_target, *false_target;
  Reg val;

  Branch(Reg src, BasicBlock *true_dst, BasicBlock *false_dst)
      : Terminator(BRANCH), val{src}, true_target{true_dst}, false_target{
                                                                 false_dst} {}

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&val}; }
  unordered_set<Reg> use() const override { return {val}; }
};

struct Switch : Terminator {
  Reg val;
  map<int, BasicBlock *> targets;
  BasicBlock *default_target;

  virtual void emit(std::ostream &os) const override;
  virtual void add_use_def() override;
  virtual void remove_use_def() override;
  virtual void change_use(Reg old_reg, Reg new_reg) override;
  virtual std::vector<Reg *> reg_ptrs() override { return {&val}; }
  unordered_set<Reg> use() const override { return {val}; }

  Switch() : Terminator(SWITCH) {}
  Switch(Reg val, map<int, BasicBlock *> targets, BasicBlock *default_target)
      : Terminator(SWITCH), targets(std::move(targets)), val(val),
        default_target(default_target) {}
};

} // namespace insns

} // namespace ir
