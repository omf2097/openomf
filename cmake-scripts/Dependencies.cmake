# Dependencies.cmake
# this file locates all third party dependencies, and creates easy to use
# targets like `openomf::argtable` for the openomf build scripts to link to.

# set module path only temporarily, to discourage find_package being used outside of this script
set(ORIGINAL_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)


# SDL2, SDL2_mixer
add_library(openomf::SDL2 INTERFACE IMPORTED)
add_library(openomf::SDL2main INTERFACE IMPORTED)
add_library(openomf::SDL2_mixer INTERFACE IMPORTED)
if(VCPKG_TOOLCHAIN)
    find_package(SDL2 CONFIG REQUIRED)
    target_link_libraries(openomf::SDL2 INTERFACE "$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>")
    target_link_libraries(openomf::SDL2main INTERFACE SDL2::SDL2main openomf::SDL2)

    find_package(SDL2_mixer CONFIG REQUIRED)
    target_link_libraries(openomf::SDL2_mixer INTERFACE "$<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>")
else()
    find_package(SDL2 REQUIRED)
    target_link_libraries(openomf::SDL2 INTERFACE SDL2::Core)
    target_link_libraries(openomf::SDL2main INTERFACE SDL2::Main openomf::SDL2)

    find_package(SDL2_mixer REQUIRED)
    target_link_libraries(openomf::SDL2_mixer INTERFACE SDL2::Mixer)
endif()

# xmp
add_library(openomf::xmp INTERFACE IMPORTED)
if(VCPKG_TOOLCHAIN)
    find_package(libxmp CONFIG REQUIRED)
    target_link_libraries(openomf::xmp INTERFACE "$<IF:$<TARGET_EXISTS:libxmp::xmp_shared>,libxmp::xmp_shared,libxmp::xmp_static>")
else()
    find_package(xmp REQUIRED)
    target_link_libraries(openomf::xmp INTERFACE ${XMP_LIBRARY})
    target_include_directories(openomf::xmp INTERFACE ${XMP_INCLUDE_DIR})
endif()

# argtable
add_library(openomf::argtable INTERFACE IMPORTED)
target_include_directories(openomf::argtable INTERFACE "${CMAKE_SOURCE_DIR}/src/vendored")

# enet
add_library(openomf::enet INTERFACE IMPORTED)
if(VCPKG_TOOLCHAIN)
    find_package(unofficial-enet CONFIG REQUIRED)
    target_link_libraries(openomf::enet INTERFACE unofficial::enet::enet)
else()
    find_package(enet REQUIRED)
    target_link_libraries(openomf::enet INTERFACE ${ENET_LIBRARY})
    target_include_directories(openomf::enet INTERFACE ${ENET_INCLUDE_DIR})
endif()

# confuse
add_library(openomf::confuse INTERFACE IMPORTED)
if(VCPKG_TOOLCHAIN)
    find_package(unofficial-libconfuse CONFIG REQUIRED)
    target_link_libraries(openomf::confuse INTERFACE unofficial::libconfuse::libconfuse)
else()
    find_package(confuse REQUIRED)
    target_link_libraries(openomf::confuse INTERFACE ${CONFUSE_LIBRARY})
    target_include_directories(openomf::confuse INTERFACE ${CONFUSE_INCLUDE_DIR})
endif()

if(USE_MINIUPNPC)
    add_library(openomf::miniupnpc INTERFACE IMPORTED)
    target_compile_definitions(openomf::miniupnpc INTERFACE MINIUPNPC_FOUND)
    if(VCPKG_TOOLCHAIN)
        find_package(miniupnpc CONFIG REQUIRED)
        target_link_libraries(openomf::miniupnpc INTERFACE miniupnpc::miniupnpc)
    else()
        find_package(miniupnpc REQUIRED)
        target_link_libraries(openomf::miniupnpc INTERFACE ${MINIUPNPC_LIBRARY})
        target_include_directories(openomf::miniupnpc INTERFACE ${MINIUPNPC_INCLUDE_DIR})
    endif()
endif()

if(USE_NATPMP)
    add_library(openomf::natpmp INTERFACE IMPORTED)
    target_compile_definitions(openomf::natpmp INTERFACE NATPMP_FOUND)
    if(VCPKG_TOOLCHAIN)
        find_package(libnatpmp CONFIG REQUIRED)
        target_link_libraries(openomf::natpmp INTERFACE libnatpmp::natpmp)
    else()
        find_package(natpmp REQUIRED)
        target_link_libraries(openomf::natpmp INTERFACE ${LIBNATPMP_LIBRARY})
        target_include_directories(openomf::natpmp INTERFACE ${LIBNATPMP_INCLUDE_DIR})
        target_compile_definitions(openomf::natpmp INTERFACE ${LIBNATPMP_DEFINITIONS})
    endif()
endif()

if(USE_LIBPNG)
    find_package(PNG REQUIRED)
    add_library(openomf::png INTERFACE IMPORTED)
    target_link_libraries(openomf::png INTERFACE PNG::PNG)
    target_compile_definitions(openomf::png INTERFACE PNG_FOUND)
endif()

if(USE_OPUSFILE)
    add_library(openomf::opusfile INTERFACE IMPORTED)
    target_compile_definitions(openomf::opusfile INTERFACE OPUSFILE_FOUND)
    if(VCPKG_TOOLCHAIN)
        find_package(OpusFile CONFIG REQUIRED)
        target_link_libraries(openomf::opusfile INTERFACE OpusFile::opusfile)
    else()
        find_package(OpusFile REQUIRED)
        target_link_libraries(openomf::opusfile INTERFACE ${OPUSFILE_LIBRARY})
        target_include_directories(openomf::opusfile INTERFACE ${OPUSFILE_INCLUDE_DIR})
    endif()
endif()

find_package(Epoxy REQUIRED)
add_library(openomf::epoxy UNKNOWN IMPORTED)
set_target_properties(openomf::epoxy PROPERTIES
    IMPORTED_LOCATION "${EPOXY_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${EPOXY_INCLUDE_DIR}")

if(USE_TESTS)
    find_package(CUnit REQUIRED)
    add_library(openomf::cunit INTERFACE IMPORTED)
    target_link_libraries(openomf::cunit INTERFACE ${CUNIT_LIBRARY})
    target_include_directories(openomf::cunit INTERFACE ${CUNIT_INCLUDE_DIR})
endif()


set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})
