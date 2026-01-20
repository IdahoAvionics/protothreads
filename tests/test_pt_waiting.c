#include "unity.h"
#include "pt.h"

void setUp(void) {}
void tearDown(void) {}

/* Condition flag for testing */
static int condition_flag;

/* Protothread that waits until condition_flag is true */
static PT_THREAD(thread_wait_until(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, condition_flag);
    PT_END(pt);
}

/* Test: PT_WAIT_UNTIL blocks when condition is false */
void test_wait_until_blocks_when_false(void) {
    struct pt pt;
    PT_INIT(&pt);
    condition_flag = 0;

    int result = thread_wait_until(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);
}

/* Test: PT_WAIT_UNTIL continues when condition is true */
void test_wait_until_continues_when_true(void) {
    struct pt pt;
    PT_INIT(&pt);
    condition_flag = 1;

    int result = thread_wait_until(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
}

/* Test: PT_WAIT_UNTIL resumes after condition becomes true */
void test_wait_until_resumes_after_condition_true(void) {
    struct pt pt;
    PT_INIT(&pt);
    condition_flag = 0;

    int result = thread_wait_until(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);

    condition_flag = 1;
    result = thread_wait_until(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
}

/* Protothread that waits while condition_flag is true */
static PT_THREAD(thread_wait_while(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_WAIT_WHILE(pt, condition_flag);
    PT_END(pt);
}

/* Test: PT_WAIT_WHILE blocks when condition is true */
void test_wait_while_blocks_when_true(void) {
    struct pt pt;
    PT_INIT(&pt);
    condition_flag = 1;

    int result = thread_wait_while(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);
}

/* Test: PT_WAIT_WHILE continues when condition is false */
void test_wait_while_continues_when_false(void) {
    struct pt pt;
    PT_INIT(&pt);
    condition_flag = 0;

    int result = thread_wait_while(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
}

/* Protothread that yields once */
static int yield_progress;
static PT_THREAD(thread_yield(struct pt *pt)) {
    PT_BEGIN(pt);
    yield_progress = 1;
    PT_YIELD(pt);
    yield_progress = 2;
    PT_YIELD(pt);
    yield_progress = 3;
    PT_END(pt);
}

/* Test: PT_YIELD returns PT_YIELDED */
void test_yield_returns_yielded(void) {
    struct pt pt;
    PT_INIT(&pt);
    yield_progress = 0;

    int result = thread_yield(&pt);
    TEST_ASSERT_EQUAL_INT(PT_YIELDED, result);
    TEST_ASSERT_EQUAL_INT(1, yield_progress);
}

/* Test: PT_YIELD gives control back and resumes correctly */
void test_yield_resumes_correctly(void) {
    struct pt pt;
    PT_INIT(&pt);
    yield_progress = 0;

    thread_yield(&pt);
    TEST_ASSERT_EQUAL_INT(1, yield_progress);

    thread_yield(&pt);
    TEST_ASSERT_EQUAL_INT(2, yield_progress);

    int result = thread_yield(&pt);
    TEST_ASSERT_EQUAL_INT(3, yield_progress);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
}

/* Protothread that yields until condition */
static PT_THREAD(thread_yield_until(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_YIELD_UNTIL(pt, condition_flag);
    PT_END(pt);
}

/* Test: PT_YIELD_UNTIL yields at least once even if condition is true */
void test_yield_until_yields_once_even_if_true(void) {
    struct pt pt;
    PT_INIT(&pt);
    condition_flag = 1;

    int result = thread_yield_until(&pt);
    /* First call always yields */
    TEST_ASSERT_EQUAL_INT(PT_YIELDED, result);

    /* Second call continues since condition is true */
    result = thread_yield_until(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
}

/* Test: PT_YIELD_UNTIL keeps yielding while condition is false */
void test_yield_until_keeps_yielding_while_false(void) {
    struct pt pt;
    PT_INIT(&pt);
    condition_flag = 0;

    int result = thread_yield_until(&pt);
    TEST_ASSERT_EQUAL_INT(PT_YIELDED, result);

    result = thread_yield_until(&pt);
    TEST_ASSERT_EQUAL_INT(PT_YIELDED, result);

    condition_flag = 1;
    result = thread_yield_until(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
}

/* Multiple PT_WAIT_UNTIL in same function */
static int step;
static PT_THREAD(thread_multiple_waits(struct pt *pt)) {
    PT_BEGIN(pt);
    step = 1;
    PT_WAIT_UNTIL(pt, condition_flag >= 1);
    step = 2;
    PT_WAIT_UNTIL(pt, condition_flag >= 2);
    step = 3;
    PT_WAIT_UNTIL(pt, condition_flag >= 3);
    step = 4;
    PT_END(pt);
}

/* Test: Multiple PT_WAIT_UNTIL work independently */
void test_multiple_wait_until_work_independently(void) {
    struct pt pt;
    PT_INIT(&pt);
    step = 0;
    condition_flag = 0;

    int result = thread_multiple_waits(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);
    TEST_ASSERT_EQUAL_INT(1, step);

    condition_flag = 1;
    result = thread_multiple_waits(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);
    TEST_ASSERT_EQUAL_INT(2, step);

    condition_flag = 2;
    result = thread_multiple_waits(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);
    TEST_ASSERT_EQUAL_INT(3, step);

    condition_flag = 3;
    result = thread_multiple_waits(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
    TEST_ASSERT_EQUAL_INT(4, step);
}

/* Test: Complex condition expressions work */
static int a, b;
static PT_THREAD(thread_complex_condition(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, (a > 5 && b < 10) || a == 100);
    PT_END(pt);
}

void test_complex_condition_expression(void) {
    struct pt pt;
    PT_INIT(&pt);
    a = 0;
    b = 0;

    int result = thread_complex_condition(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);

    a = 6;
    b = 5;
    result = thread_complex_condition(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_wait_until_blocks_when_false);
    RUN_TEST(test_wait_until_continues_when_true);
    RUN_TEST(test_wait_until_resumes_after_condition_true);
    RUN_TEST(test_wait_while_blocks_when_true);
    RUN_TEST(test_wait_while_continues_when_false);
    RUN_TEST(test_yield_returns_yielded);
    RUN_TEST(test_yield_resumes_correctly);
    RUN_TEST(test_yield_until_yields_once_even_if_true);
    RUN_TEST(test_yield_until_keeps_yielding_while_false);
    RUN_TEST(test_multiple_wait_until_work_independently);
    RUN_TEST(test_complex_condition_expression);
    return UNITY_END();
}
