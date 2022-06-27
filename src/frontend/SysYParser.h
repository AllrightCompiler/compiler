
// Generated from frontend/SysY.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"


namespace frontend {


class  SysYParser : public antlr4::Parser {
public:
  enum {
    Int = 1, Float = 2, Void = 3, Const = 4, If = 5, Else = 6, While = 7, 
    Break = 8, Continue = 9, Return = 10, Assign = 11, Add = 12, Sub = 13, 
    Mul = 14, Div = 15, Mod = 16, Eq = 17, Neq = 18, Lt = 19, Gt = 20, Leq = 21, 
    Geq = 22, Not = 23, And = 24, Or = 25, Comma = 26, Semicolon = 27, Lparen = 28, 
    Rparen = 29, Lbracket = 30, Rbracket = 31, Lbrace = 32, Rbrace = 33, 
    Ident = 34, Whitespace = 35, LineComment = 36, BlockComment = 37, DecIntConst = 38, 
    OctIntConst = 39, HexIntConst = 40, DecFloatConst = 41, HexFloatConst = 42, 
    StringConst = 43
  };

  enum {
    RuleCompUnit = 0, RuleCompUnitItem = 1, RuleDecl = 2, RuleConstDecl = 3, 
    RuleBType = 4, RuleConstDef = 5, RuleVarDecl = 6, RuleVarDef = 7, RuleInitVal = 8, 
    RuleFuncDef = 9, RuleFuncType = 10, RuleFuncFParams = 11, RuleFuncFParam = 12, 
    RuleBlock = 13, RuleBlockItem = 14, RuleStmt = 15, RuleExp = 16, RuleCond = 17, 
    RuleLVal = 18, RulePrimaryExp = 19, RuleIntConst = 20, RuleFloatConst = 21, 
    RuleNumber = 22, RuleUnaryExp = 23, RuleStringConst = 24, RuleFuncRParam = 25, 
    RuleFuncRParams = 26, RuleMulExp = 27, RuleAddExp = 28, RuleRelExp = 29, 
    RuleEqExp = 30, RuleLAndExp = 31, RuleLOrExp = 32
  };

  SysYParser(antlr4::TokenStream *input);
  ~SysYParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class CompUnitContext;
  class CompUnitItemContext;
  class DeclContext;
  class ConstDeclContext;
  class BTypeContext;
  class ConstDefContext;
  class VarDeclContext;
  class VarDefContext;
  class InitValContext;
  class FuncDefContext;
  class FuncTypeContext;
  class FuncFParamsContext;
  class FuncFParamContext;
  class BlockContext;
  class BlockItemContext;
  class StmtContext;
  class ExpContext;
  class CondContext;
  class LValContext;
  class PrimaryExpContext;
  class IntConstContext;
  class FloatConstContext;
  class NumberContext;
  class UnaryExpContext;
  class StringConstContext;
  class FuncRParamContext;
  class FuncRParamsContext;
  class MulExpContext;
  class AddExpContext;
  class RelExpContext;
  class EqExpContext;
  class LAndExpContext;
  class LOrExpContext; 

  class  CompUnitContext : public antlr4::ParserRuleContext {
  public:
    CompUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<CompUnitItemContext *> compUnitItem();
    CompUnitItemContext* compUnitItem(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CompUnitContext* compUnit();

  class  CompUnitItemContext : public antlr4::ParserRuleContext {
  public:
    CompUnitItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DeclContext *decl();
    FuncDefContext *funcDef();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CompUnitItemContext* compUnitItem();

  class  DeclContext : public antlr4::ParserRuleContext {
  public:
    DeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ConstDeclContext *constDecl();
    VarDeclContext *varDecl();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeclContext* decl();

  class  ConstDeclContext : public antlr4::ParserRuleContext {
  public:
    ConstDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Const();
    BTypeContext *bType();
    std::vector<ConstDefContext *> constDef();
    ConstDefContext* constDef(size_t i);
    antlr4::tree::TerminalNode *Semicolon();
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstDeclContext* constDecl();

  class  BTypeContext : public antlr4::ParserRuleContext {
  public:
    BTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    BTypeContext() = default;
    void copyFrom(BTypeContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  FloatContext : public BTypeContext {
  public:
    FloatContext(BTypeContext *ctx);

    antlr4::tree::TerminalNode *Float();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  IntContext : public BTypeContext {
  public:
    IntContext(BTypeContext *ctx);

    antlr4::tree::TerminalNode *Int();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  BTypeContext* bType();

  class  ConstDefContext : public antlr4::ParserRuleContext {
  public:
    ConstDefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Ident();
    antlr4::tree::TerminalNode *Assign();
    InitValContext *initVal();
    std::vector<antlr4::tree::TerminalNode *> Lbracket();
    antlr4::tree::TerminalNode* Lbracket(size_t i);
    std::vector<ExpContext *> exp();
    ExpContext* exp(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Rbracket();
    antlr4::tree::TerminalNode* Rbracket(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstDefContext* constDef();

  class  VarDeclContext : public antlr4::ParserRuleContext {
  public:
    VarDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    BTypeContext *bType();
    std::vector<VarDefContext *> varDef();
    VarDefContext* varDef(size_t i);
    antlr4::tree::TerminalNode *Semicolon();
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarDeclContext* varDecl();

  class  VarDefContext : public antlr4::ParserRuleContext {
  public:
    VarDefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Ident();
    std::vector<antlr4::tree::TerminalNode *> Lbracket();
    antlr4::tree::TerminalNode* Lbracket(size_t i);
    std::vector<ExpContext *> exp();
    ExpContext* exp(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Rbracket();
    antlr4::tree::TerminalNode* Rbracket(size_t i);
    antlr4::tree::TerminalNode *Assign();
    InitValContext *initVal();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarDefContext* varDef();

  class  InitValContext : public antlr4::ParserRuleContext {
  public:
    InitValContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    InitValContext() = default;
    void copyFrom(InitValContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  InitContext : public InitValContext {
  public:
    InitContext(InitValContext *ctx);

    ExpContext *exp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  InitListContext : public InitValContext {
  public:
    InitListContext(InitValContext *ctx);

    antlr4::tree::TerminalNode *Lbrace();
    antlr4::tree::TerminalNode *Rbrace();
    std::vector<InitValContext *> initVal();
    InitValContext* initVal(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  InitValContext* initVal();

  class  FuncDefContext : public antlr4::ParserRuleContext {
  public:
    FuncDefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    FuncTypeContext *funcType();
    antlr4::tree::TerminalNode *Ident();
    antlr4::tree::TerminalNode *Lparen();
    antlr4::tree::TerminalNode *Rparen();
    BlockContext *block();
    FuncFParamsContext *funcFParams();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FuncDefContext* funcDef();

  class  FuncTypeContext : public antlr4::ParserRuleContext {
  public:
    FuncTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    FuncTypeContext() = default;
    void copyFrom(FuncTypeContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  FuncType_Context : public FuncTypeContext {
  public:
    FuncType_Context(FuncTypeContext *ctx);

    BTypeContext *bType();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  VoidContext : public FuncTypeContext {
  public:
    VoidContext(FuncTypeContext *ctx);

    antlr4::tree::TerminalNode *Void();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  FuncTypeContext* funcType();

  class  FuncFParamsContext : public antlr4::ParserRuleContext {
  public:
    FuncFParamsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<FuncFParamContext *> funcFParam();
    FuncFParamContext* funcFParam(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FuncFParamsContext* funcFParams();

  class  FuncFParamContext : public antlr4::ParserRuleContext {
  public:
    FuncFParamContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    FuncFParamContext() = default;
    void copyFrom(FuncFParamContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ScalarParamContext : public FuncFParamContext {
  public:
    ScalarParamContext(FuncFParamContext *ctx);

    BTypeContext *bType();
    antlr4::tree::TerminalNode *Ident();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ArrayParamContext : public FuncFParamContext {
  public:
    ArrayParamContext(FuncFParamContext *ctx);

    BTypeContext *bType();
    antlr4::tree::TerminalNode *Ident();
    std::vector<antlr4::tree::TerminalNode *> Lbracket();
    antlr4::tree::TerminalNode* Lbracket(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Rbracket();
    antlr4::tree::TerminalNode* Rbracket(size_t i);
    std::vector<ExpContext *> exp();
    ExpContext* exp(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  FuncFParamContext* funcFParam();

  class  BlockContext : public antlr4::ParserRuleContext {
  public:
    BlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Lbrace();
    antlr4::tree::TerminalNode *Rbrace();
    std::vector<BlockItemContext *> blockItem();
    BlockItemContext* blockItem(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlockContext* block();

  class  BlockItemContext : public antlr4::ParserRuleContext {
  public:
    BlockItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DeclContext *decl();
    StmtContext *stmt();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlockItemContext* blockItem();

  class  StmtContext : public antlr4::ParserRuleContext {
  public:
    StmtContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    StmtContext() = default;
    void copyFrom(StmtContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ExprStmtContext : public StmtContext {
  public:
    ExprStmtContext(StmtContext *ctx);

    antlr4::tree::TerminalNode *Semicolon();
    ExpContext *exp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  BlockStmtContext : public StmtContext {
  public:
    BlockStmtContext(StmtContext *ctx);

    BlockContext *block();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  BreakContext : public StmtContext {
  public:
    BreakContext(StmtContext *ctx);

    antlr4::tree::TerminalNode *Break();
    antlr4::tree::TerminalNode *Semicolon();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ContinueContext : public StmtContext {
  public:
    ContinueContext(StmtContext *ctx);

    antlr4::tree::TerminalNode *Continue();
    antlr4::tree::TerminalNode *Semicolon();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  WhileContext : public StmtContext {
  public:
    WhileContext(StmtContext *ctx);

    antlr4::tree::TerminalNode *While();
    antlr4::tree::TerminalNode *Lparen();
    CondContext *cond();
    antlr4::tree::TerminalNode *Rparen();
    StmtContext *stmt();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  IfElseContext : public StmtContext {
  public:
    IfElseContext(StmtContext *ctx);

    antlr4::tree::TerminalNode *If();
    antlr4::tree::TerminalNode *Lparen();
    CondContext *cond();
    antlr4::tree::TerminalNode *Rparen();
    std::vector<StmtContext *> stmt();
    StmtContext* stmt(size_t i);
    antlr4::tree::TerminalNode *Else();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ReturnContext : public StmtContext {
  public:
    ReturnContext(StmtContext *ctx);

    antlr4::tree::TerminalNode *Return();
    antlr4::tree::TerminalNode *Semicolon();
    ExpContext *exp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AssignContext : public StmtContext {
  public:
    AssignContext(StmtContext *ctx);

    LValContext *lVal();
    antlr4::tree::TerminalNode *Assign();
    ExpContext *exp();
    antlr4::tree::TerminalNode *Semicolon();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  StmtContext* stmt();

  class  ExpContext : public antlr4::ParserRuleContext {
  public:
    ExpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AddExpContext *addExp();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpContext* exp();

  class  CondContext : public antlr4::ParserRuleContext {
  public:
    CondContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LOrExpContext *lOrExp();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CondContext* cond();

  class  LValContext : public antlr4::ParserRuleContext {
  public:
    LValContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Ident();
    std::vector<antlr4::tree::TerminalNode *> Lbracket();
    antlr4::tree::TerminalNode* Lbracket(size_t i);
    std::vector<ExpContext *> exp();
    ExpContext* exp(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Rbracket();
    antlr4::tree::TerminalNode* Rbracket(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LValContext* lVal();

  class  PrimaryExpContext : public antlr4::ParserRuleContext {
  public:
    PrimaryExpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    PrimaryExpContext() = default;
    void copyFrom(PrimaryExpContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  PrimaryExp_Context : public PrimaryExpContext {
  public:
    PrimaryExp_Context(PrimaryExpContext *ctx);

    antlr4::tree::TerminalNode *Lparen();
    ExpContext *exp();
    antlr4::tree::TerminalNode *Rparen();
    NumberContext *number();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LValExprContext : public PrimaryExpContext {
  public:
    LValExprContext(PrimaryExpContext *ctx);

    LValContext *lVal();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  PrimaryExpContext* primaryExp();

  class  IntConstContext : public antlr4::ParserRuleContext {
  public:
    IntConstContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    IntConstContext() = default;
    void copyFrom(IntConstContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  HexIntConstContext : public IntConstContext {
  public:
    HexIntConstContext(IntConstContext *ctx);

    antlr4::tree::TerminalNode *HexIntConst();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  DecIntConstContext : public IntConstContext {
  public:
    DecIntConstContext(IntConstContext *ctx);

    antlr4::tree::TerminalNode *DecIntConst();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  OctIntConstContext : public IntConstContext {
  public:
    OctIntConstContext(IntConstContext *ctx);

    antlr4::tree::TerminalNode *OctIntConst();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  IntConstContext* intConst();

  class  FloatConstContext : public antlr4::ParserRuleContext {
  public:
    FloatConstContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    FloatConstContext() = default;
    void copyFrom(FloatConstContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  DecFloatConstContext : public FloatConstContext {
  public:
    DecFloatConstContext(FloatConstContext *ctx);

    antlr4::tree::TerminalNode *DecFloatConst();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  HexFloatConstContext : public FloatConstContext {
  public:
    HexFloatConstContext(FloatConstContext *ctx);

    antlr4::tree::TerminalNode *HexFloatConst();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  FloatConstContext* floatConst();

  class  NumberContext : public antlr4::ParserRuleContext {
  public:
    NumberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IntConstContext *intConst();
    FloatConstContext *floatConst();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NumberContext* number();

  class  UnaryExpContext : public antlr4::ParserRuleContext {
  public:
    UnaryExpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    UnaryExpContext() = default;
    void copyFrom(UnaryExpContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  CallContext : public UnaryExpContext {
  public:
    CallContext(UnaryExpContext *ctx);

    antlr4::tree::TerminalNode *Ident();
    antlr4::tree::TerminalNode *Lparen();
    antlr4::tree::TerminalNode *Rparen();
    FuncRParamsContext *funcRParams();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NotContext : public UnaryExpContext {
  public:
    NotContext(UnaryExpContext *ctx);

    antlr4::tree::TerminalNode *Not();
    UnaryExpContext *unaryExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  UnaryExp_Context : public UnaryExpContext {
  public:
    UnaryExp_Context(UnaryExpContext *ctx);

    PrimaryExpContext *primaryExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  UnaryAddContext : public UnaryExpContext {
  public:
    UnaryAddContext(UnaryExpContext *ctx);

    antlr4::tree::TerminalNode *Add();
    UnaryExpContext *unaryExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  UnarySubContext : public UnaryExpContext {
  public:
    UnarySubContext(UnaryExpContext *ctx);

    antlr4::tree::TerminalNode *Sub();
    UnaryExpContext *unaryExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  UnaryExpContext* unaryExp();

  class  StringConstContext : public antlr4::ParserRuleContext {
  public:
    StringConstContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *StringConst();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StringConstContext* stringConst();

  class  FuncRParamContext : public antlr4::ParserRuleContext {
  public:
    FuncRParamContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpContext *exp();
    StringConstContext *stringConst();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FuncRParamContext* funcRParam();

  class  FuncRParamsContext : public antlr4::ParserRuleContext {
  public:
    FuncRParamsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<FuncRParamContext *> funcRParam();
    FuncRParamContext* funcRParam(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FuncRParamsContext* funcRParams();

  class  MulExpContext : public antlr4::ParserRuleContext {
  public:
    MulExpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    MulExpContext() = default;
    void copyFrom(MulExpContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  DivContext : public MulExpContext {
  public:
    DivContext(MulExpContext *ctx);

    MulExpContext *mulExp();
    antlr4::tree::TerminalNode *Div();
    UnaryExpContext *unaryExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ModContext : public MulExpContext {
  public:
    ModContext(MulExpContext *ctx);

    MulExpContext *mulExp();
    antlr4::tree::TerminalNode *Mod();
    UnaryExpContext *unaryExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MulContext : public MulExpContext {
  public:
    MulContext(MulExpContext *ctx);

    MulExpContext *mulExp();
    antlr4::tree::TerminalNode *Mul();
    UnaryExpContext *unaryExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MulExp_Context : public MulExpContext {
  public:
    MulExp_Context(MulExpContext *ctx);

    UnaryExpContext *unaryExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  MulExpContext* mulExp();
  MulExpContext* mulExp(int precedence);
  class  AddExpContext : public antlr4::ParserRuleContext {
  public:
    AddExpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    AddExpContext() = default;
    void copyFrom(AddExpContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  AddExp_Context : public AddExpContext {
  public:
    AddExp_Context(AddExpContext *ctx);

    MulExpContext *mulExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AddContext : public AddExpContext {
  public:
    AddContext(AddExpContext *ctx);

    AddExpContext *addExp();
    antlr4::tree::TerminalNode *Add();
    MulExpContext *mulExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SubContext : public AddExpContext {
  public:
    SubContext(AddExpContext *ctx);

    AddExpContext *addExp();
    antlr4::tree::TerminalNode *Sub();
    MulExpContext *mulExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  AddExpContext* addExp();
  AddExpContext* addExp(int precedence);
  class  RelExpContext : public antlr4::ParserRuleContext {
  public:
    RelExpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    RelExpContext() = default;
    void copyFrom(RelExpContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  GeqContext : public RelExpContext {
  public:
    GeqContext(RelExpContext *ctx);

    RelExpContext *relExp();
    antlr4::tree::TerminalNode *Geq();
    AddExpContext *addExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LtContext : public RelExpContext {
  public:
    LtContext(RelExpContext *ctx);

    RelExpContext *relExp();
    antlr4::tree::TerminalNode *Lt();
    AddExpContext *addExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  RelExp_Context : public RelExpContext {
  public:
    RelExp_Context(RelExpContext *ctx);

    AddExpContext *addExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LeqContext : public RelExpContext {
  public:
    LeqContext(RelExpContext *ctx);

    RelExpContext *relExp();
    antlr4::tree::TerminalNode *Leq();
    AddExpContext *addExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  GtContext : public RelExpContext {
  public:
    GtContext(RelExpContext *ctx);

    RelExpContext *relExp();
    antlr4::tree::TerminalNode *Gt();
    AddExpContext *addExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  RelExpContext* relExp();
  RelExpContext* relExp(int precedence);
  class  EqExpContext : public antlr4::ParserRuleContext {
  public:
    EqExpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    EqExpContext() = default;
    void copyFrom(EqExpContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  NeqContext : public EqExpContext {
  public:
    NeqContext(EqExpContext *ctx);

    EqExpContext *eqExp();
    antlr4::tree::TerminalNode *Neq();
    RelExpContext *relExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  EqContext : public EqExpContext {
  public:
    EqContext(EqExpContext *ctx);

    EqExpContext *eqExp();
    antlr4::tree::TerminalNode *Eq();
    RelExpContext *relExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  EqExp_Context : public EqExpContext {
  public:
    EqExp_Context(EqExpContext *ctx);

    RelExpContext *relExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  EqExpContext* eqExp();
  EqExpContext* eqExp(int precedence);
  class  LAndExpContext : public antlr4::ParserRuleContext {
  public:
    LAndExpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    LAndExpContext() = default;
    void copyFrom(LAndExpContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  LAndExp_Context : public LAndExpContext {
  public:
    LAndExp_Context(LAndExpContext *ctx);

    EqExpContext *eqExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AndContext : public LAndExpContext {
  public:
    AndContext(LAndExpContext *ctx);

    LAndExpContext *lAndExp();
    antlr4::tree::TerminalNode *And();
    EqExpContext *eqExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  LAndExpContext* lAndExp();
  LAndExpContext* lAndExp(int precedence);
  class  LOrExpContext : public antlr4::ParserRuleContext {
  public:
    LOrExpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    LOrExpContext() = default;
    void copyFrom(LOrExpContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  OrContext : public LOrExpContext {
  public:
    OrContext(LOrExpContext *ctx);

    LOrExpContext *lOrExp();
    antlr4::tree::TerminalNode *Or();
    LAndExpContext *lAndExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LOrExp_Context : public LOrExpContext {
  public:
    LOrExp_Context(LOrExpContext *ctx);

    LAndExpContext *lAndExp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  LOrExpContext* lOrExp();
  LOrExpContext* lOrExp(int precedence);

  virtual bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;
  bool mulExpSempred(MulExpContext *_localctx, size_t predicateIndex);
  bool addExpSempred(AddExpContext *_localctx, size_t predicateIndex);
  bool relExpSempred(RelExpContext *_localctx, size_t predicateIndex);
  bool eqExpSempred(EqExpContext *_localctx, size_t predicateIndex);
  bool lAndExpSempred(LAndExpContext *_localctx, size_t predicateIndex);
  bool lOrExpSempred(LOrExpContext *_localctx, size_t predicateIndex);

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

}  // namespace frontend
