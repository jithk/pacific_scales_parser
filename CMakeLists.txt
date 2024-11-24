cmake_minimum_required(VERSION 3.12)

# set the project name
project(pacific-scales-parser C CXX)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall)

# Find pthreads Library
find_package(Threads REQUIRED)

add_library(parser-lib STATIC
  src/serial_device.cc
  src/scale_data_parser.cc
)

target_include_directories(parser-lib PUBLIC include)

add_executable(pacific-parser
  src/main.cc
)

target_link_libraries(pacific-parser PRIVATE
  parser-lib
  Threads::Threads
)
