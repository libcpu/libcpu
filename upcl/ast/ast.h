#ifndef __upcl_ast_ast_h
#define __upcl_ast_ast_h

#include "types.h"

namespace upcl { namespace ast {

struct location_info {

	int first_row;
	int last_row;
	int first_column;
	int last_column;

	location_info()
	{ first_row = last_row = first_column = last_column = 0; }

#ifdef YYLVAL
	location_info(YYLVAL const *yylval)
#endif
};

class token {

public:
	enum token_type {

		LIST,
		TYPE,
		IDENTIFIER,
		STRING,
		NUMBER,
		RANGE,
		EXPRESSION,
		QUALIFIED_IDENTIFIER,
		TAGGED_VALUE,

		ARCHITECTURE,
		REGISTER_FILE,
		REGISTER_GROUP,
		REGISTER_DECLARATION,
		REGISTER_TYPE_NAME,
		REGISTER_BINDER,
		REGISTER_SPLITTER1, // []
		REGISTER_SPLITTER2, // ()
		REGISTER_ALIASING,

		BOUND_VALUE,
		TYPED_BOUND_VALUE,
		ALIAS_VALUE,

		INSTRUCTION,
		JUMP,

		STATEMENT,
		DECODER_OPERANDS
	};

private:
	token_type m_token_type;
	location_info m_token_location;

protected:
	token(token_type const &type,
		location_info const &location = location_info ()) 
	 	: m_token_type(type), m_token_location(location)
	{ }

public:
	inline location_info const &get_token_location() const
	{ return m_token_location; }
	inline token_type get_token_type() const
	{ return m_token_type; }
};

template<typename T>
class list : public token {
	std::vector<T *> m_vector;
public:
	list(T *object = 0) : token(LIST)
	{ push(object); }
	
	inline void push(T *object)
	{ if (object) m_vector.push_back(object); }
	
	inline size_t size() const
	{ return m_vector.size(); }
	
	inline T *operator[](size_t index) const
	{ return m_vector[index]; }
};

typedef list<token> token_list;

template<token::token_type Ty>
class literal : public token {
protected:
	std::string m_value;

public:
	literal(std::string const &name,
		location_info const &location = location_info())
		: token(Ty, location), m_value(name)
	{ }

	inline std::string const &get_value() const
	{ return m_value; }
};

class identifier : public literal<token::IDENTIFIER> {
	typedef literal<token::IDENTIFIER> __super_class;
public:
	identifier(std::string const &name,
		location_info const &location = location_info())
	 : __super_class(name, location)
	{ }

	inline bool is_macro() const {
		return (!m_value.empty() && *m_value.begin() == '@');
	}

	inline bool is_meta() const {
		return (!m_value.empty() && *m_value.begin() == '%');
	}

	virtual bool is_repeat() const {
		return (!m_value.empty() && *m_value.rbegin() == '?');
	}

	inline bool is_normal() const {
		return (!is_macro() && !is_meta() && !is_repeat());
	}
    
protected:
	virtual ~identifier() {}
};

class qualified_identifier : public token {
	identifier *m_id1;
	token_list *m_ids;
public:
	qualified_identifier(identifier *id1, identifier *id2 = 0)
		: token(QUALIFIED_IDENTIFIER), m_id1(id1),
		m_ids(id2 ? new token_list(id2) : 0)
	{ }
	qualified_identifier(identifier *id1, token_list *ids)
		: token(QUALIFIED_IDENTIFIER), m_id1(id1), m_ids(ids)
	{ }

	inline identifier const *get_base_identifier() const { return m_id1; }
	inline token_list const *get_identifier_list() const { return m_ids; }
};

class expression;

class repeat_identifier : public identifier {
	expression *m_expr;
public:
	repeat_identifier(std::string const &name,
		location_info const &location = location_info())
	 : identifier(name, location), m_expr(0)
	{ }
	repeat_identifier(std::string const &name,
		expression *expr,
		location_info const &location = location_info())
	 : identifier(name, location), m_expr(expr)
	{ }

	virtual bool is_repeat() const { return true; }
	inline expression const *get_expression() const { return m_expr; }
};

typedef list<identifier> identifier_list;

typedef literal<token::STRING> string;
typedef literal<token::NUMBER> number;
typedef literal<token::TYPE> type;

class tagged_value : public token {
	int m_tag;
	int m_value_type;
	union {
		token *t;
		int i;
	} m_value;

	enum {
		TOKEN,
		INT
	};

public:
	tagged_value(int tag, int value)
		: token(TAGGED_VALUE), m_tag(tag), m_value_type(INT)
	{ m_value.i = value; }

	tagged_value(int tag, token *t)
		: token(TAGGED_VALUE), m_tag(tag), m_value_type(TOKEN)
	{ m_value.t = t; }

	inline bool is_token() const { return (m_value_type == TOKEN); }
	inline token const *get_token() const { return m_value.t; }
	inline int get_value() const { return m_value.i; }
	inline int get_tag() const { return m_tag; }
};

class architecture : public token {
	string *m_name;
	token_list *m_def;

public:
	enum keywords {

		NAME,
		ENDIAN,
		DEFAULT_ENDIAN,
		BYTE_SIZE,
		WORD_SIZE,
		FLOAT_SIZE,
		ADDRESS_SIZE,
		PSR_SIZE,
		MIN_PAGE_SIZE,
		MAX_PAGE_SIZE,
		DEFAULT_PAGE_SIZE,
		PAGE_SIZE

	};

	enum {
    ENDIAN_UNDEF,
		ENDIAN_LITTLE,
		ENDIAN_BIG,
		ENDIAN_BOTH
	};

public:
	architecture(string *name, token_list *def)
		: token(ARCHITECTURE), m_name(name), m_def(def)
	{ }

	inline string const *get_name() const { return m_name; }
	inline token_list const *get_def() const { return m_def; }
};

class expression : public token {
public:
	enum expression_type {

		LITERAL,

		UNARY,
		BINARY,

		MEMORY,
		BIT_SLICE,
		BIT_COMBINE,
		CAST,
		CALL,
		CC
		
	};

private:
	expression_type m_expr_type;

protected:
	expression(expression_type const &type)
		: token(EXPRESSION), m_expr_type(type)
	{ }

public:
	inline expression_type get_expression_type() const
	{ return m_expr_type; }
};

class literal_expression : public expression {
	token *m_literal;

public:
	literal_expression(token *literal)
		: expression(LITERAL), m_literal(literal)
	{ }

public:
	inline token const *get_literal() const { return m_literal; }
};

class binary_expression : public expression {
public:
	enum binary_expression_operator {
		ADD,
		SUB,
		MUL,
		DIV,
		MOD,

		SHL,
		SHR,
		ROL,
		ROR,

		OR,
		ORCOM,
		AND,
		ANDCOM,
		XOR,
		XORCOM,

		LOR,
		LAND,

		EQ,
		NE,
		LT,
		LE,
		GT,
		GE,

		IS
	};

private:
	binary_expression_operator m_expr_oper;
	expression *m_expr1;
	expression *m_expr2;

public:
	binary_expression(binary_expression_operator const &oper,
		expression *expr1, expression *expr2)
		: expression(BINARY), m_expr_oper(oper), m_expr1(expr1),
		m_expr2(expr2)
	{ }

	inline binary_expression_operator get_expression_operator() const
	{ return m_expr_oper; }
	inline expression *get_expression1() const { return m_expr1; }
	inline expression *get_expression2() const { return m_expr2; }
};

class unary_expression : public expression {
public:
	enum unary_expression_operator {
		SUB,

		NOT,
		COM,
		NEG,

		SIGNED,
		UNSIGNED
	};

private:
	unary_expression_operator m_expr_oper;
	expression *m_expr;

public:
	unary_expression(unary_expression_operator const &oper, expression *expr)
		: expression(UNARY), m_expr_oper(oper), m_expr(expr)
	{ }

	inline unary_expression_operator get_expression_operator() const
	{ return m_expr_oper; }
	inline expression *get_expression() const { return m_expr; }
};

class range : public token {
	bool m_distance;

	expression *m_a;
	expression *m_b;

public:
	range(bool distance, expression *a, expression *b)
		: token(RANGE), m_distance(distance), m_a(a), m_b(b)
	{ }

	inline bool is_distance() const { return m_distance; }
	inline expression const *get_a() const { return m_a; }
	inline expression const *get_b() const { return m_b; }
};

class bit_slice_expression : public expression {
	expression *m_expr;
	range *m_range;

public:
	bit_slice_expression(expression *expr, range *r)
		: expression(BIT_SLICE), m_expr(expr), m_range(r)
	{ }

	inline expression const *get_expression() const { return m_expr; }
	inline range const *get_range() const { return m_range; }
};

class bit_combine_expression : public expression {
	token_list *m_exprs;

public:
	bit_combine_expression(token_list *exprs)
		: expression(BIT_COMBINE), m_exprs(exprs)
	{ }

	inline token_list const *get_expressions() const { return m_exprs; }
};

class cast_expression : public expression {
	expression *m_expr;
	type *m_type;

public:
	cast_expression(expression *expr, type *t)
		: expression(CAST), m_expr(expr), m_type(t)
	{ }

	inline expression const *get_expression() const { return m_expr; }
	inline type const *get_type() const { return m_type; }
};

class memory_expression : public expression {
	expression *m_expr;
	type *m_type;

public:
	memory_expression(expression *expr, type *t = 0)
		: expression(MEMORY), m_expr(expr), m_type(t)
	{ }

	inline expression const *get_expression() const { return m_expr; }
	inline type const *get_type() const { return m_type; }
};

class call_expression : public expression {
	identifier *m_name;
	token_list *m_exprs;

public:
	call_expression(identifier *name, token_list *exprs = 0)
		: expression(CALL), m_name(name), m_exprs(exprs)
	{ }

	inline identifier const *get_name() const { return m_name; }
	inline token_list const *get_expression_list() const { return m_exprs; }
};

class CC_expression : public expression {
	expression *m_expr;
	token_list *m_flags;

public:
	CC_expression(expression *expr, token_list *flags = 0)
		: expression(CC), m_expr(expr), m_flags(flags)
	{ }
	CC_expression(expression *expr, expression *flag)
		: expression(CC), m_expr(expr), m_flags(new token_list(flag))
	{ }

	inline expression const *get_expression() const { return m_expr; }
	inline token_list const *get_flags() const { return m_flags; }
};

class register_file : public token {
	token_list *m_groups;

public:
	register_file(token_list *groups)
		: token(REGISTER_FILE), m_groups(groups)
	{ }

	inline token_list const *get_groups() const { return m_groups; }
};

class register_group : public token {
	identifier *m_name;
	token_list *m_regs;

public:
	register_group(identifier *name, token_list *regs)
		: token(REGISTER_GROUP), m_name(name), m_regs(regs)
	{ }

	register_group(identifier *name, token *reg)
		: token(REGISTER_GROUP), m_name(name),
		m_regs(new token_list(reg))
	{ }

	inline identifier const *get_name() const { return m_name; }
	inline token_list const *get_registers() const { return m_regs; }
};

class register_type_name;

class register_declaration : public token {
	register_type_name *m_def;
	expression *m_repeat;
	token *m_binding;

public:
	register_declaration(register_type_name *def)
	 	: token(REGISTER_DECLARATION), m_def(def), m_repeat(0), m_binding(0)
	{ }

	register_declaration(register_type_name *def, token *binding)
		: token(REGISTER_DECLARATION), m_def(def), m_repeat(0),
		m_binding(binding)
	{ }

	register_declaration(expression *repeat, register_type_name *def)
		: token(REGISTER_DECLARATION), m_def(def), m_repeat(repeat),
		m_binding(0)
	{ }

	register_declaration(expression *repeat, register_type_name *def,
		token *binding)
		: token(REGISTER_DECLARATION), m_def(def), m_repeat(repeat),
		m_binding(binding)
	{ }

	inline register_type_name const *get_def() const { return m_def; }
	inline expression const *get_repeat() const { return m_repeat; }
	inline token const *get_binding() const { return m_binding; }
};

class register_type_name : public token {
	type *m_type;
	identifier *m_name;

public:
	register_type_name(type *t, identifier *name)
		: token(REGISTER_TYPE_NAME), m_type(t), m_name(name)
	{ }

	inline identifier const *get_name() const { return m_name; }
	inline type const *get_type() const { return m_type; }

};

class register_binder : public token {
	identifier *m_special_reg;
	token *m_splitter;

public:
	register_binder(identifier *special_reg)
		: token(REGISTER_BINDER), m_special_reg(special_reg), m_splitter(0)
	{ }
	register_binder(identifier *special_reg, token *splitter)
		: token(REGISTER_BINDER), m_special_reg(special_reg),
		m_splitter(splitter)
	{ }

	inline identifier const *get_register() const { return m_special_reg; }
	inline token const *get_splitter() const { return m_splitter; }
};

class register_splitter : public token {
	type *m_type;
	token_list *m_values;
	expression *m_eval;
	bool m_explicit;

protected:
	register_splitter(token_type const &tktype, type *type, token_list *values,
			expression *eval, bool explic)
		: token(tktype), m_type(type), m_values(values), m_eval(eval),
		m_explicit(explic)
	{ }

public:
	inline bool is_explicit() const { return m_explicit; }
	inline type const *get_type() const { return m_type; }
	inline token_list const *get_values() const { return m_values; }
	inline expression const *get_evaluate() const { return m_eval; }
};

class register_splitter1 : public register_splitter {
public:
	register_splitter1(type *type, token_list *values, expression *eval,
		bool explic)
		: register_splitter(REGISTER_SPLITTER1, type, values, eval, explic)
	{ }
};

class register_splitter2 : public register_splitter {
public:
	register_splitter2(type *type, token_list *values, expression *eval,
		bool explic)
		: register_splitter(REGISTER_SPLITTER2, type, values, eval, explic)
	{ }
};

class bound_value : public token {
	identifier *m_name;
	token *m_value;
	bool m_bidi;

public:
	bound_value(expression *expr)
		: token(BOUND_VALUE), m_name(0), m_value(expr), m_bidi(false)
	{
	}

	bound_value(identifier *name)
		: token(BOUND_VALUE), m_name(name), m_value(0), m_bidi(false)
	{ }

	bound_value(identifier *name, expression *expr)
		: token(BOUND_VALUE), m_name(name), m_value(expr), m_bidi(false)
	{ }

	bound_value(identifier *name, identifier *alias, bool bidi)
		: token(BOUND_VALUE), m_name(name), m_value(alias), m_bidi(bidi)
	{ }

	bound_value(identifier *name, qualified_identifier *alias, bool bidi)
		: token(BOUND_VALUE), m_name(name), m_value(alias), m_bidi(bidi)
	{ }

	inline bool is_bidi() const { return m_bidi; }
	inline identifier const *get_name() const { return m_name; }
	inline token const *get_binding() const { return m_value; }
};

class typed_bound_value : public token {
	type *m_type;
	token_list *m_values;

public:
	typed_bound_value(type *t, bound_value *value)
		: token(TYPED_BOUND_VALUE), m_type(t),
		m_values(new token_list(value))
	{ }

	typed_bound_value(type *t, token_list *values)
		: token(TYPED_BOUND_VALUE), m_type(t), m_values(values)
	{ }

	inline type const *get_type() const { return m_type; }
	inline token_list const *get_values() const { return m_values; }
};

class alias_value : public token {
	bool m_bidi;
	token *m_alias;

public:
	alias_value(token *alias, bool bidi = false)
		: token(ALIAS_VALUE), m_bidi(bidi), m_alias(alias)
	{ }

	inline bool is_bidi() const { return m_bidi; }
	inline token const *get_alias() const { return m_alias; }
};

class statement : public token {
public:
	enum statement_type {
		STATEMENT_LIST,
		EXPRESSION,
		ASSIGNMENT,
		FOR,
		IF
	};

private:
	statement_type m_statement_type;
	type *m_lhs_type;

protected:
	statement(statement_type type)
		: token(STATEMENT), m_statement_type(type)
	{ }

public:
	inline void set_lhs_type(type *type) { m_lhs_type = type; }
	inline type const *get_lhs_type() const { return m_lhs_type; }
	inline statement_type get_statement_type() const { return m_statement_type; }
};

class statement_list_statement : public statement {
	token_list *m_list;
public:
	statement_list_statement(token_list *list)
		: statement(STATEMENT_LIST), m_list(list)
	{ }

public:
	inline token_list const *get_statements() const { return m_list; }
};

class expression_statement : public statement {
	expression *m_expression;
public:
	expression_statement(expression *expr)
		: statement(EXPRESSION), m_expression(expr)
	{ }

public:
	inline expression const *get_expression() const { return m_expression; }
};

class assignment_statement : public statement {
public:
	enum assignment_type {
		EQ, ADDE, SUBE, MULE, DIVE, REME, SHLE, SHRE,
		ROLE, RORE, ANDE, ANDCOME, ORE, ORCOME, XORE,
		XORCOME
	};

private:
	assignment_type m_assignment_type;
	expression *m_operand;
	expression *m_value;

public:
	assignment_statement(assignment_type type, expression *operand,
			expression *value)
		: statement(ASSIGNMENT), m_assignment_type(type),
		m_operand(operand), m_value(value)
	{ }

	inline assignment_type get_assignment_type() const { return m_assignment_type; }
	inline expression const *get_operand() const { return m_operand; }
	inline expression const *get_value() const { return m_value; }
};

class if_statement : public statement {
	expression *m_condition;
	statement *m_true;
	statement *m_false;

public:
	if_statement(expression *cond, statement *tstmt, statement *fstmt = 0)
		: statement(IF), m_condition(cond), m_true(tstmt),
		m_false(fstmt)
	{ }

	inline expression const *get_condition() const { return m_condition; }
	inline statement const *get_true() const { return m_true; }
	inline statement const *get_false() const { return m_false; }
};

class for_statement : public statement {
	token_list *m_init;
	expression *m_condition;
	token_list *m_post;

public:
	for_statement(token_list *init = 0, expression *cond = 0, token_list *post = 0)
		: statement(FOR), m_init(init), m_condition(cond), m_post(post)
	{ }

public:
	inline token_list const *get_init() const { return m_init; }
	inline expression const *get_condition() const { return m_condition; }
	inline token_list const *get_post() const { return m_post; }
};

class instruction : public token {
public:
	enum instruction_type {
		NORMAL,
		JUMP,
		MACRO
	};

protected:
	instruction_type m_instruction_type;
	identifier *m_name;
	token_list *m_body;

protected:
	instruction(instruction_type type, identifier *name)
		: token(INSTRUCTION), m_instruction_type(type),
		m_name(name), m_body(0)
	{ }
	instruction(instruction_type type, identifier *name, statement *statement)
	 	: token(INSTRUCTION), m_instruction_type(type),
		m_name(name), m_body(new token_list(statement))
	{ }
	instruction(instruction_type type, identifier *name, token_list *body)
		: token(INSTRUCTION), m_instruction_type(type),
		m_name(name), m_body(body)
	{ }

public:
	instruction(identifier *name)
		: token(INSTRUCTION), m_instruction_type(NORMAL),
		m_name(name), m_body(0)
	{ }
	instruction(identifier *name, statement *statement)
	 	: token(INSTRUCTION), m_instruction_type(NORMAL),
		m_name(name), m_body(new token_list(statement))
	{ }
	instruction(identifier *name, token_list *body)
		: token(INSTRUCTION), m_instruction_type(NORMAL),
		m_name(name), m_body(body)
	{ }

	inline instruction_type get_instruction_type() const { return m_instruction_type; }
	inline identifier const *get_name() const { return m_name; }
	inline token_list const *get_body() const { return m_body; }
};

class jump : public token {
	identifier *m_type;
	expression *m_delay;
	token_list *m_pre;
	expression *m_condition;
	token_list *m_action;


public:
	jump(identifier *type, expression *delay, token_list *pre,
			expression *condition, token_list *action)
		: token(JUMP), m_type(type), m_delay(delay), m_pre(pre),
		m_condition(condition), m_action(action)
	{ }

public:
	inline identifier const *get_type() const { return m_type; }
	inline expression const *get_delay() const { return m_delay; }
	inline token_list const *get_pre() const { return m_pre; }
	inline expression const *get_condition() const { return m_condition; }
	inline token_list const *get_action() const { return m_action; }
};

class decoder_operands : public token {
	token_list *m_operands;

public:
	decoder_operands(token_list *operands)
		: token(DECODER_OPERANDS), m_operands(operands)
	{ }

public:
	inline token_list const *get_operands() const { return m_operands; }
};

class jump_instruction : public instruction {
private:
	jump *m_jump;

public:
	jump_instruction(identifier *name, jump *info)
		: instruction(JUMP, name), m_jump(info)
	{ }

public:
	inline jump const *get_info() const { return m_jump; }
};

class macro : public instruction {
private:
	token_list *m_arguments;

public:
	macro(identifier *name, statement *stmt)
		: instruction(MACRO, name, stmt), m_arguments(0)
	{ }
	macro(identifier *name, token_list *args, statement *stmt)
		: instruction(MACRO, name, stmt), m_arguments(args)
	{ }
	macro(identifier *name, token_list *stmts)
		: instruction(MACRO, name, stmts), m_arguments(0)
	{ }
	macro(identifier *name, token_list *args, token_list *stmts)
		: instruction(MACRO, name, stmts), m_arguments(args)
	{ }

public:
	inline token_list const *get_arguments() const { return m_arguments; }
};

} }

#endif  // !__upcl_ast_ast_h
