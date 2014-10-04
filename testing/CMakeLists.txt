find_package(CUnit)

IF(CUNIT_FOUND)
    include_directories(${CUNIT_INCLUDE_DIR} . ../include/)
    set(LIBS ${CUNIT_LIBRARY})

    add_executable(openomf_test_main
        test_main.c
        test_str.c
        test_hashmap.c
        test_vector.c
        test_list.c
        test_array.c
        ../src/utils/hashmap.c
        ../src/utils/vector.c
        ../src/utils/iterator.c
        ../src/utils/str.c
        ../src/utils/list.c
        ../src/utils/array.c
    )

    # On unix platforms, add libm (sometimes needed, it seems)
    IF(UNIX)
        SET(LIBS ${LIBS} -lm)
    ENDIF(UNIX)

    target_link_libraries(openomf_test_main ${LIBS})

    # Change policy to allow "test" target name
    cmake_policy(PUSH)
    if(POLICY CMP0037)
        cmake_policy(SET CMP0037 OLD)
    endif()
    add_custom_target(test openomf_test_main)
    cmake_policy(POP)
ENDIF(CUNIT_FOUND)

