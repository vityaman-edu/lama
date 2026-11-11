add_library(liblamavm_runtime STATIC
  cc/runtime.c
  cc/gc.c
  cc/printf.S
  box.cpp
  binop.cpp
  heap.c
  sexp.cpp)
set_target_properties(liblamavm_runtime PROPERTIES
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
)

if(APPLE)
  set(LAMAVM_RUNTIME_PREFIX "_")
else()
  set(LAMAVM_RUNTIME_PREFIX "")
endif()

target_compile_definitions(liblamavm_runtime PUBLIC
  LAMAVM_RUNTIME_PREFIX="${LAMAVM_RUNTIME_PREFIX}")
target_include_directories(liblamavm_runtime PUBLIC "${PROJECT_SOURCE_DIR}/lib")
target_include_directories(liblamavm_runtime SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cc")
target_compile_options(liblamavm_runtime PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${LAMAVM_CXX_FLAGS}>"
  "$<$<COMPILE_LANGUAGE:C>:-fno-sanitize=shift-base>")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(liblamavm_runtime PRIVATE
    "$<$<COMPILE_LANGUAGE:ASM>:-Wa,--noexecstack>")
endif()
