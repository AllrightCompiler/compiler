
// Generated from frontend/SysY.g4 by ANTLR 4.8

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

  virtual antlrcpp::Any visitCompUnit(SysYParser::CompUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCompUnitItem(SysYParser::CompUnitItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecl(SysYParser::DeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstDecl(SysYParser::ConstDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInt(SysYParser::IntContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFloat(SysYParser::FloatContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstDef(SysYParser::ConstDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarDecl(SysYParser::VarDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarDef(SysYParser::VarDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInit(SysYParser::InitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInitList(SysYParser::InitListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncDef(SysYParser::FuncDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncType_(SysYParser::FuncType_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVoid(SysYParser::VoidContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncFParams(SysYParser::FuncFParamsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitScalarParam(SysYParser::ScalarParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArrayParam(SysYParser::ArrayParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlock(SysYParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlockItem(SysYParser::BlockItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAssign(SysYParser::AssignContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExprStmt(SysYParser::ExprStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlockStmt(SysYParser::BlockStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIfElse(SysYParser::IfElseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhile(SysYParser::WhileContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBreak(SysYParser::BreakContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitContinue(SysYParser::ContinueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitReturn(SysYParser::ReturnContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExp(SysYParser::ExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCond(SysYParser::CondContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLVal(SysYParser::LValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrimaryExp_(SysYParser::PrimaryExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLValExpr(SysYParser::LValExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecIntConst(SysYParser::DecIntConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOctIntConst(SysYParser::OctIntConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitHexIntConst(SysYParser::HexIntConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecFloatConst(SysYParser::DecFloatConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitHexFloatConst(SysYParser::HexFloatConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNumber(SysYParser::NumberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnaryExp_(SysYParser::UnaryExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCall(SysYParser::CallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnaryAdd(SysYParser::UnaryAddContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnarySub(SysYParser::UnarySubContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNot(SysYParser::NotContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStringConst(SysYParser::StringConstContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncRParam(SysYParser::FuncRParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncRParams(SysYParser::FuncRParamsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDiv(SysYParser::DivContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMod(SysYParser::ModContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMul(SysYParser::MulContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMulExp_(SysYParser::MulExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAddExp_(SysYParser::AddExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAdd(SysYParser::AddContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSub(SysYParser::SubContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGeq(SysYParser::GeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLt(SysYParser::LtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRelExp_(SysYParser::RelExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLeq(SysYParser::LeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGt(SysYParser::GtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNeq(SysYParser::NeqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEq(SysYParser::EqContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEqExp_(SysYParser::EqExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLAndExp_(SysYParser::LAndExp_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAnd(SysYParser::AndContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOr(SysYParser::OrContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLOrExp_(SysYParser::LOrExp_Context *ctx) override {
    return visitChildren(ctx);
  }


};

}  // namespace frontend
