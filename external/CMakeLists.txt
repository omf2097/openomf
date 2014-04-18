set(LIBSHADOWDIVE_LIBRARY_ONLY 1)

if( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libShadowDive/src" OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dumb/dumb")
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

add_custom_target(update-deps
    COMMAND git submodule update --init --recursive
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_subdirectory(libShadowDive)
add_subdirectory(dumb/dumb/cmake)

# Includes
set(DUMB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dumb/dumb/include" PARENT_SCOPE)
set(SHADOWDIVE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libShadowDive/include" PARENT_SCOPE)
