#include "antlr4-runtime.h"

#include "frontend/AstVisitor.hpp"
#include "frontend/IrGen.hpp"
#include "frontend/SysYLexer.h"
#include "frontend/SysYParser.h"
#include "frontend/Typer.hpp"

#include "mediumend/optimizer.hpp"

#include "backend/armv7/passes.hpp"
#include "backend/armv7/program.hpp"

#include "backend/llvm/program.hpp"

#include "common/argparse.hpp"
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

istream &get_input(int argc, char *argv[]) {
  for (auto i = 1; i < argc;) {
    auto const arg = std::string_view{argv[i]};
    if (arg == "-o") {
      i += 2;
    } else if (!arg.empty() && arg.front() == '-') {
      ++i;
    } else {
      static ifstream ifs;
      ifs.open(argv[i]);
      return ifs;
    }
  }
  return cin;
}

ostream &get_output(int argc, char *argv[]) {
  auto const output = get_option(argc, argv, "-o");
  if (output == nullptr) {
    return cout;
  } else {
    static ofstream ofs;
    ofs.open(output);
    return ofs;
  }
}

int main(int argc, char *argv[]) {
  auto &is = get_input(argc, argv);
  auto &os = get_output(argc, argv);

  ThrowingErrorListner error_listener;
  ANTLRInputStream input{is};
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
    if (has_option(argc, argv, "--ast")) {
      ast.print(os, 0);
      return 0;
    }

    frontend::Typer typer;
    typer.visit_compile_unit(ast);

    frontend::IrGen ir_gen;
    ir_gen.visit_compile_unit(ast);

    auto &ir_program = ir_gen.get_program();
    if (has_option(argc, argv, "-O2")) {
      mediumend::run_medium(ir_program.get());
    }
    if (has_option(argc, argv, "--ir")) {
      os << *ir_program;
      return 0;
    }

    if (has_option(argc, argv, "--llvm")) {
      auto program = llvm::translate(*ir_program);
      llvm::emit_global(os, *ir_program);
      program->emit(os);
    } else {
      auto program = armv7::translate(*ir_program);
      armv7::backend_passes(*program);
      armv7::emit_global(os, *ir_program);
      program->emit(os);
    }
  } catch (const ParseCancellationException &e) {
    error(cerr) << e.what() << endl;
  } catch (const CompileError &e) {
    error(cerr) << e.what() << endl;
  }
}
