
// Generated from D:/Documents/Code/C++/compiler/frontend\SysY.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"
#include "SysYVisitor.h"


/**
 * This class provides an empty implementation of SysYVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SysYBaseVisitor : public SysYVisitor {
public:

  virtual std::any visitStart(SysYParser::StartContext *ctx) override {
    return visitChildren(ctx);
  }


};

