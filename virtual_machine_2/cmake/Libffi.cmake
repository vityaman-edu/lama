include(ExternalProject)

find_program(LIBFFI_MAKE NAMES gmake make REQUIRED)

set(libffi_install_dir "${CMAKE_CURRENT_BINARY_DIR}/_deps/libffi-install")
set(libffi_include_dir "${libffi_install_dir}/include/contrib/libffi")
set(libffi_archive "${libffi_install_dir}/lib/libffi.a")
string(TOUPPER "${CMAKE_BUILD_TYPE}" libffi_build_type)
set(libffi_cflags "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${libffi_build_type}}")
set(libffi_configure_args)

if(APPLE)
  list(APPEND libffi_configure_args --host=x86_64-apple-darwin)
  string(APPEND libffi_cflags " -arch x86_64")

  if(CMAKE_OSX_SYSROOT)
    string(APPEND libffi_cflags " -isysroot ${CMAKE_OSX_SYSROOT}")
  endif()

  if(CMAKE_OSX_DEPLOYMENT_TARGET)
    string(APPEND libffi_cflags " -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()
endif()

ExternalProject_Add(libffi_external
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/_deps/libffi"
  URL https://github.com/libffi/libffi/releases/download/v3.8.1/libffi-3.8.1.tar.gz
  URL_HASH SHA256=81e879c094cc5ec3186c48510c6385ac6b3e866192e8bc71a35245703757ee40
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  INSTALL_DIR "${libffi_install_dir}"
  CONFIGURE_COMMAND <SOURCE_DIR>/configure
    --prefix=<INSTALL_DIR>
    --libdir=<INSTALL_DIR>/lib
    --includedir=${libffi_include_dir}
    --disable-shared
    --enable-static
    --with-pic
    --disable-docs
    "CC=${CMAKE_C_COMPILER}"
    "CFLAGS=${libffi_cflags}"
    ${libffi_configure_args}
  BUILD_COMMAND "${LIBFFI_MAKE}"
  INSTALL_COMMAND "${LIBFFI_MAKE}" install
  BUILD_BYPRODUCTS "${libffi_archive}")

file(MAKE_DIRECTORY "${libffi_include_dir}")
add_library(libffi::libffi STATIC IMPORTED)
set_target_properties(libffi::libffi PROPERTIES
  IMPORTED_LOCATION "${libffi_archive}"
  INTERFACE_INCLUDE_DIRECTORIES "${libffi_install_dir}/include;${libffi_include_dir}")
add_dependencies(libffi::libffi libffi_external)
