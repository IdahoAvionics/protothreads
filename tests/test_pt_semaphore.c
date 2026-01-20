#include "unity.h"
#include "pt-sem.h"

void setUp(void) {}
void tearDown(void) {}

/* Test: PT_SEM_INIT sets count correctly */
void test_sem_init_sets_count(void) {
    struct pt_sem sem;

    PT_SEM_INIT(&sem, 0);
    TEST_ASSERT_EQUAL_UINT(0, sem.count);

    PT_SEM_INIT(&sem, 1);
    TEST_ASSERT_EQUAL_UINT(1, sem.count);

    PT_SEM_INIT(&sem, 100);
    TEST_ASSERT_EQUAL_UINT(100, sem.count);
}

/* Test: PT_SEM_SIGNAL increments count */
void test_sem_signal_increments_count(void) {
    struct pt_sem sem;
    struct pt pt;

    PT_SEM_INIT(&sem, 0);
    PT_SEM_SIGNAL(&pt, &sem);
    TEST_ASSERT_EQUAL_UINT(1, sem.count);

    PT_SEM_SIGNAL(&pt, &sem);
    TEST_ASSERT_EQUAL_UINT(2, sem.count);

    PT_SEM_SIGNAL(&pt, &sem);
    TEST_ASSERT_EQUAL_UINT(3, sem.count);
}

/* Thread that waits on semaphore */
static struct pt_sem test_sem;
static int after_wait;

static PT_THREAD(thread_sem_wait(struct pt *pt)) {
    PT_BEGIN(pt);
    after_wait = 0;
    PT_SEM_WAIT(pt, &test_sem);
    after_wait = 1;
    PT_END(pt);
}

/* Test: PT_SEM_WAIT blocks when count is 0 */
void test_sem_wait_blocks_when_zero(void) {
    struct pt pt;
    PT_INIT(&pt);
    PT_SEM_INIT(&test_sem, 0);
    after_wait = -1;

    int result = thread_sem_wait(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);
    TEST_ASSERT_EQUAL_INT(0, after_wait);
}

/* Test: PT_SEM_WAIT continues when count > 0 */
void test_sem_wait_continues_when_positive(void) {
    struct pt pt;
    PT_INIT(&pt);
    PT_SEM_INIT(&test_sem, 1);
    after_wait = -1;

    int result = thread_sem_wait(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
    TEST_ASSERT_EQUAL_INT(1, after_wait);
}

/* Test: PT_SEM_WAIT decrements count */
void test_sem_wait_decrements_count(void) {
    struct pt pt;
    PT_INIT(&pt);
    PT_SEM_INIT(&test_sem, 3);

    thread_sem_wait(&pt);
    TEST_ASSERT_EQUAL_UINT(2, test_sem.count);
}

/* Test: Semaphore wait/signal unblocks waiting thread */
void test_sem_signal_unblocks_waiter(void) {
    struct pt pt, dummy_pt;
    PT_INIT(&pt);
    PT_SEM_INIT(&test_sem, 0);
    after_wait = -1;

    /* First call blocks */
    int result = thread_sem_wait(&pt);
    TEST_ASSERT_EQUAL_INT(PT_WAITING, result);
    TEST_ASSERT_EQUAL_INT(0, after_wait);

    /* Signal the semaphore */
    PT_SEM_SIGNAL(&dummy_pt, &test_sem);
    TEST_ASSERT_EQUAL_UINT(1, test_sem.count);

    /* Now thread should continue */
    result = thread_sem_wait(&pt);
    TEST_ASSERT_EQUAL_INT(PT_ENDED, result);
    TEST_ASSERT_EQUAL_INT(1, after_wait);
    TEST_ASSERT_EQUAL_UINT(0, test_sem.count); /* Decremented */
}

/* Producer-consumer pattern test */
static struct pt_sem mutex, full, empty;
static int buffer[4];
static int buffer_head, buffer_tail;
static int produced_count, consumed_count;

static void add_to_buffer(int item) {
    buffer[buffer_head] = item;
    buffer_head = (buffer_head + 1) % 4;
}

static int get_from_buffer(void) {
    int item = buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % 4;
    return item;
}

static PT_THREAD(producer(struct pt *pt)) {
    static int i;
    PT_BEGIN(pt);

    for (i = 0; i < 8; i++) {
        PT_SEM_WAIT(pt, &full);
        PT_SEM_WAIT(pt, &mutex);
        add_to_buffer(i + 1);
        produced_count++;
        PT_SEM_SIGNAL(pt, &mutex);
        PT_SEM_SIGNAL(pt, &empty);
    }

    PT_END(pt);
}

static PT_THREAD(consumer(struct pt *pt)) {
    static int i;
    static int item;
    PT_BEGIN(pt);

    for (i = 0; i < 8; i++) {
        PT_SEM_WAIT(pt, &empty);
        PT_SEM_WAIT(pt, &mutex);
        item = get_from_buffer();
        consumed_count++;
        (void)item; /* Suppress unused warning */
        PT_SEM_SIGNAL(pt, &mutex);
        PT_SEM_SIGNAL(pt, &full);
    }

    PT_END(pt);
}

/* Test: Producer-consumer with semaphores */
void test_producer_consumer(void) {
    struct pt pt_producer, pt_consumer;
    PT_INIT(&pt_producer);
    PT_INIT(&pt_consumer);

    PT_SEM_INIT(&mutex, 1);
    PT_SEM_INIT(&full, 4);  /* Buffer size */
    PT_SEM_INIT(&empty, 0);

    buffer_head = 0;
    buffer_tail = 0;
    produced_count = 0;
    consumed_count = 0;

    int producer_running = 1;
    int consumer_running = 1;
    int iterations = 0;

    while ((producer_running || consumer_running) && iterations < 100) {
        if (producer_running) {
            producer_running = PT_SCHEDULE(producer(&pt_producer));
        }
        if (consumer_running) {
            consumer_running = PT_SCHEDULE(consumer(&pt_consumer));
        }
        iterations++;
    }

    TEST_ASSERT_EQUAL_INT(8, produced_count);
    TEST_ASSERT_EQUAL_INT(8, consumed_count);
}

/* Test: Mutex pattern (binary semaphore) */
static struct pt_sem test_mutex;
static int critical_section_count;
static int max_concurrent;

static PT_THREAD(thread_mutex_user(struct pt *pt)) {
    PT_BEGIN(pt);

    PT_SEM_WAIT(pt, &test_mutex);
    critical_section_count++;
    if (critical_section_count > max_concurrent) {
        max_concurrent = critical_section_count;
    }
    PT_YIELD(pt);
    critical_section_count--;
    PT_SEM_SIGNAL(pt, &test_mutex);

    PT_END(pt);
}

void test_mutex_mutual_exclusion(void) {
    struct pt pt1, pt2;
    PT_INIT(&pt1);
    PT_INIT(&pt2);
    PT_SEM_INIT(&test_mutex, 1);
    critical_section_count = 0;
    max_concurrent = 0;

    int running1 = 1, running2 = 1;
    int iterations = 0;

    while ((running1 || running2) && iterations < 20) {
        if (running1) {
            running1 = PT_SCHEDULE(thread_mutex_user(&pt1));
        }
        if (running2) {
            running2 = PT_SCHEDULE(thread_mutex_user(&pt2));
        }
        iterations++;
    }

    /* At most one thread in critical section at a time */
    TEST_ASSERT_EQUAL_INT(1, max_concurrent);
}

/* Test: Multiple waiters get unblocked as signals arrive */
static int waiter_finished[3];

static PT_THREAD(waiter_thread(struct pt *pt, int id)) {
    PT_BEGIN(pt);
    PT_SEM_WAIT(pt, &test_sem);
    waiter_finished[id] = 1;
    PT_END(pt);
}

void test_multiple_waiters_unblocked(void) {
    struct pt pt0, pt1, pt2, dummy_pt;
    PT_INIT(&pt0);
    PT_INIT(&pt1);
    PT_INIT(&pt2);
    PT_SEM_INIT(&test_sem, 0);
    waiter_finished[0] = 0;
    waiter_finished[1] = 0;
    waiter_finished[2] = 0;

    /* All three block */
    waiter_thread(&pt0, 0);
    waiter_thread(&pt1, 1);
    waiter_thread(&pt2, 2);

    TEST_ASSERT_EQUAL_INT(0, waiter_finished[0]);
    TEST_ASSERT_EQUAL_INT(0, waiter_finished[1]);
    TEST_ASSERT_EQUAL_INT(0, waiter_finished[2]);

    /* Signal once, one waiter proceeds */
    PT_SEM_SIGNAL(&dummy_pt, &test_sem);
    waiter_thread(&pt0, 0);
    TEST_ASSERT_EQUAL_INT(1, waiter_finished[0]);

    /* Signal again, another waiter proceeds */
    PT_SEM_SIGNAL(&dummy_pt, &test_sem);
    waiter_thread(&pt1, 1);
    TEST_ASSERT_EQUAL_INT(1, waiter_finished[1]);

    /* Signal again, last waiter proceeds */
    PT_SEM_SIGNAL(&dummy_pt, &test_sem);
    waiter_thread(&pt2, 2);
    TEST_ASSERT_EQUAL_INT(1, waiter_finished[2]);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sem_init_sets_count);
    RUN_TEST(test_sem_signal_increments_count);
    RUN_TEST(test_sem_wait_blocks_when_zero);
    RUN_TEST(test_sem_wait_continues_when_positive);
    RUN_TEST(test_sem_wait_decrements_count);
    RUN_TEST(test_sem_signal_unblocks_waiter);
    RUN_TEST(test_producer_consumer);
    RUN_TEST(test_mutex_mutual_exclusion);
    RUN_TEST(test_multiple_waiters_unblocked);
    return UNITY_END();
}
