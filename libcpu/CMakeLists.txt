PROJECT(libcpu)

ADD_LIBRARY(cpu STATIC
			cpu_generic.cpp
			libcpu_main.cpp)

TARGET_LINK_LIBRARIES(cpu ${GUEST_ARCHITECTURES_ENABLED})
