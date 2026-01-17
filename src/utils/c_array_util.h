/**
 * @file c_array_util.h
 * @brief Utility macros for working with C arrays.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef C_ARRAY_UTIL_H
#define C_ARRAY_UTIL_H

/**
 * @brief Get the number of elements in a static C array.
 * @details This macro divides the total size of the array by the size of one element.
 *          Only works with actual arrays, not pointers.
 * @param array A static C array (not a pointer)
 * @return The number of elements in the array
 */
#define N_ELEMENTS(array) (sizeof(array) / sizeof((array)[0]))

#endif // C_ARRAY_UTIL_H
