
// Generated from D:/Documents/Code/C++/compiler/frontend\SysY.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"
#include "SysYParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SysYParser.
 */
class  SysYVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SysYParser.
   */
    virtual std::any visitStart(SysYParser::StartContext *context) = 0;


};

