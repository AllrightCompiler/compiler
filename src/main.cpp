#include "antlr4-runtime.h"

#include "frontend/AstVisitor.hpp"
#include "frontend/SysYLexer.h"
#include "frontend/SysYParser.h"
#include "frontend/Typer.hpp"
#include "frontend/IrGen.hpp"

#include "mediumend/optmizer.hpp"

#include "backend/armv7/program.hpp"
#include "backend/armv7/passes.hpp"

#include "common/errors.hpp"
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
  string source = "input.sy";
  if (argc > 1)
    source = argv[1];
  ifstream ifs{source};

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
    // ast.print(cout, 0);

    frontend::Typer typer;
    typer.visit_compile_unit(ast);

    frontend::IrGen ir_gen;
    ir_gen.visit_compile_unit(ast);
    std::cout << " before -------------------\n" << *ir_gen.get_program();

    auto &ir_program = ir_gen.get_program();
    mediumend::run_medium(ir_program.get());
    std::cout << " after --------------------\n" << *ir_gen.get_program();

    // auto program = armv7::translate(*ir_program);
    // armv7::backend_passes(*program);

    // armv7::emit_global(std::cout, *ir_program);
    // program->emit(std::cout);
  } catch (const ParseCancellationException &e) {
    error(cerr) << e.what() << endl;
  } catch (const CompileError &e) {
    error(cerr) << e.what() << endl;
  }
  return 0;
}
