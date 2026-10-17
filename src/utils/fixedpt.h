/**
 * @file fixedpt.h
 * @brief Fixed point arithmetic for game simulation.
 * @details Wraps the vendored fixedptc library (src/vendored/fixedptc).
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef FIXEDPT_H
#define FIXEDPT_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define FIXEDPT_BITS 32
#define FIXEDPT_WBITS 24

// Drop the upstream string functions as unsafe.
#define fixedpt_str fixedptc_upstream_str
#define fixedpt_cstr fixedptc_upstream_cstr
#include "vendored/fixedptc/fixedptc.h"
#undef fixedpt_str
#undef fixedpt_cstr

#endif // FIXEDPT_H
