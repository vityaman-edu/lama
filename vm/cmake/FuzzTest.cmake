option(FUZZTEST_FUZZING_MODE "Enable coverage-guided fuzzing on Linux" OFF)

if(FUZZTEST_FUZZING_MODE)
  if(NOT BUILD_TESTING)
    message(FATAL_ERROR "Fuzzing requires BUILD_TESTING=ON")
  endif()

  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
     NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "FuzzTest coverage-guided fuzzing requires Clang on Linux")
  endif()

  if(LAMAVM_SANITIZER STREQUAL "memory")
    message(FATAL_ERROR "FuzzTest uses AddressSanitizer; MemorySanitizer is incompatible")
  endif()
endif()

if(BUILD_TESTING)
  include("${CMAKE_CURRENT_LIST_DIR}/GoogleTest.cmake")

  block(SCOPE_FOR VARIABLES)
    # Suppress warnings only while compiling external dependency sources.
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

    # Configure Abseil before FuzzTest's C++17 default to preserve its C++23 ABI.
    set(ABSL_PROPAGATE_CXX_STD ON)
    set(ABSL_ENABLE_INSTALL ON)
    FetchContent_Declare(
      abseil-cpp
      GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
      GIT_TAG 5650e9cf76d3be4318d5fa3af38ee483ddfd5e4a
      SYSTEM
    )
    FetchContent_MakeAvailable(abseil-cpp)

    set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "Build ANTLR dependency tests")
    FetchContent_Declare(
      fuzztest
      GIT_REPOSITORY https://github.com/google/fuzztest.git
      GIT_TAG 704efb341c23011cab2a750efcdd16ad04882c80
      SYSTEM
    )
    FetchContent_MakeAvailable(fuzztest)
  endblock()

  fuzztest_setup_fuzzing_flags()
endif()
