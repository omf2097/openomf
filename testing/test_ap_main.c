#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "utils/log.h"

void ap_money_test_suite(CU_pSuite suite);
void ap_tournament_goal_test_suite(CU_pSuite suite);

int main(int argc, char **argv) {
    int ret = 0;

    log_init();

    if(CU_initialize_registry() != CUE_SUCCESS) {
        log_close();
        return CU_get_error();
    }

    CU_pSuite ap_money_suite = CU_add_suite("AP Money", NULL, NULL);
    if(ap_money_suite == NULL) {
        goto end;
    }
    ap_money_test_suite(ap_money_suite);

    CU_pSuite ap_tournament_goal_suite = CU_add_suite("AP Tournament Goal", NULL, NULL);
    if(ap_tournament_goal_suite == NULL) {
        goto end;
    }
    ap_tournament_goal_test_suite(ap_tournament_goal_suite);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

end:
    if(CU_get_number_of_tests_failed() != 0) {
        ret = 1;
    }
    CU_ErrorCode cu_err = CU_get_error();
    if(cu_err != CUE_SUCCESS) {
        fprintf(stderr, "CUnit error: %s\n", CU_get_error_msg());
        ret = 1;
    }
    log_close();
    return ret;
}
