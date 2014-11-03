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

#include <unistd.h>

#include "utest.h"

#define TEST_ERROR_BUFSZ 1024

static void test_usage(const char *, int)
    __attribute__ ((noreturn));
static void test_die(const char *, ...)
    __attribute__ ((format(printf, 1, 2), noreturn));

struct test_suite {
    const char *name;

    FILE *output;
    test_header_printer header_printer;
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
test_suite_new(const char *name) {
    struct test_suite *suite;

    suite = malloc(sizeof(struct test_suite));
    if (!suite) {
        fprintf(stderr, "cannot allocate %zu bytes: %s\n",
                sizeof(struct test_suite), strerror(errno));
        abort();
    }
    memset(suite, 0, sizeof(struct test_suite));

    suite->name = name;

    suite->output = stdout;
    suite->header_printer = test_print_header_terminal;
    suite->result_printer = test_print_results_terminal;
    suite->report_function = test_report_terminal;

    return suite;
}

void
test_suite_delete(struct test_suite *suite) {
    if (!suite)
        return;

    fclose(suite->output);

    memset(suite, 0, sizeof(struct test_suite));
    free(suite);
}

void
test_suite_initialize_from_args(struct test_suite *suite,
                                int argc, char **argv) {
    const char *output_path;
    const char *format;
    FILE *output;
    int opt;

    output_path = "-";
    format = "terminal";

    opterr = 0;
    while ((opt = getopt(argc, argv, "f:ho:")) != -1) {
        switch (opt) {
        case 'f':
            format = optarg;
            break;

        case 'h':
            test_usage(argv[0], 0);
            break;

        case 'o':
            output_path = optarg;
            break;

        case '?':
            test_usage(argv[0], 1);
        }
    }

    /* Output */
    if (strcmp(output_path, "-") == 0) {
        output = stdout;
    } else {
        output = fopen(output_path, "w");
        if (!output) {
            test_die("cannot open file %s: %s",
                     output_path, strerror(errno));
        }
    }

    test_suite_set_output(suite, output);

    /* Format */
    if (strcmp(format, "terminal") == 0) {
        test_suite_set_header_printer(suite, test_print_header_terminal);
        test_suite_set_result_printer(suite, test_print_results_terminal);
        test_suite_set_report_function(suite, test_report_terminal);
    } else if (strcmp(format, "json") == 0) {
        test_suite_set_header_printer(suite, test_print_header_json);
        test_suite_set_result_printer(suite, test_print_results_json);
        test_suite_set_report_function(suite, test_report_json);
    } else {
        test_die("unknown format '%s'", format);
    }
}

void
test_suite_set_output(struct test_suite *suite, FILE *output) {
    suite->output = output;
}

void
test_suite_set_report_function(struct test_suite *suite,
                               test_report_function function) {
    suite->report_function = function;
}

void
test_suite_set_header_printer(struct test_suite *suite,
                              test_header_printer function) {
    suite->header_printer = function;
}

void
test_suite_set_result_printer(struct test_suite *suite,
                              test_result_printer function) {
    suite->result_printer = function;
}

void
test_suite_start(struct test_suite *suite) {
    if (suite->header_printer)
        suite->header_printer(suite->output, suite->name);
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
        suite->report_function(suite->output, test_name, true, NULL, 0, NULL);
    return 0;
}

bool
test_suite_passed(const struct test_suite *suite) {
    return suite->nb_passed_tests == suite->nb_tests;
}

void
test_suite_print_results(const struct test_suite *suite) {
    if (!suite->result_printer)
        return;

    suite->result_printer(suite->output,
                          suite->nb_tests,
                          suite->nb_passed_tests, suite->nb_failed_tests);
}

void
test_suite_print_results_and_exit(struct test_suite *suite) {
    int exit_code;

    test_suite_print_results(suite);
    exit_code = test_suite_passed(suite) ? 0 : 1;
    test_suite_delete(suite);

    exit(exit_code);
}

void
test_abort(struct test_context *ctx, const char *file, int line,
           const char *fmt, ...) {
    struct test_suite *suite;
    char errmsg[TEST_ERROR_BUFSZ];
    va_list ap;

    suite = ctx->test_suite;

    if (suite->report_function) {
        va_start(ap, fmt);
        vsnprintf(errmsg, TEST_ERROR_BUFSZ, fmt, ap);
        va_end(ap);

        suite->report_function(suite->output, ctx->test_name, false,
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

        if (c == '"' || c == '\r' || c == '\n' || c == '\t') {
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
        } else if (isprint((unsigned char)c)) {
            if (olen >= sizeof(buf) - 1)
                goto overflow;

            *optr++ = data[i];

            olen += 1;
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
    return strdup(buf);

overflow:
    fprintf(stderr, "data too large for buffer\n");
    abort();
}

static void
test_usage(const char *argv0, int exit_code) {
    printf("Usage: %s [-hop]\n"
            "\n"
            "Options:\n"
            "  -h            display help\n"
            "  -f <format>   select the format used for output\n"
            "  -o <filename> print output to a file\n"
            "\n"
            "Formats:\n"
            "  terminal      human-readable text for ansi terminals\n"
            "  json          rfc 4627 format\n",
            argv0);
    exit(exit_code);
}

static void
test_die(const char *fmt, ...) {
    va_list ap;

    fprintf(stderr, "fatal error: ");

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    putc('\n', stderr);
    exit(1);
}
