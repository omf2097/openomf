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

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Epoxy
    REQUIRED_VARS EPOXY_LIBRARY EPOXY_INCLUDE_DIR
)

mark_as_advanced(EPOXY_INCLUDE_DIR EPOXY_LIBRARY EPOXY_SEARCH_PATHS)
