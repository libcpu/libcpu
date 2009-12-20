void
i8086_reg()
{
	// R
	register_def *ax = new register_def("ax", c::type::get_integer_type(16));
	register_def *ah = new sub_register_def(ax, "al",
			c::type::get_integer_type(8), 0, 8);
	register_def *ah = new sub_register_def(ax, "ah",
			c::type::get_integer_type(8), 8, 8);

	register_def *bx = new register_def("bx", c::type::get_integer_type(16));
	register_def *bh = new sub_register_def(ax, "bl",
			c::type::get_integer_type(8), 0, 8);
	register_def *bh = new sub_register_def(ax, "bh",
			c::type::get_integer_type(8), 8, 8);

	register_def *cx = new register_def("cx", c::type::get_integer_type(16));
	register_def *ch = new sub_register_def(ax, "cl",
			c::type::get_integer_type(8), 0, 8);
	register_def *ch = new sub_register_def(ax, "ch",
			c::type::get_integer_type(8), 8, 8);

	register_def *dx = new register_def("dx", c::type::get_integer_type(16));
	register_def *dh = new sub_register_def(ax, "dl",
			c::type::get_integer_type(8), 0, 8);
	register_def *dh = new sub_register_def(ax, "dh",
			c::type::get_integer_type(8), 8, 8);

	register_def *sp = new register_def("sp", c::type::get_integer_type(16));
	register_def *bp = new register_def("bp", c::type::get_integer_type(16));
	register_def *si = new register_def("si", c::type::get_integer_type(16));
	register_def *si = new register_def("di", c::type::get_integer_type(16));

	// SEG
	register_def *cs = new register_def("cs", c::type::get_integer_type(16));
	register_def *ds = new register_def("ds", c::type::get_integer_type(16));
	register_def *ss = new register_def("ss", c::type::get_integer_type(16));
	register_def *es = new register_def("es", c::type::get_integer_type(16));

	// S
	register_def *ip = new register_def("ip", c::type::get_integer_type(16));
	register_def *pc = new register_def("pc", c::type::get_integer_type(24),
			SPECIAL_REGISTER_PC);
	register_def *pc_off = new bound_sub_register_def(pc, "off", ip,
			true, 0, 16);
	register_def *pc_seg = new bound_sub_register_def(pc, "seg", cs,
			false, 16, 16);

	register_def *flags = new register_def("flags",
			c::type::get_integer_type(16), SPECIAL_REGISTER_PSR);
	register_def *flags_C = new bound_sub_register_def(flags, "C",
			c::type::get_integer_type(1), SPECIAL_REGISTER_C, 0, 1);
	register_def *flags_P = new bound_sub_register_def(flags, "P",
			c::type::get_integer_type(1), SPECIAL_REGISTER_P, 3, 1);
	register_def *flags_A = new bound_sub_register_def(flags, "A",
			c::type::get_integer_type(1), flags_C, false, 5, 1);
	register_def *flags_Z = new bound_sub_register_def(flags, "Z",
			c::type::get_integer_type(1), SPECIAL_REGISTER_Z, 7, 1);
	register_def *flags_S = new bound_sub_register_def(flags, "S",
			c::type::get_integer_type(1), SPECIAL_REGISTER_N, 8, 1);
	register_def *flags_T = new sub_register_def(flags, "T",
			c::type::get_integer_type(1), 9, 1);
	register_def *flags_I = new sub_register_def(flags, "I",
			c::type::get_integer_type(1), 10, 1);
	register_def *flags_D = new sub_register_def(flags, "D",
			c::type::get_integer_type(1), 11, 1);
	register_def *flags_O = new bound_sub_register_def(flags, "V",
			c::type::get_integer_type(1), SPECIAL_REGISTER_V, 12, 1);

}

void
sparc_def()
{
	//  globals
	register_def *g0 = new hardwired_register_def("g0", 
			new c::type::get_integer_type(32), 0);

	register_def *g1 = new register_def("g1",
			new c::type::get_integer_type(32));
	register_def *g2 = new register_def("g2",
			new c::type::get_integer_type(32));
	register_def *g3 = new register_def("g3",
			new c::type::get_integer_type(32));
	register_def *g4 = new register_def("g4",
			new c::type::get_integer_type(32));
	register_def *g5 = new register_def("g5",
			new c::type::get_integer_type(32));
	register_def *g6 = new register_def("g6",
			new c::type::get_integer_type(32));
	register_def *g7 = new register_def("g7",
			new c::type::get_integer_type(32));

	register_def *cwp = new register_def("cwp",
			new c::type::get_integer_type(32));
	register_def *cwp_0 = new sub_register_def(cwp, "cwp",
			c::type::get_integer_type(5), 0, 5);

	register_def *w[384];

	register_def *i0 = new dynamic_register_def("i0",
		new register_indexer(w, expression::Sub(expression::Add(
				expression::fromInteger(0), expression::Shl(cwp_0,
				expression::fromInteger(4))), expression::fromInteger(8))));

	register_def *r0 = new sub_register_def(g0, "r0");
	register_def *r8 = new sub_register_def(l0, "r8");
	register_def *r16 = new sub_register_def(i0, "r16");
}

void
m88k_reg()
{
	register_def *d0 = new register_def("d0", c::type::get_integer_type(64));
	register_def *d0_0 = new sub_register_def(d0, "d0_0",
			c::type::get_integer_type(32), r0, true, 0, 32);
	register_def *d0_1 = new sub_register_def(d0, "d0_1",
			c::type::get_integer_type(32), r1, true, 32, 32);
		
}
