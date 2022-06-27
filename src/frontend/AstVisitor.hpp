#pragma once

#include "frontend/SysYBaseVisitor.h"
#include "frontend/SysYParser.h"
#include "frontend/ast.hpp"

namespace frontend {

using namespace ast;

class AstVisitor : public SysYBaseVisitor {
public:
  [[nodiscard]] CompileUnit const &compileUnit() const;

  antlrcpp::Any visitCompUnit(SysYParser::CompUnitContext *ctx) override;

  antlrcpp::Any visitConstDecl(SysYParser::ConstDeclContext *ctx) override;

  antlrcpp::Any visitInt(SysYParser::IntContext *ctx) override;

  antlrcpp::Any visitFloat(SysYParser::FloatContext *ctx) override;

  antlrcpp::Any visitVarDecl(SysYParser::VarDeclContext *ctx) override;

  antlrcpp::Any visitInit(SysYParser::InitContext *ctx) override;

  antlrcpp::Any visitInitList(SysYParser::InitListContext *ctx) override;

  antlrcpp::Any visitFuncDef(SysYParser::FuncDefContext *ctx) override;

  antlrcpp::Any visitVoid(SysYParser::VoidContext *ctx) override;

  antlrcpp::Any visitScalarParam(SysYParser::ScalarParamContext *ctx) override;

  antlrcpp::Any visitArrayParam(SysYParser::ArrayParamContext *ctx) override;

  antlrcpp::Any visitBlock(SysYParser::BlockContext *ctx) override;

  antlrcpp::Any visitAssign(SysYParser::AssignContext *ctx) override;

  antlrcpp::Any visitExprStmt(SysYParser::ExprStmtContext *ctx) override;

  antlrcpp::Any visitBlockStmt(SysYParser::BlockStmtContext *ctx) override;

  antlrcpp::Any visitIfElse(SysYParser::IfElseContext *ctx) override;

  antlrcpp::Any visitWhile(SysYParser::WhileContext *ctx) override;

  antlrcpp::Any visitBreak(SysYParser::BreakContext *ctx) override;

  antlrcpp::Any visitContinue(SysYParser::ContinueContext *ctx) override;

  antlrcpp::Any visitReturn(SysYParser::ReturnContext *ctx) override;

  antlrcpp::Any visitLVal(SysYParser::LValContext *ctx) override;

  antlrcpp::Any visitLValExpr(SysYParser::LValExprContext *ctx) override;

  antlrcpp::Any visitDecIntConst(SysYParser::DecIntConstContext *ctx) override;

  antlrcpp::Any visitOctIntConst(SysYParser::OctIntConstContext *ctx) override;

  antlrcpp::Any visitHexIntConst(SysYParser::HexIntConstContext *ctx) override;

  antlrcpp::Any
  visitDecFloatConst(SysYParser::DecFloatConstContext *ctx) override;

  antlrcpp::Any
  visitHexFloatConst(SysYParser::HexFloatConstContext *ctx) override;

  antlrcpp::Any visitCall(SysYParser::CallContext *ctx) override;

  antlrcpp::Any visitUnaryAdd(SysYParser::UnaryAddContext *ctx) override;

  antlrcpp::Any visitUnarySub(SysYParser::UnarySubContext *ctx) override;

  antlrcpp::Any visitNot(SysYParser::NotContext *ctx) override;

  antlrcpp::Any visitStringConst(SysYParser::StringConstContext *ctx) override;

  antlrcpp::Any visitFuncRParam(SysYParser::FuncRParamContext *ctx) override;

  antlrcpp::Any visitFuncRParams(SysYParser::FuncRParamsContext *ctx) override;

  antlrcpp::Any visitDiv(SysYParser::DivContext *ctx) override;

  antlrcpp::Any visitMod(SysYParser::ModContext *ctx) override;

  antlrcpp::Any visitMul(SysYParser::MulContext *ctx) override;

  antlrcpp::Any visitAdd(SysYParser::AddContext *ctx) override;

  antlrcpp::Any visitSub(SysYParser::SubContext *ctx) override;

  antlrcpp::Any visitGeq(SysYParser::GeqContext *ctx) override;

  antlrcpp::Any visitLt(SysYParser::LtContext *ctx) override;

  antlrcpp::Any visitLeq(SysYParser::LeqContext *ctx) override;

  antlrcpp::Any visitGt(SysYParser::GtContext *ctx) override;

  antlrcpp::Any visitNeq(SysYParser::NeqContext *ctx) override;

  antlrcpp::Any visitEq(SysYParser::EqContext *ctx) override;

  antlrcpp::Any visitAnd(SysYParser::AndContext *ctx) override;

  antlrcpp::Any visitOr(SysYParser::OrContext *ctx) override;

  antlrcpp::Any visitNumber(SysYParser::NumberContext *ctx) override;

private:
  std::vector<std::unique_ptr<Expression>>
  visitDimensions(const std::vector<SysYParser::ExpContext *> &ctxs);

  std::unique_ptr<CompileUnit> m_compile_unit;
};

} // namespace frontend
