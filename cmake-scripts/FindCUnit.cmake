set(CUNIT_SEARCH_PATHS /usr/local/ /usr /opt)

find_path(
  CUNIT_INCLUDE_DIR CUnit/CUnit.h
  HINTS
  PATH_SUFFIXES include
  PATHS ${CUNIT_SEARCH_PATHS})
find_library(
  CUNIT_LIBRARY cunit
  HINTS
  PATH_SUFFIXES lib64 lib bin
  PATHS ${CUNIT_SEARCH_PATHS})

if(CUNIT_INCLUDE_DIR AND CUNIT_LIBRARY)
  set(CUNIT_FOUND TRUE)
endif(CUNIT_INCLUDE_DIR AND CUNIT_LIBRARY)

if(CUNIT_FOUND)
  message(STATUS "Found CUnit: ${CUNIT_LIBRARY}")
else(CUNIT_FOUND)
  message(STATUS "Could not find CUnit. Tests will not be available.")
endif(CUNIT_FOUND)

mark_as_advanced(CUNIT_INCLUDE_DIR CUNIT_LIBRARY CUNIT_SEARCH_PATHS)
