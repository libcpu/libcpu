PROJECT(test_next68k)

ADD_EXECUTABLE(test_next68k main.cpp loader.cpp next_lib.cpp)
TARGET_LINK_LIBRARIES(test_next68k cpu)
TARGET_LINK_LLVM(test_next68k)
