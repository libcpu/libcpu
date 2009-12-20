%{

#ifdef __cplusplus
extern "C" int yylex(void);
extern "C" int yyparse(void);
extern "C" void yyerror(char const *);
#else
extern int yylex(void);
extern void yyerror(char const *);
#endif

%}

/* Constants */
%token T_DEC T_OCT T_HEX T_BIN
%token T_STRING

/* Literals */
%token T_IDENTIFIER T_MACRO_IDENTIFIER T_META_IDENTIFIER
%token T_REPEAT_IDENTIFIER
%token T_TYPE

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
%token K_IF K_UNLESS K_ELSE K_WHILE K_FOR
%token K_AUGMENT_CC K_AUGMENT_SIGNED K_AUGMENT_UNSIGNED
%token K_MEM
%token K_JUMP K_TYPE K_CONDITION K_DELAY K_ACTION

/* Symbols */
%token T_REPEAT
%token T_BIND_RIGHT T_BIND_LEFT T_BIND_BIDI
%token T_UPTO
%token T_EQ T_NE T_LE T_GE
%token T_LOR T_LAND
%token T_SHL T_SHR T_SHLC T_SHRC
%token T_ROL T_ROR T_ROLC T_RORC
%token T_ANDCOM T_ORCOM T_XORCOM
%token T_ADDE T_SUBE T_MULE T_DIVE T_REME
%token T_ORE T_ANDE T_XORE
%token T_SHLE T_SHRE T_SHLCE T_SHRCE
%token T_ROLE T_RORE T_ROLCE T_RORCE
%token T_ANDCOME T_ORCOME T_XORCOME

%start root

%%

string: T_STRING
	  ;

identifier: T_IDENTIFIER
		  ;

number: T_DEC
	  | T_HEX
	  | T_OCT
	  | T_BIN
	  ;

meta_identifier: T_META_IDENTIFIER
			   ;

macro_identifier: T_MACRO_IDENTIFIER
				;

repeatible_identifier: T_REPEAT_IDENTIFIER
					 | T_REPEAT_IDENTIFIER ':' number
					 | T_REPEAT_IDENTIFIER ':' '(' expression ')'
					 ;

qualified_identifier: identifier
					| identifier '.' identifier
					;

type: T_TYPE
	;

/** Declarations **/

/**
 * Arch
 */

arch_decl: K_ARCH string '{' arch_body '}'
		 | K_ARCH string '{' arch_body '}' ';'
		 ;

arch_body: arch_stmts
		 | arch_body arch_stmts
		 ;

arch_stmts: K_NAME string ';'
		  | K_ENDIAN K_LITTLE ';'
		  | K_ENDIAN K_BIG ';'
		  | K_ENDIAN K_BOTH ';'
		  | K_DEFAULT_ENDIAN K_LITTLE ';'
		  | K_DEFAULT_ENDIAN K_BIG ';'
		  | K_BYTE_SIZE expression ';'
		  | K_WORD_SIZE expression ';'
		  | K_FLOAT_SIZE expression ';'
		  | K_ADDRESS_SIZE expression ';'
		  | K_PSR_SIZE expression ';'
		  | K_MIN_PAGE_SIZE expression ';'
		  | K_MAX_PAGE_SIZE expression ';'
		  | K_DEFAULT_PAGE_SIZE expression ';'
		  | K_PAGE_SIZE expression ';'
		  | register_file_decl
		  ;

register_file_decl: K_REGISTER_FILE '{' register_file_body '}'
				  | K_REGISTER_FILE '{' register_file_body '}' ';'
				  ;

register_file_body: register_file_stmts
				  | register_file_body register_file_stmts
				  ;

register_file_stmts: K_GROUP identifier ':' reg_full_decl ';'
				   | K_GROUP identifier '{' group_body '}'
				   | K_GROUP identifier '{' group_body '}' ';'
				   | K_GROUP identifier '{' group_body_union_list '}'
				   | K_GROUP identifier '{' group_body_union_list '}' ';'
				   ;

group_body: reg_full_decl
		  | group_body ',' reg_full_decl
		  ;

group_body_union: '{' group_body '}'
				;


group_body_union_list: group_body_union
					 | group_body_union_list ',' group_body_union
					 ;

reg_full_decl: '[' reg_decl ']'
			 | '[' reg_decl binding_format ']'
			 | '[' reg_decl binding_alias ']'
			 | '[' expression T_REPEAT reg_decl ']'
			 | '[' expression T_REPEAT reg_decl binding_format ']'
			 | '[' expression T_REPEAT reg_decl binding_alias ']'
			 ;

reg_decl: type identifier
		| type repeatible_identifier
		;

binding_format: T_BIND_RIGHT '{' expression '}'
			  | T_BIND_RIGHT register_splitter
			  | T_BIND_RIGHT binding_register
			  | T_BIND_RIGHT binding_register register_splitter
			  ;

binding_register: meta_identifier
				;

register_splitter: /* Union */ 
				   '[' typed_value_bind_list ']' 
				 | K_EVALUATE '(' expression ')' '[' typed_value_bind_list ']' 
		  		 | K_EXPLICIT '[' typed_value_bind_list ']' 
		  		 | K_EXPLICIT K_EVALUATE '(' expression ')' '[' typed_value_bind_list ']' 
		  		 | type '[' typed_value_bind_list ']' 
		  		 | type K_EVALUATE '(' expression ')' '[' typed_value_bind_list ']' 
		  		 | type K_EXPLICIT '[' typed_value_bind_list ']' 
		  		 | type K_EXPLICIT K_EVALUATE '(' expression ')' '[' typed_value_bind_list ']' 
		  		 /* Field-Bind */
		  		 | type '(' value_bind_list2 ')'
		  		 | type K_EVALUATE '(' expression ')' '(' value_bind_list2 ')'
		  		 | type K_EXPLICIT '(' value_bind_list2 ')'
		  		 | type K_EXPLICIT K_EVALUATE '(' expression ')' '(' value_bind_list2 ')'
		  		 ;

typed_value_bind: type value_bind
				| type '[' value_bind_list_ext ']'
				;

typed_value_bind_list: typed_value_bind
					 | typed_value_bind_list ',' typed_value_bind
					 ;

value_bind_list_ext: value_bind_ext
			   	   | value_bind_list_ext ',' value_bind_ext
			   	   ;

value_bind_list_simple: value_bind
			   		  | value_bind_list_simple ',' value_bind
			   		  ;

value_bind_list2: value_bind
			    | value_bind_list2 ':' value_bind
			    ;

value_bind: number
		  | identifier
		  | identifier T_BIND_RIGHT meta_identifier
		  | identifier T_BIND_LEFT qualified_identifier
		  | identifier T_BIND_BIDI qualified_identifier
		  | identifier T_BIND_LEFT '(' expression ')'
		  ;

value_bind_ext: value_bind
			  | type '[' value_bind_list_simple ']'
			  ;

binding_alias: T_BIND_LEFT expression
			 | T_BIND_LEFT repeatible_identifier
			 | T_BIND_BIDI repeatible_identifier
			 | T_BIND_LEFT identifier '.' repeatible_identifier
			 | T_BIND_BIDI identifier '.' repeatible_identifier
			 ;

multi_qualified_identifier: identifier '.' '[' identifier_list ']'
						  ;

identifier_list: identifier
			   | identifier_list ',' identifier
			   ;

base_operand: qualified_identifier
	 		| multi_qualified_identifier
	   		| meta_identifier
	   		| number
	   		| mem_expr
	   		| bit_comber
	   		| '(' expression ')'
	   		;

operand: base_operand
	   | base_operand bit_slicer
	   ;

bit_slicer: '[' expression ':' expression ']'
		  | '[' expression T_UPTO expression ']'
		  ;

bit_comber: '(' operand_comma_list ')'
		  ;

operand_comma_list: operand
                  | operand_comma_list ':' operand
                  ;

expression: binary_expr
		  ;

primary_expr: operand
			| cast_expr
			| call_expr
			| macro_expr
			| mem_expr
			| CC_expr
			| SU_expr
			; 

cast_expr: '[' type expression ']'
		 ;

call_expr: identifier '(' ')'
		 ;

macro_expr: macro_identifier '(' ')'
		  | macro_identifier '(' expression_list ')'
		  ;

expression_list: expression
			   | expression_list ',' expression
			   ;


mem_expr: K_MEM '[' expression ']'
		| type K_MEM '[' expression ']'
		;

SU_expr: K_AUGMENT_SIGNED '(' expression ')'
	   | K_AUGMENT_UNSIGNED '(' expression ')'
	   ;

CC_expr: K_AUGMENT_CC '(' expression ')'
	   | K_AUGMENT_CC '(' expression ',' CC_flag ')'
	   | K_AUGMENT_CC '(' expression ',' '[' CC_flag_list ']' ')'
	   ;

CC_flag: identifier
	   | '!' identifier
	   ;

CC_flag_list: CC_flag
			| CC_flag_list ',' CC_flag
			;


unary_expr: primary_expr
		  | '+' expression
		  | '-' expression
		  | '!' expression
		  | '~' expression
		  ;

binary_expr: unary_expr
		   /* Logical */
		   | binary_expr T_LOR expression
		   | binary_expr T_LAND expression
		   /* Relational */
		   | binary_expr T_EQ expression
		   | binary_expr T_NE expression
		   | binary_expr '<' expression
		   | binary_expr T_LE expression
		   | binary_expr T_GE expression
		   | binary_expr '>' expression
		   /* Additive */
		   | binary_expr '+' expression
		   | binary_expr '-' expression
		   | binary_expr '|' expression
		   | binary_expr T_ORCOM expression
		   | binary_expr '^' expression
		   | binary_expr T_XORCOM expression
		   /* Multiplicative */
		   | binary_expr '*' expression
		   | binary_expr '/' expression
		   | binary_expr '%' expression
		   | binary_expr T_SHL expression
		   | binary_expr T_SHLC expression
		   | binary_expr T_SHR expression
		   | binary_expr T_SHRC expression
		   | binary_expr T_ROL expression
		   | binary_expr T_ROLC expression
		   | binary_expr T_ROR expression
		   | binary_expr T_RORC expression
		   | binary_expr '&' expression
		   | binary_expr T_ANDCOM expression
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
           | operand T_SHLCE expression
           | operand T_SHRE expression
           | operand T_SHRCE expression
           | operand T_ROLE expression
           | operand T_ROLCE expression
           | operand T_RORE expression
           | operand T_RORCE expression
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
				  | toplevel_decl_list toplevel_decl
		 		  ;

root: arch_decl toplevel_decl_list
	;

