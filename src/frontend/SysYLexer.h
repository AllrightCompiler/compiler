
// Generated from frontend/SysY.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"


namespace frontend {


class  SysYLexer : public antlr4::Lexer {
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

}  // namespace frontend
