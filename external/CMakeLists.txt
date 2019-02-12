set(LIBSHADOWDIVE_LIBRARY_ONLY 1)

add_subdirectory(libShadowDive)

# Includes
set(SHADOWDIVE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libShadowDive/include" PARENT_SCOPE)
