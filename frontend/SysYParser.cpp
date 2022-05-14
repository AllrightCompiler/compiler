
// Generated from SysY.g4 by ANTLR 4.10.1


#include "SysYVisitor.h"

#include "SysYParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct SysYParserStaticData final {
  SysYParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SysYParserStaticData(const SysYParserStaticData&) = delete;
  SysYParserStaticData(SysYParserStaticData&&) = delete;
  SysYParserStaticData& operator=(const SysYParserStaticData&) = delete;
  SysYParserStaticData& operator=(SysYParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

std::once_flag sysyParserOnceFlag;
SysYParserStaticData *sysyParserStaticData = nullptr;

void sysyParserInitialize() {
  assert(sysyParserStaticData == nullptr);
  auto staticData = std::make_unique<SysYParserStaticData>(
    std::vector<std::string>{
      "start"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "'int'", "'float'", 
      "'void'", "'return'", "'if'", "'else'", "'do'", "'while'", "'for'", 
      "'break'", "'continue'", "'const'", "'=='", "'!='", "'&&'", "'||'", 
      "'<='", "'>='", "'+'", "'-'", "'*'", "'/'", "'%'", "'<'", "'>'", "':'", 
      "';'", "'!'", "'~'", "','", "'.'", "'='", "'\\u003F'", "'('", "')'", 
      "'['", "']'", "'{'", "'}'"
    },
    std::vector<std::string>{
      "", "Decl", "ConstDecl", "BType", "ConstDef", "ConstInitVal", "VarDecl", 
      "VarDef", "InitVal", "FuncDef", "FuncType", "FuncFParams", "FuncFParam", 
      "Block", "BlockItem", "Stmt", "Exp", "Cond", "LVal", "PrimaryExp", 
      "Number", "UnaryExp", "UnaryOp", "FuncRParams", "MulExp", "AddExp", 
      "RelExp", "EqExp", "LAndExp", "LOrExp", "ConstExp", "INT", "FLOAT", 
      "VOID", "RETURN", "IF", "ELSE", "DO", "WHILE", "FOR", "BREAK", "CONTINUE", 
      "CONST", "EQU", "NEQ", "AND", "OR", "LEQ", "GEQ", "PLUS", "MINUS", 
      "TIMES", "SLASH", "MOD", "LT", "GT", "COLON", "SEMICOLON", "LNOT", 
      "BNOT", "COMMA", "DOT", "ASSIGN", "QUESTION", "LPAREN", "RPAREN", 
      "LBRACK", "RBRACK", "LBRACE", "RBRACE", "FLOATNUM", "INTEGER", "IDENTIFIER", 
      "STRING", "ESC", "WS", "LINE_COMMENT", "COMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,77,11,2,0,7,0,1,0,5,0,4,8,0,10,0,12,0,7,9,0,1,0,1,0,1,0,0,0,1,0,0,
  	1,2,0,1,1,9,9,10,0,5,1,0,0,0,2,4,7,0,0,0,3,2,1,0,0,0,4,7,1,0,0,0,5,3,
  	1,0,0,0,5,6,1,0,0,0,6,8,1,0,0,0,7,5,1,0,0,0,8,9,5,0,0,1,9,1,1,0,0,0,1,
  	5
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  sysyParserStaticData = staticData.release();
}

}

SysYParser::SysYParser(TokenStream *input) : SysYParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

SysYParser::SysYParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  SysYParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *sysyParserStaticData->atn, sysyParserStaticData->decisionToDFA, sysyParserStaticData->sharedContextCache, options);
}

SysYParser::~SysYParser() {
  delete _interpreter;
}

const atn::ATN& SysYParser::getATN() const {
  return *sysyParserStaticData->atn;
}

std::string SysYParser::getGrammarFileName() const {
  return "SysY.g4";
}

const std::vector<std::string>& SysYParser::getRuleNames() const {
  return sysyParserStaticData->ruleNames;
}

const dfa::Vocabulary& SysYParser::getVocabulary() const {
  return sysyParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SysYParser::getSerializedATN() const {
  return sysyParserStaticData->serializedATN;
}


//----------------- StartContext ------------------------------------------------------------------

SysYParser::StartContext::StartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::StartContext::EOF() {
  return getToken(SysYParser::EOF, 0);
}

std::vector<tree::TerminalNode *> SysYParser::StartContext::Decl() {
  return getTokens(SysYParser::Decl);
}

tree::TerminalNode* SysYParser::StartContext::Decl(size_t i) {
  return getToken(SysYParser::Decl, i);
}

std::vector<tree::TerminalNode *> SysYParser::StartContext::FuncDef() {
  return getTokens(SysYParser::FuncDef);
}

tree::TerminalNode* SysYParser::StartContext::FuncDef(size_t i) {
  return getToken(SysYParser::FuncDef, i);
}


size_t SysYParser::StartContext::getRuleIndex() const {
  return SysYParser::RuleStart;
}


std::any SysYParser::StartContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitStart(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::StartContext* SysYParser::start() {
  StartContext *_localctx = _tracker.createInstance<StartContext>(_ctx, getState());
  enterRule(_localctx, 0, SysYParser::RuleStart);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(5);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Decl

    || _la == SysYParser::FuncDef) {
      setState(2);
      _la = _input->LA(1);
      if (!(_la == SysYParser::Decl

      || _la == SysYParser::FuncDef)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(7);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(8);
    match(SysYParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void SysYParser::initialize() {
  std::call_once(sysyParserOnceFlag, sysyParserInitialize);
}
