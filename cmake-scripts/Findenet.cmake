set(ENET_SEARCH_PATHS /usr/local/ /usr /opt)

find_path(
  ENET_INCLUDE_DIR enet/enet.h
  HINTS ${ENET_ROOT}
  PATH_SUFFIXES include include/enet
  PATHS ${ENET_SEARCH_PATHS})
find_library(
  ENET_LIBRARY enet
  HINTS ${ENET_ROOT}
  PATH_SUFFIXES lib64 lib bin
  PATHS ${ENET_SEARCH_PATHS})

if(ENET_INCLUDE_DIR AND ENET_LIBRARY)
  set(ENET_FOUND TRUE)
endif(ENET_INCLUDE_DIR AND ENET_LIBRARY)

if(ENET_FOUND)
  message(STATUS "Found ENet: ${ENET_LIBRARY}")
else(ENET_FOUND)
  message(WARNING "Could not find ENet")
endif(ENET_FOUND)

mark_as_advanced(ENET_INCLUDE_DIR ENET_LIBRARY ENET_SEARCH_PATHS)
