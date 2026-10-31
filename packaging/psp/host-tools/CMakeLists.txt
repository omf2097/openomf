cmake_minimum_required(VERSION 3.16)
project(OpenOMFHostLanguageTool C)

# The PSP executable is cross-compiled, but the existing OpenOMF languagetool
# must run on the build host. Compile just its native dependencies here.
get_filename_component(OMF_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

add_executable(languagetool
    "${OMF_ROOT}/tools/languagetool/main.c"
    "${OMF_ROOT}/src/vendored/argtable/argtable3.c"
    "${OMF_ROOT}/src/formats/error.c"
    "${OMF_ROOT}/src/formats/language.c"
    "${OMF_ROOT}/src/formats/internal/memreader.c"
    "${OMF_ROOT}/src/formats/internal/memwriter.c"
    "${OMF_ROOT}/src/formats/internal/reader.c"
    "${OMF_ROOT}/src/formats/internal/writer.c"
    "${OMF_ROOT}/src/utils/c_string_util.c"
    "${OMF_ROOT}/src/utils/cp437.c"
    "${OMF_ROOT}/src/utils/crash.c"
    "${OMF_ROOT}/src/utils/log.c"
    "${OMF_ROOT}/src/utils/list.c"
    "${OMF_ROOT}/src/utils/iterator.c"
    "${OMF_ROOT}/src/utils/path.c"
    "${OMF_ROOT}/src/utils/str.c"
    "${OMF_ROOT}/src/utils/vector.c"
)
target_include_directories(languagetool PRIVATE "${OMF_ROOT}/src" "${OMF_ROOT}/src/vendored/argtable")
target_link_libraries(languagetool PRIVATE PkgConfig::SDL2)
