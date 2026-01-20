#include "unity.h"
#include "pt.h"

void setUp(void) {}
void tearDown(void) {}

/* Test: PT_WAITING equals 0 */
void test_pt_waiting_value(void) {
    TEST_ASSERT_EQUAL_INT(0, PT_WAITING);
}

/* Test: PT_YIELDED equals 1 */
void test_pt_yielded_value(void) {
    TEST_ASSERT_EQUAL_INT(1, PT_YIELDED);
}

/* Test: PT_EXITED equals 2 */
void test_pt_exited_value(void) {
    TEST_ASSERT_EQUAL_INT(2, PT_EXITED);
}

/* Test: PT_ENDED equals 3 */
void test_pt_ended_value(void) {
    TEST_ASSERT_EQUAL_INT(3, PT_ENDED);
}

/* Test: PT_INIT initializes lc to initial state */
void test_pt_init_sets_lc(void) {
    struct pt pt;
    pt.lc = 999; /* Set to non-zero value first */
    PT_INIT(&pt);
    /* After init, lc should be 0 (for switch) or NULL (for addrlabels) */
    TEST_ASSERT_EQUAL_INT(0, (int)(size_t)pt.lc);
}

/* Test: PT_INIT can be called multiple times safely */
void test_pt_init_multiple_times(void) {
    struct pt pt;
    PT_INIT(&pt);
    PT_INIT(&pt);
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL_INT(0, (int)(size_t)pt.lc);
}

/* Protothread that ends immediately */
static PT_THREAD(thread_ends_immediately(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_END(pt);
}

/* Test: PT_END returns PT_ENDED */
void test_pt_end_returns_ended(void) {
    struct pt pt;
    PT_INIT(&pt);
    int result = thread_ends_immediately(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
}

/* Test: PT_END reinitializes protothread (can be run again) */
void test_pt_end_reinitializes(void) {
    struct pt pt;
    PT_INIT(&pt);
    thread_ends_immediately(&pt);
    /* Should be able to run again after PT_END reinitialized it */
    int result = thread_ends_immediately(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
}

/* Protothread that exits immediately */
static PT_THREAD(thread_exits_immediately(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_EXIT(pt);
    PT_END(pt);
}

/* Test: PT_EXIT returns PT_EXITED */
void test_pt_exit_returns_exited(void) {
    struct pt pt;
    PT_INIT(&pt);
    int result = thread_exits_immediately(&pt);
    TEST_ASSERT_EQUAL_INT(PT_EXITED, result);
}

/* Test: PT_EXIT reinitializes protothread */
void test_pt_exit_reinitializes(void) {
    struct pt pt;
    PT_INIT(&pt);
    thread_exits_immediately(&pt);
    /* Should be able to run again after PT_EXIT reinitialized it */
    int result = thread_exits_immediately(&pt);
    TEST_ASSERT_EQUAL_INT(PT_EXITED, result);
}

/* Protothread that restarts */
static int restart_count;
static PT_THREAD(thread_restarts_once(struct pt *pt)) {
    PT_BEGIN(pt);
    restart_count++;
    if (restart_count == 1) {
        PT_RESTART(pt);
    }
    PT_END(pt);
}

/* Test: PT_RESTART returns PT_WAITING and causes restart at PT_BEGIN */
void test_pt_restart_returns_waiting(void) {
    struct pt pt;
    PT_INIT(&pt);
    restart_count = 0;

    int result = thread_restarts_once(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);
    TEST_ASSERT_EQUAL_INT(1, restart_count);

    /* After restart, runs from beginning again */
    result = thread_restarts_once(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
    TEST_ASSERT_EQUAL_INT(2, restart_count);
}

/* Protothread with static variable that persists */
static PT_THREAD(thread_with_static_var(struct pt *pt, int *out_value)) {
    static int counter;
    PT_BEGIN(pt);
    counter++;
    *out_value = counter;
    PT_YIELD(pt);
    counter++;
    *out_value = counter;
    PT_END(pt);
}

/* Test: Static variables persist across yields */
void test_static_variables_persist(void) {
    struct pt pt;
    int value = 0;
    PT_INIT(&pt);

    thread_with_static_var(&pt, &value);
    TEST_ASSERT_EQUAL_INT(1, value);

    thread_with_static_var(&pt, &value);
    TEST_ASSERT_EQUAL_INT(2, value);
}

/* Test: Code before PT_BEGIN executes on every schedule */
static int before_begin_count;
static PT_THREAD(thread_code_before_begin(struct pt *pt)) {
    before_begin_count++;
    PT_BEGIN(pt);
    PT_YIELD(pt);
    PT_YIELD(pt);
    PT_END(pt);
}

void test_code_before_begin_executes_each_schedule(void) {
    struct pt pt;
    PT_INIT(&pt);
    before_begin_count = 0;

    thread_code_before_begin(&pt); /* First yield */
    TEST_ASSERT_EQUAL_INT(1, before_begin_count);

    thread_code_before_begin(&pt); /* Second yield */
    TEST_ASSERT_EQUAL_INT(2, before_begin_count);

    thread_code_before_begin(&pt); /* End */
    TEST_ASSERT_EQUAL_INT(3, before_begin_count);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_pt_waiting_value);
    RUN_TEST(test_pt_yielded_value);
    RUN_TEST(test_pt_exited_value);
    RUN_TEST(test_pt_ended_value);
    RUN_TEST(test_pt_init_sets_lc);
    RUN_TEST(test_pt_init_multiple_times);
    RUN_TEST(test_pt_end_returns_ended);
    RUN_TEST(test_pt_end_reinitializes);
    RUN_TEST(test_pt_exit_returns_exited);
    RUN_TEST(test_pt_exit_reinitializes);
    RUN_TEST(test_pt_restart_returns_waiting);
    RUN_TEST(test_static_variables_persist);
    RUN_TEST(test_code_before_begin_executes_each_schedule);
    return UNITY_END();
}
