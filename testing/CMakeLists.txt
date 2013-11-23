find_package(CUnit)

IF(CUNIT_FOUND)
    include_directories(${CUNIT_INCLUDE_DIR} . ../include/)
    set(LIBS ${CUNIT_LIBRARY})

    add_executable(test_hashmap test_hashmap.c ../src/utils/hashmap.c)
    target_link_libraries(test_hashmap ${LIBS})
    add_test(test_hashmap ${EXECUTABLE_OUTPUT_PATH}/test_hashmap)
ENDIF(CUNIT_FOUND)

