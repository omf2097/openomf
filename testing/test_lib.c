#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <shadowdive/shadowdive.h>

sd_bk_file *bk_file;
sd_af_file *af_file;

void test_sd_bk_create(void) {
    bk_file = sd_bk_create();
    CU_ASSERT_PTR_NOT_NULL_FATAL(bk_file);
}

void test_sd_bk_delete(void) {
    sd_bk_delete(bk_file);
    //CU_ASSERT_PTR_NULL(bk_file);
}

void test_sd_af_create(void) {
    af_file = sd_af_create();
    CU_ASSERT_PTR_NOT_NULL_FATAL(af_file);
}

void test_sd_af_delete(void) {
    sd_af_delete(af_file);
    //CU_ASSERT_PTR_NULL(af_file);
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
    if(CU_add_test(suite, "test of sd_af_create", test_sd_af_create) == NULL) { goto end; }
    if(CU_add_test(suite, "test of sd_af_delete", test_sd_af_delete) == NULL) { goto end; }

    // Run tests
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
   
end:
    CU_cleanup_registry();
    return CU_get_error();
}
