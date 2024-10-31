include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO miniupnp/libnatpmp
    REF e5d6c5c35e18ed062f2053fd51c50f40ccd6cb68 # Release 20150609
    SHA512 c0f745da633eeaec9b53a0ce6d0679548e68e40c28367e9133ecb6d1e1a6134514f31c926e6606729731fc9b82280bea3bc9405dafe58805999f8c0c49950099
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
