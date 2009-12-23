
static expression *
expand_bit_slicing(expression *expr)
{
	expression *sub;

	if (expr->get_expression_operation() == expression::BIT_SLICE) {
		bit_slice_expression *bse = (bit_slice_expression *)expr;
		return expand_bit_slicing(bse->expand());
	}

	if (expr->is_constant_value())
		return expr;

	for (size_t n = 0; (sub = expr->sub_expr(n)) != 0; n++) {

		// for assignment operators, don't evaluate RHS.
		if (n == 0 && expr->get_expression_operation() == expression::ASSIGN)
			continue;

		expression *expanded = expand_bit_slicing(sub);
		if (!expanded->is_equal(sub)) {
			expr->replace_sub_expr(n--, expanded);
		}
	}

	return expr;
}

