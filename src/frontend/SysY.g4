grammar SysY;

import tokens;

compUnit : compUnitItem* EOF;
compUnitItem
    : decl
    | funcDef
    ;

decl
    : constDecl
    | varDecl
    ;

constDecl : Const bType constDef (Comma constDef)* Semicolon;

bType
    : Int  # int
    | Float  # float
    ;

constDef : Ident (Lbracket exp Rbracket)* Assign initVal;

varDecl : bType varDef (Comma varDef)* Semicolon;

varDef : Ident (Lbracket exp Rbracket)* (Assign initVal)?;

initVal
    : exp  # init
    | Lbrace (initVal (Comma initVal)*)? Rbrace  # initList
    ;

funcDef : funcType Ident Lparen funcFParams? Rparen block;

funcType
    : bType  # funcType_
    | Void  # void
    ;

funcFParams : funcFParam (Comma funcFParam)*;

funcFParam
    : bType Ident  # scalarParam
    | bType Ident Lbracket Rbracket (Lbracket exp Rbracket)*  # arrayParam
    ;

block : Lbrace blockItem* Rbrace;

blockItem
    : decl
    | stmt
    ;

stmt
    : lVal Assign exp Semicolon  # assign
    | exp? Semicolon  # exprStmt
    | block  # blockStmt
    | If Lparen cond Rparen stmt (Else stmt)?  # ifElse
    | While Lparen cond Rparen stmt  # while
    | Break Semicolon  # break
    | Continue Semicolon  # continue
    | Return exp? Semicolon  # return
    ;

exp : addExp;

cond : lOrExp;

lVal : Ident (Lbracket exp Rbracket)*;

primaryExp
    : Lparen exp Rparen  # primaryExp_
    | lVal  # lValExpr
    | number  # primaryExp_
    ;

intConst
    : DecIntConst  # decIntConst
    | OctIntConst  # octIntConst
    | HexIntConst  # hexIntConst
    ;
floatConst
    : DecFloatConst  # decFloatConst
    | HexFloatConst  # hexFloatConst
    ;
number
    : intConst
    | floatConst
    ;

unaryExp
    : primaryExp  # unaryExp_
    | Ident Lparen funcRParams? Rparen  # call
    | Add unaryExp  # unaryAdd
    | Sub unaryExp  # unarySub
    | Not unaryExp  # not
    ;

stringConst : StringConst;
funcRParam
    : exp
    | stringConst
    ;
funcRParams : funcRParam (Comma funcRParam)*;

mulExp
    : unaryExp  # mulExp_
    | mulExp Mul unaryExp  # mul
    | mulExp Div unaryExp  # div
    | mulExp Mod unaryExp  # mod
    ;

addExp
    : mulExp  # addExp_
    | addExp Add mulExp  # add
    | addExp Sub mulExp  # sub
    ;

relExp
    : addExp  # relExp_
    | relExp Lt addExp  # lt
    | relExp Gt addExp  # gt
    | relExp Leq addExp  # leq
    | relExp Geq addExp  # geq
    ;

eqExp
    : relExp  # eqExp_
    | eqExp Eq relExp  # eq
    | eqExp Neq relExp  # neq
    ;

lAndExp
    : eqExp  # lAndExp_
    | lAndExp And eqExp  # and
    ;

lOrExp
    : lAndExp  # lOrExp_
    | lOrExp Or lAndExp  # or
    ;
