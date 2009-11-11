PROJECT(test_next68k)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

ADD_EXECUTABLE(test_next68k main.cpp loader.cpp next_lib.cpp)
TARGET_LINK_LIBRARIES(test_next68k cpu)
