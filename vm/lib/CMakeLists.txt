add_subdirectory(support)
add_subdirectory(core)
add_subdirectory(bc)

add_library(liblamavm STATIC)
set_target_properties(liblamavm PROPERTIES OUTPUT_NAME lamavm)
target_link_libraries(liblamavm PUBLIC
  liblamavm_bc
  liblamavm_bc_disassembly
  liblamavm_core
  liblamavm_support
)
