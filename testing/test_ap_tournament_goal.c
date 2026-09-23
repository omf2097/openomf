#include "common.h"
#include "archipelago/ap_mechlab.h"
#include "archipelago/ap_sgmanager.h"
#include "archipelago/apconnect.h"
#include "archipelago/apstate.h"
#include "utils/path.h"

#include <string.h>

// Archipelago_GetSaveIdent() no-ops (leaves the buffer empty) when there's no live AP
// connection, which is the case in this test binary. ap_on_tournament_win() therefore
// always saves/loads under the empty ident here -- that's what we clean up afterward.
static void remove_test_state_file(void) {
    path dir = get_ap_save_directory();
    path_append(&dir, "");
    path_set_ext(&dir, ".APS");
    path_unlink(&dir);
}

void test_tournament_win_mask_accumulates_across_all_four(void) {
    memset(&APSave, 0, sizeof(APSave));

    for(int t = 0; t < 4; t++) {
        APTournament.tournament_idx = t;
        ap_on_tournament_win();
    }

    CU_ASSERT_EQUAL(APSave.tournaments_won_mask, 0x0F);
    remove_test_state_file();
}

// Regression test: entering the mechlab right after a tournament win reloads APSave from
// disk (ap_mechlab_find_player -> Archipelago_APLoadState), which used to clobber the mask
// bit ap_on_tournament_win() had just set in memory, since nothing had saved it yet. That
// silently dropped earned tournament wins and made the "All Tournaments" goal unreachable.
// ap_on_tournament_win() must persist the mask itself before returning.
void test_tournament_win_mask_survives_reload_between_wins(void) {
    memset(&APSave, 0, sizeof(APSave));

    APTournament.tournament_idx = 0;
    ap_on_tournament_win();
    APTournament.tournament_idx = 1;
    ap_on_tournament_win();

    // Simulate returning to the mechlab, which reloads AP state from disk.
    Archipelago_APLoadState("");

    CU_ASSERT_EQUAL(APSave.tournaments_won_mask, 0x03);
    remove_test_state_file();
}

void ap_tournament_goal_test_suite(CU_pSuite suite) {
    ADD_TEST("Test tournament win mask accumulates across all four tournaments",
             test_tournament_win_mask_accumulates_across_all_four);
    ADD_TEST("Test tournament win mask survives a mechlab reload between wins",
             test_tournament_win_mask_survives_reload_between_wins);
}
