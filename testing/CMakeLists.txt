find_package(CUnit)

IF(CUNIT_FOUND)
    include_directories(${CUNIT_INCLUDE_DIR} . ../include/)
    set(LIBS ${CUNIT_LIBRARY})

    add_executable(test_main 
        test_main.c
        test_str.c
        test_hashmap.c
        ../src/utils/hashmap.c
        ../src/utils/str.c
    )

    target_link_libraries(test_main ${LIBS})

    add_custom_target(test test_main)
ENDIF(CUNIT_FOUND)

