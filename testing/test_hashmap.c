#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <utils/hashmap.h>

void test_hashmap_create(void) {
    return;
}

void test_hashmap_free(void) {
    return;
}

void test_hashmap_delete(void) {
    return;
}

void test_hashmap_insert(void) {
    return;
}

void test_hashmap_get(void) {
    return;
}

void test_hashmap_iterator(void) {
    return;
}

void hashmap_test_suite(CU_pSuite suite) {

    
    // Add tests
    if(CU_add_test(suite, "Test for hashmap create", test_hashmap_create) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap delete operation", test_hashmap_delete) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap insert operation", test_hashmap_insert) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap get operation", test_hashmap_get) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap iterator", test_hashmap_iterator) == NULL) { return; }
    if(CU_add_test(suite, "Test for hashmap free operation", test_hashmap_free) == NULL) { return; }

}
