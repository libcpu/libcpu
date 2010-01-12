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

	ast::instruction *instruction;
	ast::statement *statement;

	ast::jump *jump;

	ast::decoder_operands *decoder_operands;
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
%token K_ENDIAN K_DEFAULT_ENDIAN K_LITTLE K_BIG K_PDP K_NONE K_BOTH
%token K_BYTE_SIZE K_WORD_SIZE K_FLOAT_SIZE K_ADDRESS_SIZE
%token K_PSR_SIZE
%token K_MIN_PAGE_SIZE K_MAX_PAGE_SIZE K_DEFAULT_PAGE_SIZE
%token K_PAGE_SIZE
%token K_REGISTER_FILE K_GROUP
%token K_EXPLICIT K_EVALUATE
%token K_MACRO K_INSN
%token K_IF K_ELSE K_WHILE K_FOR K_IS K_RETURN
%token K_AUGMENT_CC K_AUGMENT_SIGNED K_AUGMENT_UNSIGNED
%token K_MEM
%token K_JUMP K_TYPE K_PRE K_CONDITION K_DELAY
%token K_RESET K_DECODER_OPERANDS

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
%type <list> toplevel_decl_list toplevel_decl_list_or_null

%type <range> range

%type <expression> base_operand operand bit_combine expression arg
%type <expression> cast_expr macro_expr mem_expr SU_expr CC_expr is_expr
%type <expression> CC_flag unary_expr binary_expr primary_expr

%type <instruction> insn_decl jump_insn_decl macro_decl insn_group_decl
%type <list> insn_body insn_enclosed_body
%type <statement> insn_stmts inline_insn_stmts basic_insn_stmts
%type <statement> assign_decl if_decl for_decl while_decl flow_decl
%type <list> assignment_list assignment_list_or_null
%type <expression> expression_or_null
%type <jump> jump_decl
%type <identifier> jump_type
%type <expression> jump_condition jump_delay
%type <list> jump_pre jump_action
%type <decoder_operands> decoder_operands_decl decoder_operands_decl_or_null

%type <instruction> toplevel_decl

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
		 { $$ = new ast::instruction($2); }
		 | K_INSN identifier ':' inline_insn_stmts ';'
		 { $$ = new ast::instruction($2, $4); }
		 | K_INSN identifier insn_enclosed_body
		 { $$ = new ast::instruction($2, $3); }
		 | K_INSN identifier insn_enclosed_body ';'
		 { $$ = new ast::instruction($2, $3); }
		 ;

insn_enclosed_body: '{' insn_body '}'
				  { $$ = $2; }
				  ;

insn_body: insn_stmts
		 { $$ = new ast::token_list($1); }
         | insn_body insn_stmts
	 	 { $$ = $1; $1->push($2); }
         ;

basic_insn_stmts: CC_expr
				{ $$ = new ast::expression_statement($1); }
				| macro_expr
				{ $$ = new ast::expression_statement($1); }
				| assign_decl
			    | type assign_decl 
				{ $$ = $2; $2->set_lhs_type($1); }
          		;

insn_stmts: insn_enclosed_body
		  { $$ = new ast::statement_list_statement($1); }
  		  | flow_decl
		  | basic_insn_stmts ';'
		  { $$ = $1; }
		  ;

inline_insn_stmts: flow_decl
				 | basic_insn_stmts
		  		 ;

assign_decl: operand '=' expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::EQ, $1, $3); }
           | operand T_ADDE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::ADDE, $1, $3); }
           | operand T_SUBE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::SUBE, $1, $3); }
           | operand T_MULE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::MULE, $1, $3); }
           | operand T_DIVE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::DIVE, $1, $3); }
           | operand T_REME expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::REME, $1, $3); }
           | operand T_SHLE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::SHLE, $1, $3); }
           | operand T_SHRE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::SHRE, $1, $3); }
           | operand T_ROLE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::ROLE, $1, $3); }
           | operand T_RORE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::RORE, $1, $3); }
           | operand T_ANDE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::ANDE, $1, $3); }
           | operand T_ANDCOME expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::ANDCOME, $1, $3); }
           | operand T_ORE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::ORE, $1, $3); }
           | operand T_ORCOME expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::ORCOME, $1, $3); }
           | operand T_XORE expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::XORE, $1, $3); }
           | operand T_XORCOME expression
		   { $$ = new ast::assignment_statement(ast::assignment_statement::XORCOME, $1, $3); }
           ;

if_decl: K_IF '(' expression ')' insn_stmts
	   { $$ = new ast::if_statement($3, $5); }
	   | K_IF '(' expression ')' insn_stmts K_ELSE insn_stmts
	   { $$ = new ast::if_statement($3, $5, $7); }
	   ;

for_decl: K_FOR '(' assignment_list_or_null ';' expression_or_null ';' assignment_list_or_null ')' 
		{ $$ = new ast::for_statement($3, $5, $7); }
		;

assignment_list: assign_decl
			   { $$ = new ast::token_list($1); }
			   | assignment_list ',' assign_decl
			   { $$ = $1; $$->push($3); }
			   ;

expression_or_null:
				  { $$ = 0; }
				  | expression
				  ;

assignment_list_or_null:
					   { $$ = 0; }
					   | assignment_list
					   ;

while_decl: K_WHILE '(' expression ')' 
		  { $$ = new ast::for_statement(0, $3); }
		  ;

flow_decl: if_decl
		 | for_decl
		 | while_decl
		 ;

jump_insn_decl: K_JUMP K_INSN identifier ':' jump_decl
			  { $$ = new ast::jump_instruction($3, $5); }
			  ;

jump_decl: jump_type jump_delay jump_pre jump_condition jump_action
		 { $$ = new ast::jump($1, $2, $3, $4, $5); }
		 ;

jump_type: K_TYPE identifier
		 { $$ = $2; }
		 | K_TYPE K_RETURN
		 { $$ = new ast::identifier("return"); }
		 ;

jump_condition:
			  { $$ = 0; }
			  | K_CONDITION expression
			  { $$ = $2; }
			  ;

jump_delay:
		  { $$ = 0; }
		  | K_DELAY expression
		  { $$ = $2; }
		  ;

jump_pre:
		{ $$ = 0; }
		| K_PRE inline_insn_stmts
		{ $$ = new ast::token_list($2); }
		| K_PRE insn_enclosed_body
		{ $$ = $2; }
		;

jump_action: insn_enclosed_body
		   ;

insn_group_decl: K_GROUP K_INSN identifier '[' identifier_list ']' K_CONDITION expression ';'
			   { $$ = 0; }
		  	   ;

macro_decl: K_MACRO identifier '(' ')' ':' inline_insn_stmts ';'
		  { $$ = new ast::macro($2, $6); }
		  | K_MACRO identifier '(' identifier_list ')' ':' inline_insn_stmts ';'
		  { $$ = new ast::macro($2, $4, $7); }
		  | K_MACRO identifier '(' ')' insn_enclosed_body
		  { $$ = new ast::macro($2, $5); }
		  | K_MACRO identifier '(' identifier_list ')' insn_enclosed_body
		  { $$ = new ast::macro($2, $4, $6); }
		  ;

decoder_operands_decl: K_DECODER_OPERANDS '[' identifier_list ']' ';'
					 { $$ = new ast::decoder_operands($3); }
					 ;

decoder_operands_decl_or_null:
							 { $$ = 0; }
							 | decoder_operands_decl
							 ;

toplevel_decl: insn_decl
			 | jump_insn_decl
			 | insn_group_decl
			 | macro_decl
			 ;

toplevel_decl_list: toplevel_decl
				  { $$ = new ast::token_list($1); }
				  | toplevel_decl_list toplevel_decl
				  { $$ = $1; $1->push($2); }
		 		  ;

toplevel_decl_list_or_null:
						  { $$ = 0; }
						  | toplevel_decl_list
						  ;

root: arch_decl decoder_operands_decl_or_null toplevel_decl_list_or_null
	{ 
		g_root = new ast::token_list($1);
		if ($2 != 0) g_root->push($2);
		if ($3 != 0) g_root->push($3);
	}
	;

