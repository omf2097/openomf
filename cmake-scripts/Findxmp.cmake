
if(VCPKG_TOOLCHAIN)
    find_package(libxmp CONFIG)
    if(libxmp_FOUND)
        if(TARGET libxmp::xmp_shared)
            set(XMP_LIBRARY libxmp::xmp_shared)
        else()
            set(XMP_LIBRARY libxmp::xmp_static)
        endif()

        get_target_property(XMP_INCLUDE_DIR "${XMP_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES)

        mark_as_advanced(XMP_INCLUDE_DIR XMP_LIBRARY)
        return()
    endif()
endif()

SET(XMP_SEARCH_PATHS
    /usr/local/
    /usr
    /opt
)

FIND_PATH(XMP_INCLUDE_DIR xmp.h
    HINTS ${XMP_ROOT}
    PATH_SUFFIXES include
    PATHS ${XMP_SEARCH_PATHS}
)
FIND_LIBRARY(XMP_LIBRARY xmp xmp_dll
    HINTS ${XMP_ROOT}
    PATH_SUFFIXES lib64 lib
    PATHS ${XMP_SEARCH_PATHS}
)

IF(XMP_INCLUDE_DIR AND XMP_LIBRARY)
   SET(XMP_FOUND TRUE)
ENDIF()

IF(XMP_FOUND)
    MESSAGE(STATUS "Found libxmp: ${XMP_LIBRARY}")
ELSE()
    MESSAGE(WARNING "Could not find libxmp")
ENDIF()

mark_as_advanced(XMP_INCLUDE_DIR XMP_LIBRARY XMP_SEARCH_PATHS)
