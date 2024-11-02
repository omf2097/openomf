if(VCPKG_TOOLCHAIN)
  find_package(libxmp CONFIG)
  if(libxmp_FOUND)
    if(TARGET libxmp::xmp_shared)
      set(XMP_LIBRARY libxmp::xmp_shared)
    else()
      set(XMP_LIBRARY libxmp::xmp_static)
    endif()

    get_target_property(XMP_INCLUDE_DIR "${XMP_LIBRARY}"
                        INTERFACE_INCLUDE_DIRECTORIES)

    mark_as_advanced(XMP_INCLUDE_DIR XMP_LIBRARY)
    return()
  endif()
endif()

set(XMP_SEARCH_PATHS /usr/local/ /usr /opt)

find_path(
  XMP_INCLUDE_DIR xmp.h
  HINTS ${XMP_ROOT}
  PATH_SUFFIXES include
  PATHS ${XMP_SEARCH_PATHS})
find_library(
  XMP_LIBRARY xmp xmp_dll
  HINTS ${XMP_ROOT}
  PATH_SUFFIXES lib64 lib
  PATHS ${XMP_SEARCH_PATHS})

if(XMP_INCLUDE_DIR AND XMP_LIBRARY)
  set(XMP_FOUND TRUE)
endif()

if(XMP_FOUND)
  message(STATUS "Found libxmp: ${XMP_LIBRARY}")
else()
  message(WARNING "Could not find libxmp")
endif()

mark_as_advanced(XMP_INCLUDE_DIR XMP_LIBRARY XMP_SEARCH_PATHS)
