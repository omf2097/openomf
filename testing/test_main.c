#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

void str_test_suite(CU_pSuite suite);
void hashmap_test_suite(CU_pSuite suite);

int main(int argc, char **argv) {
    if(CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    // Init suites
    CU_pSuite str_suite = CU_add_suite("String", NULL, NULL);
    if(str_suite == NULL) goto end;
    str_test_suite(str_suite);
    
    CU_pSuite hashmap_suite = CU_add_suite("Hashmap", NULL, NULL);
    if(hashmap_suite == NULL) goto end;
    hashmap_test_suite(hashmap_suite);

    // Run tests
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
   
end:
    CU_cleanup_registry();
    return CU_get_error();
}
