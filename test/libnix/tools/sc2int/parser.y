%{
#include "sc2int.h"

extern int yylex(void);
%}

%union {
  xec_param_type_t  pt;
  int               iv;
  char             *sv;
  param_t          *prm;
  param_list_t     *prms;
  call_t           *call;
}

%token T_NAME T_NAMESPACE T_LIMIT T_BAE T_CALLS 

%token T_BYTE T_HALF T_WORD T_DWORD T_SINGLE T_DOUBLE T_EXTENDED T_VECTOR T_PTR T_ELLIPSIS T_VOID T_INTPTR

%token<iv> T_NUMBER
%token<sv> T_IDENTIFIER
%token<sv> T_STRING

%token T_NEWLINE

%type <pt> data_type

%type <prm>  param
%type <prm>  return_result
%type <prms> syscall_parameters
%type <prms> syscall0_parameters

%type <call> syscall_declaration

%start root

%%

data_type:
  T_BYTE       { $$ = XEC_PARAM_BYTE; }
  | T_HALF     { $$ = XEC_PARAM_HALF; }
  | T_WORD     { $$ = XEC_PARAM_WORD; }
  | T_DWORD    { $$ = XEC_PARAM_DWORD; }
  | T_SINGLE   { $$ = XEC_PARAM_SINGLE; }
  | T_DOUBLE   { $$ = XEC_PARAM_DOUBLE; }
  | T_EXTENDED { $$ = XEC_PARAM_EXTENDED; }
  | T_VECTOR   { $$ = XEC_PARAM_VECTOR; }
  | T_PTR      { $$ = XEC_PARAM_POINTER; }
  | T_INTPTR   { $$ = XEC_PARAM_INTPTR; }
  ;

param:
  data_type { $$ = param_new ($1); } 
  ;

syscall0_parameters:
  param
    { $$ = param_list_new ($1); }
  | syscall0_parameters ',' param
    { $$ = param_list_link ($1, $3); }
  ;

syscall_parameters:
  syscall0_parameters
  | syscall0_parameters ',' T_ELLIPSIS
    { $$ = param_list_link ($1, param_new_ellipsis ()); }
  ;

void_or_empty:
  T_VOID
  |
  ;

return_result:
  T_VOID
    { $$ = NULL; }
  | param
  ;

syscall_declaration:
  T_NUMBER return_result T_IDENTIFIER '(' void_or_empty ')'
    { $$ = call_new ($1, $3, $2, NULL); }
  | T_NUMBER return_result T_IDENTIFIER '(' syscall_parameters ')'
    { $$ = call_new ($1, $3, $2, $5); }
  ;

syscall_declarations:
  syscall_declaration T_NEWLINE
    { call_list_link (g_calls, $1); }
  | syscall_declarations syscall_declaration T_NEWLINE
    { call_list_link (g_calls, $2); }
  ;

calls_declaration:
  T_CALLS
  ;

calls_declarations:
  calls_declaration T_NEWLINE syscall_declarations
  ;

name_declaration:
  T_NAME ':' T_STRING
    {
      if (g_gbl_name != NULL)
        free (g_gbl_name);

      g_gbl_name = $3;
    }
  ;

bae_declaration:
  T_BAE ':' T_IDENTIFIER
    {
      if (g_gbl_bae != NULL)
        free (g_gbl_bae);

      g_gbl_bae = $3;
    }
  ;

ns_declaration:
  T_NAMESPACE ':' T_IDENTIFIER
    {
      if (g_gbl_ns != NULL)
        free (g_gbl_ns);

      g_gbl_ns = $3;
    }
  ;

limit_declaration:
  T_LIMIT ':' T_NUMBER
    { g_gbl_limit = $3; }
  ;

head_declaration:
  name_declaration
  | ns_declaration
  | bae_declaration
  | limit_declaration
  ;

head_declarations:
  head_declaration T_NEWLINE
  | head_declarations head_declaration T_NEWLINE
  ;

root:
  head_declarations calls_declarations
  ;

