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

#include <errno.h>
#include <stdio.h>

#include "utest.h"

char *test_json_escape(const char *);

void
test_report_json(FILE *output, const char *test_name, bool success,
                 const char *file, int line, const char *errmsg) {
    static __thread bool first_report = true;

    char *escaped_test_name;

    if (first_report) {
        fprintf(output, "     ");
    } else {
        fprintf(output, "    ,");
    }

    escaped_test_name = test_json_escape(test_name);
    fprintf(output, "\"%s\": {\n", escaped_test_name);
    free(escaped_test_name);

    if (success) {
        fprintf(output,
                "      \"passed\": true\n");
    } else {
        char *escaped_errmsg;

        escaped_errmsg = test_json_escape(errmsg);

        fprintf(output,
                "      \"passed\": false,\n"
                "      \"file\": \"%s\",\n"
                "      \"line\": %d,\n"
                "      \"error_message\": \"%s\"\n",
                file, line, escaped_errmsg);

        free(escaped_errmsg);
    }

    fprintf(output, "    }\n");

    first_report = false;
}

void
test_print_header_json(FILE *output, const char *suite_name) {
    char *escaped_suite_name;

    escaped_suite_name = test_json_escape(suite_name);

    fprintf(output,
            "{\n"
            "  \"name\": \"%s\",\n"
            "  \"tests\": {\n",
            escaped_suite_name);

    free(escaped_suite_name);
}

void
test_print_results_json(FILE *output, size_t nb_tests,
                        size_t nb_passed_tests, size_t nb_failed_tests) {
    fprintf(output,
            "  },\n"
            "  \"results\": {\n"
            "    \"nb_tests\": %zu,\n"
            "    \"nb_passed_tests\": %zu,\n"
            "    \"nb_failed_tests\": %zu\n"
            "  }\n"
            "}\n",
            nb_tests, nb_passed_tests, nb_failed_tests);
}

char *
test_json_escape(const char *str) {
    static const char *hex_digits = "0123456789abcdef";

    const char *iptr;
    char *escaped_str, *optr;
    size_t len;

    /* Compute the length of the escaped string */
    iptr = str;

    len = 0;
    while (*iptr != '\0') {
        if (*iptr == '"' || *iptr == '\\') {
            len += 2;
        } else if ((*iptr >= 0x00 && *iptr <= 0x1f) || *iptr == 0x7f) {
            len += 6;
        } else {
            len += 1;
        }

        iptr++;
    }

    /* Escape the string */
    escaped_str = malloc(len + 1);
    if (!escaped_str) {
        fprintf(stderr, "cannot allocate %zu bytes: %s\n",
                len, strerror(errno));
        abort();
    }

    iptr = str;
    optr = escaped_str;

    len = 0;
    while (*iptr != '\0') {
        unsigned char c;

        c = (unsigned char)*iptr;

        if (c == '"' || c == '\\') {
            *optr++ = '\\';
            *optr++ = *iptr++;
        } else if ((c >= 0x00 && c <= 0x1f) || c == 0x7f) {
            *optr++ = '\\';
            *optr++ = 'u';
            *optr++ = '0';
            *optr++ = '0';
            *optr++ = hex_digits[c >> 4];
            *optr++ = hex_digits[c & 0x0f];

            iptr++;
        } else {
            *optr++ = *iptr++;
        }
    }

    *optr = '\0';
    return escaped_str;
}
