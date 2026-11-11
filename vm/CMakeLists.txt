cmake_minimum_required(VERSION 3.25)

if(CMAKE_HOST_APPLE)
  set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "LaMa runtime architecture" FORCE)
endif()

project(lamavm LANGUAGES C CXX ASM)

if(NOT APPLE AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
  message(FATAL_ERROR "LaMa runtime requires an x86_64 target")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LAMAVM_CXX_FLAGS -Wall -Wextra -Wpedantic)

include(CTest)
include(cmake/Cxxopts.cmake)
include(cmake/Sanitizer.cmake)
include(cmake/FuzzTest.cmake)
include(cmake/Library.cmake)

add_subdirectory(lib)

add_executable(lamavm app/main.cpp app/args.cpp)
target_link_libraries(lamavm PRIVATE liblamavm cxxopts::cxxopts)
target_compile_options(lamavm PRIVATE ${LAMAVM_CXX_FLAGS})

include(cmake/Test.cmake)
