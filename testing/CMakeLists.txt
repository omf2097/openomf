find_package(CUnit)

IF(CUNIT_FOUND)
    include_directories(${CUNIT_INCLUDE_DIR} . ../include/)
    set(LIBS ${CUNIT_LIBRARY})

    add_executable(test_main 
        test_main.c
        test_str.c
        test_hashmap.c
        ../src/utils/hashmap.c
        ../src/utils/iterator.c
        ../src/utils/str.c
    )
    
    # On unix platforms, add libm (sometimes needed, it seems)
    IF(UNIX)
        SET(LIBS ${LIBS} -lm)
    ENDIF(UNIX)

    target_link_libraries(test_main ${LIBS})

    add_custom_target(test test_main)
ENDIF(CUNIT_FOUND)

