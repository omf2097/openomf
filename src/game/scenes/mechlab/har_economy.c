#include "game/scenes/mechlab/har_economy.h"
#include "formats/pilot.h"
#include "utils/log.h"
#include "utils/random.h"

void purchase_random_har(sd_pilot *pilot) {
    int32_t possible_purchases[10];
    size_t n = 0;
    for(int32_t i = 0; i < 10; ++i) {
        if(pilot->money >= har_prices[i]) {
            possible_purchases[n] = i;
            ++n;
        }
    }
    assert(n > 0 && "Pilot does not have enough money to purchase any HAR");
    pilot->har_id = possible_purchases[rand_int(n)];
    pilot->money -= har_prices[pilot->har_id];
    log_debug("Pilot %s purchased HAR %u and has %d money left", pilot->name, pilot->har_id, pilot->money);
}
