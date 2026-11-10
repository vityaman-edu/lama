if(NOT BUILD_TESTING)
  return()
endif()

set(LAMAVM_FUZZ_DURATION "10s" CACHE STRING "Duration of each coverage-guided fuzz test")
add_custom_target(lamavm_tests)

function(lamavm_add_test kind)
  file(RELATIVE_PATH path "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "/" "_" target "${PROJECT_NAME}_${path}")
  add_executable(${target} ${ARGN})
  target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/test/support")
  target_link_libraries(${target} PRIVATE liblamavm)
  target_compile_options(${target} PRIVATE ${LAMAVM_CXX_FLAGS})
  add_dependencies(lamavm_tests ${target})

  if(kind STREQUAL "unit")
    target_link_libraries(${target} PRIVATE GTest::gtest_main)
    gtest_discover_tests(${target} PROPERTIES LABELS unit)
  else()
    link_fuzztest(${target})

    if(FUZZTEST_FUZZING_MODE)
      add_test(NAME ${target} COMMAND ${target}
        --fuzz_for=${LAMAVM_FUZZ_DURATION})
    else()
      add_test(NAME ${target} COMMAND ${target})
    endif()

    set_tests_properties(${target} PROPERTIES LABELS fuzz)
  endif()
endfunction()

function(lamavm_add_unit_test)
  lamavm_add_test(unit ${ARGN})
endfunction()

function(lamavm_add_fuzz_test)
  lamavm_add_test(fuzz ${ARGN})
endfunction()

add_subdirectory(test/unit)
add_subdirectory(test/fuzz)
