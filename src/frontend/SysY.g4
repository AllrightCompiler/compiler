grammar SysY;

import tokens;

compUnit : (decl | funcDef)* EOF;

decl
    : constDecl
    | varDecl
    ;

constDecl : Const bType constDef (Comma constDef)* Semicolon;

bType
    : Int
    | Float
    ;

constDef : Ident (Lbracket constExp Rbracket)* Assign constInitVal;

constInitVal
    : constExp
    | Lbrace (constInitVal (Comma constInitVal)*)? Rbrace
    ;

varDecl : bType varDef (Comma varDef)* Semicolon;

varDef : Ident (Lbracket constExp Rbracket)* (Assign initVal)?;

initVal
    : exp
    | Lbrace (initVal (Comma initVal)*)? Rbrace
    ;

funcDef : funcType Ident Lparen funcFParams? Rparen block;

funcType
    : Void
    | Int
    | Float
    ;

funcFParams : funcFParam (Comma funcFParam)*;

funcFParam : bType Ident (Lbracket Rbracket (Lbracket exp Rbracket)*)?;

block : Lbrace (blockItem)* Rbrace;

blockItem
    : decl
    | stmt
    ;

stmt
    : lVal Assign exp Semicolon
    | exp? Semicolon
    | block
    | If Lparen cond Rparen stmt (Else stmt)?
    | While Lparen cond Rparen stmt
    | Break Semicolon
    | Continue Semicolon
    | Return exp? Semicolon
    ;

exp : addExp;

cond : lOrExp;

lVal : Ident (Lbracket exp Rbracket)*;

primaryExp
    : Lparen exp Rparen
    | lVal
    | number
    ;

intConst
    : DecIntConst
    | OctIntConst
    | HexIntConst
    ;
floatConst
    : DecFloatConst
    | HexFloatConst
    ;
number
    : intConst
    | floatConst
    ;

unaryExp
    : primaryExp
    | Ident Lparen funcRParams? Rparen
    | unaryOp unaryExp
    ;

unaryOp
    : Add
    | Sub
    | Not
    ;

funcRParam
    : exp
    | StringConst
    ;
funcRParams : funcRParam (Comma funcRParam)*;

mulExp
    : unaryExp
    | mulExp (Mul | Div | Mod) unaryExp
    ;

addExp
    : mulExp
    | addExp (Add | Sub) mulExp
    ;

relExp
    : addExp
    | relExp (Lt | Gt | Leq | Geq) addExp
    ;

eqExp
    : relExp
    | eqExp (Eq | Neq) relExp
    ;

lAndExp
    : eqExp
    | lAndExp And eqExp
    ;

lOrExp
    : lAndExp
    | lOrExp Or lAndExp
    ;

constExp : addExp;
