#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <shadowdive/shadowdive.h>

void af_test_suite(CU_pSuite suite);
void bk_test_suite(CU_pSuite suite);
void palette_test_suite(CU_pSuite suite);
void rec_test_suite(CU_pSuite suite);
void trn_test_suite(CU_pSuite suite);
void stringparser_test_suite(CU_pSuite suite);
void script_test_suite(CU_pSuite suite);
void array_test_suite(CU_pSuite suite);

int main(int argc, char **argv) {
    CU_pSuite suite = NULL;

    if(CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    suite = CU_add_suite("AF files", NULL, NULL);
    if(suite == NULL) goto end;
    af_test_suite(suite);

    suite = CU_add_suite("BK files", NULL, NULL);
    if(suite == NULL) goto end;
    bk_test_suite(suite);

    suite = CU_add_suite("Palettes", NULL, NULL);
    if(suite == NULL) goto end;
    palette_test_suite(suite);

    suite = CU_add_suite("REC files", NULL, NULL);
    if(suite == NULL) goto end;
    rec_test_suite(suite);

    suite = CU_add_suite("TRN files", NULL, NULL);
    if(suite == NULL) goto end;
    trn_test_suite(suite);

    suite = CU_add_suite("Stringparser", NULL, NULL);
    if(suite == NULL) goto end;
    stringparser_test_suite(suite);

    suite = CU_add_suite("Script", NULL, NULL);
    if(suite == NULL) goto end;
    script_test_suite(suite);

    suite = CU_add_suite("Array", NULL, NULL);
    if(suite == NULL) goto end;
    array_test_suite(suite);

    // Run tests
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
   
end:
    CU_cleanup_registry();
    return CU_get_error();
}
