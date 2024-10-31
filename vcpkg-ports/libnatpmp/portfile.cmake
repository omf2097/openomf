vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO miniupnp/libnatpmp
    REF 8257134a5dcb077e40db1946554d676e444406e4
    SHA512 eb277a2507d658d5f5b8af4af44b80eabdd9ea460c498f736c2e6417dd1026994916cdafb0aaf31c0687cec38abe6abfd9179a5afe2c3c87d2203564db88b97a
    HEAD_REF master
)

vcpkg_configure_cmake(
    SOURCE_PATH "${SOURCE_PATH}"
    PREFER_NINJA
    OPTIONS
        -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
        -DBUILD_TESTING=OFF
)

vcpkg_install_cmake()
vcpkg_copy_pdbs()

if("tool" IN_LIST FEATURES)
    vcpkg_copy_tools(TOOL_NAMES natpmpc AUTO_CLEAN)
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Handle copyright
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
