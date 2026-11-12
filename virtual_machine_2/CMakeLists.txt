cmake_minimum_required(VERSION 3.25)
project(lamavm2 LANGUAGES C CXX ASM)

if(CMAKE_HOST_APPLE)
  set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "LaMa runtime architecture" FORCE)
endif()

if(NOT APPLE AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
  message(FATAL_ERROR "LaMa runtime requires an x86_64 target")
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

set(LAMAVM_CXX_FLAGS -Wall -Wextra -Wpedantic)

include(cmake/Cxxopts.cmake)
include(cmake/Sanitizer.cmake)
include(cmake/Library.cmake)

if(NOT LAMAVM_SANITIZERS)
  add_compile_options("$<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:-flto>")
  add_link_options("$<$<CONFIG:Release>:-flto>")
endif()

add_subdirectory(contrib)

file(GLOB_RECURSE LAMAVM_SOURCES CONFIGURE_DEPENDS source/*.c source/*.cpp)
add_executable(lamavm2 ${LAMAVM_SOURCES})
target_include_directories(lamavm2 PRIVATE "${PROJECT_SOURCE_DIR}/source")
target_compile_options(lamavm2 PRIVATE ${LAMAVM_CXX_FLAGS})
target_link_libraries(lamavm2 PRIVATE lama_runtime cxxopts::cxxopts)
