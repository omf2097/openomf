vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO miniupnp/libnatpmp
    REF 8257134a5dcb077e40db1946554d676e444406e4
    SHA512 eb277a2507d658d5f5b8af4af44b80eabdd9ea460c498f736c2e6417dd1026994916cdafb0aaf31c0687cec38abe6abfd9179a5afe2c3c87d2203564db88b97a
    HEAD_REF master
    PATCHES
        "install-declspec-header.patch"
        "install-targets.patch"
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/libnatpmp-config.cmake.in" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_install_cmake()
vcpkg_copy_pdbs()

vcpkg_fixup_pkgconfig()
vcpkg_cmake_config_fixup()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
  file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/bin" "${CURRENT_PACKAGES_DIR}/bin")
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
