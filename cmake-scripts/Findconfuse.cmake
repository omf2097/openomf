if(VCPKG_TOOLCHAIN)
  find_package(unofficial-libconfuse CONFIG)
  if(unofficial-libconfuse_FOUND)
    get_target_property(CONFUSE_INCLUDE_DIR unofficial::libconfuse::libconfuse
                        INTERFACE_INCLUDE_DIRECTORIES)
    set(CONFUSE_LIBRARY unofficial::libconfuse::libconfuse)

    mark_as_advanced(CONFUSE_INCLUDE_DIR CONFUSE_LIBRARY)
    return()
  endif(unofficial-libconfuse_FOUND)
endif(VCPKG_TOOLCHAIN)

set(CONFUSE_SEARCH_PATHS /usr/local/ /usr /opt)

find_path(
  CONFUSE_INCLUDE_DIR confuse.h
  HINTS
  PATH_SUFFIXES include
  PATHS ${CONFUSE_SEARCH_PATHS})
find_library(
  CONFUSE_LIBRARY confuse
  HINTS
  PATH_SUFFIXES lib64 lib bin
  PATHS ${CONFUSE_SEARCH_PATHS})

if(CONFUSE_INCLUDE_DIR AND CONFUSE_LIBRARY)
  set(CONFUSE_FOUND TRUE)
endif(CONFUSE_INCLUDE_DIR AND CONFUSE_LIBRARY)

if(CONFUSE_FOUND)
  message(STATUS "Found libConfuse: ${CONFUSE_LIBRARY}")
else(CONFUSE_FOUND)
  message(WARNING "Could not find libConfuse")
endif(CONFUSE_FOUND)

mark_as_advanced(CONFUSE_INCLUDE_DIR CONFUSE_LIBRARY CONFUSE_SEARCH_PATHS)
