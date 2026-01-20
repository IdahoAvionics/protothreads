#include "unity.h"
#include "pt.h"

void setUp(void) {}
void tearDown(void) {}

/* Simple thread that ends */
static PT_THREAD(thread_ends(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_END(pt);
}

/* Simple thread that exits */
static PT_THREAD(thread_exits(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_EXIT(pt);
    PT_END(pt);
}

/* Simple thread that yields once then ends */
static PT_THREAD(thread_yields_once(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_YIELD(pt);
    PT_END(pt);
}

/* Thread that waits */
static int wait_flag;
static PT_THREAD(thread_waits(struct pt *pt)) {
    PT_BEGIN(pt);
    PT_WAIT_UNTIL(pt, wait_flag);
    PT_END(pt);
}

/* Test: PT_SCHEDULE returns true for PT_WAITING */
void test_schedule_true_for_waiting(void) {
    struct pt pt;
    PT_INIT(&pt);
    wait_flag = 0;

    TEST_ASSERT_TRUE(PT_SCHEDULE(thread_waits(&pt)));
}

/* Test: PT_SCHEDULE returns true for PT_YIELDED */
void test_schedule_true_for_yielded(void) {
    struct pt pt;
    PT_INIT(&pt);

    TEST_ASSERT_TRUE(PT_SCHEDULE(thread_yields_once(&pt)));
}

/* Test: PT_SCHEDULE returns false for PT_EXITED */
void test_schedule_false_for_exited(void) {
    struct pt pt;
    PT_INIT(&pt);

    TEST_ASSERT_FALSE(PT_SCHEDULE(thread_exits(&pt)));
}

/* Test: PT_SCHEDULE returns false for PT_ENDED */
void test_schedule_false_for_ended(void) {
    struct pt pt;
    PT_INIT(&pt);

    TEST_ASSERT_FALSE(PT_SCHEDULE(thread_ends(&pt)));
}

/* Test: Scheduling loop pattern works */
void test_scheduling_loop_pattern(void) {
    struct pt pt;
    PT_INIT(&pt);
    int schedule_count = 0;

    while (PT_SCHEDULE(thread_yields_once(&pt))) {
        schedule_count++;
        if (schedule_count > 10) break; /* Safety limit */
    }

    TEST_ASSERT_EQUAL_INT(1, schedule_count);
}

/* Child thread for spawn tests */
static int child_step;
static PT_THREAD(child_thread(struct pt *pt)) {
    PT_BEGIN(pt);
    child_step = 1;
    PT_YIELD(pt);
    child_step = 2;
    PT_YIELD(pt);
    child_step = 3;
    PT_END(pt);
}

/* Parent thread that spawns child */
static int parent_step;
static PT_THREAD(parent_spawns_child(struct pt *pt)) {
    static struct pt child_pt;
    PT_BEGIN(pt);
    parent_step = 1;
    PT_SPAWN(pt, &child_pt, child_thread(&child_pt));
    parent_step = 2;
    PT_END(pt);
}

/* Test: PT_SPAWN initializes child and waits until complete */
void test_spawn_waits_for_child(void) {
    struct pt pt;
    PT_INIT(&pt);
    parent_step = 0;
    child_step = 0;

    int result;
    int iterations = 0;

    do {
        result = parent_spawns_child(&pt);
        iterations++;
        if (iterations > 20) break; /* Safety limit */
    } while (PT_SCHEDULE(result));

    TEST_ASSERT_EQUAL_INT(2, parent_step);
    TEST_ASSERT_EQUAL_INT(3, child_step);
}

/* Child that exits early */
static PT_THREAD(child_exits_early(struct pt *pt)) {
    PT_BEGIN(pt);
    child_step = 1;
    PT_EXIT(pt);
    child_step = 99; /* Should never reach here */
    PT_END(pt);
}

/* Parent that uses PT_WAIT_THREAD with manual init */
static PT_THREAD(parent_wait_thread(struct pt *pt)) {
    static struct pt child_pt;
    PT_BEGIN(pt);
    parent_step = 1;
    PT_INIT(&child_pt);
    PT_WAIT_THREAD(pt, child_exits_early(&child_pt));
    parent_step = 2;
    PT_END(pt);
}

/* Test: PT_WAIT_THREAD requires manual PT_INIT */
void test_wait_thread_with_manual_init(void) {
    struct pt pt;
    PT_INIT(&pt);
    parent_step = 0;
    child_step = 0;

    int result;
    int iterations = 0;

    do {
        result = parent_wait_thread(&pt);
        iterations++;
        if (iterations > 20) break;
    } while (PT_SCHEDULE(result));

    TEST_ASSERT_EQUAL_INT(2, parent_step);
    TEST_ASSERT_EQUAL_INT(1, child_step);
}

/* Test: Child PT_EXIT unblocks parent */
void test_child_exit_unblocks_parent(void) {
    struct pt pt;
    PT_INIT(&pt);
    parent_step = 0;
    child_step = 0;

    while (PT_SCHEDULE(parent_wait_thread(&pt))) {
        /* Keep scheduling */
    }

    /* Parent should have continued after child exited */
    TEST_ASSERT_EQUAL_INT(2, parent_step);
}

/* Multiple independent protothreads */
static int thread_a_count, thread_b_count;

static PT_THREAD(thread_a(struct pt *pt)) {
    PT_BEGIN(pt);
    thread_a_count++;
    PT_YIELD(pt);
    thread_a_count++;
    PT_YIELD(pt);
    thread_a_count++;
    PT_END(pt);
}

static PT_THREAD(thread_b(struct pt *pt)) {
    PT_BEGIN(pt);
    thread_b_count++;
    PT_YIELD(pt);
    thread_b_count++;
    PT_END(pt);
}

/* Test: Multiple protothreads can be scheduled fairly */
void test_multiple_threads_scheduled_fairly(void) {
    struct pt pt_a, pt_b;
    PT_INIT(&pt_a);
    PT_INIT(&pt_b);
    thread_a_count = 0;
    thread_b_count = 0;

    int running_a = 1, running_b = 1;
    int iterations = 0;

    while ((running_a || running_b) && iterations < 20) {
        if (running_a) {
            running_a = PT_SCHEDULE(thread_a(&pt_a));
        }
        if (running_b) {
            running_b = PT_SCHEDULE(thread_b(&pt_b));
        }
        iterations++;
    }

    TEST_ASSERT_EQUAL_INT(3, thread_a_count);
    TEST_ASSERT_EQUAL_INT(2, thread_b_count);
}

/* Nested child threads (grandchild) */
static int grandchild_step;

static PT_THREAD(grandchild_thread(struct pt *pt)) {
    PT_BEGIN(pt);
    grandchild_step = 1;
    PT_YIELD(pt);
    grandchild_step = 2;
    PT_END(pt);
}

static PT_THREAD(child_spawns_grandchild(struct pt *pt)) {
    static struct pt grandchild_pt;
    PT_BEGIN(pt);
    child_step = 1;
    PT_SPAWN(pt, &grandchild_pt, grandchild_thread(&grandchild_pt));
    child_step = 2;
    PT_END(pt);
}

static PT_THREAD(parent_spawns_child_with_grandchild(struct pt *pt)) {
    static struct pt child_pt;
    PT_BEGIN(pt);
    parent_step = 1;
    PT_SPAWN(pt, &child_pt, child_spawns_grandchild(&child_pt));
    parent_step = 2;
    PT_END(pt);
}

/* Test: Nested PT_SPAWN (grandchild) works */
void test_nested_spawn_grandchild(void) {
    struct pt pt;
    PT_INIT(&pt);
    parent_step = 0;
    child_step = 0;
    grandchild_step = 0;

    while (PT_SCHEDULE(parent_spawns_child_with_grandchild(&pt))) {
        /* Keep scheduling */
    }

    TEST_ASSERT_EQUAL_INT(2, parent_step);
    TEST_ASSERT_EQUAL_INT(2, child_step);
    TEST_ASSERT_EQUAL_INT(2, grandchild_step);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_schedule_true_for_waiting);
    RUN_TEST(test_schedule_true_for_yielded);
    RUN_TEST(test_schedule_false_for_exited);
    RUN_TEST(test_schedule_false_for_ended);
    RUN_TEST(test_scheduling_loop_pattern);
    RUN_TEST(test_spawn_waits_for_child);
    RUN_TEST(test_wait_thread_with_manual_init);
    RUN_TEST(test_child_exit_unblocks_parent);
    RUN_TEST(test_multiple_threads_scheduled_fairly);
    RUN_TEST(test_nested_spawn_grandchild);
    return UNITY_END();
}
