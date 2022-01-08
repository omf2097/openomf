#include "plugins/scaler_plugin.h"
#include <stdlib.h>

void scaler_init(scaler_plugin *scaler) {
    scaler->base = NULL;
    scaler->is_factor_available = NULL;
    scaler->get_factors_list = NULL;
    scaler->get_color_format = NULL;
    scaler->scale = NULL;
}

int scaler_is_factor_available(scaler_plugin *scaler, int factor) {
    if (scaler->is_factor_available != NULL) {
        return scaler->is_factor_available(factor);
    }
    return 0;
}

int scaler_get_factors_list(scaler_plugin *scaler, int **factors) {
    if (scaler->get_factors_list != NULL) {
        return scaler->get_factors_list(factors);
    }
    return 0;
}

int scaler_get_color_format(scaler_plugin *scaler) {
    if (scaler->get_color_format != NULL) {
        return scaler->get_color_format();
    }
    return 0;
}

int scaler_scale(scaler_plugin *scaler, const char *in, char *out, int w, int h, int factor) {
    if (scaler->scale != NULL) {
        return scaler->scale(in, out, w, h, factor);
    }
    return 1;
}