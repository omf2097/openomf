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

int main(int argc, char **argv) {
    CU_pSuite suite = NULL;
    if(CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    // Init suite
    suite = CU_add_suite("Hashmap", NULL, NULL);
    if(suite == NULL) {
        goto end;
    }
    
    // Add tests
    if(CU_add_test(suite, "Test for hashmap create", test_hashmap_create) == NULL) { goto end; }
    if(CU_add_test(suite, "Test for hashmap delete operation", test_hashmap_delete) == NULL) { goto end; }
    if(CU_add_test(suite, "Test for hashmap insert operation", test_hashmap_insert) == NULL) { goto end; }
    if(CU_add_test(suite, "Test for hashmap get operation", test_hashmap_get) == NULL) { goto end; }
    if(CU_add_test(suite, "Test for hashmap iterator", test_hashmap_iterator) == NULL) { goto end; }
    if(CU_add_test(suite, "Test for hashmap free operation", test_hashmap_free) == NULL) { goto end; }
    
    // Run tests
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
   
end:
    CU_cleanup_registry();
    return CU_get_error();
}
