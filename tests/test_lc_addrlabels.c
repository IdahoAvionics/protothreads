#include "unity.h"
#include "lc-addrlabels.h"

void setUp(void) {}
void tearDown(void) {}

/* Test: LC_INIT sets lc to NULL */
void test_lc_init_sets_null(void) {
    lc_t lc = (void*)0xDEADBEEF;
    LC_INIT(lc);
    TEST_ASSERT_NULL(lc);
}

/* Test: lc_t is void pointer */
void test_lc_t_is_void_pointer(void) {
    TEST_ASSERT_EQUAL_size_t(sizeof(void*), sizeof(lc_t));
}

/* Test: LC_SET stores non-null address */
void test_lc_set_stores_address(void) {
    lc_t lc;
    LC_INIT(lc);

    LC_RESUME(lc);
    LC_SET(lc);

    TEST_ASSERT_NOT_NULL(lc);
}

/*
 * Simple function using LC macros directly.
 * Uses the yield flag pattern that protothreads uses.
 * The flag is set to 1 on each entry, then set to 0 before LC_SET.
 * After resuming (jumping to label), flag is still 1, so we don't return.
 */
static int lc_function_step;
static int lc_simple_function(lc_t *lc) {
    char yield_flag = 1;
    LC_RESUME(*lc);

    lc_function_step = 1;
    yield_flag = 0; LC_SET(*lc); if (!yield_flag) return 1;

    lc_function_step = 2;
    yield_flag = 0; LC_SET(*lc); if (!yield_flag) return 1;

    lc_function_step = 3;

    LC_END(*lc);
    return 0;
}

/* Test: LC macros enable resumption */
void test_lc_resumption(void) {
    lc_t lc;
    LC_INIT(lc);
    lc_function_step = 0;

    int result = lc_simple_function(&lc);
    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(1, lc_function_step);

    result = lc_simple_function(&lc);
    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(2, lc_function_step);

    result = lc_simple_function(&lc);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(3, lc_function_step);
}

/* Test: Multiple LC_SET at different lines work */
static int multi_set_step;
static int multi_set_function(lc_t *lc) {
    char yield_flag = 1;
    LC_RESUME(*lc);

    multi_set_step = 1;
    yield_flag = 0; LC_SET(*lc); if (!yield_flag) return 1;

    multi_set_step = 2;
    yield_flag = 0; LC_SET(*lc); if (!yield_flag) return 1;

    multi_set_step = 3;
    yield_flag = 0; LC_SET(*lc); if (!yield_flag) return 1;

    multi_set_step = 4;
    yield_flag = 0; LC_SET(*lc); if (!yield_flag) return 1;

    multi_set_step = 5;

    LC_END(*lc);
    return 0;
}

void test_multiple_lc_set(void) {
    lc_t lc;
    LC_INIT(lc);
    multi_set_step = 0;

    multi_set_function(&lc);
    TEST_ASSERT_EQUAL_INT(1, multi_set_step);

    multi_set_function(&lc);
    TEST_ASSERT_EQUAL_INT(2, multi_set_step);

    multi_set_function(&lc);
    TEST_ASSERT_EQUAL_INT(3, multi_set_step);

    multi_set_function(&lc);
    TEST_ASSERT_EQUAL_INT(4, multi_set_step);

    multi_set_function(&lc);
    TEST_ASSERT_EQUAL_INT(5, multi_set_step);
}

/* Test: LC works inside if statements */
static int if_step;
static int lc_with_if(lc_t *lc, int flag) {
    char yield_flag = 1;
    LC_RESUME(*lc);

    if_step = 1;
    if (flag) {
        yield_flag = 0; LC_SET(*lc); if (!yield_flag) return 1;
        if_step = 2;
    } else {
        if_step = 3;
    }
    if_step = 4;

    LC_END(*lc);
    return 0;
}

void test_lc_inside_if(void) {
    lc_t lc;
    LC_INIT(lc);
    if_step = 0;

    int result = lc_with_if(&lc, 1);
    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(1, if_step);

    result = lc_with_if(&lc, 1);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(4, if_step);
}

/* Test: LC works inside for loops */
static int loop_count;
static int lc_with_loop(lc_t *lc) {
    static int i;
    char yield_flag = 1;
    LC_RESUME(*lc);

    for (i = 0; i < 3; i++) {
        loop_count++;
        yield_flag = 0; LC_SET(*lc); if (!yield_flag) return 1;
    }

    LC_END(*lc);
    return 0;
}

void test_lc_inside_loop(void) {
    lc_t lc;
    LC_INIT(lc);
    loop_count = 0;

    lc_with_loop(&lc);
    TEST_ASSERT_EQUAL_INT(1, loop_count);

    lc_with_loop(&lc);
    TEST_ASSERT_EQUAL_INT(2, loop_count);

    lc_with_loop(&lc);
    TEST_ASSERT_EQUAL_INT(3, loop_count);

    int result = lc_with_loop(&lc);
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* Test: LC works inside switch statements (unlike lc-switch!) */
static int switch_step;
static int lc_with_switch(lc_t *lc, int val) {
    char yield_flag = 1;
    LC_RESUME(*lc);

    switch_step = 1;
    switch (val) {
    case 1:
        yield_flag = 0; LC_SET(*lc); if (!yield_flag) return 1;
        switch_step = 2;
        break;
    case 2:
        switch_step = 3;
        break;
    }
    switch_step = 4;

    LC_END(*lc);
    return 0;
}

void test_lc_inside_switch(void) {
    lc_t lc;
    LC_INIT(lc);
    switch_step = 0;

    int result = lc_with_switch(&lc, 1);
    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(1, switch_step);

    result = lc_with_switch(&lc, 1);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(4, switch_step);
}

/* Test: Fresh LC starts at beginning */
void test_fresh_lc_starts_at_beginning(void) {
    lc_t lc;
    LC_INIT(lc);
    lc_function_step = 0;

    lc_simple_function(&lc);
    lc_simple_function(&lc);

    /* Reinitialize and start fresh */
    LC_INIT(lc);
    lc_function_step = 0;

    int result = lc_simple_function(&lc);
    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(1, lc_function_step);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_lc_init_sets_null);
    RUN_TEST(test_lc_t_is_void_pointer);
    RUN_TEST(test_lc_set_stores_address);
    RUN_TEST(test_lc_resumption);
    RUN_TEST(test_multiple_lc_set);
    RUN_TEST(test_lc_inside_if);
    RUN_TEST(test_lc_inside_loop);
    RUN_TEST(test_lc_inside_switch);
    RUN_TEST(test_fresh_lc_starts_at_beginning);
    return UNITY_END();
}
