
// Generated from frontend/SysY.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"
#include "SysYVisitor.h"


namespace frontend {

/**
 * This class provides an empty implementation of SysYVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SysYBaseVisitor : public SysYVisitor {
public:

  virtual std::any visitCompUnit(SysYParser::CompUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompUnitItem(SysYParser::CompUnitItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDecl(SysYParser::DeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstDecl(SysYParser::ConstDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInt(SysYParser::IntContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFloat(SysYParser::FloatContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstDef(SysYParser::ConstDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarDecl(SysYParser::VarDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarDef(SysYParser::VarDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInit(SysYParser::InitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitList(SysYParser::InitListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncDef(SysYParser::FuncDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncType_(SysYParser::FuncType_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVoid(SysYParser::VoidContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncFParams(SysYParser::FuncFParamsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitScalarParam(SysYParser::ScalarParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrayParam(SysYParser::ArrayParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlock(SysYParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlockItem(SysYParser::BlockItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssign(SysYParser::AssignContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExprStmt(SysYParser::ExprStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlockStmt(SysYParser::BlockStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfElse(SysYParser::IfElseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhile(SysYParser::WhileContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBreak(SysYParser::BreakContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitContinue(SysYParser::ContinueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturn(SysYParser::ReturnContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExp(SysYParser::ExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCond(SysYParser::CondContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLVal(SysYParser::LValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExp_(SysYParser::PrimaryExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLValExpr(SysYParser::LValExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDecIntConst(SysYParser::DecIntConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOctIntConst(SysYParser::OctIntConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitHexIntConst(SysYParser::HexIntConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDecFloatConst(SysYParser::DecFloatConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitHexFloatConst(SysYParser::HexFloatConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNumber(SysYParser::NumberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExp_(SysYParser::UnaryExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCall(SysYParser::CallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryAdd(SysYParser::UnaryAddContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnarySub(SysYParser::UnarySubContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNot(SysYParser::NotContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStringConst(SysYParser::StringConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncRParam(SysYParser::FuncRParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncRParams(SysYParser::FuncRParamsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDiv(SysYParser::DivContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMod(SysYParser::ModContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMul(SysYParser::MulContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMulExp_(SysYParser::MulExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddExp_(SysYParser::AddExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdd(SysYParser::AddContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSub(SysYParser::SubContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGeq(SysYParser::GeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLt(SysYParser::LtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelExp_(SysYParser::RelExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLeq(SysYParser::LeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGt(SysYParser::GtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNeq(SysYParser::NeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEq(SysYParser::EqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqExp_(SysYParser::EqExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLAndExp_(SysYParser::LAndExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAnd(SysYParser::AndContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOr(SysYParser::OrContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLOrExp_(SysYParser::LOrExp_Context *ctx) override {
    return visitChildren(ctx);
  }


};

}  // namespace frontend
