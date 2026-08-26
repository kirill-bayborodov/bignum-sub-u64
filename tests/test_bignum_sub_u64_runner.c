/**
 * @file    test_bignum_sub_u64_runner.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    29.07.2026
 *
 * @brief Интеграционный тест‑раннер для библиотеки libbignum_sub_u64.a.
 */
#include "bignum_sub_u64.h"
#include <assert.h>
#include <stdio.h>

int main() {
    printf("Running test: test_bignum_sub_u64_runner... ");
    bignum_t res = {.words = {0}, .len = 0};
    bignum_t a = {.words = {12345}, .len = 1};
    uint64_t b = 10000;

    bignum_sub_u64_status_t status = bignum_sub_u64(&res, &a, b);
    assert(status == BIGNUM_SUB_U64_OK);
    assert(res.len == 1 && res.words[0] == 2345);

    printf("PASSED\n");
    return 0;
}
