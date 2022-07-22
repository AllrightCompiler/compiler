#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include <string>
#include <string_view>

namespace ir {

const Program *prog_ctx = nullptr;

void set_print_context(const Program &program) { prog_ctx = &program; }
const Program *get_print_context() { return prog_ctx; }

const FunctionSignature &get_signature(const Program &p,
                                       const std::string &name) {
  if (p.lib_funcs.count(name))
    return p.lib_funcs.at(name).sig;
  return p.functions.at(name).sig;
}

using std::ostream;

inline std::string op_string(UnaryOp op) {
  switch (op) {
  case UnaryOp::Sub:
    return "neg";
  case UnaryOp::Not:
    return "not";
  default:
    return "[unary op]";
  }
}

inline std::string op_string(BinaryOp op, ScalarType t) {
  bool f = t == Float;
  switch (op) {
  case BinaryOp::Add:
    return f ? "fadd" : "add";
  case BinaryOp::Sub:
    return f ? "fsub" : "sub";
  case BinaryOp::Mul:
    return f ? "fmul" : "mul";
  case BinaryOp::Div:
    return f ? "fdiv" : "sdiv";
  case BinaryOp::Mod:
    return "srem";
  case BinaryOp::Eq:
  case BinaryOp::Neq:
  case BinaryOp::Lt:
  case BinaryOp::Leq:
  case BinaryOp::Gt:
  case BinaryOp::Geq: {
    std::string prefix = f ? "fcmp " : "icmp ";
    std::string mode = f ? "o" : "s";
    switch (op) {
    case BinaryOp::Eq:
      return prefix + (f ? "o" : "") + "eq";
    case BinaryOp::Neq:
      return prefix + (f ? "o" : "") + "ne";
    case BinaryOp::Lt:
      return prefix + mode + "lt";
    case BinaryOp::Leq:
      return prefix + mode + "le";
    case BinaryOp::Gt:
      return prefix + mode + "gt";
    case BinaryOp::Geq:
      return prefix + mode + "ge";
    default:
      __builtin_unreachable();
    }
  }
  default:
    return "[binary op]";
  }
}

Function::~Function() {
  if (cfg) {
    delete cfg;
  }
}

void Function::do_liveness_analysis() {
  for (auto &bb : bbs) {
    bb->live_use.clear();
    bb->def.clear();

    for (auto &insn : bb->insns) {
      auto def = insn->def();
      auto use = insn->use();

      for (auto &u : use) {
        if (!bb->def.count(u))
          bb->live_use.insert(u);
      }
      for (auto &d : def) {
        bb->def.insert(d);
      }
    }

    bb->live_in = bb->live_use;
    bb->live_out.clear();
  }

  bool changed = true;
  while (changed) {
    changed = false;
    for (auto &bb : bbs) {
      unordered_set<Reg> new_out;
      for (auto succ : bb->succ)
        new_out.insert(succ->live_in.begin(), succ->live_in.end());

      if (bb->live_out != new_out) {
        bb->live_out = std::move(new_out);
        changed = true;

        auto new_in = bb->live_use;
        for (auto &e : bb->live_out)
          if (!bb->def.count(e))
            new_in.insert(e);

        bb->live_in = std::move(new_in);
      }
    }
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

void BasicBlock::pop_front() {
  insns.front()->remove_use_def();
  insns.pop_front(); // auto release
}

void BasicBlock::insert_after_phi(Instruction *insn) {
  auto it = insns.begin();
  for (; it != insns.end(); it++) {
    TypeCase(phi, ir::insns::Phi *, it->get()) {}
    else break;
  }
  insns.emplace(it, insn);
  insn->bb = this;
  // insn->add_use_def();
}

void BasicBlock::insert_after_inst(Instruction *prev, Instruction *insn) {
  auto it = insns.begin();
  for (; it != insns.end(); it++) {
    if (it->get() == prev) break;
  }
  assert(it != insns.end());
  it++;
  insns.emplace(it, insn);
  insn->bb = this;
  insn->add_use_def();
}

void BasicBlock::insert_before_ter(Instruction *insn) {
  auto it = insns.end();
  insns.emplace(--it, insn);
  insn->bb = this;
  // insn->add_use_def();
}

bool BasicBlock::remove(Instruction *insn) {
  for (auto it = insns.begin(); it != insns.end(); it++) {
    if (it->get() == insn) {
      it->release(); // important! otherwise inst will be auto deleted
      // insn->remove_use_def();
      insn->bb = nullptr;
      insns.erase(it);
      return true;
    }
  }
  return false;
}

std::vector<Instruction *>
BasicBlock::remove_if(std::function<bool(Instruction *)> f) {
  std::vector<Instruction *> ret;
  for (auto it = insns.begin(); it != insns.end();) {
    if (f(it->get())) {
      auto insn = it->release();
      insn->remove_use_def();
      insn->bb = nullptr;
      it = insns.erase(it);
      ret.push_back(insn);
    } else
      it++;
  }
  return ret;
}

void BasicBlock::change_succ(BasicBlock* old_bb, BasicBlock* new_bb){
  if(this->succ.count(old_bb)){
    auto inst = this->insns.back().get();
    this->succ.erase(old_bb);
    this->succ.insert(new_bb);
    TypeCase(br, ir::insns::Branch *, inst){
      if(br->true_target == old_bb){
        br->true_target = new_bb;
      }
      if(br->false_target == old_bb){
        br->false_target = new_bb;
      }
    }
    TypeCase(jmp, ir::insns::Jump *, inst){
      if(jmp->target == old_bb){
        jmp->target = new_bb;
      }
    }
  }  
}

void BasicBlock::change_prev(BasicBlock* old_bb, BasicBlock* new_bb){
  if(this->prev.count(old_bb)){
    this->prev.erase(old_bb);
    this->prev.insert(new_bb);
    for(auto &ins: insns){
      TypeCase(phi, ir::insns::Phi*, ins.get()){
        phi->incoming[new_bb] = phi->incoming.at(old_bb);
        phi->incoming.erase(old_bb);
      } else {
        break;
      }
    }
  }
}

void Function::clear_visit() {
  for (auto &bb : bbs) {
    bb->clear_visit();
  }
}

void Function::clear_graph() {
  for (auto &bb : bbs) {
    bb->prev.clear();
    bb->succ.clear();
  }
}

void Function::clear_dom() {
  for (auto &bb : bbs) {
    bb->dom.clear();
    bb->domby.clear();
    bb->idom = nullptr;
  }
}

namespace insns {

ostream &Output::write_reg(ostream &os) const {
  os << reg_name(dst) << " = ";
  return os;
}

void Alloca::emit(std::ostream &os) const {
  write_reg(os) << "alloca " << type_string(type);
}

void LoadImm::emit(std::ostream &os) const {
  write_reg(os) << "loadimm " << type_string(imm.type) << " "
                << imm.to_string();
}

void LoadAddr::emit(std::ostream &os) const {
  write_reg(os) << "loadaddr " << ir::var_name(var_name);
}

void Load::emit(std::ostream &os) const {
  auto ts = type_string(dst.type);
  write_reg(os) << "load " << ts << ", " << ts << "* " << reg_name(addr);
}

void Store::emit(std::ostream &os) const {
  auto ts = type_string(val.type);
  os << "store " << ts << " " << reg_name(val) << ", " << ts << "* "
     << reg_name(addr);
}

void MemUse::emit(std::ostream &os) const {
  if(call_use){
    os << "(" << reg_name(dst) << ") = (" << reg_name(dep) << ")(" << reg_name(load_src) << ")";
  } else {
    auto ts = type_string(dst.type);
    write_reg(os) << "(" << reg_name(dep) <<")[" << reg_name(load_src) << "]";
  }
}

void MemDef::emit(std::ostream &os) const {
  if(call_def){
    os << "(" << reg_name(dst) << ")[" << reg_name(store_dst) <<"] = (" << reg_name(store_val) << ")";
  } else {
    auto ts = type_string(store_val.type);
    os << "(" << reg_name(dst) << ")[" << reg_name(store_dst) <<"] = " << ts << " " << reg_name(store_val);
  }
}

void GetElementPtr::emit(std::ostream &os) const {
  // write_reg(os, ins) << " = getelementptr " << type_string(ins.type) << ", "
  //                    << type_string(ins.base.type) << " " <<
  //                    reg_name(ins.base);
  // int num_zero = ins.type.dims.size() - ins.dst.type.dims.size() + 1 -
  //                ins.indices.size() + 1; // output trick
  // while (num_zero--) {
  //   os << ", i32 0";
  // }
  // for (int i = 0; i < int(ins.indices.size()); ++i) {
  //   os << ", " << type_string(ins.indices[i].type) << " "
  //      << reg_name(ins.indices[i]);
  // }
  write_reg(os) << "getelementptr " << type_string(type) << " "
                << reg_name(base);
  for (size_t i = 0; i < indices.size(); ++i)
    os << ", " << reg_name(indices[i]);
}

void Convert::emit(std::ostream &os) const {
  write_reg(os);
  if (src.type == Int && dst.type == Float)
    os << "sitofp i32 " << reg_name(src) << " to float";
  else if (src.type == Float && dst.type == Int)
    os << "fptosi float " << reg_name(src) << " to i32";
  else
    assert(false);
}

void Call::emit(std::ostream &os) const {
  auto ctx = get_print_context();
  auto &sig = get_signature(*ctx, func);
  write_reg(os) << "call " << type_string(sig.ret_type) << " " << var_name(func)
                << "(";
  for (size_t i = 0; i < args.size(); ++i) {
    if (i != 0)
      os << ", ";
    os << type_string(sig.param_types[i]) << " " << reg_name(args[i]);
  }
  os << ")";
}

void Unary::emit(std::ostream &os) const {
  // 这里不直接以sub形式输出一元neg
  write_reg(os) << op_string(op) << " " << reg_name(src);
}

void Binary::emit(std::ostream &os) const {
  write_reg(os) << op_string(op, src1.type) << " " << type_string(src1.type)
                << " " << reg_name(src1) << ", " << reg_name(src2);
}

void Return::emit(std::ostream &os) const {
  if (val)
    os << "ret " << type_string(val->type) << " " << reg_name(val.value());
  else
    os << "ret void";
}

void Jump::emit(std::ostream &os) const {
  os << "br label " << label_name(target->label);
}

void Branch::emit(std::ostream &os) const {
  os << "br " << type_string(val.type) << " " << reg_name(val) << ", label "
     << label_name(true_target->label) << ", label "
     << label_name(false_target->label);
}

void Phi::emit(std::ostream &os) const {
  write_reg(os) << "phi " << type_string(dst.type);
  // for (auto it = incoming.begin(); it != incoming.end(); ++it) {
  //   if (it != incoming.begin())
  //     os << ", ";
  //   os << " [" << reg_name(it->second) << ", " <<
  //   label_name(it->first->label)
  //      << "]";
  // }
  int first = 1;
  for (auto bb : bb->prev) {
    if (first) {
      first = 0;
    } else {
      os << ",";
    }
    if (incoming.count(bb)) {
      os << " [" << reg_name(incoming.at(bb)) << ", " << label_name(bb->label)
         << "]";
    } else {
      os << " [" << 0 << ", " << label_name(bb->label) << "]";
    }
  }
}

} // namespace insns

ostream &operator<<(ostream &os, const BasicBlock &bb) {
  if (!bb.insns.empty()) {
    os << bb.label << ":\n";
    for (auto &insn : bb.insns) {
      os << "  ";
      insn->emit(os);
      os << '\n';
    }
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
    char buf[32];
    double v = p.fv;
    std::sprintf(buf, "0x%016llx", *reinterpret_cast<const long long *>(&v));
    os << buf;
  } else
    assert(false);
  return os;
}

void generate_array_init(string &output, Type tp, int offset,
                         std::map<int, ConstValue> *arr_val) {
  int num_elements = tp.size() / 4;
  bool all_zero = arr_val->empty();
  if (all_zero) {
    output += type_string(tp) + " zeroinitializer";
    return;
  }
  if (tp.dims.size() == 1) {
    output += type_string(tp) + " [";
    for (int i = 0; i < tp.dims[0]; i++) {
      ConstValue val(0);
      if (arr_val->count(offset + i))
        val = arr_val->at(offset + i);
      if (i > 0)
        output += ", ";
      output += type_string(tp.base_type) + " " + val.to_string();
    }
    output += "]";
  } else {
    output += type_string(tp) + " [";
    Type new_type = tp;
    new_type.dims.erase(new_type.dims.begin());
    for (int i = 0; i < tp.dims[0]; i++) {
      if (i > 0)
        output += ", ";
      generate_array_init(output, new_type, offset, arr_val);
      offset += new_type.size() / 4;
    }
    output += "]";
  }
}

void emit_global_var(std::ostream &os, const std::string &name, Var *var) {
  if (var->type.is_array()) {
    if (var->arr_val) {
      os << "@" << name << " = global ";
      string init_string;
      generate_array_init(init_string, var->type, 0, var->arr_val.get());
      os << init_string << "\n";
    } else {
      os << "@" << name << " = global " << type_string(var->type)
         << " zeroinitializer\n";
    }
  } else {
    if (var->val.has_value()) {
      os << "@" << name << " = global " << type_string(var->type) << " "
         << var->val.value() << "\n";
    } else {
      os << "@" << name << " = global " << type_string(var->type)
         << " zeroinitializer\n";
    }
  }
}

ostream &operator<<(ostream &os, const Program &p) {
  set_print_context(p);
  for (auto &[name, var] : p.global_vars)
    emit_global_var(os, name, var.get());
  for (auto &[_, f] : p.functions)
    os << f;
  return os;
}

void BasicBlock::rpo_dfs(vector<BasicBlock *> &rpo) {
  if (visit)
    return;
  visit = true;
  for (auto next : succ) {
    next->rpo_dfs(rpo);
  }
  rpo_num = rpo.size();
  rpo.emplace_back(this);
}

void BasicBlock::loop_dfs() {
  // dfs on dom tree
  this->loop = nullptr;
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
    func->loops.emplace_back(new_loop);
    while (bbs.size() > 0) {
      auto bb = bbs.back();
      bbs.pop_back();
      if (!bb->loop) {
        bb->loop = new_loop;
        if (bb != this) {
          bbs.insert(bbs.end(), bb->prev.begin(), bb->prev.end());
        }
      } else {
        new_loop->no_inner = false;
        Loop *inner_loop = bb->loop;
        while (inner_loop->outer)
          inner_loop = inner_loop->outer;
        if (inner_loop == new_loop)
          continue;
        inner_loop->outer = new_loop;
        bbs.insert(bbs.end(), inner_loop->header->prev.begin(),
                   inner_loop->header->prev.end());
      }
    }
  }
}

void Function::loop_analysis() {
  this->clear_visit();
  this->cfg->compute_dom();
  this->loops.clear();
  this->bbs.front()->loop_dfs();
  for (auto &bb : this->bbs) {
    calc_loop_level(bb->loop);
  }
}

void calc_loop_level(Loop *loop) {
  if (!loop) {
    return;
  }
  if (loop->level != -1)
    return;
  if (loop->outer == nullptr)
    loop->level = 1;
  else {
    calc_loop_level(loop->outer);
    loop->level = loop->outer->level + 1;
  }
}

namespace insns {

void Output::add_use_def() { bb->func->def_list[dst] = this; }

void Output::remove_use_def() { bb->func->def_list.erase(dst); }

void Load::add_use_def() {
  Output::add_use_def();
  bb->func->use_list[addr].insert(this);
}

void Load::remove_use_def() {
  Output::remove_use_def();
  bb->func->use_list[addr].erase(this);
}

void Load::change_use(Reg old_reg, Reg new_reg) {
  if (addr == old_reg) {
    addr = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
}

void Store::add_use_def() {
  bb->func->use_list[val].insert(this);
  bb->func->use_list[addr].insert(this);
}

void Store::remove_use_def() {
  bb->func->use_list[val].erase(this);
  bb->func->use_list[addr].erase(this);
}

void Store::change_use(Reg old_reg, Reg new_reg) {
  if (val == old_reg) {
    val = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
  if (addr == old_reg) {
    addr = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
}

void MemUse::add_use_def() {
  Output::add_use_def();
  bb->func->use_list[dep].insert(this);
  bb->func->use_list[load_src].insert(this);
}

void MemUse::remove_use_def() {
  Output::remove_use_def();
  bb->func->use_list[dep].erase(this);
  bb->func->use_list[load_src].erase(this);
}

void MemUse::change_use(Reg old_reg, Reg new_reg) {
  if(dep == old_reg){
    dep = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
  if(load_src == old_reg){
    load_src = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
}

void MemDef::add_use_def() {
  Output::add_use_def();
  bb->func->use_list[store_dst].insert(this);
  bb->func->use_list[store_val].insert(this);
}

void MemDef::remove_use_def() {
  Output::remove_use_def();
  bb->func->use_list[store_dst].erase(this);
  bb->func->use_list[store_val].erase(this);
}

void MemDef::change_use(Reg old_reg, Reg new_reg) {
  if(store_dst == old_reg){
    store_dst = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
  if(store_val == old_reg){
    store_val = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
}

void Convert::add_use_def() {
  Output::add_use_def();
  bb->func->use_list[src].insert(this);
}

void Convert::remove_use_def() {
  Output::remove_use_def();
  bb->func->use_list[src].erase(this);
}

void Convert::change_use(Reg old_reg, Reg new_reg) {
  if (src == old_reg) {
    src = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
}

void Call::add_use_def() {
  Output::add_use_def();
  for (auto &reg : args) {
    bb->func->use_list[reg].insert(this);
  }
}

void Call::remove_use_def() {
  Output::remove_use_def();
  for (auto &reg : args) {
    bb->func->use_list[reg].erase(this);
  }
}

void Call::change_use(Reg old_reg, Reg new_reg) {
  for (auto &reg : args) {
    if (reg == old_reg) {
      reg = new_reg;
      bb->func->use_list[new_reg].insert(this);
      bb->func->use_list[old_reg].erase(this);
    }
  }
}

void Unary::add_use_def() {
  Output::add_use_def();
  bb->func->use_list[src].insert(this);
}

void Unary::remove_use_def() {
  Output::remove_use_def();
  bb->func->use_list[src].erase(this);
}

void Unary::change_use(Reg old_reg, Reg new_reg) {
  if (src == old_reg) {
    src = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
}

void Binary::add_use_def() {
  Output::add_use_def();
  bb->func->use_list[src1].insert(this);
  bb->func->use_list[src2].insert(this);
}

void Binary::remove_use_def() {
  Output::remove_use_def();
  bb->func->use_list[src1].erase(this);
  bb->func->use_list[src2].erase(this);
}

void Binary::change_use(Reg old_reg, Reg new_reg) {
  if (src1 == old_reg) {
    src1 = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
  if (src2 == old_reg) {
    src2 = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
}

void Phi::add_use_def() {
  Output::add_use_def();
  for (auto &[bb, reg] : incoming) {
    bb->func->use_list[reg].insert(this);
  }
}

void Phi::remove_use_def() {
  Output::remove_use_def();
  for (auto &[bb, reg] : incoming) {
    bb->func->use_list[reg].erase(this);
  }
}

void Phi::change_use(Reg old_reg, Reg new_reg) {
  for (auto &[bb, reg] : incoming) {
    if (reg == old_reg) {
      reg = new_reg;
      bb->func->use_list[new_reg].insert(this);
      bb->func->use_list[old_reg].erase(this);
    }
  }
}

void Phi::remove_prev(BasicBlock *prev) {
  auto iter = this->incoming.find(prev);
  if (iter != this->incoming.end()) {
    auto &list = this->bb->func->use_list[this->incoming.at(prev)];
    list.erase(this);
    this->incoming.erase(iter);
  } else {
    assert(false);
  }
}

void Return::add_use_def() {
  if (val.has_value())
    bb->func->use_list[val.value()].insert(this);
}

void Return::remove_use_def() {
  if (val.has_value())
    bb->func->use_list[val.value()].erase(this);
}

void Return::change_use(Reg old_reg, Reg new_reg) {
  if (val.has_value() && val.value() == old_reg) {
    val = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
}

void Branch::add_use_def() { bb->func->use_list[val].insert(this); }

void Branch::remove_use_def() { bb->func->use_list[val].erase(this); }

void Branch::change_use(Reg old_reg, Reg new_reg) {
  if (val == old_reg) {
    val = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
}

void GetElementPtr::add_use_def() {
  Output::add_use_def();
  bb->func->use_list[base].insert(this);
  for (auto &idx : indices) {
    bb->func->use_list[idx].insert(this);
  }
}

void GetElementPtr::remove_use_def() {
  Output::remove_use_def();
  bb->func->use_list[base].erase(this);
  for (auto &idx : indices) {
    bb->func->use_list[idx].erase(this);
  }
}

void GetElementPtr::change_use(Reg old_reg, Reg new_reg) {
  if (base == old_reg) {
    base = new_reg;
    bb->func->use_list[new_reg].insert(this);
    bb->func->use_list[old_reg].erase(this);
  }
  for (auto &idx : indices) {
    if (idx == old_reg) {
      idx = new_reg;
      bb->func->use_list[new_reg].insert(this);
      bb->func->use_list[old_reg].erase(this);
    }
  }
}

} // namespace insns
} // namespace ir
