#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using std::list;
using std::unique_ptr;
using ir::Instruction;
using ir::Reg;

static const int OSP_THRESHOLD = 10; // number of continuous insts

auto check_osp(list<unique_ptr<Instruction>> &insns, list<unique_ptr<Instruction>>::iterator it) {
  BinaryOp bop;
  Reg src1, src2, dst;
  TypeCase(binary, ir::insns::Binary *, it->get()) {
    if (binary->op != BinaryOp::Add && binary->op != BinaryOp::Sub) return it;
    bop = binary->op;
    src1 = binary->src1;
    src2 = binary->src2;
    dst = binary->dst;
  } else return it;

  { // Case 1: sum1 = sum0 +- a
    auto it_begin = it;
    auto it_end = it;
    int inst_cnt = 1;
    Reg r_init = src1, r_add = src2, r_sum = dst;
    while (it_end != insns.end()) {
      if (it_end->get()->bb->func->use_list.count(r_sum) && it_end->get()->bb->func->use_list.at(r_sum).size() == 1) {
        auto it_nxt = it_end;
        it_nxt++;
        TypeCase(binary, ir::insns::Binary *, it_nxt->get()) {
          if (binary->op != bop) break;
          if (!binary->bb->func->def_list.count(binary->src1) || binary->bb->func->def_list.at(binary->src1) != it_end->get()) break;
          if (r_add != binary->src2) break;
          r_sum = binary->dst;
        } else break;
      } else break;
      it_end++;
      inst_cnt++;
    }
    if (inst_cnt >= OSP_THRESHOLD) {
      while (it_begin != it_end) {
        it_begin->get()->remove_use_def();
        it_begin = insns.erase(it_begin);
      }
      auto loadimm = new ir::insns::LoadImm(it_end->get()->bb->func->new_reg(Int), ConstValue(inst_cnt));
      loadimm->bb = it_end->get()->bb;
      loadimm->add_use_def();
      auto mul = new ir::insns::Binary(it_end->get()->bb->func->new_reg(Int), BinaryOp::Mul, r_add, loadimm->dst);
      mul->bb = it_end->get()->bb;
      mul->add_use_def();
      TypeCase(binary_end, ir::insns::Binary *, it_end->get()) {
        binary_end->remove_use_def();
        binary_end->src1 = r_init;
        binary_end->src2 = mul->dst;
        binary_end->add_use_def();
      } else assert(false);
      insns.emplace(it_end, loadimm);
      insns.emplace(it_end, mul);
      if (r_init == r_add) { // change add to mul directly
        loadimm->imm = ConstValue(inst_cnt + 1);
        Reg binary_dst = ((ir::insns::Binary *) it_end->get())->dst;
        copy_propagation(it_end->get()->bb->func->use_list, binary_dst, mul->dst);
      }
      return it_end;
    }
  }
  if (bop == BinaryOp::Add) { // Case 2: sum1 = a + sum0
    auto it_begin = it;
    auto it_end = it;
    int inst_cnt = 1;
    Reg r_init = src2, r_add = src1, r_sum = dst;
    while (it_end != insns.end()) {
      if (it_end->get()->bb->func->use_list.count(r_sum) && it_end->get()->bb->func->use_list.at(r_sum).size() == 1) {
        auto it_nxt = it_end;
        it_nxt++;
        TypeCase(binary, ir::insns::Binary *, it_nxt->get()) {
          if (binary->op != bop) break;
          if (!binary->bb->func->def_list.count(binary->src2) || binary->bb->func->def_list.at(binary->src2) != it_end->get()) break;
          if (r_add != binary->src1) break;
          r_sum = binary->dst;
        } else break;
      } else break;
      it_end++;
      inst_cnt++;
    }
    if (inst_cnt >= OSP_THRESHOLD) {
      while (it_begin != it_end) {
        it_begin->get()->remove_use_def();
        it_begin = insns.erase(it_begin);
      }
      auto loadimm = new ir::insns::LoadImm(it_end->get()->bb->func->new_reg(Int), ConstValue(inst_cnt));
      loadimm->bb = it_end->get()->bb;
      loadimm->add_use_def();
      auto mul = new ir::insns::Binary(it_end->get()->bb->func->new_reg(Int), BinaryOp::Mul, r_add, loadimm->dst);
      mul->bb = it_end->get()->bb;
      mul->add_use_def();
      TypeCase(binary_end, ir::insns::Binary *, it_end->get()) {
        binary_end->remove_use_def();
        binary_end->src1 = r_init;
        binary_end->src2 = mul->dst;
        binary_end->add_use_def();
      } else assert(false);
      insns.emplace(it_end, loadimm);
      insns.emplace(it_end, mul);
      return it_end;
    }
  }
  return it;
}

// Really special case
// In same bb
// Change continuous adds/subs to mul
void operator_strength_promotion(ir::Program *prog) {
  for (auto &func : prog->functions) {
    for (auto &bb : func.second.bbs) {
      for (auto it = bb->insns.begin(); it != bb->insns.end(); it++) {
        it = check_osp(bb->insns, it);
      }
    }
  }
}

} // namespace mediumend