#include "utils/path.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

void test_path_from_c(void) {
    path p;
    path_from_c(&p, "this/is/a/test");
    CU_ASSERT_STRING_EQUAL(p.buf, "this/is/a/test");

    path_from_c(&p, "C:\\this\\is\\a\\test");
    CU_ASSERT_STRING_EQUAL(p.buf, "C:/this/is/a/test");
}

void test_path_from_parts(void) {
    path p;
    path_from_parts(&p, "this", "is", "a", "test");
    CU_ASSERT_STRING_EQUAL(p.buf, "this/is/a/test");

    path_from_parts(&p, "this", "is", "a", "test.mp3");
    CU_ASSERT_STRING_EQUAL(p.buf, "this/is/a/test.mp3");

    path_from_parts(&p, "/", "this", "is", "a", "test");
    CU_ASSERT_STRING_EQUAL(p.buf, "/this/is/a/test");

    path_from_parts(&p, "C:\\", "this", "is", "a", "test");
    CU_ASSERT_STRING_EQUAL(p.buf, "C:/this/is/a/test");
}

void test_path_create_tmpdir(void) {
    path p;
    CU_ASSERT(path_create_tmpdir(&p) == true);
    CU_ASSERT(p.buf[0] != '\0');
}

void test_path_from_str(void) {
    str t;
    path p;

    str_from_c(&t, "this/is/a/test");
    path_from_str(&p, &t);
    CU_ASSERT_STRING_EQUAL(p.buf, "this/is/a/test");
    str_free(&t);

    str_from_c(&t, "C:\\this\\is\\a\\test");
    path_from_str(&p, &t);
    CU_ASSERT_STRING_EQUAL(p.buf, "C:/this/is/a/test");
    str_free(&t);
}

void test_path_clear(void) {
    path p;
    path_from_c(&p, "/home/user/test.file");
    CU_ASSERT_STRING_EQUAL(p.buf, "/home/user/test.file");
    path_clear(&p);
    CU_ASSERT_STRING_EQUAL(p.buf, "");
}

void test_path_is_set(void) {
    path p;
    path_from_c(&p, "/home/user/test.file");
    CU_ASSERT(path_is_set(&p) == true);
    path_clear(&p);
    CU_ASSERT(path_is_set(&p) == false);
}

void test_path_ext(void) {
    path p;
    str test;

    path_from_c(&p, "/home/user/test.file");
    path_ext(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), ".file");
    str_free(&test);

    path_from_c(&p, "/home/user/test");
    path_ext(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "/home/user/.local/test");
    path_ext(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "/");
    path_ext(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "test.file");
    path_ext(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), ".file");
    str_free(&test);

    path_from_c(&p, ".file");
    path_ext(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "/.file");
    path_ext(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "/.");
    path_ext(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "test");
    path_ext(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);
}

void test_path_parents(void) {
    path p;
    str test;

    path_from_c(&p, "/home/user/test.file");
    path_parents(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "/home/user");
    str_free(&test);

    path_from_c(&p, "/home/user/test");
    path_parents(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "/home/user");
    str_free(&test);

    path_from_c(&p, "/home/user/");
    path_parents(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "/home/user");
    str_free(&test);

    path_from_c(&p, "/");
    path_parents(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "C:/");
    path_parents(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "C:");
    str_free(&test);

    path_from_c(&p, "");
    path_parents(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);
}

void test_path_stem(void) {
    path p;
    str test;

    path_from_c(&p, "/home/user/test.file");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test");
    str_free(&test);

    path_from_c(&p, "/home/user/test");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test");
    str_free(&test);

    path_from_c(&p, "/home/user/.local/test");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test");
    str_free(&test);

    path_from_c(&p, "/home/user/");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "/");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "/.");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), ".");
    str_free(&test);

    path_from_c(&p, "test");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test");
    str_free(&test);

    path_from_c(&p, "test.");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test");
    str_free(&test);

    path_from_c(&p, ".test");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), ".test");
    str_free(&test);

    path_from_c(&p, "/.test");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), ".test");
    str_free(&test);

    path_from_c(&p, "");
    path_stem(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);
}

void test_path_filename(void) {
    path p;
    str test;

    path_from_c(&p, "/home/user/test.file");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test.file");
    str_free(&test);

    path_from_c(&p, "/home/user/test");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test");
    str_free(&test);

    path_from_c(&p, "/home/user/.local/test");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test");
    str_free(&test);

    path_from_c(&p, "/home/user/");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "/");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);

    path_from_c(&p, "/.");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), ".");
    str_free(&test);

    path_from_c(&p, "test");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test");
    str_free(&test);

    path_from_c(&p, "test.");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "test.");
    str_free(&test);

    path_from_c(&p, ".test");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), ".test");
    str_free(&test);

    path_from_c(&p, "/.test");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), ".test");
    str_free(&test);

    path_from_c(&p, "");
    path_filename(&p, &test);
    CU_ASSERT_STRING_EQUAL(str_c(&test), "");
    str_free(&test);
}

static path get_test_dir(void) {
    path base_dir, result;
    str parents;
    if(strlen(__FILE__) == 0) {
        printf("__FILE__ size is 0, unable to test!");
        abort();
    }
    path_from_c(&base_dir, __FILE__);
    path_parents(&base_dir, &parents);
    path_from_str(&result, &parents);
    path_append(&result, "path_tests");
    str_free(&parents);
    return result;
}

void test_path_exists(void) {
    path p;

    p = get_test_dir();
    CU_ASSERT(path_exists(&p) == true);

    p = get_test_dir();
    path_append(&p, "nonexistent");
    CU_ASSERT(path_exists(&p) == false);

    p = get_test_dir();
    path_append(&p, "test1.ext1");
    CU_ASSERT(path_exists(&p) == true);
}

void test_path_is_directory(void) {
    path p;

    p = get_test_dir();
    CU_ASSERT(path_is_directory(&p) == true);

    p = get_test_dir();
    path_append(&p, "nonexistent");
    CU_ASSERT(path_is_directory(&p) == false);

    p = get_test_dir();
    path_append(&p, "test1.ext1");
    CU_ASSERT(path_is_directory(&p) == false);
}

void test_path_is_file(void) {
    path p;

    p = get_test_dir();
    CU_ASSERT(path_is_file(&p) == false);

    p = get_test_dir();
    path_append(&p, "nonexistent");
    CU_ASSERT(path_is_file(&p) == false);

    p = get_test_dir();
    path_append(&p, "test1.ext1");
    CU_ASSERT(path_is_file(&p) == true);
}

void test_path_touch_and_unlink(void) {
    path tmp;
    path_create_tmpdir(&tmp);

    path_append(&tmp, "deleteable.file");
    CU_ASSERT(path_is_file(&tmp) == false);

    CU_ASSERT(path_touch(&tmp) == true);
    CU_ASSERT(path_is_file(&tmp) == true);

    CU_ASSERT(path_unlink(&tmp) == true);
    CU_ASSERT(path_is_file(&tmp) == false);
}

void test_path_mkdir_and_rmdir(void) {
    path tmp;
    path_create_tmpdir(&tmp);

    path_append(&tmp, "deleteable_dir");
    CU_ASSERT(path_is_directory(&tmp) == false);

    CU_ASSERT(path_mkdir(&tmp) == true);
    CU_ASSERT(path_is_directory(&tmp) == true);

    CU_ASSERT(path_rmdir(&tmp) == true);
    CU_ASSERT(path_is_directory(&tmp) == false);
}

void test_path_glob(void) {
    path p = get_test_dir();
    list files;

    list_create(&files);
    path_glob(&p, &files, "*.ext1");
    CU_ASSERT(list_size(&files) == 2);
    list_free(&files);

    list_create(&files);
    path_glob(&p, &files, "*.ext2");
    CU_ASSERT(list_size(&files) == 2);
    list_free(&files);

    list_create(&files);
    path_glob(&p, &files, "test*");
    CU_ASSERT(list_size(&files) == 4);
    list_free(&files);

    list_create(&files);
    path_glob(&p, &files, "test3.*");
    CU_ASSERT(list_size(&files) == 1);

    str tmp;
    str_from_c(&tmp, path_c(list_get(&files, 0)));
    CU_ASSERT(str_ends_with(&tmp, "test3.ext2"));
    str_free(&tmp);

    list_free(&files);
}

void test_path_set_ext(void) {
    path p;
    path_from_c(&p, "/this/is/test/file.ext");
    path_set_ext(&p, ".new");
    CU_ASSERT_STRING_EQUAL(p.buf, "/this/is/test/file.new");

    path_from_c(&p, "/this/is/test/file");
    path_set_ext(&p, ".new");
    CU_ASSERT_STRING_EQUAL(p.buf, "/this/is/test/file.new");

    path_from_c(&p, "/this/is/test/.file");
    path_set_ext(&p, ".new");
    CU_ASSERT_STRING_EQUAL(p.buf, "/this/is/test/.file.new");

    path_from_c(&p, "/this/is/.test/file");
    path_set_ext(&p, ".new");
    CU_ASSERT_STRING_EQUAL(p.buf, "/this/is/.test/file.new");
}

void test_path_dossify_filename(void) {
    path p;
    path_from_c(&p, "/file^name.chr");
    path_dossify_filename(&p);
    CU_ASSERT_STRING_EQUAL(p.buf, "/FILE_NAME.CHR");

    path_from_c(&p, "/name");
    path_dossify_filename(&p);
    CU_ASSERT_STRING_EQUAL(p.buf, "/NAME");
}

void test_path_append(void) {
    path p;
    path_from_c(&p, "/home/openomf");
    path_append(&p, ".local", "share", "test.ogg");
    CU_ASSERT_STRING_EQUAL(p.buf, "/home/openomf/.local/share/test.ogg");
}

#define ADD_TEST(desc, fn)                                                                                             \
    if(CU_add_test(suite, desc, fn) == NULL) {                                                                         \
        return;                                                                                                        \
    }

void path_test_suite(CU_pSuite suite) {
    ADD_TEST("test path_from_c", test_path_from_c);
    ADD_TEST("test path_from_parts", test_path_from_parts);
    ADD_TEST("test path_create_tmpdir", test_path_create_tmpdir);
    ADD_TEST("test path_from_str", test_path_from_str);
    ADD_TEST("test path_clear", test_path_clear);
    ADD_TEST("test path_is_set", test_path_is_set);
    ADD_TEST("test path_ext", test_path_ext);
    ADD_TEST("test path_parents", test_path_parents);
    ADD_TEST("test path_stem", test_path_stem);
    ADD_TEST("test path_filename", test_path_filename);
    ADD_TEST("test path_exists", test_path_exists);
    ADD_TEST("test path_is_directory", test_path_is_directory);
    ADD_TEST("test path_is_file", test_path_is_file);
    ADD_TEST("test path_touch & path_unlink", test_path_touch_and_unlink);
    ADD_TEST("test path_mkdir & path_rmdir", test_path_mkdir_and_rmdir);
    ADD_TEST("test path_glob", test_path_glob);
    ADD_TEST("test path_set_ext", test_path_set_ext);
    ADD_TEST("test path_dossify_filename", test_path_dossify_filename);
    ADD_TEST("test path_append", test_path_append);
}
