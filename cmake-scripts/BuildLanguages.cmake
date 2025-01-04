#
# BuildLanguages.cmake has two jobs:
# - it generates src/resources/generated_languages.h from LanguageStrings.cmake, and
# - if option BUILD_LANGUAGES is on, builds the .LNG files from resources/translations/*.TXT
#

macro(lang_str enum_name)
    if(${ARGC} GREATER 1)
        set(count "${ARGV1}")

        if(${count} LESS 1)
            message(FATAL_ERROR "Cannot add less than 1 of lang_str '${enum_name}'.")
        endif()

        string(APPEND LANG_ENUM "\n    ${enum_name} = ${Lang_Count},")
        string(APPEND LANG_ENUM "\n    ${enum_name}_Last = ${Lang_Count} + (${count} - 1),\n")
        math(EXPR Lang_Count "${Lang_Count} + ${count}")
    else()
        string(APPEND LANG_ENUM "\n    ${enum_name} = ${Lang_Count},\n")
        math(EXPR Lang_Count "${Lang_Count} + 1")
    endif()
endmacro()
set(Lang_Count 0)
set(LANG_ENUM "")
include("cmake-scripts/LanguageStrings.cmake")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/resources/generated_languages.h.in" "${CMAKE_CURRENT_BINARY_DIR}/src/resources/generated_languages.h")

message(STATUS "BuildLanguages' Lang_Count: ${Lang_Count}")


if(NOT BUILD_LANGUAGES)
    # early out, having written generated_languages.h
    return()
endif()


if(WIN32)
    set(LANGUAGE_INSTALL_PATH "openomf/resources/")
else()
    set(LANGUAGE_INSTALL_PATH "share/games/openomf/")
endif()

# glob all language sources
set(LANG_SRC "${PROJECT_SOURCE_DIR}/resources/translations")
file(GLOB OPENOMF_LANGS
    LIST_DIRECTORIES false
    CONFIGURE_DEPENDS
    "${LANG_SRC}/*.TXT"
)

# generate custom target info
set(BUILD_LANG_COMMANDS)
set(BUILD_LANG_SOURCES)
foreach(TXT ${OPENOMF_LANGS})
    get_filename_component(LANG "${TXT}" NAME_WE)
    set(LNG "${CMAKE_CURRENT_BINARY_DIR}/resources/${LANG}.LNG")
    list(APPEND BUILD_LANG_SORUCES "${TXT}")
    list(APPEND BUILD_LANG_COMMANDS
        DEPENDS "${TXT}"
        BYPRODUCTS "${LNG}"
        COMMAND ${CMAKE_COMMAND} -E echo_append "${LANG}, "
        COMMAND "$<TARGET_FILE:languagetool>" --import "${TXT}" --output "${LNG}" --check-count ${Lang_Count}
    )
    install(FILES "${LNG}" DESTINATION "${LANGUAGE_INSTALL_PATH}")
endforeach()


add_custom_target(build_languages
    COMMAND ${CMAKE_COMMAND} -E echo_append "Building Languages... "
    ${BUILD_LANG_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E echo_append "done"
)
target_sources(build_languages PRIVATE ${BUILD_LANG_SOURCES})
add_dependencies(openomf build_languages)
add_dependencies(build_languages languagetool)
