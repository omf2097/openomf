find_package(CUnit)

IF(CUNIT_FOUND)
    include_directories(${CUNIT_INCLUDE_DIR} . ../include/)
    set(LIBS ${CUNIT_LIBRARY} shadowdive)

    IF(USE_PNG)
        set(LIBS ${LIBS} ${PNG_LIBRARY} ${ZLIB_LIBRARY})
    ENDIF()

    add_executable(test_lib
        test_lib.c
        test_af.c
        test_bk.c
        test_rec.c
        test_palette.c
        test_trn.c
        test_stringparser.c
    )

    IF(UNIX)
        SET(LIBS ${LIBS} -lm)
    ENDIF(UNIX)

    target_link_libraries(test_lib ${LIBS})
    add_custom_target(test test_lib)
ENDIF(CUNIT_FOUND)
