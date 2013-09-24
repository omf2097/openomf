#include "resources/af_loader.h"
#include <shadowdive/shadowdive.h>

int load_af_file(af *a, const char *filename) {
	// Load up AF file from libSD
	sd_af_file *tmp = sd_af_create();
	if(sd_af_load(tmp, filename) != SD_SUCCESS) {
		sd_af_delete(tmp);
		return 1;
	}

	// Convert
	af_create(a, tmp);
	sd_af_delete(tmp);
	return 0;
}