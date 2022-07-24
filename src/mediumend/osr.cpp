#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

#include <stack>
#include <queue>

namespace mediumend {

using ir::Instruction;
using ir::insns::Output;
using ir::Reg;

namespace osr {

static int dfs_num;
static std::unordered_map<Instruction *, int> dfn, low;
static std::stack<Instruction *> insts_stack;
static std::unordered_set<Instruction *> in_stack;
static std::queue<Instruction *> all_insts;
static std::unordered_set<Instruction *> visited;
// Inductive Variable
// static std::unordered_set<Instruction *> IV;
static std::unordered_map<Instruction *, BasicBlock *> header;
static std::unordered_map<std::tuple<BinaryOp, Instruction *, Instruction *>, Reg> hashMap;

// Region Const
bool isRC(Instruction *name, BasicBlock *header) {
  TypeCase(loadimm, ir::insns::LoadImm *, name) return true;
  if (header != name->bb && header->domby.count(name->bb)) return true;
  return false;
}

Output *copyDef(Instruction *iv, Reg result) {
  TypeCase(phi, ir::insns::Phi *, iv) {
    std::vector<BasicBlock *> bbs;
    std::vector<Reg> regs;
    for (auto pair : phi->incoming) {
      bbs.push_back(pair.first);
      regs.push_back(pair.second);
    }
    return new ir::insns::Phi(result, bbs, regs);
  }
  TypeCase(binary, ir::insns::Binary *, iv) {
    return new ir::insns::Binary(result, binary->op, binary->src1, binary->src2);
  }
  assert(false);
}

Reg reduce(BinaryOp opcode, ScalarType stype, Output *iv, Output *rc);

Reg apply(BinaryOp opcode, ScalarType stype, Output *op1, Output *op2) {
  debug(std::cerr) << "apply: " << op1->dst.id << " " << op2->dst.id << std::endl;
  if (hashMap.count(std::make_tuple(opcode, op1, op2))) return hashMap.at(std::make_tuple(opcode, op1, op2));
  if (header.at(op1) != nullptr && isRC(op2, header.at(op1))) {
    return reduce(opcode, stype, op1, op2);
  }
  if (header.at(op2) != nullptr && isRC(op1, header.at(op2))) {
    return reduce(opcode, stype, op2, op1);
  }
  Reg result = op1->bb->func->new_reg(stype);
  hashMap[std::make_tuple(opcode, op1, op2)] = result;
  auto newOper = new ir::insns::Binary(result, opcode, op1->dst, op2->dst);
  header[newOper] = nullptr;
  if (op1->bb->domby.count(op2->bb)) {
    op1->bb->insert_before_ter(newOper);
  } else if (op2->bb->domby.count(op1->bb)) {
    op2->bb->insert_before_ter(newOper);
  } else assert(false);
  newOper->add_use_def();
  all_insts.push(newOper);
  return result;
}

Reg reduce(BinaryOp opcode, ScalarType stype, Output *iv, Output *rc) {
  debug(std::cerr) << "reduce: " << iv->dst.id << " " << rc->dst.id << std::endl;
  if (hashMap.count(std::make_tuple(opcode, iv, rc))) return hashMap.at(std::make_tuple(opcode, iv, rc));
  Reg result = iv->bb->func->new_reg(stype);
  hashMap[std::make_tuple(opcode, iv, rc)] = result;
  Output *new_iv = copyDef(iv, result); // copy a new iv, perform op on operands of new iv
  new_iv->bb = iv->bb;
  header[new_iv] = header.at(iv);
  auto use = new_iv->use();
  for (auto reg : use) {
    Output *inst_use = static_cast<Output *> (new_iv->bb->func->def_list.at(reg));
    if (header.at(inst_use) == header.at(iv)) { // operand is iv, recursive
      new_iv->change_use(reg, reduce(opcode, stype, inst_use, rc));
    } else TypeCase(phi, ir::insns::Phi *, new_iv) {
      new_iv->change_use(reg, apply(opcode, stype, inst_use, rc));
    } else if (opcode == BinaryOp::Mul) {
      new_iv->change_use(reg, apply(opcode, stype, inst_use, rc));
    }
  }
  new_iv->bb->insert_after_inst(iv, new_iv);
  all_insts.push(new_iv);
  return result;
}

void replace(ir::insns::Binary *node, Instruction *iv, Instruction *rc) {
  debug(std::cerr) << "replace: " << node->dst.id << std::endl;
  Output *_iv = static_cast<Output *> (iv);
  Output *_rc = static_cast<Output *> (rc);
  Reg result = reduce(node->op, node->dst.type, _iv, _rc);
  copy_propagation(node->bb->func->use_list, node->dst, result);
  header[node] = header.at(iv);
}

// check iv * rc, rc * iv, iv +- rc, rc + iv & replace
bool check_replace(Instruction *inst) {
  TypeCase(binary, ir::insns::Binary *, inst) {
    Instruction *i1 = binary->bb->func->def_list.at(binary->src1);
    Instruction *i2 = binary->bb->func->def_list.at(binary->src2);
    switch (binary->op) {
      // iv * rc, rc * iv
      // iv + rc, rc + iv
      case BinaryOp::Mul:
      case BinaryOp::Add:
        if (header.count(i1) && header.at(i1) != nullptr && isRC(i2, header.at(i1))) {
          replace(binary, i1, i2);
          return true;
        }
        if (header.count(i2) && header.at(i2) != nullptr && isRC(i1, header.at(i2))) {
          replace(binary, i2, i1);
          return true;
        }
        break;
      // iv - rc
      case BinaryOp::Sub:
        if (header.count(i1) && header.at(i1) != nullptr && isRC(i2, header.at(i1))) {
          replace(binary, i1, i2);
          return true;
        }
        break;
      default:
        break;
    }
  }
  return false;
}

// determine if scc is inductive variable
void classifyIV(const unordered_set<Instruction *> &SCC) {
  BasicBlock *head = (*SCC.begin())->bb;
  for (auto inst : SCC) {
    if (head->rpo_num > inst->bb->rpo_num) {
      head = inst->bb;
    }
  }
  bool flag = true;
  for (auto inst : SCC) {
    // only contain phi, +, -, copy
    TypeCase(phi, ir::insns::Phi *, inst) {}
    else TypeCase(binary, ir::insns::Binary *, inst) {
      if (binary->op != BinaryOp::Add && binary->op != BinaryOp::Sub) {
        flag = false;
        break;
      }
    } else {
      flag = false;
      break;
    }
    // only contain IV & RC
    auto use = inst->use();
    for (auto r : use) {
      auto inst_use = inst->bb->func->def_list.at(r);
      if (!SCC.count(inst_use) && !isRC(inst_use, head)) {
        flag = false;
        break;
      }
    }
  }
  if (flag) {
    for (auto inst : SCC) {
      debug(std::cerr) << "find inductive variable: " << ((ir::insns::Output *) inst)->dst.id << std::endl;
      header[inst] = head;
    }
  } else {
    for (auto inst : SCC) {
      if (!check_replace(inst)) header[inst] = nullptr;
    }
  }
}

void processSCC(const unordered_set<Instruction *> &SCC) {
  if (SCC.size() == 1) {
    auto inst = *SCC.begin();
    if (!check_replace(inst)) header[inst] = nullptr;
  } else {
    classifyIV(SCC);
  }
}

// Tarjan algorithm
void dfs(Instruction *inst) {
  dfn[inst] = dfs_num++;
  visited.insert(inst);
  low[inst] = dfn[inst];
  insts_stack.push(inst);
  in_stack.insert(inst);
  auto use = inst->use();
  for (auto reg: use) {
    auto inst_use = inst->bb->func->def_list.at(reg);
    if (!visited.count(inst_use)) {
      dfs(inst_use);
      low[inst] = std::min(low[inst], low[inst_use]);
    }
    if (dfn[inst_use] < dfn[inst] && in_stack.count(inst_use)) {
      low[inst] = std::min(low[inst], dfn[inst_use]);
    }
  }
  if (low[inst] == dfn[inst]) { // find a SCC
    std::unordered_set<Instruction *> SCC;
    Instruction *i;
    do {
      i = insts_stack.top();
      insts_stack.pop();
      in_stack.erase(i);
      SCC.insert(i);
    } while (i != inst);
    processSCC(SCC);
  }
}

void insert_params(ir::Function *func) {
  for (int i = 1; i <= func->sig.param_types.size(); i++) {
    if (func->sig.param_types[i].is_array()) {
      func->bbs.front()->push_front(new ir::insns::Output(Reg(Int, i)));
    } else {
      func->bbs.front()->push_front(new ir::insns::Output(Reg(func->sig.param_types[i].base_type, i)));
    }
  }
}

void remove_params(ir::Function *func) {
  for (int i = 1; i <= func->sig.param_types.size(); i++) {
    func->bbs.front()->pop_front();
  }
}

void osr(ir::Function *func) {
  insert_params(func);
  func->cfg->build();
  func->cfg->compute_dom();
  func->cfg->compute_rpo();

  // clear global variables
  dfs_num = 0;
  dfn.clear();
  low.clear();
  while (!insts_stack.empty()) insts_stack.pop();
  visited.clear();
  header.clear();
  hashMap.clear();
  for (auto &bb : func->bbs) {
    for (auto &insn : bb->insns) {
      all_insts.push(insn.get());
    }
  }
  while (!all_insts.empty()) {
    auto inst = all_insts.front();
    all_insts.pop();
    if (!visited.count(inst)) dfs(inst);
  }

  remove_params(func);
}

} // namespace osr

void operator_strength_reduction(ir::Program *prog) {
  for (auto &func : prog->functions) {
    osr::osr(&func.second);
  }
}

} // namespace mediumend