/**
 * @file bignum_sub_u64.h
 * @brief Public API for subtracting one unsigned 64-bit word from a bignum.
 * @details The module exposes one operation over caller-owned bignum records. The
 *          result is written only after all validation and negative-result checks
 *          succeed; error returns therefore preserve the destination byte-for-byte.
 *          The operation is reentrant and uses no mutable global state.
 * @version 1.0.0
 * @since 1.0.0
 */
#ifndef BIGNUM_SUB_U64_H
#define BIGNUM_SUB_U64_H

#include <bignum.h>
#include <stdint.h>

#ifndef BIGNUM_CAPACITY
#error "bignum.h must define BIGNUM_CAPACITY"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reports the outcome of bignum_sub_u64.
 * @details A success status guarantees that result contains the normalized
 *          difference. Every error status guarantees that the caller-owned
 *          destination is unchanged and may be retried after correcting inputs.
 */
typedef enum bignum_sub_u64_status {
    BIGNUM_SUB_U64_OK = 0, /**< Difference stored; result is normalized and valid. */
    BIGNUM_SUB_U64_ERR_NULL_PTR = -1, /**< result or a is NULL; result is unchanged. */
    BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT = -2, /**< a is smaller than b; result is unchanged. */
    BIGNUM_SUB_U64_ERR_BUFFER_OVERLAP = -3, /**< Distinct records overlap; result is unchanged. */
    BIGNUM_SUB_U64_ERR_BAD_LENGTH = -4 /**< a->len exceeds BIGNUM_CAPACITY; result is unchanged. */
} bignum_sub_u64_status_t;

/**
 * @brief Subtracts one uint64_t value from a non-negative bignum.
 * @details Validation checks NULL pointers, the input length and forbidden
 *          partial overlap before reading or writing the destination. In-place
 *          operation (`result == a`) is allowed. A one-word input uses a direct
 *          comparison/subtraction; a multi-word input subtracts from the low
 *          word and propagates borrow through higher words, then removes leading
 *          zero words from the reported length. The unused physical tail is not
 *          part of the bignum value and is not required to be modified.
 * @param[out] result Caller-allocated destination record; caller retains ownership.
 *                    It may alias `a` exactly, but may not partially overlap `a`.
 * @param[in] a Caller-owned immutable input record with `0 <= a->len <= BIGNUM_CAPACITY`.
 * @param[in] b Unsigned 64-bit subtrahend; no ownership or lifetime transfer occurs.
 * @return A named bignum_sub_u64_status_t value. On BIGNUM_SUB_U64_OK, result is
 *         normalized and contains `a - b`; on every error, result is unchanged.
 * @pre result and a are valid bignum_t objects unless NULL is intentionally being tested.
 * @pre If result and a are distinct, their complete records do not partially overlap.
 * @post Success sets result->len to the normalized word count and writes all value words.
 * @warning The mathematical result must be non-negative; underflow is reported rather
 *          than represented with a sign. The function does not allocate memory.
 * @thread_safety Safe for concurrent calls when each call uses independent records.
 * @complexity O(a->len) time in the worst case and O(1) auxiliary space.
 */
bignum_sub_u64_status_t bignum_sub_u64(bignum_t *result, const bignum_t *a, uint64_t b);

#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_SUB_U64_H */
