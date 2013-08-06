#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <shadowdive/shadowdive.h>

sd_bk_file *file;

void test_sd_bk_create(void) {
    file = sd_bk_create();
    CU_ASSERT(file != NULL);
}

void test_sd_bk_delete(void) {
    if(file != NULL) {
        sd_bk_delete(file);
        CU_ASSERT(file == NULL);
    }
}

int main(int argc, char **argv) {
    CU_pSuite suite = NULL;

    if(CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    // Init suite
    suite = CU_add_suite("Shadowdive", NULL, NULL);
    if(suite == NULL) {
        goto end;
    }
    
    // Add tests
    if(CU_add_test(suite, "test of sd_bk_create", test_sd_bk_create) == NULL) { goto end; }
    if(CU_add_test(suite, "test of sd_bk_delete", test_sd_bk_delete) == NULL) { goto end; }

    // Run tests
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
   
end:
    CU_cleanup_registry();
    return CU_get_error();
}
