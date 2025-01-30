#
# BuildLanguages.cmake has one job:
# if option BUILD_LANGUAGES is on, builds the .LNG files from resources/translations/*.TXT
#

if(NOT BUILD_LANGUAGES)
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

file(READ "${LANG_SRC}/langstr_count" Lang_Count)
string(REGEX REPLACE "\n+$" "" Lang_Count "${Lang_Count}")

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
        COMMAND "$<TARGET_FILE:languagetool>" --import "${TXT}" --output "${LNG}" --check-count "${Lang_Count}"
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
