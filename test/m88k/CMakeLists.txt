PROJECT(test_m88k)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIBCPU_RUNTIME_OUTPUT_DIRECTORY})
ADD_EXECUTABLE(test_m88k main.cpp)
TARGET_LINK_LIBRARIES(test_m88k cpu)

### Run88 
IF(APPLE)
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/test/libnix/xec-compat/lib
						${CMAKE_SOURCE_DIR}/test/libnix/xec-compat/engine
						${CMAKE_SOURCE_DIR}/test/libnix/xec-compat/arch
						${CMAKE_SOURCE_DIR}/test/libnix/nix
						${CMAKE_SOURCE_DIR}/test/libnix
						${CMAKE_BINARY_DIR}/test/libnix/xec-compat
						${CMAKE_SOURCE_DIR}/test/libnix/obsd41/arch/m88k)

	ADD_EXECUTABLE(run_m88k run88.cpp loader.c)
	TARGET_LINK_LIBRARIES(run_m88k cpu obsd41)
ENDIF()

IF(APPLE AND NOT UNIVERSAL)
  IF(CMAKE_SIZEOF_VOID_P MATCHES 8)
    # Ensure __PAGEZERO is 4GB and...
    TARGET_LINK_LIBRARIES(run_m88k "-Wl,-pagezero_size,0x100000000")
    # ...is readable and writeable.
    TARGET_LINK_LIBRARIES(run_m88k "-Wl,-segprot,__PAGEZERO,rw-,rw-")
  ENDIF()
ENDIF()

