#include "main.h"
#include "util/util.h"

void test_util_util_text_parse_ascii_digit() {
    ELMA_ASSERT(!util::text::parse_ascii_digit('/'));

    auto v0 = util::text::parse_ascii_digit('0');
    ELMA_ASSERT(v0);
    ELMA_ASSERT(*v0 == 0);

    auto v1 = util::text::parse_ascii_digit('1');
    ELMA_ASSERT(v1);
    ELMA_ASSERT(*v1 == 1);

    auto v2 = util::text::parse_ascii_digit('2');
    ELMA_ASSERT(v2);
    ELMA_ASSERT(*v2 == 2);

    auto v3 = util::text::parse_ascii_digit('3');
    ELMA_ASSERT(v3);
    ELMA_ASSERT(*v3 == 3);

    auto v4 = util::text::parse_ascii_digit('4');
    ELMA_ASSERT(v4);
    ELMA_ASSERT(*v4 == 4);

    auto v5 = util::text::parse_ascii_digit('5');
    ELMA_ASSERT(v5);
    ELMA_ASSERT(*v5 == 5);

    auto v6 = util::text::parse_ascii_digit('6');
    ELMA_ASSERT(v6);
    ELMA_ASSERT(*v6 == 6);

    auto v7 = util::text::parse_ascii_digit('7');
    ELMA_ASSERT(v7);
    ELMA_ASSERT(*v7 == 7);

    auto v8 = util::text::parse_ascii_digit('8');
    ELMA_ASSERT(v8);
    ELMA_ASSERT(*v8 == 8);

    auto v9 = util::text::parse_ascii_digit('9');
    ELMA_ASSERT(v9);
    ELMA_ASSERT(*v9 == 9);

    ELMA_ASSERT(!util::text::parse_ascii_digit(':'));
}
