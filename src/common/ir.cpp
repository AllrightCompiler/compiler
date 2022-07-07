#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include <string>
#include <string_view>

namespace ir {

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
  for (; i < t.nr_dims(); ++i)
    s += "[" + std::to_string(t.dims[i]) + "]";
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
  write_reg(os, ins) << " = load " << ts << ", " << ts << "* "
                     << reg_name(ins.addr);
  return os;
}

ostream &operator<<(ostream &os, const Store &ins) {
  auto ts = type_string(ins.val.type);
  os << "store " << ts << " " << reg_name(ins.val) << ", " << ts << "* "
     << reg_name(ins.addr);
  return os;
}

ostream &operator<<(ostream &os, const GetElementPtr &ins) {
  write_reg(os, ins) << " = getelementptr " << type_string(ins.type) << " "
                     << reg_name(ins.base);
  for (int i = 0; i < int(ins.indices.size()); ++i) {
    os << ", " << reg_name(ins.indices[i]);
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
  write_reg(os, ins) << " = call " << var_name(ins.func) << "(";
  for (int i = 0; i < int(ins.args.size()); ++i) {
    if (i != 0)
      os << ", ";
    os << reg_name(ins.args[i]);
  }
  os << ")";
  return os;
}

ostream &operator<<(ostream &os, const Unary &ins) {
  write_reg(os, ins) << " = " << op_string(ins.op) << " " << reg_name(ins.src);
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
  write_reg(os, ins) << " = " << "Phi";
  for (auto each : ins.incoming) {
    os << " [" << label_name(each.first->label) << ", "
       << reg_name(each.second) << "]";
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

ostream &operator<<(ostream &os, const Program &p) {
  for (auto &[_, f] : p.functions)
    os << f;
  return os;
}
namespace insns {

void Output::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  def_list[dst] = this;
}

void Output::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  def_list.erase(dst);
}

void Load::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::add_use_def(use_list, def_list);
  use_list[addr].push_back(this);
}

void Load::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::remove_use_def(use_list, def_list);
  use_list[addr].remove(this);
}

void Load::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  if(addr == old_reg){
    addr = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
}

void Store::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  use_list[val].push_back(this);
  use_list[addr].push_back(this);
}

void Store::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  use_list[val].remove(this);
  use_list[addr].remove(this);
}

void Store::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  if(val == old_reg){
    val = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
  if(addr == old_reg){
    addr = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
}

void Convert::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::add_use_def(use_list, def_list);
  use_list[src].push_back(this);
}

void Convert::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::remove_use_def(use_list, def_list);
  use_list[src].remove(this);
}

void Convert::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  if(src == old_reg){
    src = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
}

void Call::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::add_use_def(use_list, def_list);
  for(auto &reg : args){
    use_list[reg].push_back(this);
  }
}

void Call::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::remove_use_def(use_list, def_list);
  for(auto &reg : args){
    use_list[reg].remove(this);
  }
}

void Call::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  for(auto &reg : args){
    if(reg == old_reg){
      reg = new_reg;
      use_list[new_reg].push_back(this);
      use_list[old_reg].remove(this);
    }
  }
}

void Unary::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::add_use_def(use_list, def_list);
  use_list[src].push_back(this);
}

void Unary::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::remove_use_def(use_list, def_list);
  use_list[src].remove(this);
}

void Unary::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  if(src == old_reg){
    src = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
}

void Binary::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::add_use_def(use_list, def_list);
  use_list[src1].push_back(this);
  use_list[src2].push_back(this);
}

void Binary::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::remove_use_def(use_list, def_list);
  use_list[src1].remove(this);
  use_list[src2].remove(this);
}

void Binary::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  if(src1 == old_reg){
    src1 = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
  if(src2 == old_reg){
    src2 = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
}

void Phi::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::add_use_def(use_list, def_list);
  for(auto &[bb, reg] : incoming){
    use_list[reg].push_back(this);
  }
}

void Phi::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::remove_use_def(use_list, def_list);
  for(auto &[bb, reg] : incoming){
    use_list[reg].remove(this);
  }
}

void Phi::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  for(auto &[bb, reg] : incoming){
    if(reg == old_reg){
      reg = new_reg;
      use_list[new_reg].push_back(this);
      use_list[old_reg].remove(this);
    }
  }
}

void Return::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  if(val.has_value())
    use_list[val.value()].push_back(this);
}

void Return::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  if(val.has_value())
    use_list[val.value()].remove(this);
}

void Return::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  if(val.has_value() && val.value() == old_reg){
    val = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
}

void Branch::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  use_list[val].push_back(this);
}

void Branch::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  use_list[val].remove(this);
}

void Branch::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  if(val == old_reg){
    val = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
}

void GetElementPtr::add_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::add_use_def(use_list, def_list);
  use_list[base].push_back(this);
  for(auto &idx : indices){
    use_list[idx].push_back(this);
  }
}

void GetElementPtr::remove_use_def(unordered_map<Reg, list<Instruction *>> &use_list, unordered_map<Reg, Instruction*> &def_list){
  Output::remove_use_def(use_list, def_list);
  use_list[base].remove(this);
  for(auto &idx : indices){
    use_list[idx].remove(this);
  }
}

void GetElementPtr::change_use(unordered_map<Reg, list<Instruction *>> &use_list, Reg old_reg, Reg new_reg){
  if(base == old_reg){
    base = new_reg;
    use_list[new_reg].push_back(this);
    use_list[old_reg].remove(this);
  }
  for(auto &idx : indices){
    if(idx == old_reg){
      idx = new_reg;
      use_list[new_reg].push_back(this);
      use_list[old_reg].remove(this);
    }
  }
}


}
} // namespace ir
