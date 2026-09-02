#include "common.h"
#include "archipelago/apconnect.h"

void test_ap_money_award_first_check(void) {
    CU_ASSERT_EQUAL(ap_money_award(3000, 150, 0), 3000);
}

void test_ap_money_award_grows_with_count(void) {
    CU_ASSERT_EQUAL(ap_money_award(3000, 150, 1), 3150);
    CU_ASSERT_EQUAL(ap_money_award(3000, 150, 4), 3600);
}

void test_ap_money_award_flat_when_step_zero(void) {
    CU_ASSERT_EQUAL(ap_money_award(15000, 0, 0), 15000);
    CU_ASSERT_EQUAL(ap_money_award(15000, 0, 50), 15000);
}

void ap_money_test_suite(CU_pSuite suite) {
    ADD_TEST("Test ap_money_award first check equals base", test_ap_money_award_first_check);
    ADD_TEST("Test ap_money_award grows with received count", test_ap_money_award_grows_with_count);
    ADD_TEST("Test ap_money_award stays flat when step is zero", test_ap_money_award_flat_when_step_zero);
}
