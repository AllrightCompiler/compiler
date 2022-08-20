#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

#include <cassert>

namespace mediumend {

ConstValue const_compute(ir::Instruction *inst, const ConstValue &oprand) {
  ConstValue res;
  TypeCase(unary, ir::insns::Unary *, inst) {
    res.type = unary->dst.type;
    assert(unary->src.type == oprand.type);
    switch (unary->op) {
      case UnaryOp::Not:
        if (unary->dst.type == ScalarType::Int) res.iv = !oprand.iv;
        else res.fv = !oprand.fv;
        break;
      case UnaryOp::Sub:
        if (unary->dst.type == ScalarType::Int) res.iv = -oprand.iv;
        else res.fv = -oprand.fv;
        break;
      default:
        assert(false);
    }
  } else TypeCase(convert, ir::insns::Convert *, inst) {
    res.type = convert->dst.type;
    assert(convert->src.type == oprand.type);
    if (convert->dst.type == ScalarType::Int){
      if (convert->src.type == ScalarType::Int) res.iv = oprand.iv;
      else res.iv = (int)oprand.fv;
    } else {
      if (convert->src.type == ScalarType::Int) res.fv = (float)oprand.iv;
      else res.fv = oprand.fv;
    }
  } else assert(false);
  return res;
}

ConstValue const_compute(ir::Instruction *inst, const ConstValue &op1, const ConstValue &op2) {
  ConstValue res;
  TypeCase(binary, ir::insns::Binary *, inst) {
    res.type = binary->dst.type;
    assert(binary->src1.type == op1.type && binary->src2.type == op2.type);
    assert(binary->src1.type == binary->src2.type);
    switch (binary->op) {
      case BinaryOp::Add:
        if (binary->dst.type == ScalarType::Int) res.iv = op1.iv + op2.iv;
        else res.fv = op1.fv + op2.fv;
        break;
      case BinaryOp::Sub:
        if (binary->dst.type == ScalarType::Int) res.iv = op1.iv - op2.iv;
        else res.fv = op1.fv - op2.fv;
        break;
      case BinaryOp::Mul:
        if (binary->dst.type == ScalarType::Int) res.iv = op1.iv * op2.iv;
        else res.fv = op1.fv * op2.fv;
        break;
      case BinaryOp::Div:
        if (binary->dst.type == ScalarType::Int) res.iv = op1.iv / op2.iv;
        else res.fv = op1.fv / op2.fv;
        break;
      case BinaryOp::Mod:
        assert(binary->dst.type == ScalarType::Int);
        res.iv = op1.iv % op2.iv;
        break;
      // TODO:对于EQ和NEQ的处理需要再考虑一下？
      case BinaryOp::Eq:
        if (binary->src1.type == ScalarType::Int) res.iv = (op1.iv == op2.iv);
        else res.iv = (op1.fv == op2.fv);
        break;
      case BinaryOp::Neq:
        if (binary->src1.type == ScalarType::Int) res.iv = (op1.iv != op2.iv);
        else res.iv = (op1.fv != op2.fv);
        break;
      case BinaryOp::Lt:
        if (binary->src1.type == ScalarType::Int) res.iv = (op1.iv < op2.iv);
        else res.iv = (op1.fv < op2.fv);
        break;
      case BinaryOp::Gt:
        if (binary->src1.type == ScalarType::Int) res.iv = (op1.iv > op2.iv);
        else res.iv = (op1.fv > op2.fv);
        break;
      case BinaryOp::Leq:
        if (binary->src1.type == ScalarType::Int) res.iv = (op1.iv <= op2.iv);
        else res.iv = (op1.fv <= op2.fv);
        break;
      case BinaryOp::Geq:
        if (binary->src1.type == ScalarType::Int) res.iv = (op1.iv >= op2.iv);
        else res.iv = (op1.fv >= op2.fv);
        break;
      case BinaryOp::Shl:
        assert(binary->dst.type == ScalarType::Int);
        res.iv = op1.iv << op2.iv;
        break;
      case BinaryOp::Shr:
        assert(binary->dst.type == ScalarType::Int);
        res.iv = op1.iv >> op2.iv;
        break;
      default:
        assert(false);
    }
  } else assert(false);
  return res;
}

}