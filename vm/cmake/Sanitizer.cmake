set(LAMAVM_SANITIZER "none" CACHE STRING "Sanitizer: none, address, undefined, memory")
set_property(CACHE LAMAVM_SANITIZER PROPERTY STRINGS none address undefined memory)

if(LAMAVM_SANITIZER STREQUAL "address")
  set(sanitizer_flags -fsanitize=address,undefined -fno-sanitize-recover=all)
elseif(LAMAVM_SANITIZER STREQUAL "undefined")
  set(sanitizer_flags -fsanitize=undefined -fno-sanitize-recover=all)
elseif(LAMAVM_SANITIZER STREQUAL "memory")
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
     NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "MemorySanitizer requires Clang on Linux")
  endif()

  include(CheckPIESupported)
  check_pie_supported()
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(sanitizer_flags -fsanitize=memory -fsanitize-memory-track-origins=2
      -fno-sanitize-recover=all)
elseif(NOT LAMAVM_SANITIZER STREQUAL "none")
  message(FATAL_ERROR "Unknown LAMAVM_SANITIZER: ${LAMAVM_SANITIZER}")
endif()

if(sanitizer_flags)
  add_compile_options(${sanitizer_flags} -fno-omit-frame-pointer)
  add_link_options(${sanitizer_flags})
endif()
