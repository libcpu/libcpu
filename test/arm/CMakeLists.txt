PROJECT(test_arm)

ADD_EXECUTABLE(test_arm main.cpp)
TARGET_LINK_LIBRARIES(test_arm cpu)
TARGET_LINK_LLVM(test_arm)
