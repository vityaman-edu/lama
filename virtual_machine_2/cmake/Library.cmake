function(lamavm_add_library name)
  file(GLOB sources CONFIGURE_DEPENDS *.c *.cpp *.h *.hpp)
  add_library(${name} STATIC ${sources})
  target_include_directories(${name} PUBLIC "${PROJECT_SOURCE_DIR}/source")
  target_compile_options(${name} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${LAMAVM_CXX_FLAGS}>")

  if(ARGN)
    target_link_libraries(${name} PUBLIC ${ARGN})
  endif()
endfunction()
