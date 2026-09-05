/*
    imtst.h - small immediate mode single-header test framework for C and C++

    This is a tiny test runner intended for basic unit test scenarios. Supports test suites and cases, skip/only
    filtering, setup and teardown hooks, assertion reporting, execution timing, output redirecting to custom file
    stream, and optional allocation tracking.

    It takes inspiration from immediate mode gui libraries and runs the tests as soon as you declare them.

Usage:

    In exactly one translation unit do the following:

        #define IMTST_IMPLEMENTATION
        #include "imtst.h"

    Then define test functions with this signature:

        int32_t my_test(void) {
            IMTST_ASSERT(1 + 1 == 2);
            return 0;
        }

    Define and run test suites with the suite macros, minimal example:

        int main(void) {
            imtst_set_output_file(stdout); // Without this line the program crashes immediately.
            imtst_stop_on_first_fail(false);

            IMTST_BEGIN_TEST_SUITE("math")
                IMTST_TEST("addition", my_test)
            IMTST_END_TEST_SUITE()

            imtst_result();
            return 0;
        }

Public macros:

    IMTST_BEGIN_TEST_SUITE(name)
    IMTST_BEGIN_TEST_SUITE2(name, before_all, before_each, after_all, after_each)
    IMTST_BEGIN_TEST_SUITE_SKIP(name)
    IMTST_BEGIN_TEST_SUITE_SKIP2(name, before_all, before_each, after_all, after_each)
    IMTST_BEGIN_TEST_SUITE_ONLY(name)
    IMTST_BEGIN_TEST_SUITE_ONLY2(name, before_all, before_each, after_all, after_each)
    IMTST_END_TEST_SUITE()

    IMTST_TEST(name, test_function)
    IMTST_TEST2(name, test_function, before, after)
    IMTST_TEST_SKIP(name, test_function)
    IMTST_TEST_SKIP2(name, test_function, before, after)
    IMTST_TEST_ONLY(name, test_function)
    IMTST_TEST_ONLY2(name, test_function, before, after)

    IMTST_ASSERT(expr)

Configuration:

    Define IMTST_IMPLEMENTATION in one translation unit before including this file to emit the implementation.

    Define IMTST_NO_ANSI before including this file to disable ANSI color escape sequences in output.

    Define IMTST_RUN_TESTS before including this file to build the embedded self-test/demo program.
    This is intended to be ran during development of the framework not during normal use.

Allocation tracking:

    Use imtst_set_allocation_tracking with callbacks for total allocated bytes and currently in-use bytes. This allows
    imtst to report bytes allocated during each test and treats a test as leaking only when currently in-use bytes
    increase between the start and end of that test. Existing baseline memory is allowed.

Notes:

    imtst_set_output_file should be called before running tests.

    imtst_stop_on_first_fail(false) lets the runner continue after failed assertions; otherwise a failed assertion
    prints and aborts.

    imtst_turn_on_only_mode(true) runs only suites and tests explicitly marked ONLY. A test marked with only that is
    defined inside a suite that is not defined as only will NOT run.
*/

// TODO: Should probably add repetition testing as well.

#ifndef INCLUDE_IMTST_H
#define INCLUDE_IMTST_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

//======================================================================================================================
// ANSI CODES
//======================================================================================================================

#ifdef IMTST_NO_ANSI

#define IMTST_ANSI_RESET() ""

#define IMTST_ANSI_BOLD_START() ""
#define IMTST_ANSI_BOLD(text) text

#define IMTST_ANSI_RED_START() ""
#define IMTST_ANSI_RED(text) text

#define IMTST_ANSI_GREEN_START() ""
#define IMTST_ANSI_GREEN(text) text

#define IMTST_ANSI_YELLOW_START() ""
#define IMTST_ANSI_YELLOW(text) text

#else

#define IMTST_ANSI_RESET() "\x1b[0m" // Reset all attributes

#define IMTST_ANSI_BOLD_START() "\x1b[1m"
#define IMTST_ANSI_BOLD(text) IMTST_ANSI_BOLD_START() text IMTST_ANSI_RESET()

#define IMTST_ANSI_RED_START() "\x1b[31m"
#define IMTST_ANSI_RED(text) IMTST_ANSI_RED_START() text IMTST_ANSI_RESET()

#define IMTST_ANSI_GREEN_START() "\x1b[32m"
#define IMTST_ANSI_GREEN(text) IMTST_ANSI_GREEN_START() text IMTST_ANSI_RESET()

#define IMTST_ANSI_YELLOW_START() "\x1b[33m"
#define IMTST_ANSI_YELLOW(text) IMTST_ANSI_YELLOW_START() text IMTST_ANSI_RESET()

#endif // IMTST_NO_ANSI

//======================================================================================================================
// TYPE DEFINITIONS
//======================================================================================================================

typedef int32_t (*imtst_run_test_t)(void);

typedef void (*imtst_before_t)(void);
typedef void (*imtst_after_t)(void);

typedef struct imtst_test_case {
    uint32_t id;
    const char* label;
    imtst_run_test_t run_test;
    bool skipped;
    bool only;
    imtst_before_t before;
    imtst_after_t after;
} imtst_test_case;

typedef void (*imtst_before_all_t)(void);
typedef void (*imtst_before_each_t)(void);
typedef void (*imtst_after_all_t)(void);
typedef void (*imtst_after_each_t)(void);

typedef struct imtst_test_suite {
    const char* label;

    imtst_before_all_t before_all;
    imtst_before_each_t before_each;
    imtst_after_all_t after_all;
    imtst_after_each_t after_each;

    imtst_test_case* test_cases;
    size_t test_cases_count;

    bool skipped;
    bool only;
} imtst_test_suite;

typedef uint64_t (*imtst_tracking_function_t)(void);

typedef struct imtst_allocation_tracking {
    const char* allocator_name;
    imtst_tracking_function_t get_total_allocated_bytes;
    imtst_tracking_function_t get_currently_in_use_bytes;
} imtst_allocation_tracking;

//======================================================================================================================
// PUBLIC INTERFACE
//======================================================================================================================

#ifdef __cplusplus
extern "C" {
#endif

int32_t imtst_run_test(imtst_test_case* tcase);
void imtst_run_test_suite(imtst_test_suite* suite);
void imtst_set_allocation_tracking(imtst_allocation_tracking allocation_tracking);
void imtst_stop_on_first_fail(bool toggle);
void imtst_turn_on_only_mode(bool toggle);
void imtst_set_output_file(FILE* file);
int32_t imtst_failed_assert(const char* expr, const char* file, uint32_t line, const char* function);
void imtst_result(void);
uint32_t imtst_next_test_id(void);

#ifdef __cplusplus
}
#endif // __cplusplus

//======================================================================================================================
// PUBLIC MACROS
//======================================================================================================================

#ifdef __cplusplus
#define IMTST_NULL nullptr
#else
#define IMTST_NULL NULL
#endif

#define IMTST_BEGIN_TEST_SUITE_IMPL(_name, _skip, _only, _before_all, _before_each, _after_all, _after_each)           \
do {                                                                                                                   \
    imtst_test_suite _imtst_sute;                                                                                      \
    memset(&_imtst_sute, 0, sizeof(_imtst_sute));                                                                      \
    _imtst_sute.label = (_name);                                                                                       \
    _imtst_sute.skipped = (_skip);                                                                                     \
    _imtst_sute.only = (_only);                                                                                        \
    _imtst_sute.before_all = (_before_all);                                                                            \
    _imtst_sute.before_each = (_before_each);                                                                          \
    _imtst_sute.after_all = (_after_all);                                                                              \
    _imtst_sute.after_each = (_after_each);                                                                            \
    imtst_test_case internal_cases[] = {

#define IMTST_TEST_IMPL(_name, _test_function, _skipped, _only, _before, _after)                                       \
    { imtst_next_test_id(), (_name), (_test_function), (_skipped), (_only), (_before), (_after) },

#define IMTST_END_TEST_SUITE_IMPL()                                                                                    \
    };                                                                                                                 \
    _imtst_sute.test_cases = internal_cases;                                                                           \
    _imtst_sute.test_cases_count = sizeof(internal_cases) / sizeof(internal_cases[0]);                                 \
    imtst_run_test_suite(&_imtst_sute);                                                                                \
} while(0);

#define IMTST_BEGIN_TEST_SUITE(_name) \
    IMTST_BEGIN_TEST_SUITE_IMPL(_name, false, false, IMTST_NULL, IMTST_NULL, IMTST_NULL, IMTST_NULL)
#define IMTST_BEGIN_TEST_SUITE2(_name, _before_all, _before_each, _after_all, _after_each) \
    IMTST_BEGIN_TEST_SUITE_IMPL(_name, false, false, _before_all, _before_each, _after_all, _after_each)
#define IMTST_BEGIN_TEST_SUITE_SKIP(_name) \
    IMTST_BEGIN_TEST_SUITE_IMPL(_name, true, false, IMTST_NULL, IMTST_NULL, IMTST_NULL, IMTST_NULL)
#define IMTST_BEGIN_TEST_SUITE_SKIP2(_name, _before_all, _before_each, _after_all, _after_each) \
    IMTST_BEGIN_TEST_SUITE_IMPL(_name, true, false, _before_all, _before_each, _after_all, _after_each)
#define IMTST_BEGIN_TEST_SUITE_ONLY(_name) \
    IMTST_BEGIN_TEST_SUITE_IMPL(_name, false, true, IMTST_NULL, IMTST_NULL, IMTST_NULL, IMTST_NULL)
#define IMTST_BEGIN_TEST_SUITE_ONLY2(_name, _before_all, _before_each, _after_all, _after_each) \
    IMTST_BEGIN_TEST_SUITE_IMPL(_name, false, true, _before_all, _before_each, _after_all, _after_each)

#define IMTST_END_TEST_SUITE() IMTST_END_TEST_SUITE_IMPL()

#define IMTST_TEST(_name, _test_function) \
    IMTST_TEST_IMPL(_name, _test_function, false, false, IMTST_NULL, IMTST_NULL)
#define IMTST_TEST2(_name, _test_function, _before, _after) \
    IMTST_TEST_IMPL(_name, _test_function, false, false, _before, _after)
#define IMTST_TEST_SKIP(_name, _test_function) \
    IMTST_TEST_IMPL(_name, _test_function, true, false, IMTST_NULL, IMTST_NULL)
#define IMTST_TEST_SKIP2(_name, _test_function, _before, _after) \
    IMTST_TEST_IMPL(_name, _test_function, true, false, _before, _after)
#define IMTST_TEST_ONLY(_name, _test_function) \
    IMTST_TEST_IMPL(_name, _test_function, false, true, IMTST_NULL, IMTST_NULL)
#define IMTST_TEST_ONLY2(_name, _test_function, _before, _after) \
    IMTST_TEST_IMPL(_name, _test_function, false, true, _before, _after)

#define IMTST_ASSERT(expr)                                                                                             \
do {                                                                                                                   \
    if (!(expr)) {                                                                                                     \
        return imtst_failed_assert((#expr), __FILE__, __LINE__, __func__);                                             \
    }                                                                                                                  \
} while (0)

//======================================================================================================================
// IMPLEMENTATION
//======================================================================================================================

#ifdef IMTST_IMPLEMENTATION

#define IMTST_MICROSECOND  1000ull
#define IMTST_MILLISECOND  1000000ull
#define IMTST_SECOND       1000000000ull
#define IMTST_MINUTE       (60ull * IMTST_SECOND)
#define IMTST_HOUR         (60ull * IMTST_MINUTE)

#define IMTST_ELAPSED_TIME_TO_STR_BUFFER_SIZE 256
#define IMTST_MEMORY_USED_TO_STR_BUFFER_SIZE 128

static const char* imtst_g_allocator_name = IMTST_NULL;
static imtst_tracking_function_t imtst_g_total_allocated_bytes_cb = IMTST_NULL;
static imtst_tracking_function_t imtst_g_currently_in_use_bytes_cb = IMTST_NULL;
static bool imtst_g_stop_on_first_fail = true;
static bool imtst_g_ok = true;
static bool imtst_g_only_mode_active = false;
static FILE* imtst_g_output_file = IMTST_NULL;

//======================================================================================================================
// STATIC FUNCTIONS
//======================================================================================================================

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static uint64_t imtst_get_time(void)
{
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;

    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);

    return (uint64_t)(counter.QuadPart * 1000000000ULL / frequency.QuadPart);
}

#else

#include <time.h>

static uint64_t imtst_get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#endif

static char* imtst_elapsed_time_to_str(
    char out[IMTST_ELAPSED_TIME_TO_STR_BUFFER_SIZE],
    uint64_t delta_time_ns)
{
    const size_t buffer_size = IMTST_ELAPSED_TIME_TO_STR_BUFFER_SIZE;

    memset(out, 0, buffer_size);

    if (delta_time_ns >= IMTST_HOUR) {
        uint32_t hours = (uint32_t)(delta_time_ns / IMTST_HOUR);
        uint32_t minutes = (uint32_t)((delta_time_ns % IMTST_HOUR) / IMTST_MINUTE);
        uint32_t seconds = (uint32_t)((delta_time_ns % IMTST_MINUTE) / IMTST_SECOND);
        uint64_t milliseconds = (delta_time_ns % IMTST_SECOND) / IMTST_MILLISECOND;

        snprintf(
            out, buffer_size,
            "%02" PRIu32 "h %02" PRIu32 "m %02" PRIu32 "s %" PRIu64 "ms",
            hours, minutes, seconds, milliseconds
        );
    }
    else if (delta_time_ns >= IMTST_MINUTE) {
        uint32_t minutes = (uint32_t)(delta_time_ns / IMTST_MINUTE);
        uint32_t seconds = (uint32_t)((delta_time_ns % IMTST_MINUTE) / IMTST_SECOND);
        uint64_t milliseconds = (delta_time_ns % IMTST_SECOND) / IMTST_MILLISECOND;

        snprintf(
            out, buffer_size,
            "%02" PRIu32 "m %02" PRIu32 "s %" PRIu64 "ms",
            minutes, seconds, milliseconds
        );
    }
    else if (delta_time_ns >= IMTST_SECOND) {
        uint32_t seconds = (uint32_t)(delta_time_ns / IMTST_SECOND);
        uint64_t milliseconds = (delta_time_ns % IMTST_SECOND) / IMTST_MILLISECOND;

        snprintf(
            out, buffer_size,
            "%02" PRIu32 "s %" PRIu64 "ms",
            seconds, milliseconds
        );
    }
    else if (delta_time_ns >= IMTST_MILLISECOND) {
        uint64_t milliseconds = delta_time_ns / IMTST_MILLISECOND;
        uint64_t microseconds = (delta_time_ns % IMTST_MILLISECOND) / IMTST_MICROSECOND;
        uint64_t nanoseconds = delta_time_ns % IMTST_MICROSECOND;

        snprintf(
            out, buffer_size,
            "%" PRIu64 "ms %" PRIu64 "us %" PRIu64 "ns",
            milliseconds, microseconds, nanoseconds
        );
    }
    else {
        snprintf(out, buffer_size, "%" PRIu64 "ns", delta_time_ns);
    }

    return out;
}

static char* imtst_memory_used_to_str(
    char out[IMTST_MEMORY_USED_TO_STR_BUFFER_SIZE],
    size_t delta_memory)
{
    const size_t buffer_size = IMTST_MEMORY_USED_TO_STR_BUFFER_SIZE;

    memset(out, 0, buffer_size);

    if (delta_memory >= 1024ull * 1024ull * 1024ull) {
        double gb = (double)delta_memory / (1024.0 * 1024.0 * 1024.0);
        snprintf(out, buffer_size, "%.2f GB", gb);
    }
    else if (delta_memory >= 1024ull * 1024ull) {
        double mb = (double)delta_memory / (1024.0 * 1024.0);
        snprintf(out, buffer_size, "%.2f MB", mb);
    }
    else if (delta_memory >= 1024ull) {
        double kb = (double)delta_memory / 1024.0;
        snprintf(out, buffer_size, "%.2f KB", kb);
    }
    else {
        snprintf(out, buffer_size, "%.2f B", (double)delta_memory);
    }

    return out;
}

//======================================================================================================================
// PUBLIC FUNCTIONS
//======================================================================================================================

int32_t imtst_run_test(imtst_test_case* tcase) {
    if (imtst_g_only_mode_active && !tcase->only) {
        return 0;
    }

    if (tcase->skipped) {
        fprintf(
            imtst_g_output_file,
            "  [TEST № %d %s] %s\n",
            tcase->id,
            IMTST_ANSI_BOLD(IMTST_ANSI_YELLOW("SKIPPED")),
            tcase->label
        );
        return 0;
    }

    if (tcase->before) {
        tcase->before();
    }

    uint64_t test_case_run_start = imtst_get_time();
    uint64_t allocations_before_test_case = imtst_g_total_allocated_bytes_cb
        ? imtst_g_total_allocated_bytes_cb()
        : 0;
    uint64_t in_use_before_test_case = imtst_g_currently_in_use_bytes_cb
        ? imtst_g_currently_in_use_bytes_cb()
        : 0;

    fprintf(imtst_g_output_file, "  [TEST № %d RUNNING] %s\n", tcase->id, tcase->label);

    int32_t ret = tcase->run_test();

    uint64_t test_case_run_elapsed = imtst_get_time() - test_case_run_start;
    char elapsed_time_str[IMTST_ELAPSED_TIME_TO_STR_BUFFER_SIZE];
    imtst_elapsed_time_to_str(elapsed_time_str, test_case_run_elapsed);

    if (ret == 0) {
        if (imtst_g_total_allocated_bytes_cb && imtst_g_currently_in_use_bytes_cb) {
            uint64_t allocated_bytes_in_test_case = imtst_g_total_allocated_bytes_cb() - allocations_before_test_case;
            uint64_t in_use_after_test_case = imtst_g_currently_in_use_bytes_cb();
            uint64_t in_use_bytes_in_test_case = in_use_after_test_case > in_use_before_test_case
                ? in_use_after_test_case - in_use_before_test_case
                : 0;

            char allocated_bytes_str[IMTST_MEMORY_USED_TO_STR_BUFFER_SIZE];
            imtst_memory_used_to_str(allocated_bytes_str, allocated_bytes_in_test_case);
            char in_use_bytes_str[IMTST_MEMORY_USED_TO_STR_BUFFER_SIZE];
            imtst_memory_used_to_str(in_use_bytes_str, in_use_bytes_in_test_case);

            if (in_use_after_test_case <= in_use_before_test_case) {
                fprintf(
                    imtst_g_output_file,
                    "  [TEST № %d %s] %s [time: %s, memory: { allocated: %s, in_use: %s }]\n",
                    tcase->id,
                    IMTST_ANSI_BOLD(IMTST_ANSI_GREEN("PASSED")),
                    tcase->label,
                    elapsed_time_str,
                    allocated_bytes_str,
                    in_use_bytes_str
                );
            }
            else {
                fprintf(
                    imtst_g_output_file,
                    "  [TEST № %d %s] %s [time: %s, memory: { allocated: %s, in_use: %s }]\n",
                    tcase->id,
                    IMTST_ANSI_BOLD(IMTST_ANSI_RED("MEMORY LEAKED")),
                    tcase->label,
                    elapsed_time_str,
                    allocated_bytes_str,
                    in_use_bytes_str
                );
                ret = -1;
                imtst_g_ok = false;
            }
        }
        else {
            fprintf(
                imtst_g_output_file,
                "  [TEST № %d %s] %s [time: %s]\n",
                tcase->id,
                IMTST_ANSI_BOLD(IMTST_ANSI_GREEN("PASSED")),
                tcase->label,
                elapsed_time_str
            );
        }
    }
    else {
        imtst_g_ok = false;

        fprintf(
            imtst_g_output_file,
            "  [TEST № %d %s] %s [time: %s]\n",
            tcase->id,
            IMTST_ANSI_BOLD(IMTST_ANSI_RED("FAILED")),
            tcase->label,
            elapsed_time_str
        );
    }

    if (tcase->after) {
        tcase->after();
    }

    return ret;
}

int32_t imtst_failed_assert(const char* expr, const char* file, uint32_t line, const char* function) {
    if (imtst_g_stop_on_first_fail) {
        fprintf(
            imtst_g_output_file,
            "%s%sASSERT( %s ) %s:%u @ %s%s\n",
            IMTST_ANSI_RED_START(),
            IMTST_ANSI_BOLD_START(),
            expr,
            file,
            line,
            function,
            IMTST_ANSI_RESET()
        );
        abort();
    }

    return -1;
}

void imtst_run_test_suite(imtst_test_suite* suite) {
    if (imtst_g_only_mode_active && !suite->only) {
        return;
    }

    if (suite->skipped) {
        fprintf(imtst_g_output_file, "[SUITE %s] %s\n", IMTST_ANSI_BOLD(IMTST_ANSI_YELLOW("SKIPPED")), suite->label);
        return;
    }

    if (suite->before_all) {
        suite->before_all();
    }

    uint64_t suite_run_start = imtst_get_time();

    fprintf(imtst_g_output_file, "[SUITE RUNNING] %s", suite->label);
    if (imtst_g_allocator_name) {
        fprintf(imtst_g_output_file, " (allocator used = '%s')", imtst_g_allocator_name);
    }
    fprintf(imtst_g_output_file, "\n");

    int32_t failed_count = 0;
    for (size_t i = 0; i < suite->test_cases_count; i++) {
        if (suite->before_each) {
            suite->before_each();
        }
        int32_t ret = imtst_run_test(&suite->test_cases[i]);
        if (ret != 0) failed_count++;
        if (suite->after_each) {
            suite->after_each();
        }
    }

    uint64_t suite_run_elapsed = imtst_get_time() - suite_run_start;
    char suite_run_elapsed_str[IMTST_ELAPSED_TIME_TO_STR_BUFFER_SIZE];
    imtst_elapsed_time_to_str(suite_run_elapsed_str, suite_run_elapsed);

    if (failed_count == 0) {
        fprintf(
            imtst_g_output_file,
            "[SUITE %s] %s [time: %s]\n",
            IMTST_ANSI_BOLD(IMTST_ANSI_GREEN("PASSED")),
            suite->label,
            suite_run_elapsed_str
        );
    }
    else {
        fprintf(
            imtst_g_output_file,
            "[SUITE %s] [time: %s] failed_count = %d\n",
            IMTST_ANSI_BOLD(IMTST_ANSI_RED("FAILED")),
            suite_run_elapsed_str,
            failed_count
        );
    }

    if (suite->after_all) {
        suite->after_all();
    }
}

void imtst_set_allocation_tracking(imtst_allocation_tracking tracking) {
    imtst_g_allocator_name = tracking.allocator_name;
    imtst_g_total_allocated_bytes_cb = tracking.get_total_allocated_bytes;
    imtst_g_currently_in_use_bytes_cb = tracking.get_currently_in_use_bytes;
}

void imtst_stop_on_first_fail(bool toggle) {
    imtst_g_stop_on_first_fail = toggle;
}

void imtst_turn_on_only_mode(bool toggle) {
    imtst_g_only_mode_active = toggle;
}

void imtst_set_output_file(FILE* file) {
    imtst_g_output_file = file;
}

uint32_t imtst_next_test_id(void) {
    static uint32_t next_id = 0;
    return next_id++;
}

void imtst_result(void) {
    if (imtst_g_ok) {
        fprintf(imtst_g_output_file, "\n" IMTST_ANSI_BOLD(IMTST_ANSI_GREEN("Tests OK")) "\n");
    }
    else {
        fprintf(imtst_g_output_file, "\n" IMTST_ANSI_BOLD(IMTST_ANSI_RED("Tests FAILED")) "\n");
    }
}

#endif // IMTST_IMPLEMENTATION

#ifdef IMTST_RUN_TESTS

static uint64_t imtst_example_total_allocated_bytes = 0;
static uint64_t imtst_example_currently_in_use_bytes = 32;
static int32_t imtst_example_before_all_count = 0;
static int32_t imtst_example_before_each_count = 0;
static int32_t imtst_example_after_all_count = 0;
static int32_t imtst_example_after_each_count = 0;
static int32_t imtst_example_before_test_count = 0;
static int32_t imtst_example_after_test_count = 0;

int32_t passing_test() {
    IMTST_ASSERT(true);
    return 0;
}

int32_t failing_test() {
    IMTST_ASSERT(false);
    return 0;
}

int32_t allocating_without_leak_test() {
    imtst_example_total_allocated_bytes += 64;
    imtst_example_currently_in_use_bytes += 64;
    imtst_example_currently_in_use_bytes -= 64;

    return 0;
}

int32_t leaking_test() {
    imtst_example_total_allocated_bytes += 128;
    imtst_example_currently_in_use_bytes += 128;

    return 0;
}

int32_t before_after_test() {
    IMTST_ASSERT(imtst_example_before_test_count == 1);
    IMTST_ASSERT(imtst_example_after_test_count == 0);

    return 0;
}

int32_t suite_hooks_test() {
    IMTST_ASSERT(imtst_example_before_all_count == 1);
    IMTST_ASSERT(imtst_example_after_all_count == 0);
    IMTST_ASSERT(imtst_example_before_each_count > imtst_example_after_each_count);

    return 0;
}

uint64_t example_get_total_allocated_bytes() {
    return imtst_example_total_allocated_bytes;
}

uint64_t example_get_currently_in_use_bytes() {
    return imtst_example_currently_in_use_bytes;
}

void before_all() {
    imtst_example_before_all_count++;
    fprintf(imtst_g_output_file, "BEFORE_ALL\n");
}

void before_each() {
    imtst_example_before_each_count++;
    fprintf(imtst_g_output_file, "BEFORE_EACH\n");
}

void after_all() {
    imtst_example_after_all_count++;
    fprintf(imtst_g_output_file, "AFTER_ALL\n");
}

void after_each() {
    imtst_example_after_each_count++;
    fprintf(imtst_g_output_file, "AFTER_EACH\n");
}

void before() {
    imtst_example_before_test_count++;
    fprintf(imtst_g_output_file, "BEFORE_TEST\n");
}

void after() {
    imtst_example_after_test_count++;
    fprintf(imtst_g_output_file, "AFTER_TEST\n");
}

int main() {
    imtst_allocation_tracking tracking = {
        "Test Allocator",
        example_get_total_allocated_bytes,
        example_get_currently_in_use_bytes
    };
    imtst_set_allocation_tracking(tracking);

    imtst_stop_on_first_fail(false);
    imtst_turn_on_only_mode(false);
    imtst_set_output_file(stdout);

    IMTST_BEGIN_TEST_SUITE_ONLY2("Hooks and assertions", before_all, before_each, after_all, after_each)
        IMTST_TEST_ONLY("Passing assertion", passing_test)
        IMTST_TEST_ONLY2("Per-test hooks", before_after_test, before, after)
        IMTST_TEST_ONLY("Suite hooks", suite_hooks_test)
        IMTST_TEST_ONLY("Failing assertion", failing_test)
    IMTST_END_TEST_SUITE()

    IMTST_BEGIN_TEST_SUITE_ONLY("Memory tracking")
        IMTST_TEST_ONLY("Baseline memory is allowed", passing_test)
        IMTST_TEST_ONLY("Allocation freed before test end", allocating_without_leak_test)
        IMTST_TEST_ONLY("Allocation leaked by test", leaking_test)
    IMTST_END_TEST_SUITE()

    IMTST_BEGIN_TEST_SUITE_ONLY("Skip handling")
        IMTST_TEST_ONLY("Normal test still runs", passing_test)
        IMTST_TEST_SKIP("Skipped failing test", failing_test)
    IMTST_END_TEST_SUITE()

    IMTST_BEGIN_TEST_SUITE_SKIP("Skipped suite")
        IMTST_TEST("Failing test in skipped suite", failing_test)
    IMTST_END_TEST_SUITE()

    imtst_result();

    return 0;
}

#endif // IMTST_RUN_TESTS

#endif // INCLUDE_IMTST_H
