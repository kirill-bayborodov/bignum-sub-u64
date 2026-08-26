/**
 * @file    test_bignum_sub_u64.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    29.07.2026
 *
 * @brief   Детерминированные тесты для модуля bignum_sub_u64.
 */

#include "bignum_sub_u64.h"
#include <bignum_common.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define BIGNUM_CMP_GREATER     1
#define BIGNUM_CMP_EQ          0
#define BIGNUM_CMP_LESS       -1
#define BIGNUM_CMP_ERROR_NULL (int)0x80000000

int bignum_cmp(const bignum_t* a, const bignum_t* b) {
    if (!a || !b) return BIGNUM_CMP_ERROR_NULL;
    if (a->len != b->len) return (a->len > b->len) ? BIGNUM_CMP_GREATER : BIGNUM_CMP_LESS;
    if (a->len == 0) return BIGNUM_CMP_EQ;
    for (size_t i = a->len; i > 0; --i) {
        uint64_t word_a = a->words[i - 1];
        uint64_t word_b = b->words[i - 1];
        if (word_a != word_b) return (word_a > word_b) ? BIGNUM_CMP_GREATER : BIGNUM_CMP_LESS;
    }
    return BIGNUM_CMP_EQ;
}

#define RUN_TEST(test_func) \
    do { \
        printf("Running %s...\n", #test_func); \
        if (test_func()) { \
            printf("  %s: PASSED\n", #test_func); \
            tests_passed++; \
        } else { \
            printf("  %s: FAILED\n", #test_func); \
            tests_failed++; \
        } \
    } while (0)

static int tests_passed = 0;
static int tests_failed = 0;

// --- Тесты на "счастливые пути" ---

int test_simple_sub(void) {
    bignum_t a, result, expected;
    bignum_init(&a); bignum_init(&result); bignum_init(&expected);
    bignum_init_from_array(&a, (uint64_t[]){10}, 1);
    bignum_init_from_array(&expected, (uint64_t[]){5}, 1);

    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 5);
    return status == BIGNUM_SUB_U64_OK && bignum_cmp(&result, &expected) == BIGNUM_CMP_EQ && result.len == 1;
}

int test_sub_with_borrow(void) {
    bignum_t a, result, expected;
    bignum_init(&a); bignum_init(&result); bignum_init(&expected);
    bignum_init_from_array(&a, (uint64_t[]){0, 1}, 2); // 2^64
    bignum_init_from_array(&expected, (uint64_t[]){0xFFFFFFFFFFFFFFFFULL}, 1);

    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 1);
    return status == BIGNUM_SUB_U64_OK && bignum_cmp(&result, &expected) == BIGNUM_CMP_EQ && result.len == 1;
}

int test_sub_a_longer_no_borrow(void) {
    bignum_t a, result, expected;
    bignum_init(&a); bignum_init(&result); bignum_init(&expected);
    bignum_init_from_array(&a, (uint64_t[]){10, 20}, 2);
    bignum_init_from_array(&expected, (uint64_t[]){5, 20}, 2);

    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 5);
    return status == BIGNUM_SUB_U64_OK && bignum_cmp(&result, &expected) == BIGNUM_CMP_EQ && result.len == 2;
}

int test_multi_word_borrow_chain(void) {
    bignum_t a, result, expected;
    bignum_init(&a); bignum_init(&result); bignum_init(&expected);
    bignum_init_from_array(&a, (uint64_t[]){0, 0, 1}, 3); // 2^128
    bignum_init_from_array(&expected, (uint64_t[]){0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL}, 2);

    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 1);
    return status == BIGNUM_SUB_U64_OK && bignum_cmp(&result, &expected) == BIGNUM_CMP_EQ && result.len == 2;
}

// --- Тесты на граничные случаи и нормализацию ---

int test_sub_to_zero_and_normalize(void) {
    bignum_t a, result, expected;
    bignum_init(&a); bignum_init(&result); bignum_init(&expected);
    bignum_init_from_array(&a, (uint64_t[]){100}, 1);
    bignum_init_from_array(&expected, (uint64_t[]){0}, 0);

    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 100);
    return status == BIGNUM_SUB_U64_OK && bignum_cmp(&result, &expected) == BIGNUM_CMP_EQ && result.len == 0;
}

int test_sub_zero_operand(void) {
    bignum_t a, result, expected;
    bignum_init(&a); bignum_init(&result); bignum_init(&expected);
    uint64_t arr_a[] = {123, 456};
    bignum_init_from_array(&a, arr_a, 2);
    bignum_init_from_array(&expected, arr_a, 2);

    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 0);
    return status == BIGNUM_SUB_U64_OK && bignum_cmp(&result, &expected) == BIGNUM_CMP_EQ && result.len == 2;
}

int test_in_place_sub(void) {
    bignum_t a, expected;
    bignum_init(&a); bignum_init(&expected);
    bignum_init_from_array(&a, (uint64_t[]){10}, 1);
    bignum_init_from_array(&expected, (uint64_t[]){5}, 1);

    bignum_sub_u64_status_t status = bignum_sub_u64(&a, &a, 5);
    return status == BIGNUM_SUB_U64_OK && bignum_cmp(&a, &expected) == BIGNUM_CMP_EQ && a.len == 1;
}

// --- Тесты на обработку ошибок ---

int test_err_null_pointer(void) {
    bignum_t a, result;
    bignum_init(&a); bignum_init(&result);
    bignum_init_from_array(&a, (uint64_t[]){1}, 1);

    bool r1 = (bignum_sub_u64(NULL, &a, 1) == BIGNUM_SUB_U64_ERR_NULL_PTR);
    bool r2 = (bignum_sub_u64(&result, NULL, 1) == BIGNUM_SUB_U64_ERR_NULL_PTR);
    return r1 && r2;
}

int test_err_negative_result(void) {
    bignum_t a, result;
    bignum_init(&a); bignum_init(&result);
    bignum_init_from_array(&a, (uint64_t[]){5}, 1);
    return bignum_sub_u64(&result, &a, 10) == BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT;
}

int test_err_negative_zero_minus_nonzero(void) {
    bignum_t a, result;
    bignum_init(&a); bignum_init(&result);
    bignum_init_from_array(&a, (uint64_t[]){0}, 0); // a.len = 0
    return bignum_sub_u64(&result, &a, 1) == BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT;
}

int test_err_capacity_exceeded(void) {
    bignum_t a, result;
    bignum_init(&a); bignum_init(&result);
    bignum_init_from_array(&a, (uint64_t[]){1}, 1);
    a.len = BIGNUM_CAPACITY + 1;

    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 1);
    a.len = 1; // Восстанавливаем
    return status == BIGNUM_SUB_U64_ERR_BAD_LENGTH;
}

int test_err_buffer_overlap(void) {
    bignum_t a;
    bignum_init(&a);
    bignum_init_from_array(&a, (uint64_t[]){10}, 1);

    // Создаем указатель, который частично перекрывает a
    bignum_t *overlap_res = (bignum_t *)((unsigned char *)&a + 1);
    return bignum_sub_u64(overlap_res, &a, 5) == BIGNUM_SUB_U64_ERR_BUFFER_OVERLAP;
}

/** @brief Verifies that a negative-result error leaves the destination byte-for-byte unchanged. */
int test_transactional_negative(void) {
    bignum_t a, result, before;
    bignum_init_from_array(&a, (uint64_t[]){5, 0xA5A5A5A5A5A5A5A5ULL}, 1);
    memset(&result, 0x5C, sizeof(result));
    before = result;
    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 6);
    return status == BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT &&
           memcmp(&result, &before, sizeof(result)) == 0;
}

/** @brief Verifies that a bad input length leaves the destination unchanged. */
int test_transactional_bad_length(void) {
    bignum_t a, result, before;
    bignum_init_from_array(&a, (uint64_t[]){9}, 1);
    a.len = BIGNUM_CAPACITY + 1U;
    memset(&result, 0xC3, sizeof(result));
    before = result;
    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 1);
    return status == BIGNUM_SUB_U64_ERR_BAD_LENGTH &&
           memcmp(&result, &before, sizeof(result)) == 0;
}

/** @brief Verifies exact subtraction at the maximum one-word unsigned value. */
int test_max_word_exact_subtraction(void) {
    bignum_t a, result;
    bignum_init_from_array(&a, (uint64_t[]){UINT64_MAX}, 1);
    memset(&result, 0xA7, sizeof(result));
    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, UINT64_MAX);
    return status == BIGNUM_SUB_U64_OK && result.len == 0 &&
           result.words[0] == 0;
}

/** @brief Verifies a borrow which propagates across zero words to a high limb. */
int test_borrow_chain_with_prefilled_destination(void) {
    bignum_t a, result;
    bignum_init_from_array(&a,
        (uint64_t[]){0, 0, 0, UINT64_C(1)}, 4);
    memset(&result, 0xD1, sizeof(result));
    bignum_sub_u64_status_t status = bignum_sub_u64(&result, &a, 1);
    if (status != BIGNUM_SUB_U64_OK || result.len != 3) return 0;
    for (size_t i = 0; i < 3; ++i)
        if (result.words[i] != UINT64_MAX) return 0;
    return 1;
}

/** @brief Verifies that an in-place zero subtraction preserves the canonical value. */
int test_in_place_zero_subtraction(void) {
    bignum_t value;
    bignum_init_from_array(&value, (uint64_t[]){UINT64_C(7), UINT64_C(8)}, 2);
    bignum_sub_u64_status_t status = bignum_sub_u64(&value, &value, 0);
    return status == BIGNUM_SUB_U64_OK && value.len == 2 &&
           value.words[0] == 7 && value.words[1] == 8;
}

int main() {
    printf("\n--- Launching Deterministic Tests for bignum_sub_u64 ---\n");

    printf("\n--- Running Happy Path Tests ---\n");
    RUN_TEST(test_simple_sub);
    RUN_TEST(test_sub_with_borrow);
    RUN_TEST(test_sub_a_longer_no_borrow);
    RUN_TEST(test_multi_word_borrow_chain);

    printf("\n--- Running Boundary and Normalization Tests ---\n");
    RUN_TEST(test_sub_to_zero_and_normalize);
    RUN_TEST(test_sub_zero_operand);
    RUN_TEST(test_in_place_sub);

    printf("\n--- Running Error Handling Tests ---\n");
    RUN_TEST(test_err_null_pointer);
    RUN_TEST(test_err_negative_result);
    RUN_TEST(test_err_negative_zero_minus_nonzero);
    RUN_TEST(test_err_capacity_exceeded);
    RUN_TEST(test_err_buffer_overlap);
    RUN_TEST(test_transactional_negative);
    RUN_TEST(test_transactional_bad_length);
    RUN_TEST(test_max_word_exact_subtraction);
    RUN_TEST(test_borrow_chain_with_prefilled_destination);
    RUN_TEST(test_in_place_zero_subtraction);

    printf("\n--- Test Summary ---\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("\n----------------------\n");

    return tests_failed > 0 ? 1 : 0;
}
