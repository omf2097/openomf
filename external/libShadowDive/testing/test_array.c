#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "shadowdive/internal/array.h"
#include <shadowdive/shadowdive.h>

int *array = NULL;
int array_size = 5;

void test_array_create(void) {
    sd_array_create((void*)&array, sizeof(int), array_size);
    CU_ASSERT(array != NULL);
    for(int i = 0; i < array_size; i++) {
        array[i] = i;
    }
}

void test_array_push(void) {
    int new = 5;
    sd_array_resize((void**)&array, sizeof(int), array_size+1);
    sd_array_push((void*)array, sizeof(int), &array_size, &new);
    CU_ASSERT(array_size == 6);
    CU_ASSERT(array[5] = 5);
}

void test_array_pop(void) {
    int old = 0;
    sd_array_pop((void*)array, sizeof(int), &array_size, (void**)&old);
    sd_array_resize((void**)&array, sizeof(int), array_size);
    CU_ASSERT(old == 5);
    CU_ASSERT(array_size == 5);
}

void test_array_insert(void) {
    int new_item = -1;

    // Add item to start
    sd_array_resize((void**)&array, sizeof(int), array_size+1);
    sd_array_insert((void*)array, sizeof(int), &array_size, 0, &new_item);
    CU_ASSERT(array_size == 6);
    for(int i = 0; i < array_size; i++) {
        CU_ASSERT(array[i] == i-1);
    }

    // Add item to middle
    sd_array_resize((void**)&array, sizeof(int), array_size+1);
    sd_array_insert((void*)array, sizeof(int), &array_size, 3, &new_item);
    CU_ASSERT(array_size == 7);
    for(int i = 0; i < array_size; i++) {
        if(i == 3) {
            CU_ASSERT(array[i] == -1)
        } else if(i > 3) {
            CU_ASSERT(array[i] == i-2);
        } else {
            CU_ASSERT(array[i] == i-1);
        }
    }
}

void test_array_delete(void) {
    sd_array_delete((void*)array, sizeof(int), &array_size, 3);
    CU_ASSERT(array_size == 6);
    for(int i = 0; i < array_size; i++) {
        CU_ASSERT(array[i] == i-1);
    }

    sd_array_delete((void*)array, sizeof(int), &array_size, 0);
    CU_ASSERT(array_size == 5);
    for(int i = 0; i < array_size; i++) {
        CU_ASSERT(array[i] == i);
    }

    sd_array_resize((void**)&array, sizeof(int), array_size);
}

void test_array_free(void) {
    sd_array_free((void*)&array);
    CU_ASSERT(array == NULL);
}

void array_test_suite(CU_pSuite suite) {
    if(CU_add_test(suite, "test of sd_array_create", test_array_create) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_array_push", test_array_push) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_array_pop", test_array_pop) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_array_insert", test_array_insert) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_array_delete", test_array_delete) == NULL) { return; }
    if(CU_add_test(suite, "test of sd_array_free", test_array_free) == NULL) { return; }
}
