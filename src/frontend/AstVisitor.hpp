#pragma once

#include "frontend/SysYBaseVisitor.h"
#include "frontend/SysYParser.h"
#include "frontend/ast.hpp"

namespace frontend {

using namespace ast;

class AstVisitor : public SysYBaseVisitor {
public:
  CompileUnit const &compileUnit() const;

  std::any visitCompUnit(SysYParser::CompUnitContext *const ctx) override;

  std::any visitConstDecl(SysYParser::ConstDeclContext *const ctx) override;

  std::any visitInt(SysYParser::IntContext *const ctx) override;

  std::any visitFloat(SysYParser::FloatContext *const ctx) override;

  std::any visitVarDecl(SysYParser::VarDeclContext *const ctx) override;

  std::any visitInit(SysYParser::InitContext *const ctx) override;

  std::any visitInitList(SysYParser::InitListContext *const ctx) override;

  std::any visitFuncDef(SysYParser::FuncDefContext *const ctx) override;

  std::any visitVoid(SysYParser::VoidContext *const ctx) override;

  std::any visitScalarParam(SysYParser::ScalarParamContext *const ctx) override;

  std::any visitArrayParam(SysYParser::ArrayParamContext *const ctx) override;

  std::any visitBlock(SysYParser::BlockContext *const ctx) override;

  std::any visitAssign(SysYParser::AssignContext *const ctx) override;

  std::any visitExprStmt(SysYParser::ExprStmtContext *const ctx) override;

  std::any visitBlockStmt(SysYParser::BlockStmtContext *const ctx) override;

  std::any visitIfElse(SysYParser::IfElseContext *const ctx) override;

  std::any visitWhile(SysYParser::WhileContext *const ctx) override;

  std::any visitBreak(SysYParser::BreakContext *const ctx) override;

  std::any visitContinue(SysYParser::ContinueContext *const ctx) override;

  std::any visitReturn(SysYParser::ReturnContext *const ctx) override;

  std::any visitLVal(SysYParser::LValContext *const ctx) override;

  std::any visitLValExpr(SysYParser::LValExprContext *const ctx) override;

  std::any visitDecIntConst(SysYParser::DecIntConstContext *const ctx) override;

  std::any visitOctIntConst(SysYParser::OctIntConstContext *const ctx) override;

  std::any visitHexIntConst(SysYParser::HexIntConstContext *const ctx) override;

  std::any
  visitDecFloatConst(SysYParser::DecFloatConstContext *const ctx) override;

  std::any
  visitHexFloatConst(SysYParser::HexFloatConstContext *const ctx) override;

  std::any visitCall(SysYParser::CallContext *const ctx) override;

  std::any visitUnaryAdd(SysYParser::UnaryAddContext *const ctx) override;

  std::any visitUnarySub(SysYParser::UnarySubContext *const ctx) override;

  std::any visitNot(SysYParser::NotContext *const ctx) override;

  std::any visitStringConst(SysYParser::StringConstContext *const ctx) override;

  std::any visitFuncRParam(SysYParser::FuncRParamContext *const ctx) override;

  std::any visitFuncRParams(SysYParser::FuncRParamsContext *const ctx) override;

  std::any visitDiv(SysYParser::DivContext *const ctx) override;

  std::any visitMod(SysYParser::ModContext *const ctx) override;

  std::any visitMul(SysYParser::MulContext *const ctx) override;

  std::any visitAdd(SysYParser::AddContext *const ctx) override;

  std::any visitSub(SysYParser::SubContext *const ctx) override;

  std::any visitGeq(SysYParser::GeqContext *const ctx) override;

  std::any visitLt(SysYParser::LtContext *const ctx) override;

  std::any visitLeq(SysYParser::LeqContext *const ctx) override;

  std::any visitGt(SysYParser::GtContext *const ctx) override;

  std::any visitNeq(SysYParser::NeqContext *const ctx) override;

  std::any visitEq(SysYParser::EqContext *const ctx) override;

  std::any visitAnd(SysYParser::AndContext *const ctx) override;

  std::any visitOr(SysYParser::OrContext *const ctx) override;

  std::any visitNumber(SysYParser::NumberContext *const ctx) override;

private:
  std::vector<std::unique_ptr<Expression>>
    visitDimensions(const std::vector<SysYParser::ExpContext *> &ctxs);

  std::unique_ptr<CompileUnit> m_compile_unit;
};

} // namespace frontend
