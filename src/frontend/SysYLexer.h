
// Generated from frontend/SysY.g4 by ANTLR 4.8

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

  SysYLexer(antlr4::CharStream *input);
  ~SysYLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

}  // namespace frontend
