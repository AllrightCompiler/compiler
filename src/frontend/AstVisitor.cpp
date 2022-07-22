#include <cassert>
#include <memory>
#include <string>

#include "common/utils.hpp"
#include "frontend/AstVisitor.hpp"
#include "frontend/ast.hpp"

using namespace frontend;

CompileUnit const &AstVisitor::compileUnit() const {
  return *this->m_compile_unit;
}

antlrcpp::Any
AstVisitor::visitCompUnit(SysYParser::CompUnitContext *const ctx) {
  std::vector<CompileUnit::Child> children;
  for (auto item : ctx->compUnitItem()) {
    if (auto decl = item->decl()) {
      auto const decls =
          decl->accept(this).as<std::shared_ptr<std::vector<Declaration *>>>();
      for (auto d : *decls) {
        children.emplace_back(std::unique_ptr<Declaration>(d));
      }
    } else if (auto func_ = item->funcDef()) {
      auto const func = func_->accept(this).as<Function *>();
      children.emplace_back(std::unique_ptr<Function>(func));
    } else {
      assert(false);
    }
  }
  auto compile_unit = new CompileUnit(std::move(children));
  m_compile_unit.reset(compile_unit);
  return compile_unit;
}

antlrcpp::Any
AstVisitor::visitConstDecl(SysYParser::ConstDeclContext *const ctx) {
  auto const base_type_ = ctx->bType()->accept(this).as<ScalarType *>();
  std::unique_ptr<ScalarType> base_type(base_type_);
  std::vector<Declaration *> ret;
  for (auto def : ctx->constDef()) {
    Identifier ident(def->Ident()->getText());
    auto dimensions = this->visitDimensions(def->exp());
    std::unique_ptr<SysYType> type;
    if (dimensions.empty()) {
      type = std::make_unique<ScalarType>(*base_type);
    } else {
      type =
          std::make_unique<ArrayType>(*base_type, std::move(dimensions), false);
    }
    auto const init_ = def->initVal()->accept(this).as<Initializer *>();
    std::unique_ptr<Initializer> init(init_);
    // TODO 检查 constexpr
    // 编译期常量不在这里检查
    ret.push_back(new Declaration(std::move(type), std::move(ident),
                                  std::move(init), true));
  }
  return std::make_shared<std::vector<Declaration *>>(std::move(ret));
}

antlrcpp::Any AstVisitor::visitInt(SysYParser::IntContext *const ctx) {
  return new ScalarType(Int);
}

antlrcpp::Any AstVisitor::visitFloat(SysYParser::FloatContext *const ctx) {
  return new ScalarType(Float);
}

antlrcpp::Any AstVisitor::visitVarDecl(SysYParser::VarDeclContext *const ctx) {
  auto const base_type_ = ctx->bType()->accept(this).as<ScalarType *>();
  std::unique_ptr<ScalarType> base_type(base_type_);
  std::vector<Declaration *> ret;
  for (auto def : ctx->varDef()) {
    Identifier ident(def->Ident()->getText());
    auto dimensions = this->visitDimensions(def->exp());
    std::unique_ptr<SysYType> type;
    if (dimensions.empty()) {
      type = std::make_unique<ScalarType>(*base_type);
    } else {
      type =
          std::make_unique<ArrayType>(*base_type, std::move(dimensions), false);
    }
    std::unique_ptr<Initializer> init;
    if (auto init_val = def->initVal()) {
      init.reset(init_val->accept(this).as<Initializer *>());
    }
    ret.push_back(new Declaration(std::move(type), std::move(ident),
                                  std::move(init), false));
  }
  return std::make_shared<std::vector<Declaration *>>(std::move(ret));
}

antlrcpp::Any AstVisitor::visitInit(SysYParser::InitContext *const ctx) {
  auto expr_ = ctx->exp()->accept(this).as<Expression *>();
  std::unique_ptr<Expression> expr(expr_);
  return new Initializer(std::move(expr));
}

antlrcpp::Any
AstVisitor::visitInitList(SysYParser::InitListContext *const ctx) {
  std::vector<std::unique_ptr<Initializer>> values;
  for (auto init : ctx->initVal()) {
    auto const value = init->accept(this).as<Initializer *>();
    values.emplace_back(value);
  }
  return new Initializer(std::move(values));
}

antlrcpp::Any AstVisitor::visitFuncDef(SysYParser::FuncDefContext *const ctx) {
  auto const type_ = ctx->funcType()->accept(this).as<ScalarType *>();
  std::unique_ptr<ScalarType> type(type_);
  Identifier ident(ctx->Ident()->getText());
  std::vector<std::unique_ptr<Parameter>> params;
  if (auto params_ = ctx->funcFParams()) {
    for (auto param_ : params_->funcFParam()) {
      auto const param = param_->accept(this).as<Parameter *>();
      params.emplace_back(param);
    }
  }
  auto const body_ = ctx->block()->accept(this).as<Block *>();
  std::unique_ptr<Block> body(body_);
  return new Function(std::move(type), std::move(ident), std::move(params),
                      std::move(body));
}

antlrcpp::Any AstVisitor::visitVoid(SysYParser::VoidContext *const ctx) {
  return static_cast<ScalarType *>(nullptr);
}

antlrcpp::Any
AstVisitor::visitScalarParam(SysYParser::ScalarParamContext *const ctx) {
  auto const type_ = ctx->bType()->accept(this).as<ScalarType *>();
  std::unique_ptr<SysYType> type(type_);
  Identifier ident(ctx->Ident()->getText());
  return new Parameter(std::move(type), std::move(ident));
}

antlrcpp::Any AstVisitor::visitArrayParam(SysYParser::ArrayParamContext *ctx) {
  auto const basic_type_ = ctx->bType()->accept(this).as<ScalarType *>();
  std::unique_ptr<ScalarType> basic_type(basic_type_);
  Identifier ident(ctx->Ident()->getText());
  auto dimensions = this->visitDimensions(ctx->exp());
  std::unique_ptr<SysYType> type(
      new ArrayType(*basic_type, std::move(dimensions), true));
  return new Parameter(std::move(type), std::move(ident));
}

antlrcpp::Any AstVisitor::visitBlock(SysYParser::BlockContext *const ctx) {
  std::vector<Block::Child> children;
  for (auto item : ctx->blockItem()) {
    if (auto decl = item->decl()) {
      auto const decls =
          decl->accept(this).as<std::shared_ptr<std::vector<Declaration *>>>();
      for (auto d : *decls) {
        children.emplace_back(std::unique_ptr<Declaration>(d));
      }
    } else if (auto stmt_ = item->stmt()) {
      auto const stmt = stmt_->accept(this).as<Statement *>();
      children.emplace_back(std::unique_ptr<Statement>(stmt));
    } else {
      assert(false);
    }
  }
  return new Block(std::move(children));
}

antlrcpp::Any AstVisitor::visitAssign(SysYParser::AssignContext *const ctx) {
  auto const lhs_ = ctx->lVal()->accept(this).as<LValue *>();
  std::unique_ptr<LValue> lhs(lhs_);
  auto const rhs_ = ctx->exp()->accept(this).as<Expression *>();
  std::unique_ptr<Expression> rhs(rhs_);
  auto const ret = new Assignment(std::move(lhs), std::move(rhs));
  return static_cast<Statement *>(ret);
}

antlrcpp::Any
AstVisitor::visitExprStmt(SysYParser::ExprStmtContext *const ctx) {
  std::unique_ptr<Expression> expr;
  if (auto expr_ = ctx->exp()) {
    expr.reset(expr_->accept(this).as<Expression *>());
  }
  auto const ret = new ExprStmt(std::move(expr));
  return static_cast<Statement *>(ret);
}

antlrcpp::Any
AstVisitor::visitBlockStmt(SysYParser::BlockStmtContext *const ctx) {
  auto const ret = ctx->block()->accept(this).as<Block *>();
  return static_cast<Statement *>(ret);
}

antlrcpp::Any AstVisitor::visitIfElse(SysYParser::IfElseContext *const ctx) {
  auto const cond_ = ctx->cond()->accept(this).as<Expression *>();
  std::unique_ptr<Expression> cond(cond_);
  auto const then_ = ctx->stmt(0)->accept(this).as<Statement *>();
  std::unique_ptr<Statement> then(then_);
  std::unique_ptr<Statement> else_;
  if (ctx->Else() != nullptr) {
    else_.reset(ctx->stmt(1)->accept(this).as<Statement *>());
  }
  auto const ret =
      new IfElse(std::move(cond), std::move(then), std::move(else_));
  return static_cast<Statement *>(ret);
}

antlrcpp::Any AstVisitor::visitWhile(SysYParser::WhileContext *const ctx) {
  auto const cond_ = ctx->cond()->accept(this).as<Expression *>();
  std::unique_ptr<Expression> cond(cond_);
  auto const body_ = ctx->stmt()->accept(this).as<Statement *>();
  std::unique_ptr<Statement> body(body_);
  auto const ret = new While(std::move(cond), std::move(body));
  return static_cast<Statement *>(ret);
}

antlrcpp::Any AstVisitor::visitBreak(SysYParser::BreakContext *const ctx) {
  auto const ret = new Break;
  return static_cast<Statement *>(ret);
}

antlrcpp::Any
AstVisitor::visitContinue(SysYParser::ContinueContext *const ctx) {
  auto const ret = new Continue;
  return static_cast<Statement *>(ret);
}

antlrcpp::Any AstVisitor::visitReturn(SysYParser::ReturnContext *const ctx) {
  std::unique_ptr<Expression> res;
  if (auto exp = ctx->exp()) {
    res.reset(exp->accept(this).as<Expression *>());
  }
  auto const ret = new Return(std::move(res));
  return static_cast<Statement *>(ret);
}

antlrcpp::Any AstVisitor::visitLVal(SysYParser::LValContext *const ctx) {
  Identifier ident(ctx->Ident()->getText());
  std::vector<std::unique_ptr<Expression>> indices;
  for (auto exp : ctx->exp()) {
    auto const index = exp->accept(this).as<Expression *>();
    indices.emplace_back(index);
  }
  return new LValue(std::move(ident), std::move(indices));
}

antlrcpp::Any
AstVisitor::visitPrimaryExp_(SysYParser::PrimaryExp_Context *const ctx) {
  if (ctx->number()) {
    return ctx->number()->accept(this);
  } else {
    assert(ctx->exp());
    return ctx->exp()->accept(this);
  }
}

antlrcpp::Any
AstVisitor::visitLValExpr(SysYParser::LValExprContext *const ctx) {
  auto const lval = ctx->lVal()->accept(this).as<LValue *>();
  return static_cast<Expression *>(lval);
}

antlrcpp::Any
AstVisitor::visitDecIntConst(SysYParser::DecIntConstContext *const ctx) {
  return int(std::stol(ctx->getText(), nullptr, 10));
}

antlrcpp::Any
AstVisitor::visitOctIntConst(SysYParser::OctIntConstContext *const ctx) {
  return int(std::stol(ctx->getText(), nullptr, 8));
}

antlrcpp::Any
AstVisitor::visitHexIntConst(SysYParser::HexIntConstContext *const ctx) {
  return int(std::stol(ctx->getText(), nullptr, 16));
}

antlrcpp::Any
AstVisitor::visitDecFloatConst(SysYParser::DecFloatConstContext *const ctx) {
  return std::stof(ctx->getText());
}

antlrcpp::Any
AstVisitor::visitHexFloatConst(SysYParser::HexFloatConstContext *const ctx) {
  return std::stof(ctx->getText());
}

antlrcpp::Any AstVisitor::visitCall(SysYParser::CallContext *const ctx) {
  Identifier ident(ctx->Ident()->getText());
  std::vector<Call::Argument> args;
  auto args_ctx = ctx->funcRParams();
  if (args_ctx) {
    for (auto arg_ : args_ctx->funcRParam()) {
      if (auto exp = arg_->exp()) {
        auto const arg = exp->accept(this).as<Expression *>();
        args.emplace_back(std::unique_ptr<Expression>(arg));
      } else if (auto str = arg_->stringConst()) {
        auto arg = str->accept(this).as<std::shared_ptr<std::string>>();
        args.emplace_back(std::move(*arg));
      } else {
        assert(false);
      }
    }
  }
  auto const ret =
      new Call(std::move(ident), std::move(args), ctx->getStart()->getLine());
  return static_cast<Expression *>(ret);
}

antlrcpp::Any
AstVisitor::visitUnaryAdd(SysYParser::UnaryAddContext *const ctx) {
  auto const operand = ctx->unaryExp()->accept(this).as<Expression *>();
  auto const ret =
      new UnaryExpr(UnaryOp::Add, std::unique_ptr<Expression>(operand));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any
AstVisitor::visitUnarySub(SysYParser::UnarySubContext *const ctx) {
  auto const operand = ctx->unaryExp()->accept(this).as<Expression *>();
  auto const ret =
      new UnaryExpr(UnaryOp::Sub, std::unique_ptr<Expression>(operand));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitNot(SysYParser::NotContext *const ctx) {
  auto const operand = ctx->unaryExp()->accept(this).as<Expression *>();
  auto const ret =
      new UnaryExpr(UnaryOp::Not, std::unique_ptr<Expression>(operand));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any
AstVisitor::visitStringConst(SysYParser::StringConstContext *const ctx) {
  return std::make_shared<std::string>(ctx->getText());
}

antlrcpp::Any AstVisitor::visitMul(SysYParser::MulContext *const ctx) {
  auto const lhs = ctx->mulExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->unaryExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Mul, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitDiv(SysYParser::DivContext *const ctx) {
  auto const lhs = ctx->mulExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->unaryExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Div, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitMod(SysYParser::ModContext *const ctx) {
  auto const lhs = ctx->mulExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->unaryExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Mod, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitAdd(SysYParser::AddContext *const ctx) {
  auto const lhs = ctx->addExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->mulExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Add, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitSub(SysYParser::SubContext *const ctx) {
  auto const lhs = ctx->addExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->mulExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Sub, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitLt(SysYParser::LtContext *const ctx) {
  auto const lhs = ctx->relExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->addExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Lt, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitGt(SysYParser::GtContext *const ctx) {
  auto const lhs = ctx->relExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->addExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Gt, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitLeq(SysYParser::LeqContext *const ctx) {
  auto const lhs = ctx->relExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->addExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Leq, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitGeq(SysYParser::GeqContext *const ctx) {
  auto const lhs = ctx->relExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->addExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Geq, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitEq(SysYParser::EqContext *const ctx) {
  auto const lhs = ctx->eqExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->relExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Eq, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitNeq(SysYParser::NeqContext *const ctx) {
  auto const lhs = ctx->eqExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->relExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Neq, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitAnd(SysYParser::AndContext *const ctx) {
  auto const lhs = ctx->lAndExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->eqExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::And, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitOr(SysYParser::OrContext *const ctx) {
  auto const lhs = ctx->lOrExp()->accept(this).as<Expression *>();
  auto const rhs = ctx->lAndExp()->accept(this).as<Expression *>();
  auto const ret =
      new BinaryExpr(BinaryOp::Or, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

antlrcpp::Any AstVisitor::visitNumber(SysYParser::NumberContext *const ctx) {
  if (ctx->intConst()) {
    auto val = ctx->intConst()->accept(this).as<IntLiteral::Value>();
    auto literal = new IntLiteral{val};
    return static_cast<Expression *>(literal);
  }
  if (ctx->floatConst()) {
    auto val = ctx->floatConst()->accept(this).as<FloatLiteral::Value>();
    auto literal = new FloatLiteral{val};
    return static_cast<Expression *>(literal);
  }
  assert(false);
  return static_cast<Expression *>(nullptr);
}

std::vector<std::unique_ptr<Expression>>
AstVisitor::visitDimensions(const std::vector<SysYParser::ExpContext *> &ctxs) {
  std::vector<std::unique_ptr<Expression>> ret;
  for (auto expr_ctx : ctxs) {
    auto expr_ = expr_ctx->accept(this).as<Expression *>();
    ret.emplace_back(expr_);
  }
  return ret;
}
