PROJECT(libcpu)

ADD_LIBRARY(cpu SHARED
			cpu_generic.cpp
			tag.cpp
			optimize.cpp
			libcpu_main.cpp)

TARGET_LINK_LIBRARIES(cpu ${GUEST_ARCHITECTURES_ENABLED})
TARGET_LINK_LLVM(cpu)
