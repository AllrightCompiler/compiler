
// Generated from frontend/SysY.g4 by ANTLR 4.8


#include "SysYVisitor.h"

#include "SysYParser.h"


using namespace antlrcpp;
using namespace frontend;
using namespace antlr4;

SysYParser::SysYParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

SysYParser::~SysYParser() {
  delete _interpreter;
}

std::string SysYParser::getGrammarFileName() const {
  return "SysY.g4";
}

const std::vector<std::string>& SysYParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& SysYParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- CompUnitContext ------------------------------------------------------------------

SysYParser::CompUnitContext::CompUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::CompUnitContext::EOF() {
  return getToken(SysYParser::EOF, 0);
}

std::vector<SysYParser::CompUnitItemContext *> SysYParser::CompUnitContext::compUnitItem() {
  return getRuleContexts<SysYParser::CompUnitItemContext>();
}

SysYParser::CompUnitItemContext* SysYParser::CompUnitContext::compUnitItem(size_t i) {
  return getRuleContext<SysYParser::CompUnitItemContext>(i);
}


size_t SysYParser::CompUnitContext::getRuleIndex() const {
  return SysYParser::RuleCompUnit;
}


antlrcpp::Any SysYParser::CompUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCompUnit(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::CompUnitContext* SysYParser::compUnit() {
  CompUnitContext *_localctx = _tracker.createInstance<CompUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, SysYParser::RuleCompUnit);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(69);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SysYParser::Int)
      | (1ULL << SysYParser::Float)
      | (1ULL << SysYParser::Void)
      | (1ULL << SysYParser::Const))) != 0)) {
      setState(66);
      compUnitItem();
      setState(71);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(72);
    match(SysYParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CompUnitItemContext ------------------------------------------------------------------

SysYParser::CompUnitItemContext::CompUnitItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::DeclContext* SysYParser::CompUnitItemContext::decl() {
  return getRuleContext<SysYParser::DeclContext>(0);
}

SysYParser::FuncDefContext* SysYParser::CompUnitItemContext::funcDef() {
  return getRuleContext<SysYParser::FuncDefContext>(0);
}


size_t SysYParser::CompUnitItemContext::getRuleIndex() const {
  return SysYParser::RuleCompUnitItem;
}


antlrcpp::Any SysYParser::CompUnitItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCompUnitItem(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::CompUnitItemContext* SysYParser::compUnitItem() {
  CompUnitItemContext *_localctx = _tracker.createInstance<CompUnitItemContext>(_ctx, getState());
  enterRule(_localctx, 2, SysYParser::RuleCompUnitItem);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(76);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(74);
      decl();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(75);
      funcDef();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclContext ------------------------------------------------------------------

SysYParser::DeclContext::DeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::ConstDeclContext* SysYParser::DeclContext::constDecl() {
  return getRuleContext<SysYParser::ConstDeclContext>(0);
}

SysYParser::VarDeclContext* SysYParser::DeclContext::varDecl() {
  return getRuleContext<SysYParser::VarDeclContext>(0);
}


size_t SysYParser::DeclContext::getRuleIndex() const {
  return SysYParser::RuleDecl;
}


antlrcpp::Any SysYParser::DeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::DeclContext* SysYParser::decl() {
  DeclContext *_localctx = _tracker.createInstance<DeclContext>(_ctx, getState());
  enterRule(_localctx, 4, SysYParser::RuleDecl);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(80);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Const: {
        enterOuterAlt(_localctx, 1);
        setState(78);
        constDecl();
        break;
      }

      case SysYParser::Int:
      case SysYParser::Float: {
        enterOuterAlt(_localctx, 2);
        setState(79);
        varDecl();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstDeclContext ------------------------------------------------------------------

SysYParser::ConstDeclContext::ConstDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::ConstDeclContext::Const() {
  return getToken(SysYParser::Const, 0);
}

SysYParser::BTypeContext* SysYParser::ConstDeclContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

std::vector<SysYParser::ConstDefContext *> SysYParser::ConstDeclContext::constDef() {
  return getRuleContexts<SysYParser::ConstDefContext>();
}

SysYParser::ConstDefContext* SysYParser::ConstDeclContext::constDef(size_t i) {
  return getRuleContext<SysYParser::ConstDefContext>(i);
}

tree::TerminalNode* SysYParser::ConstDeclContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

std::vector<tree::TerminalNode *> SysYParser::ConstDeclContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::ConstDeclContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}


size_t SysYParser::ConstDeclContext::getRuleIndex() const {
  return SysYParser::RuleConstDecl;
}


antlrcpp::Any SysYParser::ConstDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitConstDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ConstDeclContext* SysYParser::constDecl() {
  ConstDeclContext *_localctx = _tracker.createInstance<ConstDeclContext>(_ctx, getState());
  enterRule(_localctx, 6, SysYParser::RuleConstDecl);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(82);
    match(SysYParser::Const);
    setState(83);
    bType();
    setState(84);
    constDef();
    setState(89);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Comma) {
      setState(85);
      match(SysYParser::Comma);
      setState(86);
      constDef();
      setState(91);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(92);
    match(SysYParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BTypeContext ------------------------------------------------------------------

SysYParser::BTypeContext::BTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::BTypeContext::getRuleIndex() const {
  return SysYParser::RuleBType;
}

void SysYParser::BTypeContext::copyFrom(BTypeContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- FloatContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::FloatContext::Float() {
  return getToken(SysYParser::Float, 0);
}

SysYParser::FloatContext::FloatContext(BTypeContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::FloatContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFloat(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IntContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::IntContext::Int() {
  return getToken(SysYParser::Int, 0);
}

SysYParser::IntContext::IntContext(BTypeContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::IntContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitInt(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::BTypeContext* SysYParser::bType() {
  BTypeContext *_localctx = _tracker.createInstance<BTypeContext>(_ctx, getState());
  enterRule(_localctx, 8, SysYParser::RuleBType);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(96);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Int: {
        _localctx = dynamic_cast<BTypeContext *>(_tracker.createInstance<SysYParser::IntContext>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(94);
        match(SysYParser::Int);
        break;
      }

      case SysYParser::Float: {
        _localctx = dynamic_cast<BTypeContext *>(_tracker.createInstance<SysYParser::FloatContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(95);
        match(SysYParser::Float);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstDefContext ------------------------------------------------------------------

SysYParser::ConstDefContext::ConstDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::ConstDefContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

tree::TerminalNode* SysYParser::ConstDefContext::Assign() {
  return getToken(SysYParser::Assign, 0);
}

SysYParser::InitValContext* SysYParser::ConstDefContext::initVal() {
  return getRuleContext<SysYParser::InitValContext>(0);
}

std::vector<tree::TerminalNode *> SysYParser::ConstDefContext::Lbracket() {
  return getTokens(SysYParser::Lbracket);
}

tree::TerminalNode* SysYParser::ConstDefContext::Lbracket(size_t i) {
  return getToken(SysYParser::Lbracket, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::ConstDefContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::ConstDefContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::ConstDefContext::Rbracket() {
  return getTokens(SysYParser::Rbracket);
}

tree::TerminalNode* SysYParser::ConstDefContext::Rbracket(size_t i) {
  return getToken(SysYParser::Rbracket, i);
}


size_t SysYParser::ConstDefContext::getRuleIndex() const {
  return SysYParser::RuleConstDef;
}


antlrcpp::Any SysYParser::ConstDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitConstDef(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ConstDefContext* SysYParser::constDef() {
  ConstDefContext *_localctx = _tracker.createInstance<ConstDefContext>(_ctx, getState());
  enterRule(_localctx, 10, SysYParser::RuleConstDef);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(98);
    match(SysYParser::Ident);
    setState(105);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Lbracket) {
      setState(99);
      match(SysYParser::Lbracket);
      setState(100);
      exp();
      setState(101);
      match(SysYParser::Rbracket);
      setState(107);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(108);
    match(SysYParser::Assign);
    setState(109);
    initVal();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDeclContext ------------------------------------------------------------------

SysYParser::VarDeclContext::VarDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::BTypeContext* SysYParser::VarDeclContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

std::vector<SysYParser::VarDefContext *> SysYParser::VarDeclContext::varDef() {
  return getRuleContexts<SysYParser::VarDefContext>();
}

SysYParser::VarDefContext* SysYParser::VarDeclContext::varDef(size_t i) {
  return getRuleContext<SysYParser::VarDefContext>(i);
}

tree::TerminalNode* SysYParser::VarDeclContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

std::vector<tree::TerminalNode *> SysYParser::VarDeclContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::VarDeclContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}


size_t SysYParser::VarDeclContext::getRuleIndex() const {
  return SysYParser::RuleVarDecl;
}


antlrcpp::Any SysYParser::VarDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitVarDecl(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::VarDeclContext* SysYParser::varDecl() {
  VarDeclContext *_localctx = _tracker.createInstance<VarDeclContext>(_ctx, getState());
  enterRule(_localctx, 12, SysYParser::RuleVarDecl);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(111);
    bType();
    setState(112);
    varDef();
    setState(117);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Comma) {
      setState(113);
      match(SysYParser::Comma);
      setState(114);
      varDef();
      setState(119);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(120);
    match(SysYParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDefContext ------------------------------------------------------------------

SysYParser::VarDefContext::VarDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::VarDefContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

std::vector<tree::TerminalNode *> SysYParser::VarDefContext::Lbracket() {
  return getTokens(SysYParser::Lbracket);
}

tree::TerminalNode* SysYParser::VarDefContext::Lbracket(size_t i) {
  return getToken(SysYParser::Lbracket, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::VarDefContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::VarDefContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::VarDefContext::Rbracket() {
  return getTokens(SysYParser::Rbracket);
}

tree::TerminalNode* SysYParser::VarDefContext::Rbracket(size_t i) {
  return getToken(SysYParser::Rbracket, i);
}

tree::TerminalNode* SysYParser::VarDefContext::Assign() {
  return getToken(SysYParser::Assign, 0);
}

SysYParser::InitValContext* SysYParser::VarDefContext::initVal() {
  return getRuleContext<SysYParser::InitValContext>(0);
}


size_t SysYParser::VarDefContext::getRuleIndex() const {
  return SysYParser::RuleVarDef;
}


antlrcpp::Any SysYParser::VarDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitVarDef(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::VarDefContext* SysYParser::varDef() {
  VarDefContext *_localctx = _tracker.createInstance<VarDefContext>(_ctx, getState());
  enterRule(_localctx, 14, SysYParser::RuleVarDef);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(122);
    match(SysYParser::Ident);
    setState(129);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Lbracket) {
      setState(123);
      match(SysYParser::Lbracket);
      setState(124);
      exp();
      setState(125);
      match(SysYParser::Rbracket);
      setState(131);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(134);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysYParser::Assign) {
      setState(132);
      match(SysYParser::Assign);
      setState(133);
      initVal();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InitValContext ------------------------------------------------------------------

SysYParser::InitValContext::InitValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::InitValContext::getRuleIndex() const {
  return SysYParser::RuleInitVal;
}

void SysYParser::InitValContext::copyFrom(InitValContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- InitContext ------------------------------------------------------------------

SysYParser::ExpContext* SysYParser::InitContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

SysYParser::InitContext::InitContext(InitValContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::InitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitInit(this);
  else
    return visitor->visitChildren(this);
}
//----------------- InitListContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::InitListContext::Lbrace() {
  return getToken(SysYParser::Lbrace, 0);
}

tree::TerminalNode* SysYParser::InitListContext::Rbrace() {
  return getToken(SysYParser::Rbrace, 0);
}

std::vector<SysYParser::InitValContext *> SysYParser::InitListContext::initVal() {
  return getRuleContexts<SysYParser::InitValContext>();
}

SysYParser::InitValContext* SysYParser::InitListContext::initVal(size_t i) {
  return getRuleContext<SysYParser::InitValContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::InitListContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::InitListContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}

SysYParser::InitListContext::InitListContext(InitValContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::InitListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitInitList(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::InitValContext* SysYParser::initVal() {
  InitValContext *_localctx = _tracker.createInstance<InitValContext>(_ctx, getState());
  enterRule(_localctx, 16, SysYParser::RuleInitVal);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(149);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Add:
      case SysYParser::Sub:
      case SysYParser::Not:
      case SysYParser::Lparen:
      case SysYParser::Ident:
      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst:
      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        _localctx = dynamic_cast<InitValContext *>(_tracker.createInstance<SysYParser::InitContext>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(136);
        exp();
        break;
      }

      case SysYParser::Lbrace: {
        _localctx = dynamic_cast<InitValContext *>(_tracker.createInstance<SysYParser::InitListContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(137);
        match(SysYParser::Lbrace);
        setState(146);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & ((1ULL << SysYParser::Add)
          | (1ULL << SysYParser::Sub)
          | (1ULL << SysYParser::Not)
          | (1ULL << SysYParser::Lparen)
          | (1ULL << SysYParser::Lbrace)
          | (1ULL << SysYParser::Ident)
          | (1ULL << SysYParser::DecIntConst)
          | (1ULL << SysYParser::OctIntConst)
          | (1ULL << SysYParser::HexIntConst)
          | (1ULL << SysYParser::DecFloatConst)
          | (1ULL << SysYParser::HexFloatConst))) != 0)) {
          setState(138);
          initVal();
          setState(143);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysYParser::Comma) {
            setState(139);
            match(SysYParser::Comma);
            setState(140);
            initVal();
            setState(145);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(148);
        match(SysYParser::Rbrace);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncDefContext ------------------------------------------------------------------

SysYParser::FuncDefContext::FuncDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::FuncTypeContext* SysYParser::FuncDefContext::funcType() {
  return getRuleContext<SysYParser::FuncTypeContext>(0);
}

tree::TerminalNode* SysYParser::FuncDefContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

tree::TerminalNode* SysYParser::FuncDefContext::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

tree::TerminalNode* SysYParser::FuncDefContext::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

SysYParser::BlockContext* SysYParser::FuncDefContext::block() {
  return getRuleContext<SysYParser::BlockContext>(0);
}

SysYParser::FuncFParamsContext* SysYParser::FuncDefContext::funcFParams() {
  return getRuleContext<SysYParser::FuncFParamsContext>(0);
}


size_t SysYParser::FuncDefContext::getRuleIndex() const {
  return SysYParser::RuleFuncDef;
}


antlrcpp::Any SysYParser::FuncDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncDef(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncDefContext* SysYParser::funcDef() {
  FuncDefContext *_localctx = _tracker.createInstance<FuncDefContext>(_ctx, getState());
  enterRule(_localctx, 18, SysYParser::RuleFuncDef);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(151);
    funcType();
    setState(152);
    match(SysYParser::Ident);
    setState(153);
    match(SysYParser::Lparen);
    setState(155);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysYParser::Int

    || _la == SysYParser::Float) {
      setState(154);
      funcFParams();
    }
    setState(157);
    match(SysYParser::Rparen);
    setState(158);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncTypeContext ------------------------------------------------------------------

SysYParser::FuncTypeContext::FuncTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::FuncTypeContext::getRuleIndex() const {
  return SysYParser::RuleFuncType;
}

void SysYParser::FuncTypeContext::copyFrom(FuncTypeContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- FuncType_Context ------------------------------------------------------------------

SysYParser::BTypeContext* SysYParser::FuncType_Context::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

SysYParser::FuncType_Context::FuncType_Context(FuncTypeContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::FuncType_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncType_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- VoidContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::VoidContext::Void() {
  return getToken(SysYParser::Void, 0);
}

SysYParser::VoidContext::VoidContext(FuncTypeContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::VoidContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitVoid(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::FuncTypeContext* SysYParser::funcType() {
  FuncTypeContext *_localctx = _tracker.createInstance<FuncTypeContext>(_ctx, getState());
  enterRule(_localctx, 20, SysYParser::RuleFuncType);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(162);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Int:
      case SysYParser::Float: {
        _localctx = dynamic_cast<FuncTypeContext *>(_tracker.createInstance<SysYParser::FuncType_Context>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(160);
        bType();
        break;
      }

      case SysYParser::Void: {
        _localctx = dynamic_cast<FuncTypeContext *>(_tracker.createInstance<SysYParser::VoidContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(161);
        match(SysYParser::Void);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFParamsContext ------------------------------------------------------------------

SysYParser::FuncFParamsContext::FuncFParamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysYParser::FuncFParamContext *> SysYParser::FuncFParamsContext::funcFParam() {
  return getRuleContexts<SysYParser::FuncFParamContext>();
}

SysYParser::FuncFParamContext* SysYParser::FuncFParamsContext::funcFParam(size_t i) {
  return getRuleContext<SysYParser::FuncFParamContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::FuncFParamsContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::FuncFParamsContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}


size_t SysYParser::FuncFParamsContext::getRuleIndex() const {
  return SysYParser::RuleFuncFParams;
}


antlrcpp::Any SysYParser::FuncFParamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncFParams(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncFParamsContext* SysYParser::funcFParams() {
  FuncFParamsContext *_localctx = _tracker.createInstance<FuncFParamsContext>(_ctx, getState());
  enterRule(_localctx, 22, SysYParser::RuleFuncFParams);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(164);
    funcFParam();
    setState(169);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Comma) {
      setState(165);
      match(SysYParser::Comma);
      setState(166);
      funcFParam();
      setState(171);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFParamContext ------------------------------------------------------------------

SysYParser::FuncFParamContext::FuncFParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::FuncFParamContext::getRuleIndex() const {
  return SysYParser::RuleFuncFParam;
}

void SysYParser::FuncFParamContext::copyFrom(FuncFParamContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ScalarParamContext ------------------------------------------------------------------

SysYParser::BTypeContext* SysYParser::ScalarParamContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

tree::TerminalNode* SysYParser::ScalarParamContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

SysYParser::ScalarParamContext::ScalarParamContext(FuncFParamContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::ScalarParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitScalarParam(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ArrayParamContext ------------------------------------------------------------------

SysYParser::BTypeContext* SysYParser::ArrayParamContext::bType() {
  return getRuleContext<SysYParser::BTypeContext>(0);
}

tree::TerminalNode* SysYParser::ArrayParamContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

std::vector<tree::TerminalNode *> SysYParser::ArrayParamContext::Lbracket() {
  return getTokens(SysYParser::Lbracket);
}

tree::TerminalNode* SysYParser::ArrayParamContext::Lbracket(size_t i) {
  return getToken(SysYParser::Lbracket, i);
}

std::vector<tree::TerminalNode *> SysYParser::ArrayParamContext::Rbracket() {
  return getTokens(SysYParser::Rbracket);
}

tree::TerminalNode* SysYParser::ArrayParamContext::Rbracket(size_t i) {
  return getToken(SysYParser::Rbracket, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::ArrayParamContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::ArrayParamContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

SysYParser::ArrayParamContext::ArrayParamContext(FuncFParamContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::ArrayParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitArrayParam(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::FuncFParamContext* SysYParser::funcFParam() {
  FuncFParamContext *_localctx = _tracker.createInstance<FuncFParamContext>(_ctx, getState());
  enterRule(_localctx, 24, SysYParser::RuleFuncFParam);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(188);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 16, _ctx)) {
    case 1: {
      _localctx = dynamic_cast<FuncFParamContext *>(_tracker.createInstance<SysYParser::ScalarParamContext>(_localctx));
      enterOuterAlt(_localctx, 1);
      setState(172);
      bType();
      setState(173);
      match(SysYParser::Ident);
      break;
    }

    case 2: {
      _localctx = dynamic_cast<FuncFParamContext *>(_tracker.createInstance<SysYParser::ArrayParamContext>(_localctx));
      enterOuterAlt(_localctx, 2);
      setState(175);
      bType();
      setState(176);
      match(SysYParser::Ident);
      setState(177);
      match(SysYParser::Lbracket);
      setState(178);
      match(SysYParser::Rbracket);
      setState(185);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysYParser::Lbracket) {
        setState(179);
        match(SysYParser::Lbracket);
        setState(180);
        exp();
        setState(181);
        match(SysYParser::Rbracket);
        setState(187);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

SysYParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::BlockContext::Lbrace() {
  return getToken(SysYParser::Lbrace, 0);
}

tree::TerminalNode* SysYParser::BlockContext::Rbrace() {
  return getToken(SysYParser::Rbrace, 0);
}

std::vector<SysYParser::BlockItemContext *> SysYParser::BlockContext::blockItem() {
  return getRuleContexts<SysYParser::BlockItemContext>();
}

SysYParser::BlockItemContext* SysYParser::BlockContext::blockItem(size_t i) {
  return getRuleContext<SysYParser::BlockItemContext>(i);
}


size_t SysYParser::BlockContext::getRuleIndex() const {
  return SysYParser::RuleBlock;
}


antlrcpp::Any SysYParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::BlockContext* SysYParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 26, SysYParser::RuleBlock);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(190);
    match(SysYParser::Lbrace);
    setState(194);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SysYParser::Int)
      | (1ULL << SysYParser::Float)
      | (1ULL << SysYParser::Const)
      | (1ULL << SysYParser::If)
      | (1ULL << SysYParser::While)
      | (1ULL << SysYParser::Break)
      | (1ULL << SysYParser::Continue)
      | (1ULL << SysYParser::Return)
      | (1ULL << SysYParser::Add)
      | (1ULL << SysYParser::Sub)
      | (1ULL << SysYParser::Not)
      | (1ULL << SysYParser::Semicolon)
      | (1ULL << SysYParser::Lparen)
      | (1ULL << SysYParser::Lbrace)
      | (1ULL << SysYParser::Ident)
      | (1ULL << SysYParser::DecIntConst)
      | (1ULL << SysYParser::OctIntConst)
      | (1ULL << SysYParser::HexIntConst)
      | (1ULL << SysYParser::DecFloatConst)
      | (1ULL << SysYParser::HexFloatConst))) != 0)) {
      setState(191);
      blockItem();
      setState(196);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(197);
    match(SysYParser::Rbrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockItemContext ------------------------------------------------------------------

SysYParser::BlockItemContext::BlockItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::DeclContext* SysYParser::BlockItemContext::decl() {
  return getRuleContext<SysYParser::DeclContext>(0);
}

SysYParser::StmtContext* SysYParser::BlockItemContext::stmt() {
  return getRuleContext<SysYParser::StmtContext>(0);
}


size_t SysYParser::BlockItemContext::getRuleIndex() const {
  return SysYParser::RuleBlockItem;
}


antlrcpp::Any SysYParser::BlockItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBlockItem(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::BlockItemContext* SysYParser::blockItem() {
  BlockItemContext *_localctx = _tracker.createInstance<BlockItemContext>(_ctx, getState());
  enterRule(_localctx, 28, SysYParser::RuleBlockItem);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(201);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Int:
      case SysYParser::Float:
      case SysYParser::Const: {
        enterOuterAlt(_localctx, 1);
        setState(199);
        decl();
        break;
      }

      case SysYParser::If:
      case SysYParser::While:
      case SysYParser::Break:
      case SysYParser::Continue:
      case SysYParser::Return:
      case SysYParser::Add:
      case SysYParser::Sub:
      case SysYParser::Not:
      case SysYParser::Semicolon:
      case SysYParser::Lparen:
      case SysYParser::Lbrace:
      case SysYParser::Ident:
      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst:
      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        enterOuterAlt(_localctx, 2);
        setState(200);
        stmt();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StmtContext ------------------------------------------------------------------

SysYParser::StmtContext::StmtContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::StmtContext::getRuleIndex() const {
  return SysYParser::RuleStmt;
}

void SysYParser::StmtContext::copyFrom(StmtContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ExprStmtContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::ExprStmtContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::ExpContext* SysYParser::ExprStmtContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

SysYParser::ExprStmtContext::ExprStmtContext(StmtContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::ExprStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitExprStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BlockStmtContext ------------------------------------------------------------------

SysYParser::BlockContext* SysYParser::BlockStmtContext::block() {
  return getRuleContext<SysYParser::BlockContext>(0);
}

SysYParser::BlockStmtContext::BlockStmtContext(StmtContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::BlockStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBlockStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BreakContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::BreakContext::Break() {
  return getToken(SysYParser::Break, 0);
}

tree::TerminalNode* SysYParser::BreakContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::BreakContext::BreakContext(StmtContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::BreakContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitBreak(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ContinueContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::ContinueContext::Continue() {
  return getToken(SysYParser::Continue, 0);
}

tree::TerminalNode* SysYParser::ContinueContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::ContinueContext::ContinueContext(StmtContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::ContinueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitContinue(this);
  else
    return visitor->visitChildren(this);
}
//----------------- WhileContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::WhileContext::While() {
  return getToken(SysYParser::While, 0);
}

tree::TerminalNode* SysYParser::WhileContext::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

SysYParser::CondContext* SysYParser::WhileContext::cond() {
  return getRuleContext<SysYParser::CondContext>(0);
}

tree::TerminalNode* SysYParser::WhileContext::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

SysYParser::StmtContext* SysYParser::WhileContext::stmt() {
  return getRuleContext<SysYParser::StmtContext>(0);
}

SysYParser::WhileContext::WhileContext(StmtContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::WhileContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitWhile(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IfElseContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::IfElseContext::If() {
  return getToken(SysYParser::If, 0);
}

tree::TerminalNode* SysYParser::IfElseContext::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

SysYParser::CondContext* SysYParser::IfElseContext::cond() {
  return getRuleContext<SysYParser::CondContext>(0);
}

tree::TerminalNode* SysYParser::IfElseContext::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

std::vector<SysYParser::StmtContext *> SysYParser::IfElseContext::stmt() {
  return getRuleContexts<SysYParser::StmtContext>();
}

SysYParser::StmtContext* SysYParser::IfElseContext::stmt(size_t i) {
  return getRuleContext<SysYParser::StmtContext>(i);
}

tree::TerminalNode* SysYParser::IfElseContext::Else() {
  return getToken(SysYParser::Else, 0);
}

SysYParser::IfElseContext::IfElseContext(StmtContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::IfElseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitIfElse(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ReturnContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::ReturnContext::Return() {
  return getToken(SysYParser::Return, 0);
}

tree::TerminalNode* SysYParser::ReturnContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::ExpContext* SysYParser::ReturnContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

SysYParser::ReturnContext::ReturnContext(StmtContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::ReturnContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitReturn(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AssignContext ------------------------------------------------------------------

SysYParser::LValContext* SysYParser::AssignContext::lVal() {
  return getRuleContext<SysYParser::LValContext>(0);
}

tree::TerminalNode* SysYParser::AssignContext::Assign() {
  return getToken(SysYParser::Assign, 0);
}

SysYParser::ExpContext* SysYParser::AssignContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

tree::TerminalNode* SysYParser::AssignContext::Semicolon() {
  return getToken(SysYParser::Semicolon, 0);
}

SysYParser::AssignContext::AssignContext(StmtContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::AssignContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitAssign(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::StmtContext* SysYParser::stmt() {
  StmtContext *_localctx = _tracker.createInstance<StmtContext>(_ctx, getState());
  enterRule(_localctx, 30, SysYParser::RuleStmt);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(237);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
    case 1: {
      _localctx = dynamic_cast<StmtContext *>(_tracker.createInstance<SysYParser::AssignContext>(_localctx));
      enterOuterAlt(_localctx, 1);
      setState(203);
      lVal();
      setState(204);
      match(SysYParser::Assign);
      setState(205);
      exp();
      setState(206);
      match(SysYParser::Semicolon);
      break;
    }

    case 2: {
      _localctx = dynamic_cast<StmtContext *>(_tracker.createInstance<SysYParser::ExprStmtContext>(_localctx));
      enterOuterAlt(_localctx, 2);
      setState(209);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << SysYParser::Add)
        | (1ULL << SysYParser::Sub)
        | (1ULL << SysYParser::Not)
        | (1ULL << SysYParser::Lparen)
        | (1ULL << SysYParser::Ident)
        | (1ULL << SysYParser::DecIntConst)
        | (1ULL << SysYParser::OctIntConst)
        | (1ULL << SysYParser::HexIntConst)
        | (1ULL << SysYParser::DecFloatConst)
        | (1ULL << SysYParser::HexFloatConst))) != 0)) {
        setState(208);
        exp();
      }
      setState(211);
      match(SysYParser::Semicolon);
      break;
    }

    case 3: {
      _localctx = dynamic_cast<StmtContext *>(_tracker.createInstance<SysYParser::BlockStmtContext>(_localctx));
      enterOuterAlt(_localctx, 3);
      setState(212);
      block();
      break;
    }

    case 4: {
      _localctx = dynamic_cast<StmtContext *>(_tracker.createInstance<SysYParser::IfElseContext>(_localctx));
      enterOuterAlt(_localctx, 4);
      setState(213);
      match(SysYParser::If);
      setState(214);
      match(SysYParser::Lparen);
      setState(215);
      cond();
      setState(216);
      match(SysYParser::Rparen);
      setState(217);
      stmt();
      setState(220);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
      case 1: {
        setState(218);
        match(SysYParser::Else);
        setState(219);
        stmt();
        break;
      }

      }
      break;
    }

    case 5: {
      _localctx = dynamic_cast<StmtContext *>(_tracker.createInstance<SysYParser::WhileContext>(_localctx));
      enterOuterAlt(_localctx, 5);
      setState(222);
      match(SysYParser::While);
      setState(223);
      match(SysYParser::Lparen);
      setState(224);
      cond();
      setState(225);
      match(SysYParser::Rparen);
      setState(226);
      stmt();
      break;
    }

    case 6: {
      _localctx = dynamic_cast<StmtContext *>(_tracker.createInstance<SysYParser::BreakContext>(_localctx));
      enterOuterAlt(_localctx, 6);
      setState(228);
      match(SysYParser::Break);
      setState(229);
      match(SysYParser::Semicolon);
      break;
    }

    case 7: {
      _localctx = dynamic_cast<StmtContext *>(_tracker.createInstance<SysYParser::ContinueContext>(_localctx));
      enterOuterAlt(_localctx, 7);
      setState(230);
      match(SysYParser::Continue);
      setState(231);
      match(SysYParser::Semicolon);
      break;
    }

    case 8: {
      _localctx = dynamic_cast<StmtContext *>(_tracker.createInstance<SysYParser::ReturnContext>(_localctx));
      enterOuterAlt(_localctx, 8);
      setState(232);
      match(SysYParser::Return);
      setState(234);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << SysYParser::Add)
        | (1ULL << SysYParser::Sub)
        | (1ULL << SysYParser::Not)
        | (1ULL << SysYParser::Lparen)
        | (1ULL << SysYParser::Ident)
        | (1ULL << SysYParser::DecIntConst)
        | (1ULL << SysYParser::OctIntConst)
        | (1ULL << SysYParser::HexIntConst)
        | (1ULL << SysYParser::DecFloatConst)
        | (1ULL << SysYParser::HexFloatConst))) != 0)) {
        setState(233);
        exp();
      }
      setState(236);
      match(SysYParser::Semicolon);
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpContext ------------------------------------------------------------------

SysYParser::ExpContext::ExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::AddExpContext* SysYParser::ExpContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}


size_t SysYParser::ExpContext::getRuleIndex() const {
  return SysYParser::RuleExp;
}


antlrcpp::Any SysYParser::ExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitExp(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::ExpContext* SysYParser::exp() {
  ExpContext *_localctx = _tracker.createInstance<ExpContext>(_ctx, getState());
  enterRule(_localctx, 32, SysYParser::RuleExp);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(239);
    addExp(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CondContext ------------------------------------------------------------------

SysYParser::CondContext::CondContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::LOrExpContext* SysYParser::CondContext::lOrExp() {
  return getRuleContext<SysYParser::LOrExpContext>(0);
}


size_t SysYParser::CondContext::getRuleIndex() const {
  return SysYParser::RuleCond;
}


antlrcpp::Any SysYParser::CondContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCond(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::CondContext* SysYParser::cond() {
  CondContext *_localctx = _tracker.createInstance<CondContext>(_ctx, getState());
  enterRule(_localctx, 34, SysYParser::RuleCond);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(241);
    lOrExp(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LValContext ------------------------------------------------------------------

SysYParser::LValContext::LValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::LValContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

std::vector<tree::TerminalNode *> SysYParser::LValContext::Lbracket() {
  return getTokens(SysYParser::Lbracket);
}

tree::TerminalNode* SysYParser::LValContext::Lbracket(size_t i) {
  return getToken(SysYParser::Lbracket, i);
}

std::vector<SysYParser::ExpContext *> SysYParser::LValContext::exp() {
  return getRuleContexts<SysYParser::ExpContext>();
}

SysYParser::ExpContext* SysYParser::LValContext::exp(size_t i) {
  return getRuleContext<SysYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::LValContext::Rbracket() {
  return getTokens(SysYParser::Rbracket);
}

tree::TerminalNode* SysYParser::LValContext::Rbracket(size_t i) {
  return getToken(SysYParser::Rbracket, i);
}


size_t SysYParser::LValContext::getRuleIndex() const {
  return SysYParser::RuleLVal;
}


antlrcpp::Any SysYParser::LValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLVal(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::LValContext* SysYParser::lVal() {
  LValContext *_localctx = _tracker.createInstance<LValContext>(_ctx, getState());
  enterRule(_localctx, 36, SysYParser::RuleLVal);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(243);
    match(SysYParser::Ident);
    setState(250);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(244);
        match(SysYParser::Lbracket);
        setState(245);
        exp();
        setState(246);
        match(SysYParser::Rbracket); 
      }
      setState(252);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryExpContext ------------------------------------------------------------------

SysYParser::PrimaryExpContext::PrimaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::PrimaryExpContext::getRuleIndex() const {
  return SysYParser::RulePrimaryExp;
}

void SysYParser::PrimaryExpContext::copyFrom(PrimaryExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- PrimaryExp_Context ------------------------------------------------------------------

tree::TerminalNode* SysYParser::PrimaryExp_Context::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

SysYParser::ExpContext* SysYParser::PrimaryExp_Context::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

tree::TerminalNode* SysYParser::PrimaryExp_Context::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

SysYParser::NumberContext* SysYParser::PrimaryExp_Context::number() {
  return getRuleContext<SysYParser::NumberContext>(0);
}

SysYParser::PrimaryExp_Context::PrimaryExp_Context(PrimaryExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::PrimaryExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitPrimaryExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LValExprContext ------------------------------------------------------------------

SysYParser::LValContext* SysYParser::LValExprContext::lVal() {
  return getRuleContext<SysYParser::LValContext>(0);
}

SysYParser::LValExprContext::LValExprContext(PrimaryExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::LValExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLValExpr(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::PrimaryExpContext* SysYParser::primaryExp() {
  PrimaryExpContext *_localctx = _tracker.createInstance<PrimaryExpContext>(_ctx, getState());
  enterRule(_localctx, 38, SysYParser::RulePrimaryExp);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(259);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Lparen: {
        _localctx = dynamic_cast<PrimaryExpContext *>(_tracker.createInstance<SysYParser::PrimaryExp_Context>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(253);
        match(SysYParser::Lparen);
        setState(254);
        exp();
        setState(255);
        match(SysYParser::Rparen);
        break;
      }

      case SysYParser::Ident: {
        _localctx = dynamic_cast<PrimaryExpContext *>(_tracker.createInstance<SysYParser::LValExprContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(257);
        lVal();
        break;
      }

      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst:
      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        _localctx = dynamic_cast<PrimaryExpContext *>(_tracker.createInstance<SysYParser::PrimaryExp_Context>(_localctx));
        enterOuterAlt(_localctx, 3);
        setState(258);
        number();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IntConstContext ------------------------------------------------------------------

SysYParser::IntConstContext::IntConstContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::IntConstContext::getRuleIndex() const {
  return SysYParser::RuleIntConst;
}

void SysYParser::IntConstContext::copyFrom(IntConstContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- HexIntConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::HexIntConstContext::HexIntConst() {
  return getToken(SysYParser::HexIntConst, 0);
}

SysYParser::HexIntConstContext::HexIntConstContext(IntConstContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::HexIntConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitHexIntConst(this);
  else
    return visitor->visitChildren(this);
}
//----------------- DecIntConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::DecIntConstContext::DecIntConst() {
  return getToken(SysYParser::DecIntConst, 0);
}

SysYParser::DecIntConstContext::DecIntConstContext(IntConstContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::DecIntConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitDecIntConst(this);
  else
    return visitor->visitChildren(this);
}
//----------------- OctIntConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::OctIntConstContext::OctIntConst() {
  return getToken(SysYParser::OctIntConst, 0);
}

SysYParser::OctIntConstContext::OctIntConstContext(IntConstContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::OctIntConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitOctIntConst(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::IntConstContext* SysYParser::intConst() {
  IntConstContext *_localctx = _tracker.createInstance<IntConstContext>(_ctx, getState());
  enterRule(_localctx, 40, SysYParser::RuleIntConst);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(264);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::DecIntConst: {
        _localctx = dynamic_cast<IntConstContext *>(_tracker.createInstance<SysYParser::DecIntConstContext>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(261);
        match(SysYParser::DecIntConst);
        break;
      }

      case SysYParser::OctIntConst: {
        _localctx = dynamic_cast<IntConstContext *>(_tracker.createInstance<SysYParser::OctIntConstContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(262);
        match(SysYParser::OctIntConst);
        break;
      }

      case SysYParser::HexIntConst: {
        _localctx = dynamic_cast<IntConstContext *>(_tracker.createInstance<SysYParser::HexIntConstContext>(_localctx));
        enterOuterAlt(_localctx, 3);
        setState(263);
        match(SysYParser::HexIntConst);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FloatConstContext ------------------------------------------------------------------

SysYParser::FloatConstContext::FloatConstContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::FloatConstContext::getRuleIndex() const {
  return SysYParser::RuleFloatConst;
}

void SysYParser::FloatConstContext::copyFrom(FloatConstContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- DecFloatConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::DecFloatConstContext::DecFloatConst() {
  return getToken(SysYParser::DecFloatConst, 0);
}

SysYParser::DecFloatConstContext::DecFloatConstContext(FloatConstContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::DecFloatConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitDecFloatConst(this);
  else
    return visitor->visitChildren(this);
}
//----------------- HexFloatConstContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::HexFloatConstContext::HexFloatConst() {
  return getToken(SysYParser::HexFloatConst, 0);
}

SysYParser::HexFloatConstContext::HexFloatConstContext(FloatConstContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::HexFloatConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitHexFloatConst(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::FloatConstContext* SysYParser::floatConst() {
  FloatConstContext *_localctx = _tracker.createInstance<FloatConstContext>(_ctx, getState());
  enterRule(_localctx, 42, SysYParser::RuleFloatConst);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(268);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::DecFloatConst: {
        _localctx = dynamic_cast<FloatConstContext *>(_tracker.createInstance<SysYParser::DecFloatConstContext>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(266);
        match(SysYParser::DecFloatConst);
        break;
      }

      case SysYParser::HexFloatConst: {
        _localctx = dynamic_cast<FloatConstContext *>(_tracker.createInstance<SysYParser::HexFloatConstContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(267);
        match(SysYParser::HexFloatConst);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NumberContext ------------------------------------------------------------------

SysYParser::NumberContext::NumberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::IntConstContext* SysYParser::NumberContext::intConst() {
  return getRuleContext<SysYParser::IntConstContext>(0);
}

SysYParser::FloatConstContext* SysYParser::NumberContext::floatConst() {
  return getRuleContext<SysYParser::FloatConstContext>(0);
}


size_t SysYParser::NumberContext::getRuleIndex() const {
  return SysYParser::RuleNumber;
}


antlrcpp::Any SysYParser::NumberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitNumber(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::NumberContext* SysYParser::number() {
  NumberContext *_localctx = _tracker.createInstance<NumberContext>(_ctx, getState());
  enterRule(_localctx, 44, SysYParser::RuleNumber);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(272);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst: {
        enterOuterAlt(_localctx, 1);
        setState(270);
        intConst();
        break;
      }

      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        enterOuterAlt(_localctx, 2);
        setState(271);
        floatConst();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryExpContext ------------------------------------------------------------------

SysYParser::UnaryExpContext::UnaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::UnaryExpContext::getRuleIndex() const {
  return SysYParser::RuleUnaryExp;
}

void SysYParser::UnaryExpContext::copyFrom(UnaryExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- CallContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::CallContext::Ident() {
  return getToken(SysYParser::Ident, 0);
}

tree::TerminalNode* SysYParser::CallContext::Lparen() {
  return getToken(SysYParser::Lparen, 0);
}

tree::TerminalNode* SysYParser::CallContext::Rparen() {
  return getToken(SysYParser::Rparen, 0);
}

SysYParser::FuncRParamsContext* SysYParser::CallContext::funcRParams() {
  return getRuleContext<SysYParser::FuncRParamsContext>(0);
}

SysYParser::CallContext::CallContext(UnaryExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::CallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitCall(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NotContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::NotContext::Not() {
  return getToken(SysYParser::Not, 0);
}

SysYParser::UnaryExpContext* SysYParser::NotContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::NotContext::NotContext(UnaryExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::NotContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitNot(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnaryExp_Context ------------------------------------------------------------------

SysYParser::PrimaryExpContext* SysYParser::UnaryExp_Context::primaryExp() {
  return getRuleContext<SysYParser::PrimaryExpContext>(0);
}

SysYParser::UnaryExp_Context::UnaryExp_Context(UnaryExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::UnaryExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitUnaryExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnaryAddContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::UnaryAddContext::Add() {
  return getToken(SysYParser::Add, 0);
}

SysYParser::UnaryExpContext* SysYParser::UnaryAddContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::UnaryAddContext::UnaryAddContext(UnaryExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::UnaryAddContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitUnaryAdd(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnarySubContext ------------------------------------------------------------------

tree::TerminalNode* SysYParser::UnarySubContext::Sub() {
  return getToken(SysYParser::Sub, 0);
}

SysYParser::UnaryExpContext* SysYParser::UnarySubContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::UnarySubContext::UnarySubContext(UnaryExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::UnarySubContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitUnarySub(this);
  else
    return visitor->visitChildren(this);
}
SysYParser::UnaryExpContext* SysYParser::unaryExp() {
  UnaryExpContext *_localctx = _tracker.createInstance<UnaryExpContext>(_ctx, getState());
  enterRule(_localctx, 46, SysYParser::RuleUnaryExp);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(287);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      _localctx = dynamic_cast<UnaryExpContext *>(_tracker.createInstance<SysYParser::UnaryExp_Context>(_localctx));
      enterOuterAlt(_localctx, 1);
      setState(274);
      primaryExp();
      break;
    }

    case 2: {
      _localctx = dynamic_cast<UnaryExpContext *>(_tracker.createInstance<SysYParser::CallContext>(_localctx));
      enterOuterAlt(_localctx, 2);
      setState(275);
      match(SysYParser::Ident);
      setState(276);
      match(SysYParser::Lparen);
      setState(278);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << SysYParser::Add)
        | (1ULL << SysYParser::Sub)
        | (1ULL << SysYParser::Not)
        | (1ULL << SysYParser::Lparen)
        | (1ULL << SysYParser::Ident)
        | (1ULL << SysYParser::DecIntConst)
        | (1ULL << SysYParser::OctIntConst)
        | (1ULL << SysYParser::HexIntConst)
        | (1ULL << SysYParser::DecFloatConst)
        | (1ULL << SysYParser::HexFloatConst)
        | (1ULL << SysYParser::StringConst))) != 0)) {
        setState(277);
        funcRParams();
      }
      setState(280);
      match(SysYParser::Rparen);
      break;
    }

    case 3: {
      _localctx = dynamic_cast<UnaryExpContext *>(_tracker.createInstance<SysYParser::UnaryAddContext>(_localctx));
      enterOuterAlt(_localctx, 3);
      setState(281);
      match(SysYParser::Add);
      setState(282);
      unaryExp();
      break;
    }

    case 4: {
      _localctx = dynamic_cast<UnaryExpContext *>(_tracker.createInstance<SysYParser::UnarySubContext>(_localctx));
      enterOuterAlt(_localctx, 4);
      setState(283);
      match(SysYParser::Sub);
      setState(284);
      unaryExp();
      break;
    }

    case 5: {
      _localctx = dynamic_cast<UnaryExpContext *>(_tracker.createInstance<SysYParser::NotContext>(_localctx));
      enterOuterAlt(_localctx, 5);
      setState(285);
      match(SysYParser::Not);
      setState(286);
      unaryExp();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StringConstContext ------------------------------------------------------------------

SysYParser::StringConstContext::StringConstContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysYParser::StringConstContext::StringConst() {
  return getToken(SysYParser::StringConst, 0);
}


size_t SysYParser::StringConstContext::getRuleIndex() const {
  return SysYParser::RuleStringConst;
}


antlrcpp::Any SysYParser::StringConstContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitStringConst(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::StringConstContext* SysYParser::stringConst() {
  StringConstContext *_localctx = _tracker.createInstance<StringConstContext>(_ctx, getState());
  enterRule(_localctx, 48, SysYParser::RuleStringConst);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(289);
    match(SysYParser::StringConst);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncRParamContext ------------------------------------------------------------------

SysYParser::FuncRParamContext::FuncRParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysYParser::ExpContext* SysYParser::FuncRParamContext::exp() {
  return getRuleContext<SysYParser::ExpContext>(0);
}

SysYParser::StringConstContext* SysYParser::FuncRParamContext::stringConst() {
  return getRuleContext<SysYParser::StringConstContext>(0);
}


size_t SysYParser::FuncRParamContext::getRuleIndex() const {
  return SysYParser::RuleFuncRParam;
}


antlrcpp::Any SysYParser::FuncRParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncRParam(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncRParamContext* SysYParser::funcRParam() {
  FuncRParamContext *_localctx = _tracker.createInstance<FuncRParamContext>(_ctx, getState());
  enterRule(_localctx, 50, SysYParser::RuleFuncRParam);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(293);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysYParser::Add:
      case SysYParser::Sub:
      case SysYParser::Not:
      case SysYParser::Lparen:
      case SysYParser::Ident:
      case SysYParser::DecIntConst:
      case SysYParser::OctIntConst:
      case SysYParser::HexIntConst:
      case SysYParser::DecFloatConst:
      case SysYParser::HexFloatConst: {
        enterOuterAlt(_localctx, 1);
        setState(291);
        exp();
        break;
      }

      case SysYParser::StringConst: {
        enterOuterAlt(_localctx, 2);
        setState(292);
        stringConst();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncRParamsContext ------------------------------------------------------------------

SysYParser::FuncRParamsContext::FuncRParamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysYParser::FuncRParamContext *> SysYParser::FuncRParamsContext::funcRParam() {
  return getRuleContexts<SysYParser::FuncRParamContext>();
}

SysYParser::FuncRParamContext* SysYParser::FuncRParamsContext::funcRParam(size_t i) {
  return getRuleContext<SysYParser::FuncRParamContext>(i);
}

std::vector<tree::TerminalNode *> SysYParser::FuncRParamsContext::Comma() {
  return getTokens(SysYParser::Comma);
}

tree::TerminalNode* SysYParser::FuncRParamsContext::Comma(size_t i) {
  return getToken(SysYParser::Comma, i);
}


size_t SysYParser::FuncRParamsContext::getRuleIndex() const {
  return SysYParser::RuleFuncRParams;
}


antlrcpp::Any SysYParser::FuncRParamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitFuncRParams(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::FuncRParamsContext* SysYParser::funcRParams() {
  FuncRParamsContext *_localctx = _tracker.createInstance<FuncRParamsContext>(_ctx, getState());
  enterRule(_localctx, 52, SysYParser::RuleFuncRParams);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(295);
    funcRParam();
    setState(300);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysYParser::Comma) {
      setState(296);
      match(SysYParser::Comma);
      setState(297);
      funcRParam();
      setState(302);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MulExpContext ------------------------------------------------------------------

SysYParser::MulExpContext::MulExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::MulExpContext::getRuleIndex() const {
  return SysYParser::RuleMulExp;
}

void SysYParser::MulExpContext::copyFrom(MulExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- DivContext ------------------------------------------------------------------

SysYParser::MulExpContext* SysYParser::DivContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

tree::TerminalNode* SysYParser::DivContext::Div() {
  return getToken(SysYParser::Div, 0);
}

SysYParser::UnaryExpContext* SysYParser::DivContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::DivContext::DivContext(MulExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::DivContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitDiv(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ModContext ------------------------------------------------------------------

SysYParser::MulExpContext* SysYParser::ModContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

tree::TerminalNode* SysYParser::ModContext::Mod() {
  return getToken(SysYParser::Mod, 0);
}

SysYParser::UnaryExpContext* SysYParser::ModContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::ModContext::ModContext(MulExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::ModContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitMod(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MulContext ------------------------------------------------------------------

SysYParser::MulExpContext* SysYParser::MulContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

tree::TerminalNode* SysYParser::MulContext::Mul() {
  return getToken(SysYParser::Mul, 0);
}

SysYParser::UnaryExpContext* SysYParser::MulContext::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::MulContext::MulContext(MulExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::MulContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitMul(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MulExp_Context ------------------------------------------------------------------

SysYParser::UnaryExpContext* SysYParser::MulExp_Context::unaryExp() {
  return getRuleContext<SysYParser::UnaryExpContext>(0);
}

SysYParser::MulExp_Context::MulExp_Context(MulExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::MulExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitMulExp_(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::MulExpContext* SysYParser::mulExp() {
   return mulExp(0);
}

SysYParser::MulExpContext* SysYParser::mulExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::MulExpContext *_localctx = _tracker.createInstance<MulExpContext>(_ctx, parentState);
  SysYParser::MulExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 54;
  enterRecursionRule(_localctx, 54, SysYParser::RuleMulExp, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<MulExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(304);
    unaryExp();
    _ctx->stop = _input->LT(-1);
    setState(317);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(315);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<MulContext>(_tracker.createInstance<MulExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleMulExp);
          setState(306);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(307);
          match(SysYParser::Mul);
          setState(308);
          unaryExp();
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<DivContext>(_tracker.createInstance<MulExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleMulExp);
          setState(309);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(310);
          match(SysYParser::Div);
          setState(311);
          unaryExp();
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<ModContext>(_tracker.createInstance<MulExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleMulExp);
          setState(312);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(313);
          match(SysYParser::Mod);
          setState(314);
          unaryExp();
          break;
        }

        } 
      }
      setState(319);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- AddExpContext ------------------------------------------------------------------

SysYParser::AddExpContext::AddExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::AddExpContext::getRuleIndex() const {
  return SysYParser::RuleAddExp;
}

void SysYParser::AddExpContext::copyFrom(AddExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- AddExp_Context ------------------------------------------------------------------

SysYParser::MulExpContext* SysYParser::AddExp_Context::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

SysYParser::AddExp_Context::AddExp_Context(AddExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::AddExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitAddExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AddContext ------------------------------------------------------------------

SysYParser::AddExpContext* SysYParser::AddContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

tree::TerminalNode* SysYParser::AddContext::Add() {
  return getToken(SysYParser::Add, 0);
}

SysYParser::MulExpContext* SysYParser::AddContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

SysYParser::AddContext::AddContext(AddExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::AddContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitAdd(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SubContext ------------------------------------------------------------------

SysYParser::AddExpContext* SysYParser::SubContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

tree::TerminalNode* SysYParser::SubContext::Sub() {
  return getToken(SysYParser::Sub, 0);
}

SysYParser::MulExpContext* SysYParser::SubContext::mulExp() {
  return getRuleContext<SysYParser::MulExpContext>(0);
}

SysYParser::SubContext::SubContext(AddExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::SubContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitSub(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::AddExpContext* SysYParser::addExp() {
   return addExp(0);
}

SysYParser::AddExpContext* SysYParser::addExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::AddExpContext *_localctx = _tracker.createInstance<AddExpContext>(_ctx, parentState);
  SysYParser::AddExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 56;
  enterRecursionRule(_localctx, 56, SysYParser::RuleAddExp, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<AddExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(321);
    mulExp(0);
    _ctx->stop = _input->LT(-1);
    setState(331);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(329);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<AddContext>(_tracker.createInstance<AddExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleAddExp);
          setState(323);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(324);
          match(SysYParser::Add);
          setState(325);
          mulExp(0);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<SubContext>(_tracker.createInstance<AddExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleAddExp);
          setState(326);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(327);
          match(SysYParser::Sub);
          setState(328);
          mulExp(0);
          break;
        }

        } 
      }
      setState(333);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- RelExpContext ------------------------------------------------------------------

SysYParser::RelExpContext::RelExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::RelExpContext::getRuleIndex() const {
  return SysYParser::RuleRelExp;
}

void SysYParser::RelExpContext::copyFrom(RelExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- GeqContext ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::GeqContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

tree::TerminalNode* SysYParser::GeqContext::Geq() {
  return getToken(SysYParser::Geq, 0);
}

SysYParser::AddExpContext* SysYParser::GeqContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::GeqContext::GeqContext(RelExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::GeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitGeq(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LtContext ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::LtContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

tree::TerminalNode* SysYParser::LtContext::Lt() {
  return getToken(SysYParser::Lt, 0);
}

SysYParser::AddExpContext* SysYParser::LtContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::LtContext::LtContext(RelExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::LtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- RelExp_Context ------------------------------------------------------------------

SysYParser::AddExpContext* SysYParser::RelExp_Context::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::RelExp_Context::RelExp_Context(RelExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::RelExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitRelExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LeqContext ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::LeqContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

tree::TerminalNode* SysYParser::LeqContext::Leq() {
  return getToken(SysYParser::Leq, 0);
}

SysYParser::AddExpContext* SysYParser::LeqContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::LeqContext::LeqContext(RelExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::LeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLeq(this);
  else
    return visitor->visitChildren(this);
}
//----------------- GtContext ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::GtContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

tree::TerminalNode* SysYParser::GtContext::Gt() {
  return getToken(SysYParser::Gt, 0);
}

SysYParser::AddExpContext* SysYParser::GtContext::addExp() {
  return getRuleContext<SysYParser::AddExpContext>(0);
}

SysYParser::GtContext::GtContext(RelExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::GtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitGt(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::RelExpContext* SysYParser::relExp() {
   return relExp(0);
}

SysYParser::RelExpContext* SysYParser::relExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::RelExpContext *_localctx = _tracker.createInstance<RelExpContext>(_ctx, parentState);
  SysYParser::RelExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 58;
  enterRecursionRule(_localctx, 58, SysYParser::RuleRelExp, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<RelExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(335);
    addExp(0);
    _ctx->stop = _input->LT(-1);
    setState(351);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(349);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LtContext>(_tracker.createInstance<RelExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleRelExp);
          setState(337);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(338);
          match(SysYParser::Lt);
          setState(339);
          addExp(0);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<GtContext>(_tracker.createInstance<RelExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleRelExp);
          setState(340);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(341);
          match(SysYParser::Gt);
          setState(342);
          addExp(0);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<LeqContext>(_tracker.createInstance<RelExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleRelExp);
          setState(343);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(344);
          match(SysYParser::Leq);
          setState(345);
          addExp(0);
          break;
        }

        case 4: {
          auto newContext = _tracker.createInstance<GeqContext>(_tracker.createInstance<RelExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleRelExp);
          setState(346);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(347);
          match(SysYParser::Geq);
          setState(348);
          addExp(0);
          break;
        }

        } 
      }
      setState(353);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- EqExpContext ------------------------------------------------------------------

SysYParser::EqExpContext::EqExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::EqExpContext::getRuleIndex() const {
  return SysYParser::RuleEqExp;
}

void SysYParser::EqExpContext::copyFrom(EqExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- NeqContext ------------------------------------------------------------------

SysYParser::EqExpContext* SysYParser::NeqContext::eqExp() {
  return getRuleContext<SysYParser::EqExpContext>(0);
}

tree::TerminalNode* SysYParser::NeqContext::Neq() {
  return getToken(SysYParser::Neq, 0);
}

SysYParser::RelExpContext* SysYParser::NeqContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

SysYParser::NeqContext::NeqContext(EqExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::NeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitNeq(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EqContext ------------------------------------------------------------------

SysYParser::EqExpContext* SysYParser::EqContext::eqExp() {
  return getRuleContext<SysYParser::EqExpContext>(0);
}

tree::TerminalNode* SysYParser::EqContext::Eq() {
  return getToken(SysYParser::Eq, 0);
}

SysYParser::RelExpContext* SysYParser::EqContext::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

SysYParser::EqContext::EqContext(EqExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::EqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitEq(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EqExp_Context ------------------------------------------------------------------

SysYParser::RelExpContext* SysYParser::EqExp_Context::relExp() {
  return getRuleContext<SysYParser::RelExpContext>(0);
}

SysYParser::EqExp_Context::EqExp_Context(EqExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::EqExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitEqExp_(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::EqExpContext* SysYParser::eqExp() {
   return eqExp(0);
}

SysYParser::EqExpContext* SysYParser::eqExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::EqExpContext *_localctx = _tracker.createInstance<EqExpContext>(_ctx, parentState);
  SysYParser::EqExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 60;
  enterRecursionRule(_localctx, 60, SysYParser::RuleEqExp, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<EqExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(355);
    relExp(0);
    _ctx->stop = _input->LT(-1);
    setState(365);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(363);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<EqContext>(_tracker.createInstance<EqExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleEqExp);
          setState(357);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(358);
          match(SysYParser::Eq);
          setState(359);
          relExp(0);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<NeqContext>(_tracker.createInstance<EqExpContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleEqExp);
          setState(360);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(361);
          match(SysYParser::Neq);
          setState(362);
          relExp(0);
          break;
        }

        } 
      }
      setState(367);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- LAndExpContext ------------------------------------------------------------------

SysYParser::LAndExpContext::LAndExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::LAndExpContext::getRuleIndex() const {
  return SysYParser::RuleLAndExp;
}

void SysYParser::LAndExpContext::copyFrom(LAndExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- LAndExp_Context ------------------------------------------------------------------

SysYParser::EqExpContext* SysYParser::LAndExp_Context::eqExp() {
  return getRuleContext<SysYParser::EqExpContext>(0);
}

SysYParser::LAndExp_Context::LAndExp_Context(LAndExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::LAndExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLAndExp_(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AndContext ------------------------------------------------------------------

SysYParser::LAndExpContext* SysYParser::AndContext::lAndExp() {
  return getRuleContext<SysYParser::LAndExpContext>(0);
}

tree::TerminalNode* SysYParser::AndContext::And() {
  return getToken(SysYParser::And, 0);
}

SysYParser::EqExpContext* SysYParser::AndContext::eqExp() {
  return getRuleContext<SysYParser::EqExpContext>(0);
}

SysYParser::AndContext::AndContext(LAndExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::AndContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitAnd(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::LAndExpContext* SysYParser::lAndExp() {
   return lAndExp(0);
}

SysYParser::LAndExpContext* SysYParser::lAndExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::LAndExpContext *_localctx = _tracker.createInstance<LAndExpContext>(_ctx, parentState);
  SysYParser::LAndExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 62;
  enterRecursionRule(_localctx, 62, SysYParser::RuleLAndExp, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<LAndExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(369);
    eqExp(0);
    _ctx->stop = _input->LT(-1);
    setState(376);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<AndContext>(_tracker.createInstance<LAndExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleLAndExp);
        setState(371);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(372);
        match(SysYParser::And);
        setState(373);
        eqExp(0); 
      }
      setState(378);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- LOrExpContext ------------------------------------------------------------------

SysYParser::LOrExpContext::LOrExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysYParser::LOrExpContext::getRuleIndex() const {
  return SysYParser::RuleLOrExp;
}

void SysYParser::LOrExpContext::copyFrom(LOrExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- OrContext ------------------------------------------------------------------

SysYParser::LOrExpContext* SysYParser::OrContext::lOrExp() {
  return getRuleContext<SysYParser::LOrExpContext>(0);
}

tree::TerminalNode* SysYParser::OrContext::Or() {
  return getToken(SysYParser::Or, 0);
}

SysYParser::LAndExpContext* SysYParser::OrContext::lAndExp() {
  return getRuleContext<SysYParser::LAndExpContext>(0);
}

SysYParser::OrContext::OrContext(LOrExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::OrContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitOr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LOrExp_Context ------------------------------------------------------------------

SysYParser::LAndExpContext* SysYParser::LOrExp_Context::lAndExp() {
  return getRuleContext<SysYParser::LAndExpContext>(0);
}

SysYParser::LOrExp_Context::LOrExp_Context(LOrExpContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SysYParser::LOrExp_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysYVisitor*>(visitor))
    return parserVisitor->visitLOrExp_(this);
  else
    return visitor->visitChildren(this);
}

SysYParser::LOrExpContext* SysYParser::lOrExp() {
   return lOrExp(0);
}

SysYParser::LOrExpContext* SysYParser::lOrExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysYParser::LOrExpContext *_localctx = _tracker.createInstance<LOrExpContext>(_ctx, parentState);
  SysYParser::LOrExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 64;
  enterRecursionRule(_localctx, 64, SysYParser::RuleLOrExp, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<LOrExp_Context>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(380);
    lAndExp(0);
    _ctx->stop = _input->LT(-1);
    setState(387);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<OrContext>(_tracker.createInstance<LOrExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleLOrExp);
        setState(382);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(383);
        match(SysYParser::Or);
        setState(384);
        lAndExp(0); 
      }
      setState(389);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

bool SysYParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 27: return mulExpSempred(dynamic_cast<MulExpContext *>(context), predicateIndex);
    case 28: return addExpSempred(dynamic_cast<AddExpContext *>(context), predicateIndex);
    case 29: return relExpSempred(dynamic_cast<RelExpContext *>(context), predicateIndex);
    case 30: return eqExpSempred(dynamic_cast<EqExpContext *>(context), predicateIndex);
    case 31: return lAndExpSempred(dynamic_cast<LAndExpContext *>(context), predicateIndex);
    case 32: return lOrExpSempred(dynamic_cast<LOrExpContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool SysYParser::mulExpSempred(MulExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 3);
    case 1: return precpred(_ctx, 2);
    case 2: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::addExpSempred(AddExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 3: return precpred(_ctx, 2);
    case 4: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::relExpSempred(RelExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 5: return precpred(_ctx, 4);
    case 6: return precpred(_ctx, 3);
    case 7: return precpred(_ctx, 2);
    case 8: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::eqExpSempred(EqExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 9: return precpred(_ctx, 2);
    case 10: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::lAndExpSempred(LAndExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 11: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysYParser::lOrExpSempred(LOrExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 12: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

// Static vars and initialization.
std::vector<dfa::DFA> SysYParser::_decisionToDFA;
atn::PredictionContextCache SysYParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN SysYParser::_atn;
std::vector<uint16_t> SysYParser::_serializedATN;

std::vector<std::string> SysYParser::_ruleNames = {
  "compUnit", "compUnitItem", "decl", "constDecl", "bType", "constDef", 
  "varDecl", "varDef", "initVal", "funcDef", "funcType", "funcFParams", 
  "funcFParam", "block", "blockItem", "stmt", "exp", "cond", "lVal", "primaryExp", 
  "intConst", "floatConst", "number", "unaryExp", "stringConst", "funcRParam", 
  "funcRParams", "mulExp", "addExp", "relExp", "eqExp", "lAndExp", "lOrExp"
};

std::vector<std::string> SysYParser::_literalNames = {
  "", "'int'", "'float'", "'void'", "'const'", "'if'", "'else'", "'while'", 
  "'break'", "'continue'", "'return'", "'='", "'+'", "'-'", "'*'", "'/'", 
  "'%'", "'=='", "'!='", "'<'", "'>'", "'<='", "'>='", "'!'", "'&&'", "'||'", 
  "','", "';'", "'('", "')'", "'['", "']'", "'{'", "'}'"
};

std::vector<std::string> SysYParser::_symbolicNames = {
  "", "Int", "Float", "Void", "Const", "If", "Else", "While", "Break", "Continue", 
  "Return", "Assign", "Add", "Sub", "Mul", "Div", "Mod", "Eq", "Neq", "Lt", 
  "Gt", "Leq", "Geq", "Not", "And", "Or", "Comma", "Semicolon", "Lparen", 
  "Rparen", "Lbracket", "Rbracket", "Lbrace", "Rbrace", "Ident", "Whitespace", 
  "LineComment", "BlockComment", "DecIntConst", "OctIntConst", "HexIntConst", 
  "DecFloatConst", "HexFloatConst", "StringConst"
};

dfa::Vocabulary SysYParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> SysYParser::_tokenNames;

SysYParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0x2d, 0x189, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 
    0x9, 0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 0x7, 
    0x4, 0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 0x4, 0xb, 
    0x9, 0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 0xe, 0x9, 0xe, 
    0x4, 0xf, 0x9, 0xf, 0x4, 0x10, 0x9, 0x10, 0x4, 0x11, 0x9, 0x11, 0x4, 
    0x12, 0x9, 0x12, 0x4, 0x13, 0x9, 0x13, 0x4, 0x14, 0x9, 0x14, 0x4, 0x15, 
    0x9, 0x15, 0x4, 0x16, 0x9, 0x16, 0x4, 0x17, 0x9, 0x17, 0x4, 0x18, 0x9, 
    0x18, 0x4, 0x19, 0x9, 0x19, 0x4, 0x1a, 0x9, 0x1a, 0x4, 0x1b, 0x9, 0x1b, 
    0x4, 0x1c, 0x9, 0x1c, 0x4, 0x1d, 0x9, 0x1d, 0x4, 0x1e, 0x9, 0x1e, 0x4, 
    0x1f, 0x9, 0x1f, 0x4, 0x20, 0x9, 0x20, 0x4, 0x21, 0x9, 0x21, 0x4, 0x22, 
    0x9, 0x22, 0x3, 0x2, 0x7, 0x2, 0x46, 0xa, 0x2, 0xc, 0x2, 0xe, 0x2, 0x49, 
    0xb, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x3, 0x3, 0x3, 0x5, 0x3, 0x4f, 0xa, 
    0x3, 0x3, 0x4, 0x3, 0x4, 0x5, 0x4, 0x53, 0xa, 0x4, 0x3, 0x5, 0x3, 0x5, 
    0x3, 0x5, 0x3, 0x5, 0x3, 0x5, 0x7, 0x5, 0x5a, 0xa, 0x5, 0xc, 0x5, 0xe, 
    0x5, 0x5d, 0xb, 0x5, 0x3, 0x5, 0x3, 0x5, 0x3, 0x6, 0x3, 0x6, 0x5, 0x6, 
    0x63, 0xa, 0x6, 0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x7, 
    0x7, 0x6a, 0xa, 0x7, 0xc, 0x7, 0xe, 0x7, 0x6d, 0xb, 0x7, 0x3, 0x7, 0x3, 
    0x7, 0x3, 0x7, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x7, 0x8, 0x76, 
    0xa, 0x8, 0xc, 0x8, 0xe, 0x8, 0x79, 0xb, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 
    0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x7, 0x9, 0x82, 0xa, 0x9, 
    0xc, 0x9, 0xe, 0x9, 0x85, 0xb, 0x9, 0x3, 0x9, 0x3, 0x9, 0x5, 0x9, 0x89, 
    0xa, 0x9, 0x3, 0xa, 0x3, 0xa, 0x3, 0xa, 0x3, 0xa, 0x3, 0xa, 0x7, 0xa, 
    0x90, 0xa, 0xa, 0xc, 0xa, 0xe, 0xa, 0x93, 0xb, 0xa, 0x5, 0xa, 0x95, 
    0xa, 0xa, 0x3, 0xa, 0x5, 0xa, 0x98, 0xa, 0xa, 0x3, 0xb, 0x3, 0xb, 0x3, 
    0xb, 0x3, 0xb, 0x5, 0xb, 0x9e, 0xa, 0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 
    0x3, 0xc, 0x3, 0xc, 0x5, 0xc, 0xa5, 0xa, 0xc, 0x3, 0xd, 0x3, 0xd, 0x3, 
    0xd, 0x7, 0xd, 0xaa, 0xa, 0xd, 0xc, 0xd, 0xe, 0xd, 0xad, 0xb, 0xd, 0x3, 
    0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 
    0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x7, 0xe, 0xba, 0xa, 0xe, 0xc, 0xe, 
    0xe, 0xe, 0xbd, 0xb, 0xe, 0x5, 0xe, 0xbf, 0xa, 0xe, 0x3, 0xf, 0x3, 0xf, 
    0x7, 0xf, 0xc3, 0xa, 0xf, 0xc, 0xf, 0xe, 0xf, 0xc6, 0xb, 0xf, 0x3, 0xf, 
    0x3, 0xf, 0x3, 0x10, 0x3, 0x10, 0x5, 0x10, 0xcc, 0xa, 0x10, 0x3, 0x11, 
    0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x5, 0x11, 0xd4, 
    0xa, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 
    0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x5, 0x11, 0xdf, 0xa, 0x11, 0x3, 
    0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 
    0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x5, 0x11, 0xed, 
    0xa, 0x11, 0x3, 0x11, 0x5, 0x11, 0xf0, 0xa, 0x11, 0x3, 0x12, 0x3, 0x12, 
    0x3, 0x13, 0x3, 0x13, 0x3, 0x14, 0x3, 0x14, 0x3, 0x14, 0x3, 0x14, 0x3, 
    0x14, 0x7, 0x14, 0xfb, 0xa, 0x14, 0xc, 0x14, 0xe, 0x14, 0xfe, 0xb, 0x14, 
    0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x5, 
    0x15, 0x106, 0xa, 0x15, 0x3, 0x16, 0x3, 0x16, 0x3, 0x16, 0x5, 0x16, 
    0x10b, 0xa, 0x16, 0x3, 0x17, 0x3, 0x17, 0x5, 0x17, 0x10f, 0xa, 0x17, 
    0x3, 0x18, 0x3, 0x18, 0x5, 0x18, 0x113, 0xa, 0x18, 0x3, 0x19, 0x3, 0x19, 
    0x3, 0x19, 0x3, 0x19, 0x5, 0x19, 0x119, 0xa, 0x19, 0x3, 0x19, 0x3, 0x19, 
    0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x5, 0x19, 0x122, 
    0xa, 0x19, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1b, 0x3, 0x1b, 0x5, 0x1b, 0x128, 
    0xa, 0x1b, 0x3, 0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x7, 0x1c, 0x12d, 0xa, 0x1c, 
    0xc, 0x1c, 0xe, 0x1c, 0x130, 0xb, 0x1c, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 
    0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 
    0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x7, 0x1d, 0x13e, 0xa, 0x1d, 0xc, 0x1d, 
    0xe, 0x1d, 0x141, 0xb, 0x1d, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 
    0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x7, 0x1e, 0x14c, 
    0xa, 0x1e, 0xc, 0x1e, 0xe, 0x1e, 0x14f, 0xb, 0x1e, 0x3, 0x1f, 0x3, 0x1f, 
    0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 
    0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 
    0x7, 0x1f, 0x160, 0xa, 0x1f, 0xc, 0x1f, 0xe, 0x1f, 0x163, 0xb, 0x1f, 
    0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 
    0x20, 0x3, 0x20, 0x3, 0x20, 0x7, 0x20, 0x16e, 0xa, 0x20, 0xc, 0x20, 
    0xe, 0x20, 0x171, 0xb, 0x20, 0x3, 0x21, 0x3, 0x21, 0x3, 0x21, 0x3, 0x21, 
    0x3, 0x21, 0x3, 0x21, 0x7, 0x21, 0x179, 0xa, 0x21, 0xc, 0x21, 0xe, 0x21, 
    0x17c, 0xb, 0x21, 0x3, 0x22, 0x3, 0x22, 0x3, 0x22, 0x3, 0x22, 0x3, 0x22, 
    0x3, 0x22, 0x7, 0x22, 0x184, 0xa, 0x22, 0xc, 0x22, 0xe, 0x22, 0x187, 
    0xb, 0x22, 0x3, 0x22, 0x2, 0x8, 0x38, 0x3a, 0x3c, 0x3e, 0x40, 0x42, 
    0x23, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 0x10, 0x12, 0x14, 0x16, 0x18, 
    0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 
    0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e, 0x40, 0x42, 0x2, 0x2, 0x2, 
    0x19f, 0x2, 0x47, 0x3, 0x2, 0x2, 0x2, 0x4, 0x4e, 0x3, 0x2, 0x2, 0x2, 
    0x6, 0x52, 0x3, 0x2, 0x2, 0x2, 0x8, 0x54, 0x3, 0x2, 0x2, 0x2, 0xa, 0x62, 
    0x3, 0x2, 0x2, 0x2, 0xc, 0x64, 0x3, 0x2, 0x2, 0x2, 0xe, 0x71, 0x3, 0x2, 
    0x2, 0x2, 0x10, 0x7c, 0x3, 0x2, 0x2, 0x2, 0x12, 0x97, 0x3, 0x2, 0x2, 
    0x2, 0x14, 0x99, 0x3, 0x2, 0x2, 0x2, 0x16, 0xa4, 0x3, 0x2, 0x2, 0x2, 
    0x18, 0xa6, 0x3, 0x2, 0x2, 0x2, 0x1a, 0xbe, 0x3, 0x2, 0x2, 0x2, 0x1c, 
    0xc0, 0x3, 0x2, 0x2, 0x2, 0x1e, 0xcb, 0x3, 0x2, 0x2, 0x2, 0x20, 0xef, 
    0x3, 0x2, 0x2, 0x2, 0x22, 0xf1, 0x3, 0x2, 0x2, 0x2, 0x24, 0xf3, 0x3, 
    0x2, 0x2, 0x2, 0x26, 0xf5, 0x3, 0x2, 0x2, 0x2, 0x28, 0x105, 0x3, 0x2, 
    0x2, 0x2, 0x2a, 0x10a, 0x3, 0x2, 0x2, 0x2, 0x2c, 0x10e, 0x3, 0x2, 0x2, 
    0x2, 0x2e, 0x112, 0x3, 0x2, 0x2, 0x2, 0x30, 0x121, 0x3, 0x2, 0x2, 0x2, 
    0x32, 0x123, 0x3, 0x2, 0x2, 0x2, 0x34, 0x127, 0x3, 0x2, 0x2, 0x2, 0x36, 
    0x129, 0x3, 0x2, 0x2, 0x2, 0x38, 0x131, 0x3, 0x2, 0x2, 0x2, 0x3a, 0x142, 
    0x3, 0x2, 0x2, 0x2, 0x3c, 0x150, 0x3, 0x2, 0x2, 0x2, 0x3e, 0x164, 0x3, 
    0x2, 0x2, 0x2, 0x40, 0x172, 0x3, 0x2, 0x2, 0x2, 0x42, 0x17d, 0x3, 0x2, 
    0x2, 0x2, 0x44, 0x46, 0x5, 0x4, 0x3, 0x2, 0x45, 0x44, 0x3, 0x2, 0x2, 
    0x2, 0x46, 0x49, 0x3, 0x2, 0x2, 0x2, 0x47, 0x45, 0x3, 0x2, 0x2, 0x2, 
    0x47, 0x48, 0x3, 0x2, 0x2, 0x2, 0x48, 0x4a, 0x3, 0x2, 0x2, 0x2, 0x49, 
    0x47, 0x3, 0x2, 0x2, 0x2, 0x4a, 0x4b, 0x7, 0x2, 0x2, 0x3, 0x4b, 0x3, 
    0x3, 0x2, 0x2, 0x2, 0x4c, 0x4f, 0x5, 0x6, 0x4, 0x2, 0x4d, 0x4f, 0x5, 
    0x14, 0xb, 0x2, 0x4e, 0x4c, 0x3, 0x2, 0x2, 0x2, 0x4e, 0x4d, 0x3, 0x2, 
    0x2, 0x2, 0x4f, 0x5, 0x3, 0x2, 0x2, 0x2, 0x50, 0x53, 0x5, 0x8, 0x5, 
    0x2, 0x51, 0x53, 0x5, 0xe, 0x8, 0x2, 0x52, 0x50, 0x3, 0x2, 0x2, 0x2, 
    0x52, 0x51, 0x3, 0x2, 0x2, 0x2, 0x53, 0x7, 0x3, 0x2, 0x2, 0x2, 0x54, 
    0x55, 0x7, 0x6, 0x2, 0x2, 0x55, 0x56, 0x5, 0xa, 0x6, 0x2, 0x56, 0x5b, 
    0x5, 0xc, 0x7, 0x2, 0x57, 0x58, 0x7, 0x1c, 0x2, 0x2, 0x58, 0x5a, 0x5, 
    0xc, 0x7, 0x2, 0x59, 0x57, 0x3, 0x2, 0x2, 0x2, 0x5a, 0x5d, 0x3, 0x2, 
    0x2, 0x2, 0x5b, 0x59, 0x3, 0x2, 0x2, 0x2, 0x5b, 0x5c, 0x3, 0x2, 0x2, 
    0x2, 0x5c, 0x5e, 0x3, 0x2, 0x2, 0x2, 0x5d, 0x5b, 0x3, 0x2, 0x2, 0x2, 
    0x5e, 0x5f, 0x7, 0x1d, 0x2, 0x2, 0x5f, 0x9, 0x3, 0x2, 0x2, 0x2, 0x60, 
    0x63, 0x7, 0x3, 0x2, 0x2, 0x61, 0x63, 0x7, 0x4, 0x2, 0x2, 0x62, 0x60, 
    0x3, 0x2, 0x2, 0x2, 0x62, 0x61, 0x3, 0x2, 0x2, 0x2, 0x63, 0xb, 0x3, 
    0x2, 0x2, 0x2, 0x64, 0x6b, 0x7, 0x24, 0x2, 0x2, 0x65, 0x66, 0x7, 0x20, 
    0x2, 0x2, 0x66, 0x67, 0x5, 0x22, 0x12, 0x2, 0x67, 0x68, 0x7, 0x21, 0x2, 
    0x2, 0x68, 0x6a, 0x3, 0x2, 0x2, 0x2, 0x69, 0x65, 0x3, 0x2, 0x2, 0x2, 
    0x6a, 0x6d, 0x3, 0x2, 0x2, 0x2, 0x6b, 0x69, 0x3, 0x2, 0x2, 0x2, 0x6b, 
    0x6c, 0x3, 0x2, 0x2, 0x2, 0x6c, 0x6e, 0x3, 0x2, 0x2, 0x2, 0x6d, 0x6b, 
    0x3, 0x2, 0x2, 0x2, 0x6e, 0x6f, 0x7, 0xd, 0x2, 0x2, 0x6f, 0x70, 0x5, 
    0x12, 0xa, 0x2, 0x70, 0xd, 0x3, 0x2, 0x2, 0x2, 0x71, 0x72, 0x5, 0xa, 
    0x6, 0x2, 0x72, 0x77, 0x5, 0x10, 0x9, 0x2, 0x73, 0x74, 0x7, 0x1c, 0x2, 
    0x2, 0x74, 0x76, 0x5, 0x10, 0x9, 0x2, 0x75, 0x73, 0x3, 0x2, 0x2, 0x2, 
    0x76, 0x79, 0x3, 0x2, 0x2, 0x2, 0x77, 0x75, 0x3, 0x2, 0x2, 0x2, 0x77, 
    0x78, 0x3, 0x2, 0x2, 0x2, 0x78, 0x7a, 0x3, 0x2, 0x2, 0x2, 0x79, 0x77, 
    0x3, 0x2, 0x2, 0x2, 0x7a, 0x7b, 0x7, 0x1d, 0x2, 0x2, 0x7b, 0xf, 0x3, 
    0x2, 0x2, 0x2, 0x7c, 0x83, 0x7, 0x24, 0x2, 0x2, 0x7d, 0x7e, 0x7, 0x20, 
    0x2, 0x2, 0x7e, 0x7f, 0x5, 0x22, 0x12, 0x2, 0x7f, 0x80, 0x7, 0x21, 0x2, 
    0x2, 0x80, 0x82, 0x3, 0x2, 0x2, 0x2, 0x81, 0x7d, 0x3, 0x2, 0x2, 0x2, 
    0x82, 0x85, 0x3, 0x2, 0x2, 0x2, 0x83, 0x81, 0x3, 0x2, 0x2, 0x2, 0x83, 
    0x84, 0x3, 0x2, 0x2, 0x2, 0x84, 0x88, 0x3, 0x2, 0x2, 0x2, 0x85, 0x83, 
    0x3, 0x2, 0x2, 0x2, 0x86, 0x87, 0x7, 0xd, 0x2, 0x2, 0x87, 0x89, 0x5, 
    0x12, 0xa, 0x2, 0x88, 0x86, 0x3, 0x2, 0x2, 0x2, 0x88, 0x89, 0x3, 0x2, 
    0x2, 0x2, 0x89, 0x11, 0x3, 0x2, 0x2, 0x2, 0x8a, 0x98, 0x5, 0x22, 0x12, 
    0x2, 0x8b, 0x94, 0x7, 0x22, 0x2, 0x2, 0x8c, 0x91, 0x5, 0x12, 0xa, 0x2, 
    0x8d, 0x8e, 0x7, 0x1c, 0x2, 0x2, 0x8e, 0x90, 0x5, 0x12, 0xa, 0x2, 0x8f, 
    0x8d, 0x3, 0x2, 0x2, 0x2, 0x90, 0x93, 0x3, 0x2, 0x2, 0x2, 0x91, 0x8f, 
    0x3, 0x2, 0x2, 0x2, 0x91, 0x92, 0x3, 0x2, 0x2, 0x2, 0x92, 0x95, 0x3, 
    0x2, 0x2, 0x2, 0x93, 0x91, 0x3, 0x2, 0x2, 0x2, 0x94, 0x8c, 0x3, 0x2, 
    0x2, 0x2, 0x94, 0x95, 0x3, 0x2, 0x2, 0x2, 0x95, 0x96, 0x3, 0x2, 0x2, 
    0x2, 0x96, 0x98, 0x7, 0x23, 0x2, 0x2, 0x97, 0x8a, 0x3, 0x2, 0x2, 0x2, 
    0x97, 0x8b, 0x3, 0x2, 0x2, 0x2, 0x98, 0x13, 0x3, 0x2, 0x2, 0x2, 0x99, 
    0x9a, 0x5, 0x16, 0xc, 0x2, 0x9a, 0x9b, 0x7, 0x24, 0x2, 0x2, 0x9b, 0x9d, 
    0x7, 0x1e, 0x2, 0x2, 0x9c, 0x9e, 0x5, 0x18, 0xd, 0x2, 0x9d, 0x9c, 0x3, 
    0x2, 0x2, 0x2, 0x9d, 0x9e, 0x3, 0x2, 0x2, 0x2, 0x9e, 0x9f, 0x3, 0x2, 
    0x2, 0x2, 0x9f, 0xa0, 0x7, 0x1f, 0x2, 0x2, 0xa0, 0xa1, 0x5, 0x1c, 0xf, 
    0x2, 0xa1, 0x15, 0x3, 0x2, 0x2, 0x2, 0xa2, 0xa5, 0x5, 0xa, 0x6, 0x2, 
    0xa3, 0xa5, 0x7, 0x5, 0x2, 0x2, 0xa4, 0xa2, 0x3, 0x2, 0x2, 0x2, 0xa4, 
    0xa3, 0x3, 0x2, 0x2, 0x2, 0xa5, 0x17, 0x3, 0x2, 0x2, 0x2, 0xa6, 0xab, 
    0x5, 0x1a, 0xe, 0x2, 0xa7, 0xa8, 0x7, 0x1c, 0x2, 0x2, 0xa8, 0xaa, 0x5, 
    0x1a, 0xe, 0x2, 0xa9, 0xa7, 0x3, 0x2, 0x2, 0x2, 0xaa, 0xad, 0x3, 0x2, 
    0x2, 0x2, 0xab, 0xa9, 0x3, 0x2, 0x2, 0x2, 0xab, 0xac, 0x3, 0x2, 0x2, 
    0x2, 0xac, 0x19, 0x3, 0x2, 0x2, 0x2, 0xad, 0xab, 0x3, 0x2, 0x2, 0x2, 
    0xae, 0xaf, 0x5, 0xa, 0x6, 0x2, 0xaf, 0xb0, 0x7, 0x24, 0x2, 0x2, 0xb0, 
    0xbf, 0x3, 0x2, 0x2, 0x2, 0xb1, 0xb2, 0x5, 0xa, 0x6, 0x2, 0xb2, 0xb3, 
    0x7, 0x24, 0x2, 0x2, 0xb3, 0xb4, 0x7, 0x20, 0x2, 0x2, 0xb4, 0xbb, 0x7, 
    0x21, 0x2, 0x2, 0xb5, 0xb6, 0x7, 0x20, 0x2, 0x2, 0xb6, 0xb7, 0x5, 0x22, 
    0x12, 0x2, 0xb7, 0xb8, 0x7, 0x21, 0x2, 0x2, 0xb8, 0xba, 0x3, 0x2, 0x2, 
    0x2, 0xb9, 0xb5, 0x3, 0x2, 0x2, 0x2, 0xba, 0xbd, 0x3, 0x2, 0x2, 0x2, 
    0xbb, 0xb9, 0x3, 0x2, 0x2, 0x2, 0xbb, 0xbc, 0x3, 0x2, 0x2, 0x2, 0xbc, 
    0xbf, 0x3, 0x2, 0x2, 0x2, 0xbd, 0xbb, 0x3, 0x2, 0x2, 0x2, 0xbe, 0xae, 
    0x3, 0x2, 0x2, 0x2, 0xbe, 0xb1, 0x3, 0x2, 0x2, 0x2, 0xbf, 0x1b, 0x3, 
    0x2, 0x2, 0x2, 0xc0, 0xc4, 0x7, 0x22, 0x2, 0x2, 0xc1, 0xc3, 0x5, 0x1e, 
    0x10, 0x2, 0xc2, 0xc1, 0x3, 0x2, 0x2, 0x2, 0xc3, 0xc6, 0x3, 0x2, 0x2, 
    0x2, 0xc4, 0xc2, 0x3, 0x2, 0x2, 0x2, 0xc4, 0xc5, 0x3, 0x2, 0x2, 0x2, 
    0xc5, 0xc7, 0x3, 0x2, 0x2, 0x2, 0xc6, 0xc4, 0x3, 0x2, 0x2, 0x2, 0xc7, 
    0xc8, 0x7, 0x23, 0x2, 0x2, 0xc8, 0x1d, 0x3, 0x2, 0x2, 0x2, 0xc9, 0xcc, 
    0x5, 0x6, 0x4, 0x2, 0xca, 0xcc, 0x5, 0x20, 0x11, 0x2, 0xcb, 0xc9, 0x3, 
    0x2, 0x2, 0x2, 0xcb, 0xca, 0x3, 0x2, 0x2, 0x2, 0xcc, 0x1f, 0x3, 0x2, 
    0x2, 0x2, 0xcd, 0xce, 0x5, 0x26, 0x14, 0x2, 0xce, 0xcf, 0x7, 0xd, 0x2, 
    0x2, 0xcf, 0xd0, 0x5, 0x22, 0x12, 0x2, 0xd0, 0xd1, 0x7, 0x1d, 0x2, 0x2, 
    0xd1, 0xf0, 0x3, 0x2, 0x2, 0x2, 0xd2, 0xd4, 0x5, 0x22, 0x12, 0x2, 0xd3, 
    0xd2, 0x3, 0x2, 0x2, 0x2, 0xd3, 0xd4, 0x3, 0x2, 0x2, 0x2, 0xd4, 0xd5, 
    0x3, 0x2, 0x2, 0x2, 0xd5, 0xf0, 0x7, 0x1d, 0x2, 0x2, 0xd6, 0xf0, 0x5, 
    0x1c, 0xf, 0x2, 0xd7, 0xd8, 0x7, 0x7, 0x2, 0x2, 0xd8, 0xd9, 0x7, 0x1e, 
    0x2, 0x2, 0xd9, 0xda, 0x5, 0x24, 0x13, 0x2, 0xda, 0xdb, 0x7, 0x1f, 0x2, 
    0x2, 0xdb, 0xde, 0x5, 0x20, 0x11, 0x2, 0xdc, 0xdd, 0x7, 0x8, 0x2, 0x2, 
    0xdd, 0xdf, 0x5, 0x20, 0x11, 0x2, 0xde, 0xdc, 0x3, 0x2, 0x2, 0x2, 0xde, 
    0xdf, 0x3, 0x2, 0x2, 0x2, 0xdf, 0xf0, 0x3, 0x2, 0x2, 0x2, 0xe0, 0xe1, 
    0x7, 0x9, 0x2, 0x2, 0xe1, 0xe2, 0x7, 0x1e, 0x2, 0x2, 0xe2, 0xe3, 0x5, 
    0x24, 0x13, 0x2, 0xe3, 0xe4, 0x7, 0x1f, 0x2, 0x2, 0xe4, 0xe5, 0x5, 0x20, 
    0x11, 0x2, 0xe5, 0xf0, 0x3, 0x2, 0x2, 0x2, 0xe6, 0xe7, 0x7, 0xa, 0x2, 
    0x2, 0xe7, 0xf0, 0x7, 0x1d, 0x2, 0x2, 0xe8, 0xe9, 0x7, 0xb, 0x2, 0x2, 
    0xe9, 0xf0, 0x7, 0x1d, 0x2, 0x2, 0xea, 0xec, 0x7, 0xc, 0x2, 0x2, 0xeb, 
    0xed, 0x5, 0x22, 0x12, 0x2, 0xec, 0xeb, 0x3, 0x2, 0x2, 0x2, 0xec, 0xed, 
    0x3, 0x2, 0x2, 0x2, 0xed, 0xee, 0x3, 0x2, 0x2, 0x2, 0xee, 0xf0, 0x7, 
    0x1d, 0x2, 0x2, 0xef, 0xcd, 0x3, 0x2, 0x2, 0x2, 0xef, 0xd3, 0x3, 0x2, 
    0x2, 0x2, 0xef, 0xd6, 0x3, 0x2, 0x2, 0x2, 0xef, 0xd7, 0x3, 0x2, 0x2, 
    0x2, 0xef, 0xe0, 0x3, 0x2, 0x2, 0x2, 0xef, 0xe6, 0x3, 0x2, 0x2, 0x2, 
    0xef, 0xe8, 0x3, 0x2, 0x2, 0x2, 0xef, 0xea, 0x3, 0x2, 0x2, 0x2, 0xf0, 
    0x21, 0x3, 0x2, 0x2, 0x2, 0xf1, 0xf2, 0x5, 0x3a, 0x1e, 0x2, 0xf2, 0x23, 
    0x3, 0x2, 0x2, 0x2, 0xf3, 0xf4, 0x5, 0x42, 0x22, 0x2, 0xf4, 0x25, 0x3, 
    0x2, 0x2, 0x2, 0xf5, 0xfc, 0x7, 0x24, 0x2, 0x2, 0xf6, 0xf7, 0x7, 0x20, 
    0x2, 0x2, 0xf7, 0xf8, 0x5, 0x22, 0x12, 0x2, 0xf8, 0xf9, 0x7, 0x21, 0x2, 
    0x2, 0xf9, 0xfb, 0x3, 0x2, 0x2, 0x2, 0xfa, 0xf6, 0x3, 0x2, 0x2, 0x2, 
    0xfb, 0xfe, 0x3, 0x2, 0x2, 0x2, 0xfc, 0xfa, 0x3, 0x2, 0x2, 0x2, 0xfc, 
    0xfd, 0x3, 0x2, 0x2, 0x2, 0xfd, 0x27, 0x3, 0x2, 0x2, 0x2, 0xfe, 0xfc, 
    0x3, 0x2, 0x2, 0x2, 0xff, 0x100, 0x7, 0x1e, 0x2, 0x2, 0x100, 0x101, 
    0x5, 0x22, 0x12, 0x2, 0x101, 0x102, 0x7, 0x1f, 0x2, 0x2, 0x102, 0x106, 
    0x3, 0x2, 0x2, 0x2, 0x103, 0x106, 0x5, 0x26, 0x14, 0x2, 0x104, 0x106, 
    0x5, 0x2e, 0x18, 0x2, 0x105, 0xff, 0x3, 0x2, 0x2, 0x2, 0x105, 0x103, 
    0x3, 0x2, 0x2, 0x2, 0x105, 0x104, 0x3, 0x2, 0x2, 0x2, 0x106, 0x29, 0x3, 
    0x2, 0x2, 0x2, 0x107, 0x10b, 0x7, 0x28, 0x2, 0x2, 0x108, 0x10b, 0x7, 
    0x29, 0x2, 0x2, 0x109, 0x10b, 0x7, 0x2a, 0x2, 0x2, 0x10a, 0x107, 0x3, 
    0x2, 0x2, 0x2, 0x10a, 0x108, 0x3, 0x2, 0x2, 0x2, 0x10a, 0x109, 0x3, 
    0x2, 0x2, 0x2, 0x10b, 0x2b, 0x3, 0x2, 0x2, 0x2, 0x10c, 0x10f, 0x7, 0x2b, 
    0x2, 0x2, 0x10d, 0x10f, 0x7, 0x2c, 0x2, 0x2, 0x10e, 0x10c, 0x3, 0x2, 
    0x2, 0x2, 0x10e, 0x10d, 0x3, 0x2, 0x2, 0x2, 0x10f, 0x2d, 0x3, 0x2, 0x2, 
    0x2, 0x110, 0x113, 0x5, 0x2a, 0x16, 0x2, 0x111, 0x113, 0x5, 0x2c, 0x17, 
    0x2, 0x112, 0x110, 0x3, 0x2, 0x2, 0x2, 0x112, 0x111, 0x3, 0x2, 0x2, 
    0x2, 0x113, 0x2f, 0x3, 0x2, 0x2, 0x2, 0x114, 0x122, 0x5, 0x28, 0x15, 
    0x2, 0x115, 0x116, 0x7, 0x24, 0x2, 0x2, 0x116, 0x118, 0x7, 0x1e, 0x2, 
    0x2, 0x117, 0x119, 0x5, 0x36, 0x1c, 0x2, 0x118, 0x117, 0x3, 0x2, 0x2, 
    0x2, 0x118, 0x119, 0x3, 0x2, 0x2, 0x2, 0x119, 0x11a, 0x3, 0x2, 0x2, 
    0x2, 0x11a, 0x122, 0x7, 0x1f, 0x2, 0x2, 0x11b, 0x11c, 0x7, 0xe, 0x2, 
    0x2, 0x11c, 0x122, 0x5, 0x30, 0x19, 0x2, 0x11d, 0x11e, 0x7, 0xf, 0x2, 
    0x2, 0x11e, 0x122, 0x5, 0x30, 0x19, 0x2, 0x11f, 0x120, 0x7, 0x19, 0x2, 
    0x2, 0x120, 0x122, 0x5, 0x30, 0x19, 0x2, 0x121, 0x114, 0x3, 0x2, 0x2, 
    0x2, 0x121, 0x115, 0x3, 0x2, 0x2, 0x2, 0x121, 0x11b, 0x3, 0x2, 0x2, 
    0x2, 0x121, 0x11d, 0x3, 0x2, 0x2, 0x2, 0x121, 0x11f, 0x3, 0x2, 0x2, 
    0x2, 0x122, 0x31, 0x3, 0x2, 0x2, 0x2, 0x123, 0x124, 0x7, 0x2d, 0x2, 
    0x2, 0x124, 0x33, 0x3, 0x2, 0x2, 0x2, 0x125, 0x128, 0x5, 0x22, 0x12, 
    0x2, 0x126, 0x128, 0x5, 0x32, 0x1a, 0x2, 0x127, 0x125, 0x3, 0x2, 0x2, 
    0x2, 0x127, 0x126, 0x3, 0x2, 0x2, 0x2, 0x128, 0x35, 0x3, 0x2, 0x2, 0x2, 
    0x129, 0x12e, 0x5, 0x34, 0x1b, 0x2, 0x12a, 0x12b, 0x7, 0x1c, 0x2, 0x2, 
    0x12b, 0x12d, 0x5, 0x34, 0x1b, 0x2, 0x12c, 0x12a, 0x3, 0x2, 0x2, 0x2, 
    0x12d, 0x130, 0x3, 0x2, 0x2, 0x2, 0x12e, 0x12c, 0x3, 0x2, 0x2, 0x2, 
    0x12e, 0x12f, 0x3, 0x2, 0x2, 0x2, 0x12f, 0x37, 0x3, 0x2, 0x2, 0x2, 0x130, 
    0x12e, 0x3, 0x2, 0x2, 0x2, 0x131, 0x132, 0x8, 0x1d, 0x1, 0x2, 0x132, 
    0x133, 0x5, 0x30, 0x19, 0x2, 0x133, 0x13f, 0x3, 0x2, 0x2, 0x2, 0x134, 
    0x135, 0xc, 0x5, 0x2, 0x2, 0x135, 0x136, 0x7, 0x10, 0x2, 0x2, 0x136, 
    0x13e, 0x5, 0x30, 0x19, 0x2, 0x137, 0x138, 0xc, 0x4, 0x2, 0x2, 0x138, 
    0x139, 0x7, 0x11, 0x2, 0x2, 0x139, 0x13e, 0x5, 0x30, 0x19, 0x2, 0x13a, 
    0x13b, 0xc, 0x3, 0x2, 0x2, 0x13b, 0x13c, 0x7, 0x12, 0x2, 0x2, 0x13c, 
    0x13e, 0x5, 0x30, 0x19, 0x2, 0x13d, 0x134, 0x3, 0x2, 0x2, 0x2, 0x13d, 
    0x137, 0x3, 0x2, 0x2, 0x2, 0x13d, 0x13a, 0x3, 0x2, 0x2, 0x2, 0x13e, 
    0x141, 0x3, 0x2, 0x2, 0x2, 0x13f, 0x13d, 0x3, 0x2, 0x2, 0x2, 0x13f, 
    0x140, 0x3, 0x2, 0x2, 0x2, 0x140, 0x39, 0x3, 0x2, 0x2, 0x2, 0x141, 0x13f, 
    0x3, 0x2, 0x2, 0x2, 0x142, 0x143, 0x8, 0x1e, 0x1, 0x2, 0x143, 0x144, 
    0x5, 0x38, 0x1d, 0x2, 0x144, 0x14d, 0x3, 0x2, 0x2, 0x2, 0x145, 0x146, 
    0xc, 0x4, 0x2, 0x2, 0x146, 0x147, 0x7, 0xe, 0x2, 0x2, 0x147, 0x14c, 
    0x5, 0x38, 0x1d, 0x2, 0x148, 0x149, 0xc, 0x3, 0x2, 0x2, 0x149, 0x14a, 
    0x7, 0xf, 0x2, 0x2, 0x14a, 0x14c, 0x5, 0x38, 0x1d, 0x2, 0x14b, 0x145, 
    0x3, 0x2, 0x2, 0x2, 0x14b, 0x148, 0x3, 0x2, 0x2, 0x2, 0x14c, 0x14f, 
    0x3, 0x2, 0x2, 0x2, 0x14d, 0x14b, 0x3, 0x2, 0x2, 0x2, 0x14d, 0x14e, 
    0x3, 0x2, 0x2, 0x2, 0x14e, 0x3b, 0x3, 0x2, 0x2, 0x2, 0x14f, 0x14d, 0x3, 
    0x2, 0x2, 0x2, 0x150, 0x151, 0x8, 0x1f, 0x1, 0x2, 0x151, 0x152, 0x5, 
    0x3a, 0x1e, 0x2, 0x152, 0x161, 0x3, 0x2, 0x2, 0x2, 0x153, 0x154, 0xc, 
    0x6, 0x2, 0x2, 0x154, 0x155, 0x7, 0x15, 0x2, 0x2, 0x155, 0x160, 0x5, 
    0x3a, 0x1e, 0x2, 0x156, 0x157, 0xc, 0x5, 0x2, 0x2, 0x157, 0x158, 0x7, 
    0x16, 0x2, 0x2, 0x158, 0x160, 0x5, 0x3a, 0x1e, 0x2, 0x159, 0x15a, 0xc, 
    0x4, 0x2, 0x2, 0x15a, 0x15b, 0x7, 0x17, 0x2, 0x2, 0x15b, 0x160, 0x5, 
    0x3a, 0x1e, 0x2, 0x15c, 0x15d, 0xc, 0x3, 0x2, 0x2, 0x15d, 0x15e, 0x7, 
    0x18, 0x2, 0x2, 0x15e, 0x160, 0x5, 0x3a, 0x1e, 0x2, 0x15f, 0x153, 0x3, 
    0x2, 0x2, 0x2, 0x15f, 0x156, 0x3, 0x2, 0x2, 0x2, 0x15f, 0x159, 0x3, 
    0x2, 0x2, 0x2, 0x15f, 0x15c, 0x3, 0x2, 0x2, 0x2, 0x160, 0x163, 0x3, 
    0x2, 0x2, 0x2, 0x161, 0x15f, 0x3, 0x2, 0x2, 0x2, 0x161, 0x162, 0x3, 
    0x2, 0x2, 0x2, 0x162, 0x3d, 0x3, 0x2, 0x2, 0x2, 0x163, 0x161, 0x3, 0x2, 
    0x2, 0x2, 0x164, 0x165, 0x8, 0x20, 0x1, 0x2, 0x165, 0x166, 0x5, 0x3c, 
    0x1f, 0x2, 0x166, 0x16f, 0x3, 0x2, 0x2, 0x2, 0x167, 0x168, 0xc, 0x4, 
    0x2, 0x2, 0x168, 0x169, 0x7, 0x13, 0x2, 0x2, 0x169, 0x16e, 0x5, 0x3c, 
    0x1f, 0x2, 0x16a, 0x16b, 0xc, 0x3, 0x2, 0x2, 0x16b, 0x16c, 0x7, 0x14, 
    0x2, 0x2, 0x16c, 0x16e, 0x5, 0x3c, 0x1f, 0x2, 0x16d, 0x167, 0x3, 0x2, 
    0x2, 0x2, 0x16d, 0x16a, 0x3, 0x2, 0x2, 0x2, 0x16e, 0x171, 0x3, 0x2, 
    0x2, 0x2, 0x16f, 0x16d, 0x3, 0x2, 0x2, 0x2, 0x16f, 0x170, 0x3, 0x2, 
    0x2, 0x2, 0x170, 0x3f, 0x3, 0x2, 0x2, 0x2, 0x171, 0x16f, 0x3, 0x2, 0x2, 
    0x2, 0x172, 0x173, 0x8, 0x21, 0x1, 0x2, 0x173, 0x174, 0x5, 0x3e, 0x20, 
    0x2, 0x174, 0x17a, 0x3, 0x2, 0x2, 0x2, 0x175, 0x176, 0xc, 0x3, 0x2, 
    0x2, 0x176, 0x177, 0x7, 0x1a, 0x2, 0x2, 0x177, 0x179, 0x5, 0x3e, 0x20, 
    0x2, 0x178, 0x175, 0x3, 0x2, 0x2, 0x2, 0x179, 0x17c, 0x3, 0x2, 0x2, 
    0x2, 0x17a, 0x178, 0x3, 0x2, 0x2, 0x2, 0x17a, 0x17b, 0x3, 0x2, 0x2, 
    0x2, 0x17b, 0x41, 0x3, 0x2, 0x2, 0x2, 0x17c, 0x17a, 0x3, 0x2, 0x2, 0x2, 
    0x17d, 0x17e, 0x8, 0x22, 0x1, 0x2, 0x17e, 0x17f, 0x5, 0x40, 0x21, 0x2, 
    0x17f, 0x185, 0x3, 0x2, 0x2, 0x2, 0x180, 0x181, 0xc, 0x3, 0x2, 0x2, 
    0x181, 0x182, 0x7, 0x1b, 0x2, 0x2, 0x182, 0x184, 0x5, 0x40, 0x21, 0x2, 
    0x183, 0x180, 0x3, 0x2, 0x2, 0x2, 0x184, 0x187, 0x3, 0x2, 0x2, 0x2, 
    0x185, 0x183, 0x3, 0x2, 0x2, 0x2, 0x185, 0x186, 0x3, 0x2, 0x2, 0x2, 
    0x186, 0x43, 0x3, 0x2, 0x2, 0x2, 0x187, 0x185, 0x3, 0x2, 0x2, 0x2, 0x2c, 
    0x47, 0x4e, 0x52, 0x5b, 0x62, 0x6b, 0x77, 0x83, 0x88, 0x91, 0x94, 0x97, 
    0x9d, 0xa4, 0xab, 0xbb, 0xbe, 0xc4, 0xcb, 0xd3, 0xde, 0xec, 0xef, 0xfc, 
    0x105, 0x10a, 0x10e, 0x112, 0x118, 0x121, 0x127, 0x12e, 0x13d, 0x13f, 
    0x14b, 0x14d, 0x15f, 0x161, 0x16d, 0x16f, 0x17a, 0x185, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

SysYParser::Initializer SysYParser::_init;
