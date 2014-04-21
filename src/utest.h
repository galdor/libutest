/*
 * Copyright (c) 2014 Nicolas Martyanoff
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef UTEST_UTEST_H
#define UTEST_UTEST_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct test_suite;
struct test_context;

typedef void (*test_function)(struct test_suite *, struct test_context *);
typedef void (*test_report_function)(FILE *, const char *, bool,
                                     const char *, int, const char *);
typedef void (*test_header_printer)(FILE *, const char *);
typedef void (*test_result_printer)(FILE *, size_t, size_t, size_t);

struct test_suite *test_suite_new(const char *);
void test_suite_delete(struct test_suite *);

void test_suite_initialize_from_args(struct test_suite *, int, char **);

void test_suite_set_output(struct test_suite *, FILE *);
void test_suite_set_report_function(struct test_suite *, test_report_function);
void test_suite_set_header_printer(struct test_suite *, test_header_printer);
void test_suite_set_result_printer(struct test_suite *, test_result_printer);

void test_suite_start(struct test_suite *);
int test_suite_run_test(struct test_suite *, const char *, test_function);
bool test_suite_passed(const struct test_suite *);
void test_suite_print_results(const struct test_suite *);
void test_suite_print_results_and_exit(struct test_suite *)
    __attribute__((noreturn));

void test_report_terminal(FILE *, const char *, bool,
                          const char *, int, const char *);
void test_print_header_terminal(FILE *, const char *);
void test_print_results_terminal(FILE *, size_t, size_t, size_t);

void test_report_json(FILE *, const char *, bool,
                      const char *, int, const char *);
void test_print_header_json(FILE *, const char *);
void test_print_results_json(FILE *, size_t, size_t, size_t);

void test_abort(struct test_context *, const char *, int, const char *, ...)
    __attribute__((format(printf, 4, 5)));

char *test_format_data(const char *, size_t);

#define TEST_FUNCTION_NAME(name_) \
    test_case_##name_

#define TEST(name_) \
    static void TEST_FUNCTION_NAME(name_)(struct test_suite *test_suite, \
                                          struct test_context *test_context)

#define TEST_RUN(test_suite_, test_name_) \
    test_suite_run_test(test_suite_, #test_name_, \
                        TEST_FUNCTION_NAME(test_name_))

#define TEST_ABORT(fmt_, ...) \
    test_abort(test_context, __FILE__, __LINE__, fmt_, ##__VA_ARGS__)

#define TEST_TRUE(value_)                                 \
    do {                                                  \
        const char *value_str_ = #value_;                 \
                                                          \
        if (!value_)                                      \
            TEST_ABORT("%s is not true", value_str_);     \
    } while(0)

#define TEST_FALSE(value_)                                \
    do {                                                  \
        const char *value_str_ = #value_;                 \
                                                          \
        if (value_)                                       \
            TEST_ABORT("%s is not false", value_str_);    \
    } while(0)

#define TEST_INT_EQ(value_, expected_)                    \
    do {                                                  \
        int64_t value__ = value_;                         \
        int64_t expected__ = expected_;                   \
        const char *value_str_ = #value_;                 \
                                                          \
        if (value__ != expected__) {                      \
            TEST_ABORT("%s is equal to %"PRIi64" "        \
                       "but should be equal to %"PRIi64,  \
                       value_str_, value__, expected__);  \
        }                                                 \
    } while(0)

#define TEST_UINT_EQ(value_, expected_)                   \
    do {                                                  \
        uint64_t value__ = value_;                        \
        uint64_t expected__ = expected_;                  \
        const char *value_str_ = #value_;                 \
                                                          \
        if (value__ != expected__) {                      \
            TEST_ABORT("%s is equal to %"PRIu64" "        \
                       "but should be equal to %"PRIu64,  \
                       value_str_, value__, expected__);  \
        }                                                 \
    } while(0)

#define TEST_FLOAT_EQ(value_, expected_)                  \
    do {                                                  \
        float value__ = value_;                           \
        float expected__ = expected_;                     \
        const char *value_str_ = #value_;                 \
                                                          \
        if (value__ != expected__) {                      \
            TEST_ABORT("%s is equal to %.9g "             \
                       "but should be equal to %.9g",     \
                       value_str_, value__, expected__);  \
        }                                                 \
    } while(0)

#define TEST_DOUBLE_EQ(value_, expected_)                 \
    do {                                                  \
        double value__ = value_;                          \
        double expected__ = expected_;                    \
        const char *value_str_ = #value_;                 \
                                                          \
        if (value__ != expected__) {                      \
            TEST_ABORT("%s is equal to %.17g "            \
                       "but should be equal to %.17g",    \
                       value_str_, value__, expected__);  \
        }                                                 \
    } while(0)

#define TEST_BOOL_EQ(value_, expected_)                   \
    do {                                                  \
        bool value__ = value_;                            \
        bool expected__ = expected_;                      \
        const char *value_str_ = #value_;                 \
                                                          \
        if (value__ != expected__) {                      \
            TEST_ABORT("%s is %s but should be %s",       \
                       value_str_,                        \
                       (value__ ? "true" : "false"),      \
                       (expected__ ? "true" : "false"));  \
        }                                                 \
    } while(0)

#define TEST_STRING_EQ(value_, expected_)                            \
    do {                                                             \
        const char *value__ = value_;                                \
        const char *expected__ = expected_;                          \
        const char *value_str_ = #value_;                            \
                                                                     \
        if (value__ && expected__) {                                 \
            if (strcmp(value__, expected__) != 0) {                  \
                TEST_ABORT("%s is the string \"%s\" "                \
                           "but should be the string \"%s\"",        \
                           value_str_, value__, expected__);         \
            }                                                        \
        } else if (expected__) {                                     \
            TEST_ABORT("%s is null but should be the string \"%s\"", \
                       value_str_, expected__);                      \
        } else if (value__) {                                        \
            TEST_ABORT("%s is the string \"%s\" but should be null", \
                       value_str_, value__);                         \
        }                                                            \
    } while(0)

#define TEST_MEM_EQ(value_, value_sz_, expected_, expected_sz_)          \
    do {                                                                 \
        const char *value__ = value_;                                    \
        size_t value_sz__ = value_sz_;                                   \
        const char *expected__ = expected_;                              \
        size_t expected_sz__ = expected_sz_;                             \
        const char *value_str_ = #value_;                                \
                                                                         \
        if (value__ && expected__) {                                     \
            if (value_sz__ != expected_sz__) {                           \
                TEST_ABORT("%s is %zu bytes long but should be "         \
                           "%zu bytes long",                             \
                           value_str_, value_sz__, expected_sz__);       \
            }                                                            \
            if (memcmp(value__, expected__, value_sz__) != 0) {          \
                TEST_ABORT("%s contains \"%s\" "                         \
                           "but should contain \"%s\"",                  \
                           value_str_,                                   \
                           test_format_data(value__, value_sz__),        \
                           test_format_data(expected__, expected_sz__)); \
            }                                                            \
        } else if (expected__) {                                         \
            TEST_ABORT("%s is null but should be the string \"%s\"",     \
                       value_str_,                                       \
                       test_format_data(expected__, expected_sz__));     \
        } else if (value__) {                                            \
            TEST_ABORT("%s is the string \"%s\" but should be null",     \
                       value_str_,                                       \
                       test_format_data(value__, value_sz__));           \
        }                                                                \
    } while(0)

#define TEST_PTR_EQ(value_, expected_)                         \
    do {                                                       \
        const void *value__ = value_;                          \
        const void *expected__ = expected_;                    \
        const char *value_str_ = #value_;                      \
                                                               \
        if (value__ && expected__) {                           \
            if (value__ != expected__) {                       \
                TEST_ABORT("%s is equal to %p "                \
                           "but should be equal to %p",        \
                           value_str_, value__, expected__);   \
            }                                                  \
        } else if (expected__) {                               \
            TEST_ABORT("%s is null but should be equal to %p", \
                       value_str_, expected__);                \
        } else if (value__) {                                  \
            TEST_ABORT("%s is equal to %p but should be null", \
                       value_str_, value__);                   \
        }                                                      \
    } while(0)

#define TEST_PTR_NULL(value_)                             \
    do {                                                  \
        const char *value_str_ = #value_;                 \
                                                          \
        if (value_)                                       \
            TEST_ABORT("%s is not null", value_str_);     \
    } while(0)

#define TEST_PTR_NOT_NULL(value_)                         \
    do {                                                  \
        const char *value_str_ = #value_;                 \
                                                          \
        if (!value_)                                      \
            TEST_ABORT("%s is null", value_str_);         \
    } while(0)

#endif
