lexer grammar SysY;

import tokens;

CompUnit : ( Decl | FuncDef )* EOF;

Decl : ConstDecl | VarDecl;

ConstDecl : 'const' BType ConstDef ( ',' ConstDef ) ';';

BType : INT | FLOAT;

ConstDef : IDENTIFIER ( '[' ConstExp ']' ) '=' ConstInitVal;

ConstInitVal : ConstExp 
            | '{' ( ConstInitVal ( ',' ConstInitVal ) ) '}';

VarDecl : BType VarDef { ',' VarDef } ';';

VarDef : IDENTIFIER { '[' ConstExp ']' }
| IDENTIFIER { '[' ConstExp ']' } '=' InitVal;

InitVal : Exp | '{' ( InitVal { ',' InitVal } ) '}';

FuncDef : FuncType IDENTIFIER '(' (FuncFParams) ')' Block;

FuncType : VOID | INT | FLOAT;

FuncFParams : FuncFParam { ',' FuncFParam };

FuncFParam : BType IDENTIFIER ('[' ']' ( '[' Exp ']' ));

Block : '{' ( BlockItem )* '}';

BlockItem : Decl | Stmt;

Stmt : LVal '=' Exp ';' 
    | (Exp) ';' 
    | Block
    | 'if' '(' Cond ')' Stmt ( 'else' Stmt )
    | 'while' '(' Cond ')' Stmt
    | 'break' ';' | 'continue' ';'
    | 'return' (Exp) ';';


Exp : AddExp;

Cond : LOrExp;

LVal : IDENTIFIER ('[' Exp ']');

PrimaryExp : '(' Exp ')' | LVal | Number;

Number : INTEGER | FLOATNUM;

UnaryExp : PrimaryExp | IDENTIFIER '(' (FuncRParams) ')' 
| UnaryOp UnaryExp;

UnaryOp : '+' | '-' | '!';

FuncRParams : Exp { ',' Exp };


MulExp
    : UnaryExp
    | UnaryExp ('*' | '/' | '%') MulExp
    ;


AddExp 
    : MulExp 
    | MulExp ('+' | '-') AddExp
    ;


RelExp 
    : AddExp 
    | AddExp ('<' | '>' | '<=' | '>=') RelExp 
    ;

EqExp 
    : RelExp 
    | RelExp ('==' | '!=') EqExp 
    ;

LAndExp 
    : EqExp 
    | EqExp '&&' LAndExp
    ;

LOrExp 
    : LAndExp 
    | LAndExp '||' LOrExp;

ConstExp : AddExp;
