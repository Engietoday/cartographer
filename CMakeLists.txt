cmake_minimum_required(VERSION 3.13)
project(cartographer)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE main_source src/*.cpp)
file(GLOB_RECURSE main_header src/*.h)
file(GLOB_RECURSE module_source modules/**.cpp)
file(GLOB_RECURSE module_header modules/**.h)



message(STATUS "MY_VARIABLE=${module_source}")


include_directories(include)
#include_directories(modules)
add_executable(main ${main_source})
#target_include_directories(main PUBLIC modules)

