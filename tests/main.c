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

#include <stdio.h>

#include "utest.h"

TEST(true_false) {
    TEST_TRUE(true);
    TEST_FALSE(false);
}

TEST(true_false_failure) {
    TEST_TRUE(false);
}


TEST(integers) {
    TEST_INT_EQ(-1, -1);
    TEST_UINT_EQ(1, 1);
}

TEST(integer_failure) {
    TEST_INT_EQ(3, -5);
}


TEST(reals) {
    TEST_FLOAT_EQ(1.42f, 1.42f);
    TEST_DOUBLE_EQ(-1.42e5, -1.42e5);
}

TEST(real_failure) {
    TEST_DOUBLE_EQ(1.01, 1.02);
}


TEST(booleans) {
    TEST_BOOL_EQ(true, true);
    TEST_BOOL_EQ(false, false);
}

TEST(boolean_failure) {
    TEST_BOOL_EQ(true, false);
}


TEST(strings) {
    TEST_STRING_EQ("foo", "foo");
}

TEST(string_failure_1) {
    TEST_STRING_EQ("foo", "bar");
}

TEST(string_failure_2) {
    TEST_STRING_EQ("foo", NULL);
}

TEST(string_failure_3) {
    TEST_STRING_EQ(NULL, "foo");
}


TEST(memory) {
    TEST_MEM_EQ("foobar", 3, "foo", 3);
}

TEST(memory_failure_1) {
    TEST_MEM_EQ("foobar", 3, "foo", 2);
}

TEST(memory_failure_2) {
    TEST_MEM_EQ("foo\nbar", 7, "foo\tbar", 7);
}

TEST(memory_failure_3) {
    TEST_MEM_EQ("foobar", 3, NULL, 0);
}

TEST(memory_failure_4) {
    TEST_MEM_EQ(NULL, 0, "foobar", 3);
}


TEST(pointers) {
    TEST_PTR_EQ(printf, printf);
}

TEST(pointer_failure_1) {
    TEST_PTR_EQ(printf, scanf);
}

TEST(pointer_failure_2) {
    TEST_PTR_EQ(printf, NULL);
}

TEST(pointer_failure_3) {
    TEST_PTR_EQ(NULL, scanf);
}

int
main(int argc, char **argv) {
    struct test_suite *suite;

    suite = test_suite_new("main");
    test_suite_initialize_from_args(suite, argc, argv);

    test_suite_start(suite);

    TEST_RUN(suite, true_false);
    TEST_RUN(suite, true_false_failure);

    TEST_RUN(suite, integers);
    TEST_RUN(suite, integer_failure);

    TEST_RUN(suite, reals);
    TEST_RUN(suite, real_failure);

    TEST_RUN(suite, booleans);
    TEST_RUN(suite, boolean_failure);

    TEST_RUN(suite, strings);
    TEST_RUN(suite, string_failure_1);
    TEST_RUN(suite, string_failure_2);
    TEST_RUN(suite, string_failure_3);

    TEST_RUN(suite, memory);
    TEST_RUN(suite, memory_failure_1);
    TEST_RUN(suite, memory_failure_2);
    TEST_RUN(suite, memory_failure_3);
    TEST_RUN(suite, memory_failure_4);

    TEST_RUN(suite, pointers);
    TEST_RUN(suite, pointer_failure_1);
    TEST_RUN(suite, pointer_failure_2);
    TEST_RUN(suite, pointer_failure_3);

    test_suite_print_results_and_exit(suite);
}
