%{
#include "ast/ast.h"

using namespace upcl;

extern int yylex();
extern void yyerror(char const *);

extern ast::token_list *g_root;

%}

%union {

	char *c_str;

	ast::token *token;
	ast::token_list *list;

	ast::string *string;
	ast::number *number;

	ast::identifier *identifier;
	ast::qualified_identifier *qualified_identifier;

	ast::expression *expression;

	ast::type *type;
	ast::range *range;

	ast::register_type_name *reg_type_name;
	ast::bound_value *bound_value;
	ast::typed_bound_value *typed_bound_value;
}

/* Constants */
%token <c_str> T_DEC T_OCT T_HEX T_BIN
%token <c_str> T_STRING
%destructor { free($$); } T_DEC T_OCT T_HEX T_BIN T_STRING

/* Literals */
%token <c_str> T_IDENTIFIER T_MACRO_IDENTIFIER T_META_IDENTIFIER
%token <c_str> T_REPEAT_IDENTIFIER
%token <c_str> T_TYPE
%destructor { free($$); } T_IDENTIFIER T_MACRO_IDENTIFIER
%destructor { free($$); } T_META_IDENTIFIER T_REPEAT_IDENTIFIER
%destructor { free($$); } T_TYPE

/* Keywords */
%token K_ARCH
%token K_NAME
%token K_ENDIAN K_DEFAULT_ENDIAN K_LITTLE K_BIG K_BOTH
%token K_BYTE_SIZE K_WORD_SIZE K_FLOAT_SIZE K_ADDRESS_SIZE
%token K_PSR_SIZE
%token K_MIN_PAGE_SIZE K_MAX_PAGE_SIZE K_DEFAULT_PAGE_SIZE
%token K_PAGE_SIZE
%token K_REGISTER_FILE K_GROUP
%token K_EXPLICIT K_EVALUATE
%token K_MACRO K_INSN
%token K_IF K_UNLESS K_ELSE K_WHILE K_FOR K_IS
%token K_AUGMENT_CC K_AUGMENT_SIGNED K_AUGMENT_UNSIGNED
%token K_MEM
%token K_JUMP K_TYPE K_CONDITION K_DELAY K_ACTION

/* Symbols */
%token T_REPEAT
%token T_BIND_RIGHT T_BIND_LEFT T_BIND_BIDI
%token T_UPTO
%token T_EQ T_NE T_LE T_GE
%token T_LOR T_LAND
%token T_SHL T_SHR
%token T_ROL T_ROR
%token T_ANDCOM T_ORCOM T_XORCOM
%token T_ADDE T_SUBE T_MULE T_DIVE T_REME
%token T_ORE T_ANDE T_XORE
%token T_SHLE T_SHRE
%token T_ROLE T_RORE
%token T_ANDCOME T_ORCOME T_XORCOME

%start root

%type <string> string
%type <number> number
%type <identifier> identifier macro_identifier meta_identifier
%type <identifier> repeatible_identifier binding_register
%type <qualified_identifier> qualified_identifier
%type <token>  multi_qualified_identifier
%type <token> arch_decl
%type <type> type
%type <list> arch_body
%type <token> arch_stmts register_file_decl
%type <token> register_file_stmts
%type <list> register_file_body
%type <list> group_body
%type <token> reg_full_decl
%type <reg_type_name> reg_type_name
%type <token> binding_format binding_alias 
%type <token> register_splitter
%type <token> typed_value_bind
%type <bound_value> value_bind
%type <typed_bound_value> value_bind_ext
%type <list> typed_value_bind_list value_bind_list2 value_bind_list_ext value_bind_list_simple

%type <list> identifier_list operand_comma_list arg_list
%type <list> CC_flag_list
%type <list> toplevel_decl_list

%type <range> range

%type <expression> base_operand operand bit_combine expression arg
%type <expression> cast_expr macro_expr mem_expr SU_expr CC_expr is_expr
%type <expression> CC_flag unary_expr binary_expr

%left '/' '%' '*'
%left '-' '+'

%%

string: T_STRING
	  { $$ = new ast::string($1); }
	  ;

identifier: T_IDENTIFIER
	  	  { $$ = new ast::identifier($1); }
		  ;

number: T_DEC
	  { $$ = new ast::number($1); }
	  | T_HEX
	  { $$ = new ast::number($1); }
	  | T_OCT
	  { $$ = new ast::number($1); }
	  | T_BIN
	  { $$ = new ast::number($1); }
	  ;

meta_identifier: T_META_IDENTIFIER
	  		   { $$ = new ast::identifier($1); }
			   ;

macro_identifier: T_MACRO_IDENTIFIER
	  		    { $$ = new ast::identifier($1); }
				;

repeatible_identifier: T_REPEAT_IDENTIFIER
	  		    	 { $$ = new ast::repeat_identifier($1); }
					 | T_REPEAT_IDENTIFIER ':' number
	  		    	 { $$ = new ast::repeat_identifier($1,
					 	new ast::literal_expression($3)); }
					 | T_REPEAT_IDENTIFIER ':' '(' expression ')'
	  		    	 { $$ = new ast::repeat_identifier($1, $4); }
					 ;

qualified_identifier: identifier
					{ $$ = new ast::qualified_identifier($1); }
					| identifier '.' identifier
					{ $$ = new ast::qualified_identifier($1, $3); }
					;

type: T_TYPE
	{ $$ = new ast::type($1); }
	;

/** Declarations **/

/**
 * Arch
 */

arch_decl: K_ARCH string '{' arch_body '}'
		 { $$ = new ast::architecture($2, $4); }
		 | K_ARCH string '{' arch_body '}' ';'
		 { $$ = new ast::architecture($2, $4); }
		 ;

arch_body: arch_stmts
		 { $$ = new ast::token_list($1); }
		 | arch_body arch_stmts
		 { $$ = $1; $$->push($2); }
		 ;

arch_stmts: K_NAME string ';'
		  { $$ = new ast::tagged_value(ast::architecture::NAME, new ast::literal_expression($2)); }
		  | K_ENDIAN K_LITTLE ';'
		  { $$ = new ast::tagged_value(ast::architecture::ENDIAN, ast::architecture::ENDIAN_LITTLE); }
		  | K_ENDIAN K_BIG ';'
		  { $$ = new ast::tagged_value(ast::architecture::ENDIAN, ast::architecture::ENDIAN_BIG); }
		  | K_ENDIAN K_BOTH ';'
		  { $$ = new ast::tagged_value(ast::architecture::ENDIAN, ast::architecture::ENDIAN_BOTH); }
		  | K_DEFAULT_ENDIAN K_LITTLE ';'
		  { $$ = new ast::tagged_value(ast::architecture::DEFAULT_ENDIAN, ast::architecture::ENDIAN_LITTLE); }
		  | K_DEFAULT_ENDIAN K_BIG ';'
		  { $$ = new ast::tagged_value(ast::architecture::DEFAULT_ENDIAN, ast::architecture::ENDIAN_BIG); }
		  | K_BYTE_SIZE expression ';'
		  { $$ = new ast::tagged_value(ast::architecture::BYTE_SIZE, $2); }
		  | K_WORD_SIZE expression ';'
		  { $$ = new ast::tagged_value(ast::architecture::WORD_SIZE, $2); }
		  | K_FLOAT_SIZE expression ';'
		  { $$ = new ast::tagged_value(ast::architecture::FLOAT_SIZE, $2); }
		  | K_ADDRESS_SIZE expression ';'
		  { $$ = new ast::tagged_value(ast::architecture::ADDRESS_SIZE, $2); }
		  | K_PSR_SIZE expression ';'
		  { $$ = new ast::tagged_value(ast::architecture::PSR_SIZE, $2); }
		  | K_MIN_PAGE_SIZE expression ';'
		  { $$ = new ast::tagged_value(ast::architecture::MIN_PAGE_SIZE, $2); }
		  | K_MAX_PAGE_SIZE expression ';'
		  { $$ = new ast::tagged_value(ast::architecture::MAX_PAGE_SIZE, $2); }
		  | K_DEFAULT_PAGE_SIZE expression ';'
		  { $$ = new ast::tagged_value(ast::architecture::DEFAULT_PAGE_SIZE, $2); }
		  | K_PAGE_SIZE expression ';'
		  { $$ = new ast::tagged_value(ast::architecture::PAGE_SIZE, $2); }
		  | register_file_decl
		  { $$ = $1; }
		  ;

register_file_decl: K_REGISTER_FILE '{' register_file_body '}'
				  { $$ = new ast::register_file($3); }
				  | K_REGISTER_FILE '{' register_file_body '}' ';'
				  { $$ = new ast::register_file($3); }
				  ;

register_file_body: register_file_stmts
				  { $$ = new ast::token_list($1); }
				  | register_file_body register_file_stmts
				  { $$ = $1; $1->push($2); }
				  ;

register_file_stmts: K_GROUP identifier ':' reg_full_decl ';'
				   { $$ = new ast::register_group($2, $4); }
				   | K_GROUP identifier '{' group_body '}'
				   { $$ = new ast::register_group($2, $4); }
				   | K_GROUP identifier '{' group_body '}' ';'
				   { $$ = new ast::register_group($2, $4); }
				   ;

group_body: reg_full_decl
		  { $$ = new ast::token_list($1); }
		  | group_body ',' reg_full_decl
		  { $$ = $1; $1->push($3); }
		  ;

reg_full_decl: '[' reg_type_name ']'
			 { $$ = new ast::register_declaration($2); }
			 | '[' reg_type_name binding_format ']'
			 { $$ = new ast::register_declaration($2, $3); }
			 | '[' reg_type_name binding_alias ']'
			 { $$ = new ast::register_declaration($2, $3); }
			 | '[' expression T_REPEAT reg_type_name ']'
			 { $$ = new ast::register_declaration($2, $4); }
			 | '[' expression T_REPEAT reg_type_name binding_format ']'
			 { $$ = new ast::register_declaration($2, $4, $5); }
			 | '[' expression T_REPEAT reg_type_name binding_alias ']'
			 { $$ = new ast::register_declaration($2, $4, $5); }
			 ;

reg_type_name: type identifier
			 { $$ = new ast::register_type_name($1, $2); }
			 | type repeatible_identifier
			 { $$ = new ast::register_type_name($1, $2); }
			 ;

binding_format: T_BIND_RIGHT register_splitter
			  { $$ = $2; }
			  | T_BIND_RIGHT binding_register
			  { $$ = new ast::register_binder($2); }
			  | T_BIND_RIGHT binding_register register_splitter
			  { $$ = new ast::register_binder($2, $3); }
			  ;

binding_register: meta_identifier
				;

register_splitter:
				 /* Evaluator */
				   K_EVALUATE '(' expression ')'
				 { $$ = new ast::register_splitter1(0, 0, $3, false); }
				 /* Union */ 
				 | '[' typed_value_bind_list ']' 
				 { $$ = new ast::register_splitter1(0, $2, 0, false); }
				 | K_EVALUATE '(' expression ')' '[' typed_value_bind_list ']' 
				 { $$ = new ast::register_splitter1(0, $6, $3, false); } 
		  		 | K_EXPLICIT '[' typed_value_bind_list ']' 
				 { $$ = new ast::register_splitter1(0, $3, 0, true); }
		  		 | K_EXPLICIT K_EVALUATE '(' expression ')' '[' typed_value_bind_list ']' 
				 { $$ = new ast::register_splitter1(0, $7, $4, true); }
		  		 | type '[' typed_value_bind_list ']' 
				 { $$ = new ast::register_splitter1($1, $3, 0, false); }
		  		 | type K_EVALUATE '(' expression ')' '[' typed_value_bind_list ']' 
				 { $$ = new ast::register_splitter1($1, $7, $4, false); }
		  		 | type K_EXPLICIT '[' typed_value_bind_list ']' 
				 { $$ = new ast::register_splitter1($1, $4, 0, true); }
		  		 | type K_EXPLICIT K_EVALUATE '(' expression ')' '[' typed_value_bind_list ']' 
				 { $$ = new ast::register_splitter1($1, $8, $5, true); }
		  		 /* Field-Bind */
		  		 | type '(' value_bind_list2 ')'
				 { $$ = new ast::register_splitter2($1, $3, 0, false); }
		  		 | type K_EVALUATE '(' expression ')' '(' value_bind_list2 ')'
				 { $$ = new ast::register_splitter2($1, $7, $4, false); }
		  		 | type K_EXPLICIT '(' value_bind_list2 ')'
				 { $$ = new ast::register_splitter2($1, $4, 0, true); }
		  		 | type K_EXPLICIT K_EVALUATE '(' expression ')' '(' value_bind_list2 ')'
				 { $$ = new ast::register_splitter2($1, $8, $5, true); }
		  		 ;

typed_value_bind: type value_bind
				{ $$ = new ast::typed_bound_value($1, $2); }
				| type '[' value_bind_list_ext ']'
				{ $$ = new ast::typed_bound_value($1, $3); }
				;

typed_value_bind_list: typed_value_bind
					 { $$ = new ast::token_list($1); }
					 | typed_value_bind_list ',' typed_value_bind
					 { $$ = $1; $1->push($3); }
					 ;

value_bind_list_ext: value_bind_ext
				   { $$ = new ast::token_list($1); }
			   	   | value_bind_list_ext ',' value_bind_ext
				   { $$ = $1; $1->push($3); }
			   	   ;

value_bind_list_simple: value_bind
					  { $$ = new ast::token_list($1); }
			   		  | value_bind_list_simple ',' value_bind
					  { $$ = $1; $1->push($3); }
			   		  ;

value_bind_list2: value_bind
				{ $$ = new ast::token_list($1); }
			    | value_bind_list2 ':' value_bind
				{ $$ = $1; $1->push($3); }
			    ;

value_bind: expression
		  { $$ = new ast::bound_value($1); }
		  | identifier T_BIND_RIGHT meta_identifier
		  { $$ = new ast::bound_value($1, $3, false); }
		  | identifier T_BIND_LEFT qualified_identifier
		  { $$ = new ast::bound_value($1, $3, false); }
		  | identifier T_BIND_BIDI qualified_identifier
		  { $$ = new ast::bound_value($1, $3, true); }
		  | identifier T_BIND_LEFT repeatible_identifier
		  { $$ = new ast::bound_value($1, $3, false); }
		  | identifier T_BIND_BIDI repeatible_identifier
		  { $$ = new ast::bound_value($1, $3, true); }
		  | identifier T_BIND_LEFT '(' expression ')'
		  { $$ = new ast::bound_value($1, $4); }
		  ;

value_bind_ext: value_bind
			  { $$ = new ast::typed_bound_value(0, $1); }
			  | type '[' value_bind_list_simple ']'
			  { $$ = new ast::typed_bound_value($1, $3); }
			  ;

// XXX: Note: reg_binding_alias
binding_alias: T_BIND_LEFT expression
			 { $$ = new ast::alias_value($2); }
			 | T_BIND_RIGHT identifier
			 { $$ = new ast::alias_value($2, false); }
			 | T_BIND_RIGHT repeatible_identifier
			 { $$ = new ast::alias_value($2, false); }
			 ;

multi_qualified_identifier: identifier '.' '[' identifier_list ']'
						  { $$ = new ast::qualified_identifier($1, $4); }
						  ;

identifier_list: identifier
			   { $$ = new ast::token_list($1); }
			   | identifier_list ',' identifier
			   { $$ = $1; $1->push($3); }
			   ;

base_operand: qualified_identifier
			{ $$ = new ast::literal_expression($1); }
	 		| multi_qualified_identifier
			{ $$ = new ast::literal_expression($1); }
	   		| meta_identifier
			{ $$ = new ast::literal_expression($1); }
	   		| number
			{ $$ = new ast::literal_expression($1); }
	   		| mem_expr
	   		| bit_combine
	   		| '(' expression ')'
			{ $$ = new ast::unary_expression(ast::unary_expression::SUB, $2); }
	   		;

operand: base_operand
	   | base_operand range
	   { $$ = new ast::bit_slice_expression($1, $2); }
	   ;

range: '[' expression ':' expression ']'
	 { $$ = new ast::range(true, $2, $4); }
 	 | '[' expression T_UPTO expression ']'
	 { $$ = new ast::range(false, $2, $4); }
	 ;

bit_combine: '(' operand_comma_list ')'
		   { $$ = new ast::bit_combine_expression($2); }
		   ;

operand_comma_list: operand
				  { $$ = new ast::token_list($1); }
                  | operand_comma_list ':' operand
				  { $$ = $1; $1->push($3); }
                  ;

expression: binary_expr
		  ;

primary_expr: operand
			| cast_expr
			| macro_expr
			| mem_expr
			| CC_expr
			| SU_expr
			| is_expr
			; 

cast_expr: '[' type expression ']'
		 { $$ = new ast::cast_expression($3, $2); }
		 ;

macro_expr: macro_identifier '(' ')'
		  { $$ = new ast::call_expression($1); }
		  | macro_identifier '(' arg_list ')'
		  { $$ = new ast::call_expression($1, $3); }
		  ;

arg: expression
   | string
   { $$ = new ast::literal_expression($1); }
   ;

arg_list: arg
	    { $$ = new ast::token_list($1); }
	    | arg_list ',' arg
	    { $$ = $1; $1->push($3); }
	    ;

mem_expr: K_MEM '[' expression ']'
		{ $$ = new ast::memory_expression($3); }
		| type K_MEM '[' expression ']'
		{ $$ = new ast::memory_expression($4, $1); }
		;

SU_expr: K_AUGMENT_SIGNED '(' expression ')'
	   { $$ = new ast::unary_expression(ast::unary_expression::SIGNED, $3); }
	   | K_AUGMENT_UNSIGNED '(' expression ')'
	   { $$ = new ast::unary_expression(ast::unary_expression::UNSIGNED, $3); }
	   ;

CC_expr: K_AUGMENT_CC '(' expression ')'
	   { $$ = new ast::CC_expression($3); }
	   | K_AUGMENT_CC '(' expression ',' CC_flag ')'
	   { $$ = new ast::CC_expression($3, $5); }
	   | K_AUGMENT_CC '(' expression ',' '[' CC_flag_list ']' ')'
	   { $$ = new ast::CC_expression($3, $6); }
	   ;

CC_flag: identifier
	   { $$ = new ast::literal_expression($1); }
	   | '!' identifier
	   { $$ = new ast::unary_expression(ast::unary_expression::NOT,
	   		new ast::literal_expression($2)); }
	   ;

CC_flag_list: CC_flag
			{ $$ = new ast::token_list($1); }
			| CC_flag_list ',' CC_flag
			{ $$ = $1; $1->push($3); }
			;

is_expr: expression K_IS type
	   { $$ = new ast::binary_expression(ast::binary_expression::IS,
	  		$1, new ast::literal_expression($3)); }
	   ;

unary_expr: primary_expr
		  | '+' expression
		  { $$ = $2; }
		  | '-' expression
		  { $$ = new ast::unary_expression(ast::unary_expression::NEG, $2); }
		  | '!' expression
		  { $$ = new ast::unary_expression(ast::unary_expression::NOT, $2); }
		  | '~' expression
		  { $$ = new ast::unary_expression(ast::unary_expression::COM, $2); }
		  ;

binary_expr: unary_expr
		   /* Logical */
		   | binary_expr T_LOR expression
		   { $$ = new ast::binary_expression(ast::binary_expression::LOR, $1, $3); }
		   | binary_expr T_LAND expression
		   { $$ = new ast::binary_expression(ast::binary_expression::LAND, $1, $3); }
		   /* Relational */
		   | binary_expr T_EQ expression
		   { $$ = new ast::binary_expression(ast::binary_expression::EQ, $1, $3); }
		   | binary_expr T_NE expression
		   { $$ = new ast::binary_expression(ast::binary_expression::NE, $1, $3); }
		   | binary_expr '<' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::LT, $1, $3); }
		   | binary_expr T_LE expression
		   { $$ = new ast::binary_expression(ast::binary_expression::LE, $1, $3); }
		   | binary_expr T_GE expression
		   { $$ = new ast::binary_expression(ast::binary_expression::GE, $1, $3); }
		   | binary_expr '>' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::GT, $1, $3); }
		   /* Multiplicative */
		   | binary_expr '*' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::MUL, $1, $3); }
		   | binary_expr '/' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::DIV, $1, $3); }
		   | binary_expr '%' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::MOD, $1, $3); }
		   | binary_expr T_SHL expression
		   { $$ = new ast::binary_expression(ast::binary_expression::SHL, $1, $3); }
		   | binary_expr T_SHR expression
		   { $$ = new ast::binary_expression(ast::binary_expression::SHR, $1, $3); }
		   | binary_expr T_ROL expression
		   { $$ = new ast::binary_expression(ast::binary_expression::ROL, $1, $3); }
		   | binary_expr T_ROR expression
		   { $$ = new ast::binary_expression(ast::binary_expression::ROR, $1, $3); }
		   | binary_expr '&' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::AND, $1, $3); }
		   | binary_expr T_ANDCOM expression
		   { $$ = new ast::binary_expression(ast::binary_expression::ANDCOM, $1, $3); }
		   /* Additive */
		   | binary_expr '+' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::ADD, $1, $3); }
		   | binary_expr '-' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::SUB, $1, $3); }
		   | binary_expr '|' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::OR, $1, $3); }
		   | binary_expr T_ORCOM expression
		   { $$ = new ast::binary_expression(ast::binary_expression::ORCOM, $1, $3); }
		   | binary_expr '^' expression
		   { $$ = new ast::binary_expression(ast::binary_expression::XOR, $1, $3); }
		   | binary_expr T_XORCOM expression
		   { $$ = new ast::binary_expression(ast::binary_expression::XORCOM, $1, $3); }
		   ;

insn_decl: K_INSN identifier ':' ';'
		 | K_INSN identifier ':' inline_insn_stmts ';'
         | K_INSN identifier insn_enclosed_body
         | K_INSN identifier insn_enclosed_body ';'
         ;

insn_enclosed_body: '{' insn_body '}'
				  ;

insn_body: insn_stmts
         | insn_body insn_stmts
         ;

basic_insn_stmts: expression
				| assign_decl
			    | type assign_decl 
          		;

insn_stmts: insn_enclosed_body
  		  | if_decl
		  | basic_insn_stmts ';'
		  | basic_insn_stmts K_IF '(' expression ')' ';'
		  | basic_insn_stmts K_UNLESS '(' expression ')' ';'
		  ;

/*XXX I don't like this copy ! Merge the syntax. */
inline_insn_stmts: if_decl
				 | basic_insn_stmts
				 | basic_insn_stmts K_IF '(' expression ')'
		  		 | basic_insn_stmts K_UNLESS '(' expression ')'
		  		 ;

assign_decl: operand '=' expression
           | operand T_ADDE expression
           | operand T_SUBE expression
           | operand T_MULE expression
           | operand T_DIVE expression
           | operand T_REME expression
           | operand T_SHLE expression
           | operand T_SHRE expression
           | operand T_ROLE expression
           | operand T_RORE expression
           | operand T_ANDE expression
           | operand T_ANDCOME expression
           | operand T_ORE expression
           | operand T_ORCOME expression
           | operand T_XORE expression
           | operand T_XORCOME expression
           ;

if_decl: K_IF '(' expression ')' insn_stmts
	   | K_IF '(' expression ')' insn_stmts K_ELSE insn_stmts
	   | K_UNLESS '(' expression ')' insn_stmts 
	   | K_UNLESS '(' expression ')' insn_stmts K_ELSE insn_stmts
	   ;

jump_insn_decl: K_JUMP K_INSN identifier ':' inline_jump_decl
			  | K_JUMP K_INSN identifier jump_enclosed_body
			  | K_JUMP K_INSN identifier jump_enclosed_body ';'
			  ;

inline_jump_decl: jump_type jump_delay jump_condition jump_action
				| jump_type jump_condition jump_action
				| jump_type jump_delay jump_action
				| jump_type jump_action
				;

jump_enclosed_body: '{' jump_body '}'
				  ;

jump_body: jump_type ';' jump_delay ';' jump_condition ';' jump_action
		 | jump_type ';' jump_condition ';' jump_action
		 | jump_type ';' jump_delay ';' jump_action
		 | jump_type ';' jump_action
		 ;

jump_type: K_TYPE identifier
		 ;

jump_condition: K_CONDITION expression
			  ;

jump_delay: K_DELAY expression
		  ;

jump_action: K_ACTION inline_insn_stmts ';'
		   | insn_enclosed_body
		   ;

insn_group_decl: K_GROUP K_INSN identifier '[' identifier_list ']' K_CONDITION expression ';'
		  	   ;

macro_decl: K_MACRO identifier '(' ')' ':' inline_insn_stmts ';'
		  | K_MACRO identifier '(' identifier_list ')' ':' inline_insn_stmts ';'
		  | K_MACRO identifier '(' ')' insn_enclosed_body
		  | K_MACRO identifier '(' identifier_list ')' insn_enclosed_body
		  ;

toplevel_decl: insn_decl
			 | jump_insn_decl
			 | insn_group_decl
			 | macro_decl
			 ;

toplevel_decl_list: toplevel_decl
				  //{ $$ = new token_list($1); }
				  | toplevel_decl_list toplevel_decl
				  //{ $$ = $1; $1->push($2); }
		 		  ;

root: arch_decl toplevel_decl_list
	{ g_root = new ast::token_list($1); }
	;

