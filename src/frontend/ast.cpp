#include "frontend/ast.hpp"

using namespace frontend;

void ScalarType::print(std::ostream &out, unsigned int indent) const {}

void ArrayType::print(std::ostream &out, unsigned int indent) const {}

void Identifier::print(std::ostream &out, unsigned int indent) const {}

void Parameter::print(std::ostream &out, unsigned int indent) const {}

void LValue::print(std::ostream &out, unsigned int indent) const {}

void UnaryExpr::print(std::ostream &out, unsigned int indent) const {}

void BinaryExpr::print(std::ostream &out, unsigned int indent) const {}

void IntLiteral::print(std::ostream &out, unsigned int indent) const {}

void FloatLiteral::print(std::ostream &out, unsigned int indent) const {}

void StringLiteral::print(std::ostream &out, unsigned int indent) const {}

void Call::print(std::ostream &out, unsigned int indent) const {}

void ExprStmt::print(std::ostream &out, unsigned int indent) const {}

void Assignment::print(std::ostream &out, unsigned int indent) const {}

void Initializer::print(std::ostream &out, unsigned int indent) const {}

void Declaration::print(std::ostream &out, unsigned int indent) const {}

void Block::print(std::ostream &out, unsigned int indent) const {}

void IfElse::print(std::ostream &out, unsigned int indent) const {}

void While::print(std::ostream &out, unsigned int indent) const {}

void Break::print(std::ostream &out, unsigned int indent) const {}

void Continue::print(std::ostream &out, unsigned int indent) const {}

void Return::print(std::ostream &out, unsigned int indent) const {}

void Function::print(std::ostream &out, unsigned int indent) const {}

void CompileUnit::print(std::ostream &out, unsigned int indent) const {}
