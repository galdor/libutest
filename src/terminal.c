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
#include <stdio.h>

#include "utest.h"

static char *test_escape_string_for_display(const char *);

void
test_report_terminal(FILE *output, const char *test_name, bool success,
                     const char *file, int line, const char *errmsg) {

    if (success) {
        fprintf(output, "\e[32m.\e[0m %-24s  \e[32mok\e[0m\n",
                test_name);
    } else {
        char *escaped_errmsg;

        escaped_errmsg = test_escape_string_for_display(errmsg);

        fprintf(output, "\e[31mx\e[0m %-24s  %s:%d  \e[31m%s\e[0m\n",
                test_name, file, line, escaped_errmsg);

        free(escaped_errmsg);
    }
}

void
test_print_header_terminal(FILE *output, const char *suite_name) {
    fprintf(output, "----------------------------------------"
            "----------------------------------------\n");
    fprintf(output, " %s\n", suite_name);
    fprintf(output, "----------------------------------------"
            "----------------------------------------\n\n");
}

void
test_print_results_terminal(FILE *output, size_t nb_tests,
                            size_t nb_passed_tests, size_t nb_failed_tests) {
    double ratio_passed, ratio_failed;

    ratio_passed = (double)nb_passed_tests / (double)nb_tests;
    ratio_failed = (double)nb_failed_tests / (double)nb_tests;

    putc('\n', output);

    fprintf(output, "%-16s  %zu\n", "Tests executed:",
            nb_tests);
    fprintf(output, "%-16s  %zu (%.0f%%)\n", "Tests passed:",
            nb_passed_tests, ratio_passed * 100.0);
    fprintf(output, "%-16s  %zu (%.0f%%)\n", "Tests failed:",
            nb_failed_tests, ratio_failed * 100.0);
}

static char *
test_escape_string_for_display(const char *string) {
    char *escaped_string, *optr;
    const char *iptr;

    /* Even when escaping each character with its value (\xxx), we will never
     * use more than strlen(errmsg) * 4 bytes, plus 1 for the final \0 */
    escaped_string = malloc(strlen(string) * 4 + 1);
    optr = escaped_string;

    for (iptr = string; *iptr != '\0'; iptr++) {
        switch (*iptr) {
        case '\a': *optr++ = '\\'; *optr++ = 'a'; break;
        case '\b': *optr++ = '\\'; *optr++ = 'b'; break;
        case '\t': *optr++ = '\\'; *optr++ = 't'; break;
        case '\n': *optr++ = '\\'; *optr++ = 'n'; break;
        case '\v': *optr++ = '\\'; *optr++ = 'v'; break;
        case '\f': *optr++ = '\\'; *optr++ = 'f'; break;
        case '\r': *optr++ = '\\'; *optr++ = 'r'; break;

        default:
            if (isprint((unsigned char)*iptr)) {
                *optr++ = *iptr;
            } else {
                *optr++ = '\\';
                sprintf(optr, "%03u", (unsigned char)(*iptr));
                optr += 3;
            }
            break;
        }
    }

    *optr++ = '\0';

    return escaped_string;
}
