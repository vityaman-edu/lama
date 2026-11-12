add_library(lama_runtime STATIC runtime/runtime.c runtime/gc.c runtime/printf.S)
target_compile_options(lama_runtime PRIVATE -w)
target_include_directories(lama_runtime SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/runtime")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_options(lama_runtime PRIVATE
    "$<$<COMPILE_LANGUAGE:ASM>:-Wa,--noexecstack>")
endif()
