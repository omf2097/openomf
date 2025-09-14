/**
 * Common unit-testing tools
 */
#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#define ADD_TEST(desc, fn)                                                                                             \
    if(CU_add_test(suite, desc, fn) == NULL) {                                                                         \
        return;                                                                                                        \
    }

#endif
