
// Generated from SysY.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"




class  SysYLexer : public antlr4::Lexer {
public:
  enum {
    Decl = 1, ConstDecl = 2, BType = 3, ConstDef = 4, ConstInitVal = 5, 
    VarDecl = 6, VarDef = 7, InitVal = 8, FuncDef = 9, FuncType = 10, FuncFParams = 11, 
    FuncFParam = 12, Block = 13, BlockItem = 14, Stmt = 15, Exp = 16, Cond = 17, 
    LVal = 18, PrimaryExp = 19, Number = 20, UnaryExp = 21, UnaryOp = 22, 
    FuncRParams = 23, MulExp = 24, AddExp = 25, RelExp = 26, EqExp = 27, 
    LAndExp = 28, LOrExp = 29, ConstExp = 30, INT = 31, FLOAT = 32, VOID = 33, 
    RETURN = 34, IF = 35, ELSE = 36, DO = 37, WHILE = 38, FOR = 39, BREAK = 40, 
    CONTINUE = 41, CONST = 42, EQU = 43, NEQ = 44, AND = 45, OR = 46, LEQ = 47, 
    GEQ = 48, PLUS = 49, MINUS = 50, TIMES = 51, SLASH = 52, MOD = 53, LT = 54, 
    GT = 55, COLON = 56, SEMICOLON = 57, LNOT = 58, BNOT = 59, COMMA = 60, 
    DOT = 61, ASSIGN = 62, QUESTION = 63, LPAREN = 64, RPAREN = 65, LBRACK = 66, 
    RBRACK = 67, LBRACE = 68, RBRACE = 69, FLOATNUM = 70, INTEGER = 71, 
    IDENTIFIER = 72, STRING = 73, ESC = 74, WS = 75, LINE_COMMENT = 76, 
    COMMENT = 77
  };

  explicit SysYLexer(antlr4::CharStream *input);

  ~SysYLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

