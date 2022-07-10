#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include <string>
#include <string_view>

namespace ir {

const Program *ir_print_prog = nullptr;

using std::ostream;

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

inline std::string type_string(string fun_name) {
  if(ir_print_prog->functions.count(fun_name)){
    return type_string(ir_print_prog->functions.at(fun_name).sig.ret_type);
  }
  return type_string(ir_print_prog->lib_funcs.at(fun_name).sig.ret_type);
}

std::string type_string(const Type &t) {
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
      s += "[" + std::to_string(t.dims[i]) + " x " + type_string(t.base_type) + "]";
      i = pointer ? 1 : 0;
      for (; i < t.nr_dims() - 1; ++i)
        s += "]";
    }
  }
  if (pointer)
    s += "*";
  return s;
}

std::string op_string(UnaryOp op) {
  switch (op) {
  case UnaryOp::Sub:
    return "neg";
  case UnaryOp::Not:
    return "not";
  default:
    return "[unary op]";
  }
}

std::string op_string(BinaryOp op) {
  switch (op) {
  case BinaryOp::Add:
    return "add";
  case BinaryOp::Sub:
    return "sub";
  case BinaryOp::Mul:
    return "mul";
  case BinaryOp::Div:
    return "div";
  case BinaryOp::Mod:
    return "mod";
  case BinaryOp::Eq:
    return "cmp eq";
  case BinaryOp::Neq:
    return "cmp ne";
  case BinaryOp::Lt:
    return "cmp lt";
  case BinaryOp::Leq:
    return "cmp le";
  case BinaryOp::Gt:
    return "cmp gt";
  case BinaryOp::Geq:
    return "cmp ge";
  default:
    return "[binary op]";
  }
}

void Instruction::print(std::ostream &, unsigned int) const {}
Instruction::~Instruction() {}

Function::~Function() {
  if(cfg){
    delete cfg;
  }
}

void BasicBlock::push_back(Instruction *insn) {
  insns.emplace_back(insn);
  insn->bb = this;
  insn->add_use_def();
}

void BasicBlock::push_front(Instruction *insn) {
  insns.emplace_front(insn);
  insn->bb = this;
  insn->add_use_def();
}

void BasicBlock::insert_after_phi(Instruction *insn) {
  auto it = insns.begin();
  for (; it != insns.end(); it++) {
    TypeCase(phi, ir::insns::Phi *, it->get()) {
    } else break;
  }
  insns.emplace(it, insn);
  insn->bb = this;
  // insn->add_use_def();
}

bool BasicBlock::remove(Instruction *insn) {
  for (auto it = insns.begin(); it != insns.end(); it++) {
    if (it->get() == insn) {
      it->release();  // important! otherwise inst will be auto deleted
      // insn->remove_use_def();
      insn->bb = nullptr;
      insns.erase(it);
      return true;
    }
  }
  return false;
}

std::vector<Instruction *> BasicBlock::remove_if(std::function<bool(Instruction *)> f) {
  std::vector<Instruction *> ret;
  for (auto it = insns.begin(); it != insns.end(); ) {
    if (f(it->get())) {
      auto insn = it->release();
      insn->remove_use_def();
      insn->bb = nullptr;
      it = insns.erase(it);
      ret.push_back(insn);
    } else it++;
  }
  return ret;
}

void Function::clear_visit(){
  for (auto &bb : bbs) {
    bb->clear_visit();
  }
}

void Function::clear_graph(){
  for (auto &bb : bbs) {
    bb->prev.clear();
    bb->succ.clear();
  }
}

void Function::clear_dom(){
  for (auto &bb : bbs) {
    bb->dom.clear();
    bb->domby.clear();
    bb->idom = nullptr;
  }
}
namespace insns {

ostream &write_reg(ostream &os, const Output &ins) {
  os << reg_name(ins.dst);
  return os;
}

ostream &operator<<(ostream &os, const Alloca &ins) {
  write_reg(os, ins) << " = alloca " << type_string(ins.type);
  return os;
}

ostream &operator<<(ostream &os, const LoadImm &ins) {
  auto &imm = ins.imm;
  write_reg(os, ins) << " = loadimm " << type_string(imm.type) << " ";
  if (imm.type == Float)
    os << std::to_string(imm.fv);
  else
    os << std::to_string(imm.iv);
  return os;
}

ostream &operator<<(ostream &os, const LoadAddr &ins) {
  write_reg(os, ins) << " = loadaddr " << var_name(ins.var_name);
  return os;
}

ostream &operator<<(ostream &os, const Load &ins) {
  auto ts = type_string(ins.dst.type);
  write_reg(os, ins) << " = load " << ts << ", " << type_string(ins.addr.type) << " "
                     << reg_name(ins.addr);
  return os;
}

ostream &operator<<(ostream &os, const Store &ins) {
  auto ts = type_string(ins.val.type);
  os << "store " << ts << " " << reg_name(ins.val) << ", " << type_string(ins.addr.type) << " "
     << reg_name(ins.addr);
  return os;
}

ostream &operator<<(ostream &os, const GetElementPtr &ins) {
  write_reg(os, ins) << " = getelementptr " << type_string(ins.type) << ", "
                     << type_string(ins.base.type) << " " << reg_name(ins.base);
  int num_zero = ins.type.dims.size() - ins.dst.type.dims.size() + 1 - ins.indices.size() + 1; // output trick
  while (num_zero--) {
    os << ", i32 0";
  }
  for (int i = 0; i < int(ins.indices.size()); ++i) {
    os << ", " << type_string(ins.indices[i].type) << " " << reg_name(ins.indices[i]);
  }
  return os;
}

ostream &operator<<(ostream &os, const Convert &ins) {
  write_reg(os, ins) << " = convert " << type_string(ins.src.type) << " "
                     << reg_name(ins.src) << " to "
                     << type_string(ins.dst.type);
  return os;
}

ostream &operator<<(ostream &os, const Call &ins) {
  // TODO: 类型标注，需要Program上下文
  write_reg(os, ins) << " = call " << type_string(ins.func) << " " << var_name(ins.func) << "(";
  for (int i = 0; i < int(ins.args.size()); ++i) {
    if (i != 0)
      os << ", ";
    if(ir_print_prog->lib_funcs.count(ins.func)){
      os << type_string(ir_print_prog->lib_funcs.at(ins.func).sig.param_types[i]) << " " << reg_name(ins.args[i]);
    }else{
      os << type_string(ir_print_prog->functions.at(ins.func).sig.param_types[i]) << " " << reg_name(ins.args[i]);
    }
  }
  os << ")";
  return os;
}

ostream &operator<<(ostream &os, const Unary &ins) {
  if (ins.op == UnaryOp::Sub) {
    write_reg(os, ins) << " = " << "sub i32 0, " << reg_name(ins.src);
  } else {
    write_reg(os, ins) << " = " << op_string(ins.op) << " " << reg_name(ins.src);
  }
  return os;
}

ostream &operator<<(ostream &os, const Binary &ins) {
  write_reg(os, ins) << " = " << op_string(ins.op) << " "
                     << type_string(ins.src1.type) << " " << reg_name(ins.src1)
                     << ", " << reg_name(ins.src2);
  return os;
}

ostream &operator<<(ostream &os, const Return &ins) {
  if (ins.val)
    os << "ret " << type_string(ins.val->type) << " "
       << reg_name(ins.val.value());
  else
    os << "ret void";
  return os;
}

ostream &operator<<(ostream &os, const Jump &ins) {
  os << "br label " << label_name(ins.target->label);
  return os;
}

ostream &operator<<(ostream &os, const Branch &ins) {
  os << "br " << type_string(ins.val.type) << " " << reg_name(ins.val)
     << ", label " << label_name(ins.true_target->label) << ", label "
     << label_name(ins.false_target->label);
  return os;
}

ostream &operator<<(ostream &os, const Phi &ins) {
  write_reg(os, ins) << " = " << "phi " << type_string(ins.dst.type);
  for (auto it = ins.incoming.begin(); it != ins.incoming.end(); it++) {
    if (it != ins.incoming.begin()) os << ",";
    os << " [" << reg_name(it->second) << ", "
       << label_name(it->first->label) << "]";
  }
  return os;
}

} // namespace insns

ostream &operator<<(ostream &os, const Instruction &insn) {
  using namespace insns;
  auto ins = &insn;
  TypeCase(alloca, const Alloca *, ins) {
    os << *alloca;
  } else TypeCase(load, const Load *, ins) {
    os << *load;
  } else TypeCase(load_imm, const LoadImm *, ins) {
    os << *load_imm;
  } else TypeCase(load_addr, const LoadAddr *, ins) {
    os << *load_addr;
  } else TypeCase(store, const Store *, ins) {
    os << *store;
  } else TypeCase(gep, const GetElementPtr *, ins) {
    os << *gep;
  } else TypeCase(convert, const Convert *, ins) {
    os << *convert;
  } else TypeCase(call, const Call *, ins) {
    os << *call;
  } else TypeCase(unary, const Unary *, ins) {
    os << *unary;
  } else TypeCase(binary, const Binary *, ins) {
    os << *binary;
  } else TypeCase(ret, const Return *, ins) {
    os << *ret;
  } else TypeCase(jump, const Jump *, ins) {
    os << *jump;
  } else TypeCase(branch, const Branch *, ins) {
    os << *branch;
  } else TypeCase(phi, const Phi *, ins) {
    os << *phi;
  } else {
    assert(false);
  }
  return os;
}

ostream &operator<<(ostream &os, const BasicBlock &bb) {
  if (!bb.insns.empty()) {
    os << bb.label << ":\n";
    for (auto &insn : bb.insns)
      os << "  " << *insn << "\n";
  }
  return os;
}

ostream &operator<<(ostream &os, const Function &f) {
  os << "define " << type_string(f.sig.ret_type) << " " << var_name(f.name)
     << "(";

  auto &types = f.sig.param_types;
  for (int i = 0; i < int(types.size()); ++i) {
    if (i != 0)
      os << ", ";
    os << type_string(types[i]) << " %" << std::to_string(i + 1);
  }
  os << ") {\n";
  int i = 0;
  for (auto &bb : f.bbs) {
    if (i != 0)
      os << "\n";
    os << *bb;
    i++;
  }
  return os << "}\n\n";
}

ostream &operator<<(ostream &os, const ConstValue &p) {
  if (p.type == Int) {
    os << p.iv;
  } else if (p.type == Float) {
    os << p.fv;
  } else assert(false);
  return os;
}

void generate_array_init(string &output, Type tp, int offset, std::map<int, ConstValue> *arr_val) {
  int num_elements = tp.size() / 4;
  bool all_zero = true;
  for (int i = offset; i < offset + num_elements; i++)
    if (arr_val->count(i)) all_zero = false;
  if (all_zero) {
    output += type_string(tp) + " zeroinitializer";
    return;
  }
  if (tp.dims.size() == 1) {
    output += type_string(tp) + " [";
    for (int i = 0; i < tp.dims[0]; i++) {
      ConstValue val(0);
      if (arr_val->count(offset + i)) val = arr_val->at(offset + i);
      if (i > 0) output += ", ";
      output += type_string(tp.base_type) + " " + val.toString();
    }
    output += "]";
  } else {
    output += type_string(tp) + " [";
    Type new_type = tp;
    new_type.dims.erase(new_type.dims.begin());
    for (int i = 0; i < tp.dims[0]; i++) {
      if (i > 0) output += ", ";
      generate_array_init(output, new_type, offset, arr_val);
      offset += new_type.size() / 4;
    }
    output += "]";
  }
}

ostream &operator<<(ostream &os, const Program &p) {
  ir_print_prog = &p;
  for (auto &[name, var] : p.global_vars) {
    if (var->type.is_array()) {
      if (var->arr_val) {
        os << "@" << name << " = global ";
        string init_string;
        generate_array_init(init_string, var->type, 0, var->arr_val.get());
        os << init_string << "\n";
      } else {
        os << "@" << name << " = global " << type_string(var->type) << " zeroinitializer\n";
      }
    } else {
      if (var->val.has_value()) {
        os << "@" << name << " = global " << type_string(var->type) << " " << var->val.value() << "\n";
      } else {
        os << "@" << name << " = global " << type_string(var->type) << " zeroinitializer\n";
      }
    }
  }
  for (auto &[_, f] : p.functions)
    os << f;
  return os;
}

void BasicBlock::rpo_dfs(vector<BasicBlock *> &rpo) {
  if (visit) return;
  visit = true;
  for (auto next : succ) {
    next->rpo_dfs(rpo);
  }
  rpo.emplace_back(this);
}

void BasicBlock::loop_dfs() {
  // dfs on dom tree
  for (auto next : dom) {
    next->loop_dfs();
  }
  std::vector<BasicBlock *> bbs;
  for (auto p : prev) {
    if (p->domby.count(this)) { // find back edge to header
      bbs.emplace_back(p);
    }
  }
  if (!bbs.empty()) { // form 1 loop ( TODO: nested loops with same header? )
    Loop *new_loop = new Loop(this);
    while (bbs.size() > 0) {
      auto bb = bbs.back();
      bbs.pop_back();
      if (!bb->loop) {
        bb->loop = new_loop;
        if (bb != this) {
          bbs.insert(bbs.end(), bb->prev.begin(), bb->prev.end());
        }
      } else {
        Loop *inner_loop = bb->loop;
        while (inner_loop->outer) inner_loop = inner_loop->outer;
        if (inner_loop == new_loop) continue;
        inner_loop->outer = new_loop;
        bbs.insert(bbs.end(), inner_loop->header->prev.begin(), inner_loop->header->prev.end());
      }
    }
  }
}

void calc_loop_level(Loop *loop) {
  if(!loop){
    return;
  }
  if (loop->level != -1) return;
  if (loop->outer == nullptr) loop->level = 1;
  else {
    calc_loop_level(loop->outer);
    loop->level = loop->outer->level + 1;
  }
}

namespace insns {

void Output::add_use_def(){
  bb->func->def_list[dst] = this;
}

void Output::remove_use_def(){
  bb->func->def_list.erase(dst);
}

void Load::add_use_def(){
  Output::add_use_def();
  bb->func->use_list[addr].push_back(this);
}

void Load::remove_use_def(){
  Output::remove_use_def();
  bb->func->use_list[addr].remove(this);
}

void Load::change_use(Reg old_reg, Reg new_reg){
  if(addr == old_reg){
    addr = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
}

void Store::add_use_def(){
  bb->func->use_list[val].push_back(this);
  bb->func->use_list[addr].push_back(this);
}

void Store::remove_use_def(){
  bb->func->use_list[val].remove(this);
  bb->func->use_list[addr].remove(this);
}

void Store::change_use(Reg old_reg, Reg new_reg){
  if(val == old_reg){
    val = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
  if(addr == old_reg){
    addr = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
}

void Convert::add_use_def(){
  Output::add_use_def();
  bb->func->use_list[src].push_back(this);
}

void Convert::remove_use_def(){
  Output::remove_use_def();
  bb->func->use_list[src].remove(this);
}

void Convert::change_use(Reg old_reg, Reg new_reg){
  if(src == old_reg){
    src = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
}

void Call::add_use_def(){
  Output::add_use_def();
  for(auto &reg : args){
    bb->func->use_list[reg].push_back(this);
  }
}

void Call::remove_use_def(){
  Output::remove_use_def();
  for(auto &reg : args){
    bb->func->use_list[reg].remove(this);
  }
}

void Call::change_use(Reg old_reg, Reg new_reg){
  for(auto &reg : args){
    if(reg == old_reg){
      reg = new_reg;
      bb->func->use_list[new_reg].push_back(this);
      bb->func->use_list[old_reg].remove(this);
    }
  }
}

void Unary::add_use_def(){
  Output::add_use_def();
  bb->func->use_list[src].push_back(this);
}

void Unary::remove_use_def(){
  Output::remove_use_def();
  bb->func->use_list[src].remove(this);
}

void Unary::change_use(Reg old_reg, Reg new_reg){
  if(src == old_reg){
    src = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
}

void Binary::add_use_def(){
  Output::add_use_def();
  bb->func->use_list[src1].push_back(this);
  bb->func->use_list[src2].push_back(this);
}

void Binary::remove_use_def(){
  Output::remove_use_def();
  bb->func->use_list[src1].remove(this);
  bb->func->use_list[src2].remove(this);
}

void Binary::change_use(Reg old_reg, Reg new_reg){
  if(src1 == old_reg){
    src1 = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
  if(src2 == old_reg){
    src2 = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
}

void Phi::add_use_def(){
  Output::add_use_def();
  for(auto &[bb, reg] : incoming){
    bb->func->use_list[reg].push_back(this);
  }
}

void Phi::remove_use_def(){
  Output::remove_use_def();
  for(auto &[bb, reg] : incoming){
    bb->func->use_list[reg].remove(this);
  }
}

void Phi::change_use(Reg old_reg, Reg new_reg){
  for(auto &[bb, reg] : incoming){
    if(reg == old_reg){
      reg = new_reg;
      bb->func->use_list[new_reg].push_back(this);
      bb->func->use_list[old_reg].remove(this);
    }
  }
}

void Return::add_use_def(){
  if(val.has_value())
    bb->func->use_list[val.value()].push_back(this);
}

void Return::remove_use_def(){
  if(val.has_value())
    bb->func->use_list[val.value()].remove(this);
}

void Return::change_use(Reg old_reg, Reg new_reg){
  if(val.has_value() && val.value() == old_reg){
    val = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
}

void Branch::add_use_def(){
  bb->func->use_list[val].push_back(this);
}

void Branch::remove_use_def(){
  bb->func->use_list[val].remove(this);
}

void Branch::change_use(Reg old_reg, Reg new_reg){
  if(val == old_reg){
    val = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
}

void GetElementPtr::add_use_def(){
  Output::add_use_def();
  bb->func->use_list[base].push_back(this);
  for(auto &idx : indices){
    bb->func->use_list[idx].push_back(this);
  }
}

void GetElementPtr::remove_use_def(){
  Output::remove_use_def();
  bb->func->use_list[base].remove(this);
  for(auto &idx : indices){
    bb->func->use_list[idx].remove(this);
  }
}

void GetElementPtr::change_use(Reg old_reg, Reg new_reg){
  if(base == old_reg){
    base = new_reg;
    bb->func->use_list[new_reg].push_back(this);
    bb->func->use_list[old_reg].remove(this);
  }
  for(auto &idx : indices){
    if(idx == old_reg){
      idx = new_reg;
      bb->func->use_list[new_reg].push_back(this);
      bb->func->use_list[old_reg].remove(this);
    }
  }
}

}
} // namespace ir
