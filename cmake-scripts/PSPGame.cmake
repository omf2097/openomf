# Build a complete PSP/GAME/OpenOMF tree, usable by PPSSPP and real PSPs.
# Keep freeware OMF 2097 assets out of the source tree.
set(OMF_PSP_ASSET_ARCHIVE "" CACHE FILEPATH "Path to an existing OMF 2097 assets ZIP (optional)")
set(PSP_GAME_DIR "${CMAKE_CURRENT_BINARY_DIR}/PSP/GAME/OpenOMF")
set(PSP_RESOURCE_DIR "${PSP_GAME_DIR}/resources")

if(OMF_PSP_ASSET_ARCHIVE)
    if(NOT EXISTS "${OMF_PSP_ASSET_ARCHIVE}")
        message(FATAL_ERROR "PSP asset archive not found: ${OMF_PSP_ASSET_ARCHIVE}")
    endif()
    set(psp_asset_archive "${OMF_PSP_ASSET_ARCHIVE}")
else()
    set(psp_asset_archive "${CMAKE_CURRENT_BINARY_DIR}/psp-data/openomf-assets.zip")
    if(NOT EXISTS "${psp_asset_archive}")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/psp-data")
        file(DOWNLOAD "https://www.omf2097.com/pub/files/omf/openomf-assets.zip"
            "${psp_asset_archive}" STATUS asset_download TLS_VERIFY ON TIMEOUT 120)
        list(GET asset_download 0 download_code)
        if(NOT download_code EQUAL 0)
            file(REMOVE "${psp_asset_archive}")
            message(FATAL_ERROR "Unable to download OMF 2097 game assets: ${asset_download}")
        endif()
    endif()
endif()

find_program(OMF_PSP_UNZIP NAMES unzip)
if(NOT OMF_PSP_UNZIP)
    message(FATAL_ERROR "Packaging PSP game resources requires unzip")
endif()

set(psp_asset_stamp "${CMAKE_CURRENT_BINARY_DIR}/psp-data/assets.stamp")
add_custom_command(
    OUTPUT "${psp_asset_stamp}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PSP_RESOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/psp-data"
    COMMAND "${OMF_PSP_UNZIP}" -jo -o "${psp_asset_archive}" "OMF2097/*" -d "${PSP_RESOURCE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_SOURCE_DIR}/resources/openomf.bk" "${CMAKE_SOURCE_DIR}/resources/gamecontrollerdb/gamecontrollerdb.txt"
        "${PSP_RESOURCE_DIR}"
    COMMAND "${CMAKE_COMMAND}" "-DOMF_PSP_RESOURCE_DIR=${PSP_RESOURCE_DIR}"
        -P "${CMAKE_SOURCE_DIR}/cmake-scripts/PSPVerifyResources.cmake"
    COMMAND "${CMAKE_COMMAND}" -E touch "${psp_asset_stamp}"
    DEPENDS "${psp_asset_archive}"
        "${CMAKE_SOURCE_DIR}/resources/openomf.bk"
        "${CMAKE_SOURCE_DIR}/resources/gamecontrollerdb/gamecontrollerdb.txt"
    VERBATIM
    COMMENT "Stage original OMF 2097 resources for PSP"
)

# PSP is cross-compiled. The existing languagetool must be built for the host.
# Build on demand, with CMake's own incremental rebuild checks for its sources.
set(psp_host_tools "${CMAKE_CURRENT_BINARY_DIR}/psp-host-tools")
file(GLOB_RECURSE psp_language_sources CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tools/languagetool/*.c"
    "${CMAKE_SOURCE_DIR}/src/formats/*.c"
    "${CMAKE_SOURCE_DIR}/src/utils/*.c"
    "${CMAKE_SOURCE_DIR}/src/vendored/argtable/*.c"
)
set(psp_language "${PSP_RESOURCE_DIR}/ENGLISH.DAT2")
add_custom_command(
    OUTPUT "${psp_language}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PSP_RESOURCE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -S "${CMAKE_SOURCE_DIR}/packaging/psp/host-tools"
        -B "${psp_host_tools}" "-DCMAKE_C_COMPILER=cc" "-DCMAKE_BUILD_TYPE=Release"
    COMMAND "${CMAKE_COMMAND}" --build "${psp_host_tools}" --target languagetool
    COMMAND "${psp_host_tools}/languagetool"
        -i "${CMAKE_SOURCE_DIR}/resources/ENGLISH2.TXT"
        -o "${psp_language}" --check-count 1
    DEPENDS "${CMAKE_SOURCE_DIR}/resources/ENGLISH2.TXT"
        "${CMAKE_SOURCE_DIR}/packaging/psp/host-tools/CMakeLists.txt"
        ${psp_language_sources}
    VERBATIM
    COMMENT "Build the host languagetool and generate ENGLISH.DAT2"
)

# create_pbp_file() runs as an openomf post-build command.
add_custom_command(
    OUTPUT "${PSP_GAME_DIR}/EBOOT.PBP"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PSP_GAME_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/EBOOT.PBP" "${PSP_GAME_DIR}/EBOOT.PBP"
    DEPENDS openomf
    VERBATIM
    COMMENT "Stage PSP EBOOT.PBP next to game resources"
)

add_custom_target(psp_game ALL DEPENDS
    "${PSP_GAME_DIR}/EBOOT.PBP" "${psp_asset_stamp}" "${psp_language}")
add_custom_target(psp_package
    COMMAND "${CMAKE_COMMAND}" -E tar cf "${CMAKE_CURRENT_BINARY_DIR}/openomf-psp-sdl-null.zip" --format=zip PSP
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS psp_game
    VERBATIM
    COMMENT "Package complete PSP/GAME/OpenOMF directory as ZIP"
)
message(STATUS "PSP installation directory: ${PSP_GAME_DIR}")
