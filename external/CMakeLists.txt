set(LIBSHADOWDIVE_LIBRARY_ONLY 1)

if( NOT EXISTS "${PROJECT_SOURCE_DIR}/libShadowDive/src" OR NOT EXISTS "${PROJECT_SOURCE_DIR}/dumb/dumb")
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

add_custom_target(update-deps
    COMMAND git submodule update --init --recursive)

add_subdirectory(libShadowDive)
add_subdirectory(dumb/dumb/cmake)

