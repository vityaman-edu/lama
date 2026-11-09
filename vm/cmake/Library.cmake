function(lamavm_add_library)
  file(RELATIVE_PATH LAMAVM_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/lib" "${CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "/" "_" LAMAVM_MODULE_NAME "${LAMAVM_MODULE_PATH}")
  set(LAMAVM_TARGET "liblamavm_${LAMAVM_MODULE_NAME}")
  file(GLOB LAMAVM_SOURCES CONFIGURE_DEPENDS *.cpp *.hpp)

  add_library(${LAMAVM_TARGET} OBJECT ${LAMAVM_SOURCES})
  target_include_directories(${LAMAVM_TARGET} PUBLIC "${PROJECT_SOURCE_DIR}/lib")
  target_compile_options(${LAMAVM_TARGET} PRIVATE ${LAMAVM_CXX_FLAGS})

  if(ARGN)
    target_link_libraries(${LAMAVM_TARGET} PUBLIC ${ARGN})
  endif()
endfunction()
