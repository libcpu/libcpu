#ifndef __upcl_c_type_h
#define __upcl_c_type_h

namespace upcl { namespace c {

class type {

public:
	enum type_id {
		INTEGER,
		FLOAT,
		VECTOR,
		VECTOR_INTEGER,
		VECTOR_FLOAT
	};

private:
	type_id  m_type;
	unsigned m_bits;
	unsigned m_elem_bits;

private:
	type(type_id const &type, unsigned bits)
		: m_type(type), m_bits(bits), m_elem_bits(0)
	{ }

	type(type_id const &type, unsigned bits, unsigned elem_bits)
		: m_type(type), m_bits(bits), m_elem_bits(elem_bits)
	{ }

public:
	static type *get_integer_type(unsigned bits)
	{ return new type(INTEGER, bits); }
	static type *get_float_type(unsigned bits)
	{ return new type(FLOAT, bits); }
	static type *get_vector_type(unsigned bits)
	{ return new type(VECTOR, bits); }
	static type *get_integer_vector_type(unsigned bits, unsigned elem_bits)
	{ return new type(VECTOR_INTEGER, bits, elem_bits); }
	static type *get_float_vector_type(unsigned bits, unsigned elem_bits)
	{ return new type(VECTOR_INTEGER, bits, elem_bits); }

public:
	inline size_t get_bits() const
	{ return m_bits; }
	inline size_t get_element_bits() const
	{ return m_elem_bits; }

	inline type_id get_type_id() const {
		switch (m_type) {
			case VECTOR_INTEGER:
			case VECTOR_FLOAT:
				return VECTOR;
			default:
				return m_type;
		}
	}

public:
	inline bool is_equal(type const *ty) const
	{
		if (ty == this)
			return true;

		return (ty->get_type_id() == get_type_id() &&
				ty->get_bits() == get_bits());
	}

public:// XXX WARNING: THIS API MUST BE CALLED WITH CAUTION! ONLY THE SIMPLIFY
	   //              PASS NEEDS THIS CALL, SO, IN GENERAL DON'T USE IT AND 
	   //              ASSUME IT DOESN'T EXIST!
	inline void set_bits_size(size_t size)
	{
		m_bits = size;
	}
};

} }

#endif  // !__upcl_c_type_h
