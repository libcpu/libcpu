PROJECT(test_m88k)

ADD_EXECUTABLE(test_m88k main.cpp)
TARGET_LINK_LIBRARIES(test_m88k cpu)

### Run88 
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/test/libnix/xec-compat/lib
					${CMAKE_SOURCE_DIR}/test/libnix/xec-compat/engine
					${CMAKE_SOURCE_DIR}/test/libnix/xec-compat/arch
					${CMAKE_SOURCE_DIR}/test/libnix/nix
					${CMAKE_SOURCE_DIR}/test/libnix
					${CMAKE_BINARY_DIR}/test/libnix/xec-compat
					${CMAKE_SOURCE_DIR}/test/libnix/obsd41/arch/m88k)

ADD_EXECUTABLE(run_m88k run88.cpp loader.c)
TARGET_LINK_LIBRARIES(run_m88k cpu obsd41)
