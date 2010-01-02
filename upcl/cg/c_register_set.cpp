
static std::string
make_c_compat_name(std::string const &name)
{
	size_t pos;
	pos = name.find('$');
	if (pos != std::string::npos) {
		std::string unnamed("__unnamed" + name);

		while ((pos = unnamed.find('$')) != std::string::npos)
			unnamed[pos] = '_';

		return unnamed;
	}
	return name;
}

static void
cg_print_typed_var(size_t data_size, size_t nbits, std::string const &name,
		std::string const &comment = std::string())
{
	std::stringstream ss;

	ss << "uint" << data_size << "_t " << make_c_compat_name(name);

	if (nbits > 64)
		ss << '[' << ((nbits+data_size-1) / data_size) << ']';
	else if (nbits < data_size)
		ss << " : " << nbits;

	ss << ';';
	if (!comment.empty())
		ss << " // " << comment;

	ss << std::endl;

	printf("%s", ss.str().c_str());
}

static void
cg_dump(c::register_def *def, bool maybe_unused = true)
{
	size_t nbits;
	if (def->is_sub_register())
		nbits = ((c::sub_register_def *)def)->get_master_register()->
		 	get_type()->get_bits();
	else
		nbits = def->get_type()->get_bits();

	if (nbits <= 8)
		nbits = 8;
	else if (nbits <= 16)
		nbits = 16;
	else if (nbits <= 32)
		nbits = 32;
	else if (nbits <= 64)
		nbits = 64;
	else
		nbits = 64; // ((nbits + 63) / 64) * 64;

	std::string comment;
	if (def->is_sub_register())
		comment = "sub";
	if (def->is_hardwired()) {
		if (!comment.empty()) comment += ", ";
		comment += "hardwired";
	} else if (def->get_bound_special_register() != c::NO_SPECIAL_REGISTER) {
		if (!comment.empty()) comment += ", ";
		comment += "bound to special register";
	} else if (def->get_bound_register() != 0) {
		if (!comment.empty()) comment += ", ";
		comment += "bound to register";
	}
	if (def->is_uow()) {
		if (!comment.empty()) comment += ", ";
		comment += "update-on-write";
	}

	c::sub_register_vector const &sub =
		def->get_sub_register_vector();

	if (!sub.empty()) {
		printf("union {\n");
		cg_print_typed_var(nbits, def->get_type()->get_bits(),
				def->get_name(), comment);
		printf("struct {\n");
		size_t total = 0;
		for(c::sub_register_vector::const_iterator i = sub.begin();
				i != sub.end(); i++) {
			total += (*i)->get_type()->get_bits();
			cg_dump(*i, false);
		}
		size_t unused_bits =
		 (((def->get_type()->get_bits()+nbits-1)/nbits)*nbits) - total;
		if (maybe_unused && unused_bits != 0) {
			std::stringstream ss;
			ss << "__unused_" << def->get_name() << '_' << total;
			cg_print_typed_var(nbits, unused_bits, ss.str());
		}
		printf("};\n");
		printf("} %s;", def->get_name().c_str());
		if (!comment.empty())
			printf(" // %s", comment.c_str());
		printf("\n");
	} else {
		size_t unused_bits = (maybe_unused && nbits <= 64 &&
				nbits > def->get_type()->get_bits() ?
				(nbits - def->get_type()->get_bits()) : 0);

		if (unused_bits != 0)
			printf("struct {\n");

		cg_print_typed_var(nbits, def->get_type()->get_bits(),
				def->get_name(), comment);

		if (unused_bits != 0) {
			cg_print_typed_var(nbits, unused_bits,
				"__unused_" + def->get_name());
			printf("};\n");
		}
	}
}

// this function returns if the register needs to 
// be composite or can be worked out with shifts
// and ands.
//
// this function will return true if any of the
// bitfield is greater than 1 and lesser than 8.
inline bool
needs_bitfield_split(c::register_def *reg)
{
	return false;
}

//
// hardwired: return expr;
// special evaluation: return expr;
// complex indexing: index = expr; return expr2;
// normal: return reference to register;
//

void
cg_generate_gen_get_reg_set()
{
	//printf("static cpu_value_t *arch_%s_gen_get_reg(arch_%s_t *arch, size_t reg_name)\n");
	printf("{\n");
	//printf("\tif (index >= %zu) return NULL;");
	printf("}\n");
}
