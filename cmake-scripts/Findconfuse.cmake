
if(VCPKG_TOOLCHAIN)
    find_package(unofficial-libconfuse CONFIG)
    if(unofficial-libconfuse_FOUND)
        get_target_property(CONFUSE_INCLUDE_DIR unofficial::libconfuse::libconfuse INTERFACE_INCLUDE_DIRECTORIES)
        set(CONFUSE_LIBRARY unofficial::libconfuse::libconfuse)

        mark_as_advanced(CONFUSE_INCLUDE_DIR CONFUSE_LIBRARY)
        return()
    endif(unofficial-libconfuse_FOUND)
endif(VCPKG_TOOLCHAIN)

SET(CONFUSE_SEARCH_PATHS
    /usr/local/
    /usr
    /opt
)

FIND_PATH(CONFUSE_INCLUDE_DIR confuse.h
    HINTS
    PATH_SUFFIXES include
    PATHS ${CONFUSE_SEARCH_PATHS}
)
FIND_LIBRARY(CONFUSE_LIBRARY confuse
    HINTS
    PATH_SUFFIXES lib64 lib bin
    PATHS ${CONFUSE_SEARCH_PATHS}
)

IF (CONFUSE_INCLUDE_DIR AND CONFUSE_LIBRARY)
    SET(CONFUSE_FOUND TRUE)
ENDIF (CONFUSE_INCLUDE_DIR AND CONFUSE_LIBRARY)

IF (CONFUSE_FOUND)
    MESSAGE(STATUS "Found libConfuse: ${CONFUSE_LIBRARY}")
ELSE (CONFUSE_FOUND)
    MESSAGE(WARNING "Could not find libConfuse")
ENDIF (CONFUSE_FOUND)

mark_as_advanced(CONFUSE_INCLUDE_DIR CONFUSE_LIBRARY CONFUSE_SEARCH_PATHS)
