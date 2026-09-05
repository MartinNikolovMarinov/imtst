# imtst.h

Small immediate mode single-header test framework for C and C++.

This is a tiny test runner intended for basic unit test scenarios. Supports test suites and cases, skip/only filtering, setup and teardown hooks, assertion reporting, execution timing, output redirecting to custom file stream, and optional allocation tracking.

It takes inspiration from immediate mode gui libraries and runs the tests as soon as you declare them.

## Usage

In exactly one translation unit do the following:

```c
#define IMTST_IMPLEMENTATION
#include "imtst.h"
```

Then define test functions with this signature:

```c
int32_t my_test(void) {
    IMTST_ASSERT(1 + 1 == 2);
    return 0;
}
```

Define and run test suites with the suite macros, minimal example:

```c
int main(void) {
    imtst_set_output_file(stdout); // Without this line the program crashes immediately.
    imtst_stop_on_first_fail(false);

    IMTST_BEGIN_TEST_SUITE("math")
        IMTST_TEST("addition", my_test)
    IMTST_END_TEST_SUITE()

    imtst_result();
    return 0;
}
```

## Public Macros

```c
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
```

## Configuration

Define `IMTST_IMPLEMENTATION` in one translation unit before including this file to emit the implementation.

Define `IMTST_NO_ANSI` before including this file to disable ANSI color escape sequences in output.

Define `IMTST_RUN_TESTS` before including this file to build the embedded self-test/demo program. This is intended to be ran during development of the framework not during normal use.

## Allocation Tracking

Use `imtst_set_allocation_tracking` with callbacks for total allocated bytes and currently in-use bytes. This allows imtst to report bytes allocated during each test and treats a test as leaking only when currently in-use bytes increase between the start and end of that test. Existing baseline memory is allowed.

## Notes

`imtst_set_output_file` should be called before running tests.

`imtst_stop_on_first_fail(false)` lets the runner continue after failed assertions; otherwise a failed assertion prints and aborts.

`imtst_turn_on_only_mode(true)` runs only suites and tests explicitly marked ONLY. A test marked with only that is defined inside a suite that is not defined as only will NOT run.
