option(LAMAVM_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(LAMAVM_SANITIZERS)
  add_compile_options(
    -fsanitize=address,undefined
    -fno-sanitize-recover=all
    -fno-omit-frame-pointer)
  add_link_options(
    -fsanitize=address,undefined
    -fno-sanitize-recover=all)
endif()
