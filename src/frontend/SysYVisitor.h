
// Generated from frontend/SysY.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"
#include "SysYParser.h"


namespace frontend {

/**
 * This class defines an abstract visitor for a parse tree
 * produced by SysYParser.
 */
class  SysYVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SysYParser.
   */
    virtual std::any visitCompUnit(SysYParser::CompUnitContext *context) = 0;

    virtual std::any visitCompUnitItem(SysYParser::CompUnitItemContext *context) = 0;

    virtual std::any visitDecl(SysYParser::DeclContext *context) = 0;

    virtual std::any visitConstDecl(SysYParser::ConstDeclContext *context) = 0;

    virtual std::any visitInt(SysYParser::IntContext *context) = 0;

    virtual std::any visitFloat(SysYParser::FloatContext *context) = 0;

    virtual std::any visitConstDef(SysYParser::ConstDefContext *context) = 0;

    virtual std::any visitVarDecl(SysYParser::VarDeclContext *context) = 0;

    virtual std::any visitVarDef(SysYParser::VarDefContext *context) = 0;

    virtual std::any visitInit(SysYParser::InitContext *context) = 0;

    virtual std::any visitInitList(SysYParser::InitListContext *context) = 0;

    virtual std::any visitFuncDef(SysYParser::FuncDefContext *context) = 0;

    virtual std::any visitFuncType_(SysYParser::FuncType_Context *context) = 0;

    virtual std::any visitVoid(SysYParser::VoidContext *context) = 0;

    virtual std::any visitFuncFParams(SysYParser::FuncFParamsContext *context) = 0;

    virtual std::any visitScalarParam(SysYParser::ScalarParamContext *context) = 0;

    virtual std::any visitArrayParam(SysYParser::ArrayParamContext *context) = 0;

    virtual std::any visitBlock(SysYParser::BlockContext *context) = 0;

    virtual std::any visitBlockItem(SysYParser::BlockItemContext *context) = 0;

    virtual std::any visitAssign(SysYParser::AssignContext *context) = 0;

    virtual std::any visitExprStmt(SysYParser::ExprStmtContext *context) = 0;

    virtual std::any visitBlockStmt(SysYParser::BlockStmtContext *context) = 0;

    virtual std::any visitIfElse(SysYParser::IfElseContext *context) = 0;

    virtual std::any visitWhile(SysYParser::WhileContext *context) = 0;

    virtual std::any visitBreak(SysYParser::BreakContext *context) = 0;

    virtual std::any visitContinue(SysYParser::ContinueContext *context) = 0;

    virtual std::any visitReturn(SysYParser::ReturnContext *context) = 0;

    virtual std::any visitExp(SysYParser::ExpContext *context) = 0;

    virtual std::any visitCond(SysYParser::CondContext *context) = 0;

    virtual std::any visitLVal(SysYParser::LValContext *context) = 0;

    virtual std::any visitPrimaryExp_(SysYParser::PrimaryExp_Context *context) = 0;

    virtual std::any visitLValExpr(SysYParser::LValExprContext *context) = 0;

    virtual std::any visitDecIntConst(SysYParser::DecIntConstContext *context) = 0;

    virtual std::any visitOctIntConst(SysYParser::OctIntConstContext *context) = 0;

    virtual std::any visitHexIntConst(SysYParser::HexIntConstContext *context) = 0;

    virtual std::any visitDecFloatConst(SysYParser::DecFloatConstContext *context) = 0;

    virtual std::any visitHexFloatConst(SysYParser::HexFloatConstContext *context) = 0;

    virtual std::any visitNumber(SysYParser::NumberContext *context) = 0;

    virtual std::any visitUnaryExp_(SysYParser::UnaryExp_Context *context) = 0;

    virtual std::any visitCall(SysYParser::CallContext *context) = 0;

    virtual std::any visitUnaryAdd(SysYParser::UnaryAddContext *context) = 0;

    virtual std::any visitUnarySub(SysYParser::UnarySubContext *context) = 0;

    virtual std::any visitNot(SysYParser::NotContext *context) = 0;

    virtual std::any visitStringConst(SysYParser::StringConstContext *context) = 0;

    virtual std::any visitFuncRParam(SysYParser::FuncRParamContext *context) = 0;

    virtual std::any visitFuncRParams(SysYParser::FuncRParamsContext *context) = 0;

    virtual std::any visitDiv(SysYParser::DivContext *context) = 0;

    virtual std::any visitMod(SysYParser::ModContext *context) = 0;

    virtual std::any visitMul(SysYParser::MulContext *context) = 0;

    virtual std::any visitMulExp_(SysYParser::MulExp_Context *context) = 0;

    virtual std::any visitAddExp_(SysYParser::AddExp_Context *context) = 0;

    virtual std::any visitAdd(SysYParser::AddContext *context) = 0;

    virtual std::any visitSub(SysYParser::SubContext *context) = 0;

    virtual std::any visitGeq(SysYParser::GeqContext *context) = 0;

    virtual std::any visitLt(SysYParser::LtContext *context) = 0;

    virtual std::any visitRelExp_(SysYParser::RelExp_Context *context) = 0;

    virtual std::any visitLeq(SysYParser::LeqContext *context) = 0;

    virtual std::any visitGt(SysYParser::GtContext *context) = 0;

    virtual std::any visitNeq(SysYParser::NeqContext *context) = 0;

    virtual std::any visitEq(SysYParser::EqContext *context) = 0;

    virtual std::any visitEqExp_(SysYParser::EqExp_Context *context) = 0;

    virtual std::any visitLAndExp_(SysYParser::LAndExp_Context *context) = 0;

    virtual std::any visitAnd(SysYParser::AndContext *context) = 0;

    virtual std::any visitOr(SysYParser::OrContext *context) = 0;

    virtual std::any visitLOrExp_(SysYParser::LOrExp_Context *context) = 0;


};

}  // namespace frontend
