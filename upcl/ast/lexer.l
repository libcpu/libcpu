%{
#include "ast/ast.h"

using namespace upcl;

#include "parser.hpp"

%}

BIN [0-1]
DEC [0-9]
OCT [0-7]
HEX [0-9a-fA-F]
ID  [a-zA-Z_][a-zA-Z0-9_]*

%option nounput

%%

"//".* ;
[ \t\r\n]+ ;

"arch" return K_ARCH;
"name" return K_NAME;
"endian" return K_ENDIAN;
"default_endian" return K_DEFAULT_ENDIAN;
"little" return K_LITTLE;
"big" return K_BIG;
"both" return K_BOTH;

"explicit" return K_EXPLICIT;
"evaluate" return K_EVALUATE;

"byte_size" return K_BYTE_SIZE;
"word_size" return K_WORD_SIZE;
"float_size" return K_FLOAT_SIZE;
"psr_size" return K_PSR_SIZE;
"address_size" return K_ADDRESS_SIZE;

"min_page_size" return K_MIN_PAGE_SIZE;
"max_page_size" return K_MAX_PAGE_SIZE;
"default_page_size" return K_DEFAULT_PAGE_SIZE;
"page_size" return K_PAGE_SIZE;

"register_file" return K_REGISTER_FILE;
"group" return K_GROUP;

"macro" return K_MACRO;
"insn" return K_INSN;
"jump" return K_JUMP;
"type" return K_TYPE;
"condition" return K_CONDITION;
"delay" return K_DELAY;
"action" return K_ACTION;

"is" return K_IS;

"if" return K_IF;
"unless" return K_UNLESS;
"else" return K_ELSE;
"while" return K_WHILE;
"for" return K_FOR;

"%CC" return K_AUGMENT_CC;
"%S" return K_AUGMENT_SIGNED;
"%U" return K_AUGMENT_UNSIGNED;

"%M"|"%MEM" return K_MEM;

"<-" return T_BIND_LEFT;
"->" return T_BIND_RIGHT;
"<->" return T_BIND_BIDI;

".." return T_UPTO;

"==" return T_EQ;
"!=" return T_NE;
"<=" return T_LE;
">=" return T_GE;

"||" return T_LOR;
"&&" return T_LAND;

"<<"  return T_SHL;
">>"  return T_SHR;
"^<<" return T_SHLC;
"^>>" return T_SHRC;
"<<>" return T_ROL;
">><" return T_ROR;
"^<<>" return T_ROLC;
"^>><" return T_RORC;

"&~" return T_ANDCOM;
"|~" return T_ORCOM;
"^~" return T_XORCOM;

"+=" return T_ADDE;
"-=" return T_SUBE;
"*=" return T_MULE;
"/=" return T_DIVE;
"%=" return T_REME;
"|=" return T_ORE;
"&=" return T_ANDE;
"^=" return T_XORE;
"<<=" return T_SHLE;
">>=" return T_SHRE;
"^<<=" return T_SHLCE;
"^>>=" return T_SHRCE;
"<<>=" return T_ROLE;
">><=" return T_RORE;
"^<<>=" return T_ROLCE;
"^>><=" return T_RORCE;
"&~=" return T_ANDCOME;
"|~=" return T_ORCOME;
"^~=" return T_XORCOME;

"**" return T_REPEAT;

"#"[if]{DEC}+       { yylval.c_str = strdup(yytext); return T_TYPE; }
"#v"{DEC}+          { yylval.c_str = strdup(yytext); return T_TYPE; }
"#v"{DEC}+":"{DEC}+ { yylval.c_str = strdup(yytext); return T_TYPE; }

"0"[xX]{HEX}+ { yylval.c_str = strdup(yytext); return T_HEX; }
"0"[bB]{BIN}+ { yylval.c_str = strdup(yytext); return T_BIN; }
"0"{OCT}*     { yylval.c_str = strdup(yytext); return T_OCT; }
{DEC}+        { yylval.c_str = strdup(yytext); return T_DEC; }

"\""[^"]*"\"" { yylval.c_str = strdup(yytext); return T_STRING; }
{ID}          { yylval.c_str = strdup(yytext); return T_IDENTIFIER; }
"%"{ID}       { yylval.c_str = strdup(yytext); return T_META_IDENTIFIER; }
"@"{ID}       { yylval.c_str = strdup(yytext); return T_MACRO_IDENTIFIER; }
{ID}"?"       { yylval.c_str = strdup(yytext); return T_REPEAT_IDENTIFIER; }

[+\-\*\/%\{\}\[\](),\.:;?|&<>=~^!] return yytext[0];

