#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::Reg;

    void gep_destruction(ir::Function *func) {
        for (auto &bb : func->bbs) {
            for (auto it = bb->insns.begin(); it != bb->insns.end(); ) {
                TypeCase(gep, ir::insns::GetElementPtr *, it->get()) {
                    std::vector<ir::Instruction *> insts_to_insert;
                    Reg index_reg = gep->indices[0];
                    int n_indices = gep->indices.size();
                    for (int i = 1; i < n_indices; i++) {
                        int dim_len = gep->type.dims[i];
                        Reg imm = func->new_reg(Int);
                        insts_to_insert.push_back(new ir::insns::LoadImm(imm, ConstValue(dim_len)));
                        Reg mul_res = func->new_reg(Int);
                        insts_to_insert.push_back(new ir::insns::Binary(mul_res, BinaryOp::Mul, index_reg, imm));
                        Reg t = func->new_reg(Int);
                        insts_to_insert.push_back(new ir::insns::Binary(t, BinaryOp::Add, mul_res, gep->indices[i]));
                        index_reg = t;
                    }
                    for (int i = n_indices; i < gep->type.dims.size(); i++) {
                        int dim_len = gep->type.dims[i];
                        Reg imm = func->new_reg(Int);
                        insts_to_insert.push_back(new ir::insns::LoadImm(imm, ConstValue(dim_len)));
                        Reg t = func->new_reg(Int);
                        insts_to_insert.push_back(new ir::insns::Binary(t, BinaryOp::Mul, index_reg, imm));
                        index_reg = t;
                    }
                    Reg imm_4 = func->new_reg(Int);
                    insts_to_insert.push_back(new ir::insns::LoadImm(imm_4, ConstValue(4)));
                    Reg mul_res = func->new_reg(Int);
                    insts_to_insert.push_back(new ir::insns::Binary(mul_res, BinaryOp::Mul, index_reg, imm_4));
                    insts_to_insert.push_back(new ir::insns::Binary(gep->dst, BinaryOp::Add, mul_res, gep->base));
                    auto pos = bb->remove_at(it);
                    for (auto i : insts_to_insert) {
                        bb->insert_at(pos, i);
                    }
                    it = pos;
                } else it++;
            }
        }
    }

    void gep_destruction(ir::Program *prog) {
        for (auto &func : prog->functions) {
            gep_destruction(&func.second);
        }
    }

}