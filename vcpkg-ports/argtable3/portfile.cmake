vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO argtable/argtable3
    REF "v${VERSION}"
    SHA512 623197142fd1749b2fd5bc3e51758ae49c58ec8699b6afa5ecb2d0199d98f9c05366f92c5169c8039b5c417f4774fb4a09c879a7b04ddbed9d5e43585692ed7f
    HEAD_REF master
    PATCHES
        "Remove-DllMain.patch"
)

set(ARGTABLE3_REPLACE_GETOPT ON)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DARGTABLE3_ENABLE_CONAN=OFF
        -DARGTABLE3_ENABLE_TESTS=OFF
        -DARGTABLE3_ENABLE_EXAMPLES=OFF
        -DARGTABLE3_REPLACE_GETOPT=${ARGTABLE3_REPLACE_GETOPT}
)

vcpkg_cmake_install()

vcpkg_copy_pdbs()

if(EXISTS "${CURRENT_PACKAGES_DIR}/cmake")
    vcpkg_cmake_config_fixup(CONFIG_PATH cmake)
elseif(EXISTS "${CURRENT_PACKAGES_DIR}/lib/cmake/${PORT}")
    vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/${PORT})
endif()

if(NOT ARGTABLE3_REPLACE_GETOPT)
    # fix dependency unofficial-getopt-win32
    vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/share/${PORT}/Argtable3Config.cmake" "# Generated CMake target import file."
[[
find_package(unofficial-getopt-win32 CONFIG REQUIRED)

# Generated CMake target import file.]])
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
