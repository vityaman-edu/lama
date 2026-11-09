cmake_minimum_required(VERSION 3.25)
project(lamavm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LAMAVM_CXX_FLAGS -Wall -Wextra -Wpedantic)

include(CTest)
include(cmake/Sanitizer.cmake)
include(cmake/FuzzTest.cmake)
include(cmake/Library.cmake)

add_subdirectory(lib)

add_executable(lamavm app/main.cpp)
target_link_libraries(lamavm PRIVATE liblamavm)
target_compile_options(lamavm PRIVATE ${LAMAVM_CXX_FLAGS})

include(cmake/Test.cmake)
