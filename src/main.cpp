#include "antlr4-runtime.h"

#include "frontend/AstVisitor.hpp"
#include "frontend/SysYLexer.h"
#include "frontend/SysYParser.h"

#include "common/utils.hpp"

#include <exception>
#include <iostream>

using namespace antlr4;
using namespace std;

struct ThrowingErrorListner : public antlr4::BaseErrorListener {
  virtual void syntaxError(antlr4::Recognizer *, antlr4::Token *, size_t line,
                           size_t pos, const std::string &err_msg,
                           std::exception_ptr) override {
    auto msg = to_string(line) + ":" + to_string(pos) + " " + err_msg;
    throw antlr4::ParseCancellationException{msg};
  }
};

int main(int argc, char *argv[]) {
  ifstream ifs{"input.sy"};

  ThrowingErrorListner error_listener;
  ANTLRInputStream input{ifs};
  frontend::SysYLexer lexer{&input};
  lexer.removeErrorListeners();
  lexer.addErrorListener(&error_listener);

  CommonTokenStream tokens{&lexer};
  frontend::SysYParser parser{&tokens};
  parser.removeErrorListeners();
  parser.addErrorListener(&error_listener);

  try {
    auto root = parser.compUnit();

    frontend::AstVisitor visitor;
    visitor.visitCompUnit(root);
    auto &ast = visitor.compileUnit();
    ast.print(cout, 0);
  } catch (const ParseCancellationException &e) {
    error(cerr) << e.what() << endl;
  }
  return 0;
}
