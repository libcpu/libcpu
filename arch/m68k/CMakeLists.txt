PROJECT(m68k)

ADD_CUSTOM_COMMAND(OUTPUT opcode_id.h opcode_id.cpp
			COMMAND ${PYTHON_EXECUTABLE}
			ARGS ${CMAKE_CURRENT_SOURCE_DIR}/opcode_id.py
			${CMAKE_CURRENT_SOURCE_DIR}/opcode_id.tab
			${CMAKE_CURRENT_BINARY_DIR}/opcode_id.h
			${CMAKE_CURRENT_BINARY_DIR}/opcode_id.cpp
			DEPENDS opcode_id.py opcode_id.tab)

ADD_LIBRARY(m68k STATIC
			m68k_disasm.cpp
			m68k_isa.cpp
			m68k_arch.cpp
			${CMAKE_CURRENT_BINARY_DIR}/opcode_id.cpp
			m68k_tag.cpp)
