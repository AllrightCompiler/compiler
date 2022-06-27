
// Generated from frontend/SysY.g4 by ANTLR 4.8

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
    virtual antlrcpp::Any visitCompUnit(SysYParser::CompUnitContext *context) = 0;

    virtual antlrcpp::Any visitCompUnitItem(SysYParser::CompUnitItemContext *context) = 0;

    virtual antlrcpp::Any visitDecl(SysYParser::DeclContext *context) = 0;

    virtual antlrcpp::Any visitConstDecl(SysYParser::ConstDeclContext *context) = 0;

    virtual antlrcpp::Any visitInt(SysYParser::IntContext *context) = 0;

    virtual antlrcpp::Any visitFloat(SysYParser::FloatContext *context) = 0;

    virtual antlrcpp::Any visitConstDef(SysYParser::ConstDefContext *context) = 0;

    virtual antlrcpp::Any visitVarDecl(SysYParser::VarDeclContext *context) = 0;

    virtual antlrcpp::Any visitVarDef(SysYParser::VarDefContext *context) = 0;

    virtual antlrcpp::Any visitInit(SysYParser::InitContext *context) = 0;

    virtual antlrcpp::Any visitInitList(SysYParser::InitListContext *context) = 0;

    virtual antlrcpp::Any visitFuncDef(SysYParser::FuncDefContext *context) = 0;

    virtual antlrcpp::Any visitFuncType_(SysYParser::FuncType_Context *context) = 0;

    virtual antlrcpp::Any visitVoid(SysYParser::VoidContext *context) = 0;

    virtual antlrcpp::Any visitFuncFParams(SysYParser::FuncFParamsContext *context) = 0;

    virtual antlrcpp::Any visitScalarParam(SysYParser::ScalarParamContext *context) = 0;

    virtual antlrcpp::Any visitArrayParam(SysYParser::ArrayParamContext *context) = 0;

    virtual antlrcpp::Any visitBlock(SysYParser::BlockContext *context) = 0;

    virtual antlrcpp::Any visitBlockItem(SysYParser::BlockItemContext *context) = 0;

    virtual antlrcpp::Any visitAssign(SysYParser::AssignContext *context) = 0;

    virtual antlrcpp::Any visitExprStmt(SysYParser::ExprStmtContext *context) = 0;

    virtual antlrcpp::Any visitBlockStmt(SysYParser::BlockStmtContext *context) = 0;

    virtual antlrcpp::Any visitIfElse(SysYParser::IfElseContext *context) = 0;

    virtual antlrcpp::Any visitWhile(SysYParser::WhileContext *context) = 0;

    virtual antlrcpp::Any visitBreak(SysYParser::BreakContext *context) = 0;

    virtual antlrcpp::Any visitContinue(SysYParser::ContinueContext *context) = 0;

    virtual antlrcpp::Any visitReturn(SysYParser::ReturnContext *context) = 0;

    virtual antlrcpp::Any visitExp(SysYParser::ExpContext *context) = 0;

    virtual antlrcpp::Any visitCond(SysYParser::CondContext *context) = 0;

    virtual antlrcpp::Any visitLVal(SysYParser::LValContext *context) = 0;

    virtual antlrcpp::Any visitPrimaryExp_(SysYParser::PrimaryExp_Context *context) = 0;

    virtual antlrcpp::Any visitLValExpr(SysYParser::LValExprContext *context) = 0;

    virtual antlrcpp::Any visitDecIntConst(SysYParser::DecIntConstContext *context) = 0;

    virtual antlrcpp::Any visitOctIntConst(SysYParser::OctIntConstContext *context) = 0;

    virtual antlrcpp::Any visitHexIntConst(SysYParser::HexIntConstContext *context) = 0;

    virtual antlrcpp::Any visitDecFloatConst(SysYParser::DecFloatConstContext *context) = 0;

    virtual antlrcpp::Any visitHexFloatConst(SysYParser::HexFloatConstContext *context) = 0;

    virtual antlrcpp::Any visitNumber(SysYParser::NumberContext *context) = 0;

    virtual antlrcpp::Any visitUnaryExp_(SysYParser::UnaryExp_Context *context) = 0;

    virtual antlrcpp::Any visitCall(SysYParser::CallContext *context) = 0;

    virtual antlrcpp::Any visitUnaryAdd(SysYParser::UnaryAddContext *context) = 0;

    virtual antlrcpp::Any visitUnarySub(SysYParser::UnarySubContext *context) = 0;

    virtual antlrcpp::Any visitNot(SysYParser::NotContext *context) = 0;

    virtual antlrcpp::Any visitStringConst(SysYParser::StringConstContext *context) = 0;

    virtual antlrcpp::Any visitFuncRParam(SysYParser::FuncRParamContext *context) = 0;

    virtual antlrcpp::Any visitFuncRParams(SysYParser::FuncRParamsContext *context) = 0;

    virtual antlrcpp::Any visitDiv(SysYParser::DivContext *context) = 0;

    virtual antlrcpp::Any visitMod(SysYParser::ModContext *context) = 0;

    virtual antlrcpp::Any visitMul(SysYParser::MulContext *context) = 0;

    virtual antlrcpp::Any visitMulExp_(SysYParser::MulExp_Context *context) = 0;

    virtual antlrcpp::Any visitAddExp_(SysYParser::AddExp_Context *context) = 0;

    virtual antlrcpp::Any visitAdd(SysYParser::AddContext *context) = 0;

    virtual antlrcpp::Any visitSub(SysYParser::SubContext *context) = 0;

    virtual antlrcpp::Any visitGeq(SysYParser::GeqContext *context) = 0;

    virtual antlrcpp::Any visitLt(SysYParser::LtContext *context) = 0;

    virtual antlrcpp::Any visitRelExp_(SysYParser::RelExp_Context *context) = 0;

    virtual antlrcpp::Any visitLeq(SysYParser::LeqContext *context) = 0;

    virtual antlrcpp::Any visitGt(SysYParser::GtContext *context) = 0;

    virtual antlrcpp::Any visitNeq(SysYParser::NeqContext *context) = 0;

    virtual antlrcpp::Any visitEq(SysYParser::EqContext *context) = 0;

    virtual antlrcpp::Any visitEqExp_(SysYParser::EqExp_Context *context) = 0;

    virtual antlrcpp::Any visitLAndExp_(SysYParser::LAndExp_Context *context) = 0;

    virtual antlrcpp::Any visitAnd(SysYParser::AndContext *context) = 0;

    virtual antlrcpp::Any visitOr(SysYParser::OrContext *context) = 0;

    virtual antlrcpp::Any visitLOrExp_(SysYParser::LOrExp_Context *context) = 0;


};

}  // namespace frontend
