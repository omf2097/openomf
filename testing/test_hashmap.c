#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <utils/hashmap.h>

void test_hashmap_create(void) {
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
    if(CU_add_test(suite, "test of hashmap_create", test_hashmap_create) == NULL) { goto end; }

    // Run tests
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
   
end:
    CU_cleanup_registry();
    return CU_get_error();
}
