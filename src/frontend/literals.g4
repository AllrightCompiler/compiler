lexer grammar literals;

fragment HexPrefix
    : '0x'
    | '0X'
    ;
fragment OctPrefix : '0';

fragment NonzeroDigit : [1-9];
fragment Digit : [0-9];
fragment HexDigit : [0-9a-fA-F];
fragment OctDegit : [0-7];

DecIntConst : NonzeroDigit Digit*;
OctIntConst : OctPrefix OctDegit*;
HexIntConst : HexPrefix HexDigit+;

fragment Dot : '.';

fragment Sign
    : '+'
    | '-'
    ;

fragment Exponent
    : 'e'
    | 'E'
    ;
fragment HexExponent
    : 'p'
    | 'P'
    ;

fragment DecFloatFrac
    : Digit* Dot Digit+
    | Digit+ Dot
    ;
fragment HexFloatFrac
    : HexDigit* Dot HexDigit+
    | HexDigit+ Dot
    ;

fragment DecFloatExp : Exponent Sign? Digit+;
fragment BinFloatExp : HexExponent Sign? Digit+;

DecFloatConst
    : DecFloatFrac DecFloatExp?
    | Digit+ DecFloatExp
    ;
HexFloatConst
    : HexPrefix HexFloatFrac BinFloatExp
    | HexPrefix HexDigit+ BinFloatExp
    ;

fragment Escaped : '\\'['"?\\abfnrtv];

StringConst : '"' (~['\\\r\n] | Escaped)* '"';
