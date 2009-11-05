PROJECT(test_mips)

ADD_EXECUTABLE(test_mips main.cpp)
TARGET_LINK_LIBRARIES(test_mips cpu)
TARGET_LINK_LLVM(test_mips)
