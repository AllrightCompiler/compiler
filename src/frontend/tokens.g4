lexer grammar tokens;

import literals;

Int : 'int';
Float : 'float';
Void : 'void';

Const : 'const';

If : 'if';
Else : 'else';
While : 'while';
Break : 'break';
Continue : 'continue';
Return : 'return';

Assign : '=';

Add : '+';
Sub : '-';
Mul : '*';
Div : '/';
Mod : '%';

Eq : '==';
Neq : '!=';
Lt : '<';
Gt : '>';
Leq : '<=';
Geq : '>=';

Not : '!';
And : '&&';
Or : '||';

Comma : ',';
Semicolon : ';';
Lparen : '(';
Rparen : ')';
Lbracket : '[';
Rbracket : ']';
Lbrace : '{';
Rbrace : '}';

Ident : [A-Za-z_][_0-9A-Za-z]*;

Whitespace : [ \t\r\n]+ -> skip;

LineComment : '//' ~[\r\n]* -> skip;
BlockComment : '/*' .*? '*/' -> skip;
