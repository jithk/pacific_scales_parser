cmake_minimum_required(VERSION 3.12)

# set the project name
project(pacific-scales-parser C CXX)

option(BUILD_TESTS "Build the test apps" ON)

set(CMAKE_CXX_STANDARD 14)
add_compile_options(-Wall)

add_library(parser-lib STATIC
  src/serial_device.cc
)

target_include_directories(parser-lib PUBLIC include)

add_executable(${PROJECT}
  src/main.cc
)
target_link_libraries(${PROJECT} PRIVATE parser-lib)

# if (BUILD_TESTS)
#   include(CTest)
#   add_subdirectory(tests)
# endif()

