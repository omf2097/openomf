find_package(CUnit)

IF(CUNIT_FOUND)
    include_directories(${CUNIT_INCLUDE_DIR} . ../include/)
    set(LIBS ${CUNIT_LIBRARY} shadowdive)

    add_executable(test_lib test_lib.c)

    target_link_libraries(test_lib ${LIBS})
ENDIF(CUNIT_FOUND)
