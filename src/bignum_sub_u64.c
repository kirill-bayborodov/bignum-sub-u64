/**
 * @file bignum_sub_u64.c
 * @brief C11 reference implementation of bignum_sub_u64.
 * @details The reference validates all inputs before mutating the destination,
 *          performs word-wise subtraction with borrow propagation, and publishes
 *          a normalized length. It is used for correctness, coverage and baseline
 *          benchmarking; the production build may select the independent YASM path.
 */
#include "bignum_sub_u64.h"

/**
 * @brief Detects forbidden partial overlap between two bignum records.
 * @details Exact aliasing is the supported in-place mode. For distinct records,
 *          the complete object ranges are compared so that no destination byte can
 *          overwrite an input byte during the operation.
 * @param[in] result Candidate destination record; it is known non-NULL by the caller.
 * @param[in] a Candidate input record; it is known non-NULL by the caller.
 * @return Non-zero when distinct record ranges overlap; zero otherwise.
 */
static inline int check_buffer_overlap(const bignum_t *result, const bignum_t *a)
{
    if (result == a) return 0;

    const unsigned char *result_begin = (const unsigned char *)result;
    const unsigned char *input_begin = (const unsigned char *)a;
    const unsigned char *result_end = result_begin + sizeof(bignum_t);
    const unsigned char *input_end = input_begin + sizeof(bignum_t);
    return result_begin < input_end && input_begin < result_end;
}

/**
 * @brief Subtracts one uint64_t value from a caller-owned bignum record.
 * @details The function first validates pointers, length and overlap. It then
 *          handles zero-subtrahend copy, zero input, one-word subtraction, or
 *          multi-word borrow propagation. The destination is written only on a
 *          successful path; leading zero words are removed from result->len.
 * @param[out] result Caller-allocated destination; exact aliasing with a is allowed.
 * @param[in] a Caller-owned immutable input with a valid length bound.
 * @param[in] b Unsigned 64-bit subtrahend.
 * @return Named bignum_sub_u64_status_t; all errors preserve result unchanged.
 */
bignum_sub_u64_status_t bignum_sub_u64(bignum_t *result, const bignum_t *a, uint64_t b)
{
    if (result == NULL || a == NULL) {
        return BIGNUM_SUB_U64_ERR_NULL_PTR;
    }

    /* Bounds are checked before dereferencing any input word or writing output. */
    if (a->len > BIGNUM_CAPACITY) {
        return BIGNUM_SUB_U64_ERR_BAD_LENGTH;
    }
    if (check_buffer_overlap(result, a)) {
        return BIGNUM_SUB_U64_ERR_BUFFER_OVERLAP;
    }

    /* A zero subtrahend is a value-preserving copy, including the empty value. */
    if (b == 0U) {
        if (result != a) {
            for (size_t index = 0U; index < a->len; ++index) {
                result->words[index] = a->words[index];
            }
            result->len = a->len;
        }
        return BIGNUM_SUB_U64_OK;
    }

    /* An empty non-zero input cannot represent a negative result. */
    if (a->len == 0U) {
        return BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT;
    }

    /* The one-word case can reject underflow before touching the destination. */
    if (a->len == 1U) {
        if (a->words[0] < b) {
            return BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT;
        }
        result->words[0] = a->words[0] - b;
        result->len = result->words[0] == 0U ? 0U : 1U;
        return BIGNUM_SUB_U64_OK;
    }

    /* For a multi-word input, a->len > 1 guarantees a >= 2^64 > b. */
    uint64_t borrow = a->words[0] < b ? 1U : 0U;
    result->words[0] = a->words[0] - b;
    for (size_t index = 1U; index < a->len; ++index) {
        uint64_t word = a->words[index];
        result->words[index] = word - borrow;
        borrow = word < borrow ? 1U : 0U;
    }

    /* The arithmetic result is written before canonical length is published. */
    size_t new_length = a->len;
    while (new_length > 0U && result->words[new_length - 1U] == 0U) {
        --new_length;
    }
    result->len = new_length;
    return BIGNUM_SUB_U64_OK;
}
