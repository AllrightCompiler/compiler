lexer grammar tokens;

INT : 'int';
FLOAT : 'float';
VOID: 'void';
RETURN : 'return';
IF : 'if';
ELSE : 'else';
DO : 'do';
WHILE : 'while';
FOR : 'for';
BREAK : 'break';
CONTINUE : 'continue';
CONST : 'const';

EQU : '==';
NEQ : '!=';
AND : '&&';
OR : '||';
LEQ : '<=';
GEQ : '>=';
PLUS : '+';
MINUS : '-';
TIMES : '*';
SLASH : '/';
MOD : '%';
LT : '<';
GT : '>';
COLON : ':';
SEMICOLON : ';';
LNOT : '!';
BNOT : '~';
COMMA : ',';
DOT : '.';
ASSIGN : '=';
QUESTION : '?';
LPAREN : '(';
RPAREN : ')';
LBRACK : '[';
RBRACK : ']';
LBRACE : '{';
RBRACE : '}';


FLOATNUM : [0-9]+((.)[0-9]+)?;
INTEGER :[0-9]+;
IDENTIFIER : [A-Za-z_][_0-9A-Za-z]*;

STRING : '"'(ESC|.)*?'"';
ESC : '\\"'|'\\\\';

WS : 
    [ \t\r\n] -> skip
    ;

LINE_COMMENT : '//' .*? '\r'? '\n' -> skip;
COMMENT      :'/*'.*?'*/'-> skip ;
