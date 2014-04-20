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

#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "utest.h"

#define TEST_ERROR_BUFSZ 1024

struct test_suite {
    const char *name;

    test_result_printer result_printer;
    test_report_function report_function;

    size_t nb_tests;
    size_t nb_failed_tests;
    size_t nb_passed_tests;
};

struct test_context {
    const char *test_name;
    struct test_suite *test_suite;

    jmp_buf before;
};

struct test_suite *
test_suite_new(const char *name,
               test_header_printer header_printer,
               test_result_printer result_printer,
               test_report_function report_function) {
    struct test_suite *suite;

    suite = malloc(sizeof(struct test_suite));
    if (!suite) {
        fprintf(stderr, "cannot allocate %zu bytes: %s\n",
                sizeof(struct test_suite), strerror(errno));
        abort();
    }
    memset(suite, 0, sizeof(struct test_suite));

    suite->name = name;
    suite->result_printer = result_printer;
    suite->report_function = report_function;

    if (header_printer)
        header_printer(name);

    return suite;
}

void
test_suite_delete(struct test_suite *suite) {
    if (!suite)
        return;

    memset(suite, 0, sizeof(struct test_suite));
    free(suite);
}

int
test_suite_run_test(struct test_suite *suite, const char *test_name,
                    test_function function) {
    struct test_context ctx;

    suite->nb_tests++;

    memset(&ctx, 0, sizeof(struct test_context));
    if (setjmp(ctx.before) != 0) {
        /* Test function aborted */
        suite->nb_failed_tests++;
        return -1;
    }

    ctx.test_name = test_name;
    ctx.test_suite = suite;

    function(suite, &ctx);

    suite->nb_passed_tests++;
    if (suite->report_function)
        suite->report_function(test_name, true, NULL, 0, NULL);
    return 0;
}

void
test_suite_print_results(struct test_suite *suite) {
    if (!suite->result_printer)
        return;

    suite->result_printer(suite->nb_tests,
                          suite->nb_passed_tests, suite->nb_failed_tests);
}

void
test_report_terminal(const char *test_name, bool success,
                     const char *file, int line, const char *errmsg) {
    if (success) {
        printf("\e[32m.\e[0m %-20s  \e[32mok\e[0m\n",
               test_name);
    } else {
        printf("\e[31mx\e[0m %-20s  %s:%d  \e[31m%s\e[0m\n",
               test_name, file, line, errmsg);
    }
}

void
test_print_header_terminal(const char *suite_name) {
    printf("----------------------------------------"
           "----------------------------------------\n");
    printf(" %s\n", suite_name);
    printf("----------------------------------------"
           "----------------------------------------\n\n");
}

void
test_print_results_terminal(size_t nb_tests,
                            size_t nb_passed_tests, size_t nb_failed_tests) {
    double ratio_passed, ratio_failed;

    ratio_passed = (double)nb_passed_tests / (double)nb_tests;
    ratio_failed = (double)nb_failed_tests / (double)nb_tests;

    putchar('\n');

    printf("%-20s  %zu\n", "Tests executed",
           nb_tests);
    printf("%-20s  %zu (%.0f%%)\n", "Tests passed",
           nb_passed_tests, ratio_passed * 100.0);
    printf("%-20s  %zu (%.0f%%)\n", "Tests failed",
           nb_failed_tests, ratio_failed * 100.0);

    putchar('\n');
}

void
test_abort(struct test_context *ctx, const char *file, int line,
           const char *fmt, ...) {
    char errmsg[TEST_ERROR_BUFSZ];
    va_list ap;

    if (ctx->test_suite->report_function) {
        va_start(ap, fmt);
        vsnprintf(errmsg, TEST_ERROR_BUFSZ, fmt, ap);
        va_end(ap);

        ctx->test_suite->report_function(ctx->test_name, false,
                                         file, line, errmsg);
    }

    longjmp(ctx->before, -1);
}

char *
test_format_data(const char *data, size_t sz) {
    static char buf[1024];

    char *optr;
    size_t olen;

    optr = buf;
    olen = 0;

    for (size_t i = 0; i < sz; i++) {
        char c;

        c = data[i];

        if (isprint((unsigned char)c)) {
            if (olen >= sizeof(buf) - 1)
                goto overflow;

            *optr++ = data[i];

            olen += 1;
        } else if (c == '"' || c == '\r' || c == '\n' || c == '\t') {
            if (olen >= sizeof(buf) - 2)
                goto overflow;

            *optr++ = '\\';

            if (c == '"') {
                *optr++ = data[i];
            } else if (c == '\r') {
                *optr++ = 'r';
            } else if (c == '\n') {
                *optr++ = 'n';
            } else if (c == '\t') {
                *optr++ = 't';
            }

            olen += 2;
        } else {
            char tmp[5];

            if (olen >= sizeof(buf) - 4)
                goto overflow;

            snprintf(tmp, sizeof(tmp), "\\%03hhu", (unsigned char)data[i]);

            *optr++ = tmp[0];
            *optr++ = tmp[1];
            *optr++ = tmp[2];
            *optr++ = tmp[3];
            olen += 4;
        }
    }

    *optr = '\0';
    return buf;

overflow:
    fprintf(stderr, "data too large for buffer\n");
    abort();
}
