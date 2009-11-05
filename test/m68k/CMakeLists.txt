PROJECT(test_m68k)

ADD_EXECUTABLE(test_m68k main.cpp amigaos_lib.cpp)
TARGET_LINK_LIBRARIES(test_m68k cpu)
TARGET_LINK_LLVM(test_m68k)
