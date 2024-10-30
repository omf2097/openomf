#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

void af_test_suite(CU_pSuite suite);
void bk_test_suite(CU_pSuite suite);
void palette_test_suite(CU_pSuite suite);
void rec_test_suite(CU_pSuite suite);
void trn_test_suite(CU_pSuite suite);
void script_test_suite(CU_pSuite suite);
void str_test_suite(CU_pSuite suite);
void hashmap_test_suite(CU_pSuite suite);
void vector_test_suite(CU_pSuite suite);
void list_test_suite(CU_pSuite suite);
void array_test_suite(CU_pSuite suite);
void text_render_test_suite(CU_pSuite suite);
void cp437_test_suite(CU_pSuite suite);

int main(int argc, char **argv) {
    CU_pSuite suite = NULL;
    int ret = 0;

    if(CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    suite = CU_add_suite("AF files", NULL, NULL);
    if(suite == NULL)
        goto end;
    af_test_suite(suite);

    suite = CU_add_suite("BK files", NULL, NULL);
    if(suite == NULL)
        goto end;
    bk_test_suite(suite);

    suite = CU_add_suite("Palettes", NULL, NULL);
    if(suite == NULL)
        goto end;
    palette_test_suite(suite);

    suite = CU_add_suite("REC files", NULL, NULL);
    if(suite == NULL)
        goto end;
    rec_test_suite(suite);

    suite = CU_add_suite("TRN files", NULL, NULL);
    if(suite == NULL)
        goto end;
    trn_test_suite(suite);

    suite = CU_add_suite("Script", NULL, NULL);
    if(suite == NULL)
        goto end;
    script_test_suite(suite);

    // Init suites
    CU_pSuite str_suite = CU_add_suite("String", NULL, NULL);
    if(str_suite == NULL)
        goto end;
    str_test_suite(str_suite);

    CU_pSuite hashmap_suite = CU_add_suite("Hashmap", NULL, NULL);
    if(hashmap_suite == NULL)
        goto end;
    hashmap_test_suite(hashmap_suite);

    CU_pSuite vector_suite = CU_add_suite("Vector", NULL, NULL);
    if(vector_suite == NULL)
        goto end;
    vector_test_suite(vector_suite);

    CU_pSuite list_suite = CU_add_suite("List", NULL, NULL);
    if(list_suite == NULL)
        goto end;
    list_test_suite(list_suite);

    CU_pSuite array_suite = CU_add_suite("Array", NULL, NULL);
    if(array_suite == NULL)
        goto end;
    array_test_suite(array_suite);

    CU_pSuite text_render_suite = CU_add_suite("Text Renderer", NULL, NULL);
    if(text_render_suite == NULL)
        goto end;
    text_render_test_suite(text_render_suite);

    CU_pSuite cp437_suite = CU_add_suite("Code Page 437", NULL, NULL);
    if(cp437_suite == NULL)
        goto end;
    cp437_test_suite(cp437_suite);

    // Run tests
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

end:
    if(CU_get_number_of_tests_failed() != 0)
        ret = 1;
    CU_ErrorCode cu_err = CU_get_error();
    if(cu_err != CUE_SUCCESS) {
        fprintf(stderr, "CUnit error: %s\n", CU_get_error_msg());
        ret = 1;
    }
    return ret;
}
