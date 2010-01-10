#include "c/register_def.h"
#include "c/sub_register_def.h"

//
// Code generator
//


//
struct register_def {
	unsigned    type;
	size_t      bits_size;
	size_t      aligned_size;
	size_t      byte_offset;
	char const *name;
};

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
cg_print_typed_var(std::ostream &o, size_t indent, size_t data_size,
		size_t nbits, std::string const &name,
		std::string const &comment = std::string())
{
	o << std::string(indent, '\t') 
	  << "uint" << data_size << "_t " << make_c_compat_name(name);

	if (nbits > 64)
		o << '[' << ((nbits+data_size-1) / data_size) << ']';
	else if (nbits < data_size)
		o << " : " << nbits;

	o << ';';
	if (!comment.empty())
		o << " // " << comment;

	o << std::endl;
}

static size_t
get_aligned_size_for_type(c::type const *type, bool max64 = true)
{
	size_t nbits = type->get_bits();

	if (nbits <= 8)
		nbits = 8;
	else if (nbits <= 16)
		nbits = 16;
	else if (nbits <= 32)
		nbits = 32;
	else if (nbits <= 64)
		nbits = 64;
	else if (max64)
		nbits = 64;
	else
		nbits = ((nbits + 63) / 64) * 64;

	return nbits;
}

static void
cg_dump_reg_def(std::ostream &o, size_t indent, c::register_def *def,
		bool maybe_unused = true)
{
	size_t nbits;
	if (def->is_sub_register())
		nbits = get_aligned_size_for_type(((c::sub_register_def *)def)->
				get_master_register()->get_type());
	else
		nbits = get_aligned_size_for_type(def->get_type());

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
		o << std::string(indent, '\t');
		o << "union {" << std::endl;

		indent++;

		cg_print_typed_var(o, indent, nbits, def->get_type()->get_bits(),
				def->get_name(), comment);

		o << std::string(indent, '\t');
		o << "struct {" << std::endl;

		indent++;
		size_t total = 0;
		for(c::sub_register_vector::const_iterator i = sub.begin();
				i != sub.end(); i++) {
			total += (*i)->get_type()->get_bits();
			cg_dump_reg_def(o, indent, *i, false);
		}

		size_t unused_bits =
		 (((def->get_type()->get_bits()+nbits-1)/nbits)*nbits) - total;

		if (maybe_unused && unused_bits != 0) {
			std::stringstream ss;
			ss << "__unused_" << def->get_name() << '_' << total;
			cg_print_typed_var(o, indent, nbits, unused_bits, ss.str());
		}

		indent--;

		o << std::string(indent, '\t');
		o << "};" << std::endl;

		indent--;

		o << std::string(indent, '\t');
		o << "} " << def->get_name() << ';';

		if (!comment.empty())
			o << " // " << comment;
		o << std::endl;
	} else {
		size_t unused_bits = (maybe_unused && nbits <= 64 &&
				nbits > def->get_type()->get_bits() ?
				(nbits - def->get_type()->get_bits()) : 0);

		if (unused_bits != 0) {
			o << std::string(indent, '\t');
			o << "struct {" << std::endl;
			indent++;
		}

		cg_print_typed_var(o, indent, nbits, def->get_type()->get_bits(),
				def->get_name(), comment);

		if (unused_bits != 0) {
			cg_print_typed_var(o, indent, nbits, unused_bits,
				"__unused_" + def->get_name());
			indent--;
			o << std::string(indent, '\t');
			o << "};" << std::endl;
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


static void
cg_dump_psr_bitfield(std::ostream &o, c::sub_register_def const *bf);

static void
cg_dump_psr_bitfield(std::ostream &o, c::sub_register_vector const &bitfields)
{
	for (c::sub_register_vector::const_iterator i = bitfields.begin();
			i != bitfields.end(); i++)
		cg_dump_psr_bitfield(o, (*i));
}

static void
cg_dump_psr_bitfield(std::ostream &o, c::sub_register_def const *bf)
{
	c::sub_register_vector const &sub_bitfields =
		bf->get_sub_register_vector();
	if (!sub_bitfields.empty()) {
		cg_dump_psr_bitfield(o, sub_bitfields);
		return;
	}

	uint64_t shift = 0;
	char special = 0;
	if (bf->get_first_bit()->evaluate(shift)) {
		switch (bf->get_bound_special_register()) {
			case c::SPECIAL_REGISTER_C: special = 'C'; break;
			case c::SPECIAL_REGISTER_N: special = 'N'; break;
			case c::SPECIAL_REGISTER_P: special = 'P'; break;
			case c::SPECIAL_REGISTER_V: special = 'V'; break;
			case c::SPECIAL_REGISTER_Z: special = 'Z'; break;
			default: break;
		}
	}

	o << '\t' << "{ " << shift << ", ";
	if (special != 0)
		o << '\'' << special << '\'';
	else
		o << '0';

	std::string name(bf->get_name());
	size_t dp = name.rfind('$');
	if (dp != std::string::npos)
		name = name.substr(dp);

	o << ", \"" << name << "\" }" << std::endl;
}

void
cg_dump_psr_flags(std::ostream &o, std::string const &arch_name, 
		std::string const &reg_name, c::sub_register_vector const &bitfields)
{
	o << "static flags_layout_t arch_" << arch_name << "_flags_layout[] = {" << std::endl;
	for (c::sub_register_vector::const_iterator i = bitfields.begin();
			i != bitfields.end(); i++)
		cg_dump_psr_bitfield(o, (*i));
	o << '\t' << "{ -1, 0, NULL }" << std::endl;
	o << "};" << std::endl;
}

void
cg_dump_reg_file(std::ostream &o, std::string const &arch_name,
		c::register_def_vector const &regs)
{
	// dump the register file struct
	o << "PACKED(struct reg_" << arch_name << "_t {" << std::endl;
	for (c::register_def_vector::const_iterator i = regs.begin ();
			i != regs.end(); i++)
		cg_dump_reg_def(o, 1, *i);
	o << "});" << std::endl;
}

void
cg_dump_reg_file_layout(std::ostream &o, std::string const &arch_name,
		c::register_def_vector const &regs)
{	
	o << "static register_layout_t const arch_" << arch_name << "_register_layout[] = {" << std::endl;
	size_t offset = 0;
	for (c::register_def_vector::const_iterator i = regs.begin ();
			i != regs.end(); i++) {

		size_t size = (*i)->get_type()->get_bits();
		size_t aligned_size = get_aligned_size_for_type((*i)->get_type(), false);

		if ((*i)->is_hardwired())
			goto skip;

		if (i != regs.begin())
			o << ',' << std::endl;

		o << '\t' << "{ ";
		switch ((*i)->get_type()->get_type_id()) {
			case c::type::INTEGER:
				o << "REG_TYPE_INT";
				break;
			case c::type::FLOAT:
				o << "REG_TYPE_FLOAT";
				break;
			case c::type::VECTOR:
			case c::type::VECTOR_INTEGER:
			case c::type::VECTOR_FLOAT:
				o << "REG_TYPE_VECTOR";
				break;
			default:
				o << "REG_TYPE_UNKNOWN";
				break;
		}
		o << ", ";
		o << size << ", ";
		o << aligned_size << ", ";
		o << offset << ", ";
		// flags
		switch ((*i)->get_bound_special_register()) {
			case c::SPECIAL_REGISTER_PC:
				o << "REG_FLAG_PC";
				break;
			case c::SPECIAL_REGISTER_NPC:
				o << "REG_FLAG_NPC";
				break;
			case c::SPECIAL_REGISTER_PSR:
				o << "REG_FLAG_PSR";
				break;
			default:
				o << '0';
				break;
		}
		o << ", ";
		o << '"' << (*i)->get_name() << '"';
		o << " }";

skip:
		offset += aligned_size >> 3;
	}
	o << std::endl << "};" << std::endl;
}


//
// hardwired: return expr;
// special evaluation: return expr;
// complex indexing: index = expr; return expr2;
// normal: return reference to register;
//

void
cg_generate_arch_init(std::ostream &o, std::string const &arch_name,
		std::string const &full_name, uint64_t const *tags)
{
	o << "static void" << std::endl;
	o << "arch_" << arch_name << "_init(arch_info_t *info, arch_regfile_t *rf)" << std::endl;
	o << '{' << std::endl;
	
	o << '\t' << "reg_" << arch_name << "_t *register_file = NULL;" << std::endl;
	o << std::endl;

	o << '\t' << "// Architecture definition" << std::endl;

	// name
	o << '\t' << "info->name = " << '"' << arch_name << '"' << ';' << std::endl;

	// fullname
	if (!full_name.empty())
		o << '\t' << "info->full_name = " << '"' << full_name << '"';
	else
		o << '\t' << "info->full_name = " << '"' << arch_name << '"';
	o << ';' << std::endl;

	// endian
	switch (tags[ast::architecture::ENDIAN]) {
		case ast::architecture::ENDIAN_BIG:
			o << '\t' << "info->common_flags = CPU_FLAG_ENDIAN_BIG";
			break;
		case ast::architecture::ENDIAN_LITTLE:
			o << '\t' << "info->common_flags = CPU_FLAG_ENDIAN_LITTLE";
			break;
		case ast::architecture::ENDIAN_BOTH:
			o << '\t' << "info->common_flags &= CPU_FLAG_ENDIAN_MASK";
			break;
		default:
			break;
	}
	o << ';' << std::endl;

	// minimum allocation unit
	o << '\t' << "info->byte_size = " << tags[ast::architecture::BYTE_SIZE] << ';' << std::endl;
	// maximum allocation unit
	o << '\t' << "info->word_size = " << tags[ast::architecture::WORD_SIZE] << ';' << std::endl;
	// address size
	o << '\t' << "info->address_size = " << tags[ast::architecture::ADDRESS_SIZE] << ';' << std::endl;
	// largest float size
	if (tags[ast::architecture::FLOAT_SIZE] != 0)
		o << '\t' << "info->float_size = " << tags[ast::architecture::FLOAT_SIZE] << ';' << std::endl;
	// psr size
	if (tags[ast::architecture::PSR_SIZE] != 0) {
		o << '\t' << "info->flags_size = " << tags[ast::architecture::PSR_SIZE] << ';' << std::endl;
		o << '\t' << "info->flags_layout = arch_" << arch_name << "_flags_layout;" << std::endl;
	}

	o << std::endl;
	o << '\t' << "// Register file initialization" << std::endl;
	o << '\t' << "register_file = (reg_" << arch_name << "_t *)calloc(1, sizeof(reg_" << arch_name << "_t));"
	 << std::endl;
	o << '\t' << "rf->storage = register_file;" << std::endl;
	o << '\t' << "rf->layout = reg_" << arch_name << "_layout;" << std::endl;
	o << '}' << std::endl;
}

void
cg_dump_includes(std::ostream &o, std::string const &arch_name)
{
	o << "#include <assert.h>" << std::endl;
	o << std::endl;
	o << "#include \"libcpu.h\"" << std::endl;
	o << "#include \"frontend.h\"" << std::endl;
	o << "#include \"arch_types.h\"" << std::endl;
}

void
cg_generate_arch_done(std::ostream &o, std::string const &arch_name)
{
	o << "static void" << std::endl;
	o << "arch_" << arch_name << "_done(void *feptr, arch_regfile_t *rf)" << std::endl;
	o << '{' << std::endl;
	o << '\t' << "free(rf);" << std::endl;
	o << '}' << std::endl;
}

void
cg_generate_arch_get_pc(std::ostream &o, std::string const &arch_name,
		c::register_def const *reg)
{
	o << "static addr_t" << std::endl;
	o << "arch_" << arch_name << "_get_pc(void *feptr, arch_regfile_t *rf)" << std::endl;
	o << '{' << std::endl;
	o << '\t' << "return ((reg_" << arch_name << "_t *)rf->storage)->" << reg->get_name() << ';' << std::endl;
	o << '}' << std::endl;
}

void
cg_generate_arch_get_psr(std::ostream &o, std::string const &arch_name,
		c::register_def const *reg)
{
	o << "static uint64_t" << std::endl;
	o << "arch_" << arch_name << "_get_psr(void *feptr, arch_regfile_t *rf)" << std::endl;
	o << '{' << std::endl;
	o << '\t' << "return ((reg_" << arch_name << "_t *)rf->storage)->" << reg->get_name() << ';' << std::endl;
	o << '}' << std::endl;
}

void
cg_generate_gen_get_reg_set(std::ostream &o, std::string const &arch_name,
		c::register_def_vector const &phys_regs,
		c::register_def_vector const &virt_regs)
{
	size_t offset = 0;

	o << "static bool" << std::endl
	 << "arch_" << arch_name << "_get_reg(void" << arch_name << " *register_file, "
	 "size_t reg_name, void *buffer)" << std::endl;
	o << '{' << std::endl;
	o << '\t' << "switch (reg_name) {" << std::endl;
	for (c::register_def_vector::const_iterator i = phys_regs.begin ();
			i != phys_regs.end(); i++) {
		o << "\t\t" << "case " << "REG_" << arch_name << "_" << (*i)->get_name()
		 << ":" << std::endl;
		
		size_t size = get_aligned_size_for_type((*i)->get_type());
		o << "\t\t\t" << "*(uint" << size << "_t *)buffer = "
		 << "arch->regfile." << (*i)->get_name() << ";" << std::endl;
		o << "\t\t\t" << "break;" << std::endl;
	}
	o << "\t\t" << "default:" << std::endl;
	o << "\t\t\t" << "assert(0 && \"register not defined\");" << std::endl;
	o << "\t\t\t" << "return false;" << std::endl;
	o << '\t' << "}" << std::endl;
	o << '\t' << "return true;" << std::endl;
	o << '}' << std::endl;
}
