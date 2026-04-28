grammar GPAC;

programFile
    : programDecl topLevel* EOF
    ;

programDecl
    : MAIN COLON PROGRAM ASSIGN IDENT
    ;

topLevel
    : memoryDecl
    | trackDecl
    | structDecl
    | externProcDecl
    | procDecl
    ;

memoryDecl
    : MEMORY IDENT DELEGATES IDENT
    ;

trackDecl
    : TRACK IDENT
    ;

structDecl
    : STRUCT IDENT structField* END
    ;

structField
    : IDENT COLON type
    ;

externProcDecl
    : EXTERN PROC IDENT LPAREN paramList? RPAREN returnType?
    ;

procDecl
    : PROC IDENT LPAREN paramList? RPAREN returnType? statement* END
    ;

paramList
    : param (COMMA param)*
    ;

param
    : IDENT COLON type
    ;

returnType
    : ARROW type
    ;

statement
    : letStmt
    | assignStmt
    | printStmt
    | inputStmt
    | returnStmt
    | callStmt
    | ifStmt
    | loopStmt
    | runStmt
    | exprStmt
    ;

letStmt
    : LET IDENT COLON type ASSIGN expr (IN IDENT)?
    | LET IDENT COLON type
    ;

assignStmt
    : lvalue ASSIGN expr
    ;

printStmt
    : PRINT expr ARROW CONSOLE
    ;

inputStmt
    : LET IDENT COLON type ASSIGN INPUT ARROW CONSOLE
    ;

returnStmt
    : RETURN expr?
    ;

callStmt
    : CALL IDENT LPAREN argList? RPAREN
    ;

runStmt
    : RUN IDENT LPAREN argList? RPAREN ON IDENT
    ;

ifStmt
    : IF LPAREN expr RPAREN statement* (ELSE statement*)? END
    ;

loopStmt
    : LOOP LPAREN expr RPAREN statement* END
    ;

exprStmt
    : expr
    ;

argList
    : expr (COMMA expr)*
    ;

lvalue
    : IDENT (DOT IDENT | LBRACK expr RBRACK)*
    ;

expr
    : logicalOr
    ;

logicalOr
    : logicalAnd (OR logicalAnd)*
    ;

logicalAnd
    : equality (AND equality)*
    ;

equality
    : relational ((EQ | NEQ) relational)*
    ;

relational
    : additive ((LT | LTE | GT | GTE) additive)*
    ;

additive
    : multiplicative ((PLUS | MINUS) multiplicative)*
    ;

multiplicative
    : unary ((STAR | SLASH | PERCENT) unary)*
    ;

unary
    : (NOT | MINUS | AMP) unary
    | primary
    ;

primary
    : INT_LIT
    | FLOAT_LIT
    | STRING_LIT
    | TRUE
    | FALSE
    | CALL IDENT LPAREN argList? RPAREN
    | lvalue
    | arrayLiteral
    | LPAREN expr RPAREN
    ;

arrayLiteral
    : LBRACK (expr (COMMA expr)*)? RBRACK
    ;

type
    : INT
    | FLOAT
    | BOOL
    | CHAR
    | STRING
    | PTR LT type GT
    | ARRAY LT type GT
    | IDENT
    ;

MAIN: 'Main';
PROGRAM: 'program';
PROC: 'Proc';
EXTERN: 'extern';
STRUCT: 'struct';
MEMORY: 'memory';
DELEGATES: 'delegates';
TRACK: 'track';
LET: 'let';
IN: 'in';
PRINT: 'print';
INPUT: 'input';
CONSOLE: 'console';
RETURN: 'return';
CALL: 'call';
RUN: 'run';
ON: 'on';
IF: 'if';
ELSE: 'else';
LOOP: 'loop';
END: 'end';
TRUE: 'true';
FALSE: 'false';

INT: 'int';
FLOAT: 'float';
BOOL: 'bool';
CHAR: 'char';
STRING: 'string';
PTR: 'ptr';
ARRAY: 'array';

ASSIGN: '=';
ARROW: '->';
COLON: ':';
COMMA: ',';
DOT: '.';
LPAREN: '(';
RPAREN: ')';
LBRACK: '[';
RBRACK: ']';
PLUS: '+';
MINUS: '-';
STAR: '*';
SLASH: '/';
PERCENT: '%';
AMP: '&';
NOT: '!';
AND: '&&';
OR: '||';
EQ: '==';
NEQ: '!=';
LT: '<';
LTE: '<=';
GT: '>';
GTE: '>=';

IDENT: [a-zA-Z_][a-zA-Z0-9_]*;
INT_LIT: [0-9]+;
FLOAT_LIT: [0-9]+ '.' [0-9]+;
STRING_LIT: '"' (~["\\] | '\\' .)* '"';

COMMENT: '--' ~[\r\n]* -> skip;
WS: [ \t\r\n]+ -> skip;
