enable_testing()

find_package(CUnit)

IF(CUNIT_FOUND)
    include_directories(${CUNIT_INCLUDE_DIRS} . ../include/)
    set(LIBS ${CUNIT_LIBRARIES} shadowdive)

    add_executable(test_af_reader test_af_reader.c)
    add_executable(test_bk_reader test_bk_reader.c)

    target_link_libraries(test_af_reader ${LIBS})
    target_link_libraries(test_bk_reader ${LIBS})

    add_test(test_af_reader ${CMAKE_CURRENT_BINARY_DIR}/test_af_reader)
    add_test(test_bk_reader ${CMAKE_CURRENT_BINARY_DIR}/test_bk_reader)
ENDIF(CUNIT_FOUND)
