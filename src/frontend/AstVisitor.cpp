#include <cassert>
#include <string>

#include "frontend/AstVisitor.hpp"
#include "frontend/ast.hpp"

using namespace frontend;


CompileUnit const &AstVisitor::compileUnit() const {
  return *this->m_compile_unit;
}

std::any AstVisitor::visitCompUnit(SysYParser::CompUnitContext *const ctx) {
  std::vector<CompileUnit::Child> children;
  for (auto item : ctx->compUnitItem()) {
    if (auto decl = item->decl()) {
      auto const decls =
          std::any_cast<std::vector<Declaration *>>(decl->accept(this));
      for (auto d : decls) {
        children.emplace_back(std::unique_ptr<Declaration>(d));
      }
    } else if (auto func_ = item->funcDef()) {
      auto const func = std::any_cast<Function *>(func_->accept(this));
      children.emplace_back(std::unique_ptr<Function>(func));
    } else {
      assert(false);
    }
  }
  auto compile_unit = new CompileUnit(std::move(children));
  m_compile_unit.reset(compile_unit);
  return compile_unit;
}

std::any AstVisitor::visitConstDecl(SysYParser::ConstDeclContext *const ctx) {
  auto const base_type_ =
      std::any_cast<ScalarType *>(ctx->bType()->accept(this));
  std::unique_ptr<ScalarType> base_type(base_type_);
  std::vector<Declaration *> ret;
  for (auto def : ctx->constDef()) {
    Identifier const ident(def->Ident()->getText());
    auto dimensions = this->visitDimensions(def->exp());
    std::unique_ptr<SysYType> type;
    if (dimensions.empty()) {
      type.reset(new ScalarType(*base_type));
    } else {
      type.reset(new ArrayType(*base_type, std::move(dimensions), false));
    }
    auto const init_ =
        std::any_cast<Initializer *>(def->initVal()->accept(this));
    std::unique_ptr<Initializer> init(init_);
    // TODO 检查 constexpr
    // 编译期常量不在这里检查
    ret.push_back(new Declaration(std::move(type), std::move(ident),
                                  std::move(init), true));
  }
  return std::move(ret);
}

std::any AstVisitor::visitInt(SysYParser::IntContext *const ctx) {
  return new ScalarType(Int);
}

std::any AstVisitor::visitFloat(SysYParser::FloatContext *const ctx) {
  return new ScalarType(Float);
}

std::any AstVisitor::visitVarDecl(SysYParser::VarDeclContext *const ctx) {
  auto const base_type_ =
      std::any_cast<ScalarType *>(ctx->bType()->accept(this));
  std::unique_ptr<ScalarType> base_type(base_type_);
  std::vector<Declaration *> ret;
  for (auto def : ctx->varDef()) {
    Identifier const ident(def->Ident()->getText());
    auto dimensions = this->visitDimensions(def->exp());
    std::unique_ptr<SysYType> type;
    if (dimensions.empty()) {
      type.reset(new ScalarType(*base_type));
    } else {
      type.reset(new ArrayType(*base_type, std::move(dimensions), false));
    }
    std::unique_ptr<Initializer> init;
    if (auto init_val = def->initVal()) {
      init.reset(std::any_cast<Initializer *>(init_val->accept(this)));
    }
    ret.push_back(new Declaration(std::move(type), std::move(ident),
                                  std::move(init), false));
  }
  return std::move(ret);
}

std::any AstVisitor::visitInit(SysYParser::InitContext *const ctx) {
  auto const expr_ = std::any_cast<Expression *>(ctx->exp()->accept(this));
  std::unique_ptr<Expression> expr(expr_);
  return new Initializer(std::move(expr));
}

std::any AstVisitor::visitInitList(SysYParser::InitListContext *const ctx) {
  std::vector<std::unique_ptr<Initializer>> values;
  for (auto init : ctx->initVal()) {
    auto const value = std::any_cast<Initializer *>(init->accept(this));
    values.emplace_back(value);
  }
  return new Initializer(std::move(values));
}

std::any AstVisitor::visitFuncDef(SysYParser::FuncDefContext *const ctx) {
  auto const type_ = std::any_cast<ScalarType *>(ctx->funcType()->accept(this));
  std::unique_ptr<ScalarType> type(type_);
  Identifier const ident(ctx->Ident()->getText());
  std::vector<std::unique_ptr<Parameter>> params;
  if (auto params_ = ctx->funcFParams()) {
    for (auto param_ : params_->funcFParam()) {
      auto const param = std::any_cast<Parameter *>(param_->accept(this));
      params.emplace_back(param);
    }
  }
  auto const body_ = std::any_cast<Block *>(ctx->block()->accept(this));
  std::unique_ptr<Block> body(body_);
  return new Function(std::move(type), std::move(ident), std::move(params),
                      std::move(body));
}

std::any AstVisitor::visitVoid(SysYParser::VoidContext *const ctx) {
  return static_cast<ScalarType *>(nullptr);
}

std::any
AstVisitor::visitScalarParam(SysYParser::ScalarParamContext *const ctx) {
  auto const type_ = std::any_cast<ScalarType *>(ctx->bType()->accept(this));
  std::unique_ptr<SysYType> type(type_);
  Identifier const ident(ctx->Ident()->getText());
  return new Parameter(std::move(type), std::move(ident));
}

std::any AstVisitor::visitArrayParam(SysYParser::ArrayParamContext *ctx) {
  auto const basic_type_ =
      std::any_cast<ScalarType *>(ctx->bType()->accept(this));
  std::unique_ptr<ScalarType> basic_type(basic_type_);
  Identifier const ident(ctx->Ident()->getText());
  auto dimensions = this->visitDimensions(ctx->exp());
  std::unique_ptr<SysYType> type(
      new ArrayType(*basic_type, std::move(dimensions), true));
  return new Parameter(std::move(type), std::move(ident));
}

std::any AstVisitor::visitBlock(SysYParser::BlockContext *const ctx) {
  std::vector<Block::Child> children;
  for (auto item : ctx->blockItem()) {
    if (auto decl = item->decl()) {
      auto const decls =
          std::any_cast<std::vector<Declaration *>>(decl->accept(this));
      for (auto d : decls) {
        children.emplace_back(std::unique_ptr<Declaration>(d));
      }
    } else if (auto stmt_ = item->stmt()) {
      auto const stmt = std::any_cast<Statement *>(stmt_->accept(this));
      children.emplace_back(std::unique_ptr<Statement>(stmt));
    } else {
      assert(false);
    }
  }
  return new Block(std::move(children));
}

std::any AstVisitor::visitAssign(SysYParser::AssignContext *const ctx) {
  auto const lhs_ = std::any_cast<LValue *>(ctx->lVal()->accept(this));
  std::unique_ptr<LValue> lhs(lhs_);
  auto const rhs_ = std::any_cast<Expression *>(ctx->exp()->accept(this));
  std::unique_ptr<Expression> rhs(rhs_);
  auto const ret = new Assignment(std::move(lhs), std::move(rhs));
  return static_cast<Statement *>(ret);
}

std::any AstVisitor::visitExprStmt(SysYParser::ExprStmtContext *const ctx) {
  std::unique_ptr<Expression> expr;
  if (auto expr_ = ctx->exp()) {
    expr.reset(std::any_cast<Expression *>(expr_->accept(this)));
  }
  auto const ret = new ExprStmt(std::move(expr));
  return static_cast<Statement *>(ret);
}

std::any AstVisitor::visitBlockStmt(SysYParser::BlockStmtContext *const ctx) {
  auto const ret = std::any_cast<Block *>(ctx->block()->accept(this));
  return static_cast<Statement *>(ret);
}

std::any AstVisitor::visitIfElse(SysYParser::IfElseContext *const ctx) {
  auto const cond_ = std::any_cast<Expression *>(ctx->cond()->accept(this));
  std::unique_ptr<Expression> cond(cond_);
  auto const then_ = std::any_cast<Statement *>(ctx->stmt(0)->accept(this));
  std::unique_ptr<Statement> then(then_);
  std::unique_ptr<Statement> else_;
  if (ctx->Else() != nullptr) {
    else_.reset(std::any_cast<Statement *>(ctx->stmt(1)->accept(this)));
  }
  auto const ret =
      new IfElse(std::move(cond), std::move(then), std::move(else_));
  return static_cast<Statement *>(ret);
}

std::any AstVisitor::visitWhile(SysYParser::WhileContext *const ctx) {
  auto const cond_ = std::any_cast<Expression *>(ctx->cond()->accept(this));
  std::unique_ptr<Expression> cond(cond_);
  auto const body_ = std::any_cast<Statement *>(ctx->stmt()->accept(this));
  std::unique_ptr<Statement> body(body_);
  auto const ret = new While(std::move(cond), std::move(body));
  return static_cast<Statement *>(ret);
}

std::any AstVisitor::visitBreak(SysYParser::BreakContext *const ctx) {
  auto const ret = new Break;
  return static_cast<Statement *>(ret);
}

std::any AstVisitor::visitContinue(SysYParser::ContinueContext *const ctx) {
  auto const ret = new Continue;
  return static_cast<Statement *>(ret);
}

std::any AstVisitor::visitReturn(SysYParser::ReturnContext *const ctx) {
  std::unique_ptr<Expression> res;
  if (auto exp = ctx->exp()) {
    res.reset(std::any_cast<Expression *>(exp->accept(this)));
  }
  auto const ret = new Return(std::move(res));
  return static_cast<Statement *>(ret);
}

std::any AstVisitor::visitLVal(SysYParser::LValContext *const ctx) {
  Identifier const ident(ctx->Ident()->getText());
  std::vector<std::unique_ptr<Expression>> indices;
  for (auto exp : ctx->exp()) {
    auto const index = std::any_cast<Expression *>(exp->accept(this));
    indices.emplace_back(index);
  }
  return new LValue(std::move(ident), std::move(indices));
}

std::any AstVisitor::visitLValExpr(SysYParser::LValExprContext *const ctx) {
  auto const lval = std::any_cast<LValue *>(ctx->lVal()->accept(this));
  return static_cast<Expression *>(lval);
}

std::any
AstVisitor::visitDecIntConst(SysYParser::DecIntConstContext *const ctx) {
  return std::stoi(ctx->getText(), nullptr, 10);
}

std::any
AstVisitor::visitOctIntConst(SysYParser::OctIntConstContext *const ctx) {
  return std::stoi(ctx->getText(), nullptr, 8);
}

std::any
AstVisitor::visitHexIntConst(SysYParser::HexIntConstContext *const ctx) {
  return std::stoi(ctx->getText(), nullptr, 16);
}

std::any
AstVisitor::visitDecFloatConst(SysYParser::DecFloatConstContext *const ctx) {
  return std::stof(ctx->getText());
}

std::any
AstVisitor::visitHexFloatConst(SysYParser::HexFloatConstContext *const ctx) {
  return std::stof(ctx->getText());
}

// 未使用
std::any AstVisitor::visitFuncRParam(SysYParser::FuncRParamContext *const ctx) {
  return nullptr;
}

// 未使用
std::any
AstVisitor::visitFuncRParams(SysYParser::FuncRParamsContext *const ctx) {
  return nullptr;
}

std::any AstVisitor::visitCall(SysYParser::CallContext *const ctx) {
  Identifier const ident(ctx->Ident()->getText());
  std::vector<Call::Argument> args;
  auto args_ctx = ctx->funcRParams();
  if (args_ctx) {
    for (auto arg_ : args_ctx->funcRParam()) {
      if (auto exp = arg_->exp()) {
        auto const arg = std::any_cast<Expression *>(exp->accept(this));
        args.emplace_back(std::unique_ptr<Expression>(arg));
      } else if (auto str = arg_->stringConst()) {
        auto const arg = std::any_cast<std::string>(str->accept(this));
        args.emplace_back(std::move(arg));
      } else {
        assert(false);
      }
    }
  }
  auto const ret = new Call(std::move(ident), std::move(args));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitUnaryAdd(SysYParser::UnaryAddContext *const ctx) {
  auto const operand =
      std::any_cast<Expression *>(ctx->unaryExp()->accept(this));
  auto const ret =
      new UnaryExpr(UnaryOp::Not, std::unique_ptr<Expression>(operand));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitUnarySub(SysYParser::UnarySubContext *const ctx) {
  auto const operand =
      std::any_cast<Expression *>(ctx->unaryExp()->accept(this));
  auto const ret =
      new UnaryExpr(UnaryOp::Sub, std::unique_ptr<Expression>(operand));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitNot(SysYParser::NotContext *const ctx) {
  auto const operand =
      std::any_cast<Expression *>(ctx->unaryExp()->accept(this));
  auto const ret =
      new UnaryExpr(UnaryOp::Not, std::unique_ptr<Expression>(operand));
  return static_cast<Expression *>(ret);
}

std::any
AstVisitor::visitStringConst(SysYParser::StringConstContext *const ctx) {
  return ctx->getText();
}

std::any AstVisitor::visitMul(SysYParser::MulContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->mulExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->unaryExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Mul, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitDiv(SysYParser::DivContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->mulExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->unaryExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Div, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitMod(SysYParser::ModContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->mulExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->unaryExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Mod, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitAdd(SysYParser::AddContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->addExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->mulExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Add, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitSub(SysYParser::SubContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->addExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->mulExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Sub, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitLt(SysYParser::LtContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->relExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->addExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Lt, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitGt(SysYParser::GtContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->relExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->addExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Gt, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitLeq(SysYParser::LeqContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->relExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->addExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Leq, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitGeq(SysYParser::GeqContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->relExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->addExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Geq, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitEq(SysYParser::EqContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->eqExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->relExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Eq, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitNeq(SysYParser::NeqContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->eqExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->relExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Neq, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitAnd(SysYParser::AndContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->lAndExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->eqExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::And, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitOr(SysYParser::OrContext *const ctx) {
  auto const lhs = std::any_cast<Expression *>(ctx->lOrExp()->accept(this));
  auto const rhs = std::any_cast<Expression *>(ctx->lAndExp()->accept(this));
  auto const ret =
      new BinaryExpr(BinaryOp::Or, std::unique_ptr<Expression>(lhs),
                     std::unique_ptr<Expression>(rhs));
  return static_cast<Expression *>(ret);
}

std::any AstVisitor::visitNumber(SysYParser::NumberContext *const ctx) {
  if (ctx->intConst()) {
    auto val = std::any_cast<IntLiteral::Value>(ctx->intConst()->accept(this));
    auto literal = new IntLiteral{val};
    return static_cast<Expression *>(literal);
  }
  if (ctx->floatConst()) {
    auto val =
        std::any_cast<FloatLiteral::Value>(ctx->floatConst()->accept(this));
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
    auto expr_ = std::any_cast<Expression *>(expr_ctx->accept(this));
    ret.emplace_back(expr_);
  }
  return ret;
}
