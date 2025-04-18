if(NOT BUILD_LANGUAGES)
    return()
endif()

# OMF 2097 Epic Challenge Arena
set(LANG_STRCOUNT 1013)
set(OMF_LANGS ENGLISH GERMAN)
# OpenOMF-specific
set(LANG2_STRCOUNT 1)
set(OPENOMF_LANGS DANISH)

if(WIN32)
    set(LANGUAGE_INSTALL_PATH "openomf/resources/")
else()
    set(LANGUAGE_INSTALL_PATH "share/games/openomf/")
endif()

# generate custom target info
set(BUILD_LANG_COMMANDS)
set(BUILD_LANG_SOURCES)
foreach(LANG ${OMF_LANGS})
    set(TXT2 "${PROJECT_SOURCE_DIR}/resources/${LANG}2.TXT")
    set(DAT2 "${CMAKE_CURRENT_BINARY_DIR}/resources/${LANG}.DAT2")
    list(APPEND BUILD_LANG_SORUCES "${TXT2}")
    list(APPEND BUILD_LANG_COMMANDS
        DEPENDS "${TXT2}"
        BYPRODUCTS "${DAT2}"
        COMMAND ${CMAKE_COMMAND} -E echo_append "${LANG}, "
        COMMAND "$<TARGET_FILE:languagetool>" -i "${TXT2}" -o "${DAT2}" --check-count ${LANG2_STRCOUNT}
    )
    install(FILES "${DAT2}" DESTINATION "${LANGUAGE_INSTALL_PATH}")
endforeach()
foreach(LANG ${OPENOMF_LANGS})
    set(TXT "${PROJECT_SOURCE_DIR}/resources/${LANG}.TXT")
    set(TXT2 "${PROJECT_SOURCE_DIR}/resources/${LANG}2.TXT")
    set(LNG "${CMAKE_CURRENT_BINARY_DIR}/resources/${LANG}.LNG")
    set(LNG2 "${CMAKE_CURRENT_BINARY_DIR}/resources/${LANG}.LNG2")
    list(APPEND BUILD_LANG_SORUCES "${TXT}" "${TXT2}")
    list(APPEND BUILD_LANG_COMMANDS
        DEPENDS "${TXT}" "${TXT2}"
        BYPRODUCTS "${LNG}" "{LNG2}"
        COMMAND ${CMAKE_COMMAND} -E echo_append "${LANG}, "
        COMMAND "$<TARGET_FILE:languagetool>" -i "${TXT}" -o "${LNG}" --check-count ${LANG_STRCOUNT}
        COMMAND "$<TARGET_FILE:languagetool>" -i "${TXT2}" -o "${LNG2}" --check-count ${LANG2_STRCOUNT}
    )
    install(FILES "${LNG}" "${LNG2}" DESTINATION "${LANGUAGE_INSTALL_PATH}")
endforeach()



add_custom_target(build_languages
    COMMAND ${CMAKE_COMMAND} -E echo_append "Building Languages... "
    ${BUILD_LANG_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E echo_append "done"
)
target_sources(build_languages PRIVATE ${BUILD_LANG_SORUCES})
add_dependencies(openomf build_languages)
add_dependencies(build_languages languagetool)
