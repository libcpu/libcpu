PROJECT(test_fib)

ADD_EXECUTABLE(test_fib fib.cpp)
TARGET_LINK_LIBRARIES(test_fib cpu)
