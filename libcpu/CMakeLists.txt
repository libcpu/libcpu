PROJECT(libcpu)

ADD_LIBRARY(cpu SHARED
			frontend.cpp
			disasm.cpp
			basicblock.cpp
			function.cpp
			translate.cpp
			translate_all.cpp
			translate_singlestep.cpp
			translate_singlestep_bb.cpp
			tag.cpp
			optimize.cpp
			fp.cpp
			idbg.cpp
			stat.cpp
			sha1.cpp
			interface.cpp)

ADD_DEFINITIONS(-DLIBCPU_BUILD_CORE)
TARGET_LINK_LIBRARIES(cpu ${GUEST_ARCHITECTURES_ENABLED})
if (HAVE_LIBREADLINE)
	ADD_DEFINITIONS(-DUSE_READLINE)
	TARGET_LINK_LIBRARIES(cpu readline)
endif()
TARGET_LINK_LLVM(cpu)
