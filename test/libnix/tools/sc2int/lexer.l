%{
#include <stdlib.h>
#include <string.h>

#include "xec-debug.h"

#include "sc2int.h"
#include "sc2int_parser.h"
%}

%%

"#"[^\n]*   { printf ("aaa\n") ;}
[\n]+       { g_line += strlen (yytext); return T_NEWLINE; }
[[:space:]]+  ;
"\""[^"]*"\"" { yylval.sv = (char *)malloc (strlen (yytext) - 1); 
                strncpy (yylval.sv, yytext + 1, strlen (yytext) - 2);
                return T_STRING; }

"NAME"      return T_NAME;
"NAMESPACE" return T_NAMESPACE;
"LIMIT"     return T_LIMIT;
"CALLS"     return T_CALLS;
"BAE"       return T_BAE;

"byte"      return T_BYTE;
"half"      return T_HALF;
"word"      return T_WORD;
"dword"     return T_DWORD;
"single"    return T_SINGLE;
"double"    return T_DOUBLE;
"extended"  return T_EXTENDED;
"vector"    return T_VECTOR;
"ptr"       return T_PTR;
"intptr"    return T_INTPTR;
"void"      return T_VOID;
"..."       return T_ELLIPSIS;

[\(\):,]    return yytext[0];

[0-9]+      { yylval.iv = atoi (yytext); return T_NUMBER; }

[a-zA-Z_$][a-zA-Z0-9_$]* { yylval.sv = strdup (yytext); return T_IDENTIFIER; }

. { char buf[128];
    snprintf (buf, sizeof (buf), "encountered unknown character %02x", *yytext);
    yyerror (buf); }

