include(CMakeFindDependencyMacro)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libconfuse REQUIRED IMPORTED_TARGET libconfuse)

if(NOT TARGET unofficial::libconfuse::libconfuse)
    add_library(unofficial::libconfuse::libconfuse INTERFACE IMPORTED)
    target_link_libraries(unofficial::libconfuse::libconfuse INTERFACE PkgConfig::libconfuse)
endif()
