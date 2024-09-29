set(EPOXY_SEARCH_PATHS /usr/local /usr /opt)

find_path(EPOXY_INCLUDE_DIR epoxy/gl.h
    HINTS
    PATH_SUFFIXES include
    PATHS ${EPOXY_SEARCH_PATHS}
)
find_library(EPOXY_LIBRARY epoxy
    HINTS
    PATH_SUFFIXES lib64 lib
    PATHS ${EPOXY_SEARCH_PATHS}
)

if (EPOXY_INCLUDE_DIR AND EPOXY_LIBRARY)
    set(EPOXY_FOUND TRUE)
endif ()

if (EPOXY_FOUND)
    add_library(Epoxy::Main UNKNOWN IMPORTED)
    set_target_properties(Epoxy::Main PROPERTIES
        IMPORTED_LOCATION "${EPOXY_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${EPOXY_INCLUDE_DIR}")
    message(STATUS "Found libepoxy: ${EPOXY_LIBRARY}")
else ()
    message(WARNING "Could not find libepoxy")
endif ()

mark_as_advanced(EPOXY_INCLUDE_DIR EPOXY_LIBRARY EPOXY_SEARCH_PATHS)
