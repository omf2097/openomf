#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <assert.h>

#include "utils/allocator.h"
#include "utils/c_string_util.h"

/* The basename/dirname manpage gives us these examples:
 *   path       dirname   basename
 *   /usr/lib   /usr      lib
 *   /usr/      /         usr
 *   usr        .         usr
 *   /          /         /
 *   .          .         .
 *   ..         .         ..
 */

void test_omf_basename(void) {
    const char *orig_path = "/usr/lib";
    char *path_dup = omf_strdup(orig_path);
    char *result = omf_basename(path_dup);
    CU_ASSERT(strcmp(result, "lib") == 0);
    omf_free(path_dup);

    orig_path = "/usr/";
    path_dup = omf_strdup(orig_path);
    result = omf_basename(path_dup);
    CU_ASSERT(strcmp(result, "usr") == 0);
    omf_free(path_dup);

    orig_path = "usr";
    path_dup = omf_strdup(orig_path);
    result = omf_basename(path_dup);
    CU_ASSERT(strcmp(result, "usr") == 0);
    omf_free(path_dup);

    orig_path = "/";
    path_dup = omf_strdup(orig_path);
    result = omf_basename(path_dup);
    CU_ASSERT(strcmp(result, "/") == 0);
    omf_free(path_dup);

    orig_path = ".";
    path_dup = omf_strdup(orig_path);
    result = omf_basename(path_dup);
    CU_ASSERT(strcmp(result, ".") == 0);
    omf_free(path_dup);

    orig_path = "..";
    path_dup = omf_strdup(orig_path);
    result = omf_basename(path_dup);
    CU_ASSERT(strcmp(result, "..") == 0);
    omf_free(path_dup);
}

void test_omf_dirname(void) {
    const char *orig_path = "/usr/lib";
    char *path_dup = omf_strdup(orig_path);
    char *result = omf_dirname(path_dup);
    CU_ASSERT(strcmp(result, "/usr") == 0);
    omf_free(path_dup);

    orig_path = "/usr/";
    path_dup = omf_strdup(orig_path);
    result = omf_dirname(path_dup);
    CU_ASSERT(strcmp(result, "/") == 0);
    omf_free(path_dup);

    orig_path = "usr";
    path_dup = omf_strdup(orig_path);
    result = omf_dirname(path_dup);
    CU_ASSERT(strcmp(result, ".") == 0);
    omf_free(path_dup);

    orig_path = ".";
    path_dup = omf_strdup(orig_path);
    result = omf_dirname(path_dup);
    CU_ASSERT(strcmp(result, ".") == 0);
    omf_free(path_dup);

    orig_path = "..";
    path_dup = omf_strdup(orig_path);
    result = omf_dirname(path_dup);
    CU_ASSERT(strcmp(result, ".") == 0);
    omf_free(path_dup);
}

void c_string_util_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "Test omf_basename", test_omf_basename) == NULL) {
        return;
    }
    if(CU_add_test(suite, "Test omf_dirname", test_omf_dirname) == NULL) {
        return;
    }
}
