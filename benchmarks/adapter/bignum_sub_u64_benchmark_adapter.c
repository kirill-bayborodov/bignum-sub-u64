/**
 * @file bignum_sub_u64_benchmark_adapter.c
 * @brief Benchmark-framework callbacks for bignum_sub_u64.
 * @details Each source record is normalized and uses an in-place subtraction
 *          whose result remains non-negative for every supported profile.
 * @version 1.0.0
 */
#include "bignum_sub_u64_benchmark_adapter.h"
#include "bignum_sub_u64.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define SUB_U64_FNV_OFFSET UINT64_C(1469598103934665603)
#define SUB_U64_FNV_PRIME UINT64_C(1099511628211)

/** @brief Compares two non-NULL workload tokens. */
static int equal_text(const char *left, const char *right)
{
    return left != NULL && right != NULL && strcmp(left, right) == 0;
}

/** @brief Tests whether a token belongs to a NULL-terminated vocabulary. */
static int allowed(const char *value, const char *const *list)
{
    if (value == NULL || list == NULL) return 0;
    for (size_t index = 0U; list[index] != NULL; ++index) {
        if (equal_text(value, list[index])) return 1;
    }
    return 0;
}

/** @brief Advances the deterministic adapter state generator. */
static uint64_t next_value(uint64_t *state)
{
    if (*state == 0U) *state = UINT64_C(0x9e3779b97f4a7c15);
    *state ^= *state << 7U;
    *state ^= *state >> 9U;
    *state ^= *state << 8U;
    return *state;
}

/** @brief Maps a size token to a valid bignum word length. */
static size_t choose_length(const benchmark_workload_t *workload)
{
    if (equal_text(workload->capacity_profile, "near-capacity")) {
        return BIGNUM_CAPACITY;
    }
    if (equal_text(workload->size_profile, "tiny")) return 1U;
    if (equal_text(workload->size_profile, "small")) return 4U;
    if (equal_text(workload->size_profile, "medium")) return BIGNUM_CAPACITY / 2U;
    if (equal_text(workload->size_profile, "large")) return BIGNUM_CAPACITY - 1U;
    return 2U + (size_t)(workload->seed % (BIGNUM_CAPACITY - 2U));
}

/** @brief Initializes one normalized subtraction source record. */
static void fill_state(sub_u64_benchmark_state_t *state,
                       const benchmark_workload_t *workload,
                       uint64_t sequence_index)
{
    uint64_t random_state = workload->seed ^ (sequence_index + UINT64_C(0x9e3779b97f4a7c15));
    const size_t length = choose_length(workload);
    memset(state, 0, sizeof(*state));

    if (equal_text(workload->input_kind, "zero")) {
        state->value.len = 0U;
        state->subtrahend = 0U;
        return;
    }

    state->value.len = length;
    for (size_t index = 0U; index < length; ++index) {
        state->value.words[index] = next_value(&random_state);
    }
    if (state->value.words[length - 1U] == 0U) state->value.words[length - 1U] = 1U;

    if (equal_text(workload->operation_kind, "zero")) {
        state->subtrahend = 0U;
    } else if (equal_text(workload->operation_kind, "max")) {
        state->subtrahend = UINT64_MAX;
        state->value.words[0] = UINT64_MAX;
    } else if (equal_text(workload->operation_kind, "borrow")) {
        state->subtrahend = 1U;
        state->value.words[0] = 0U;
        if (length > 1U) state->value.words[1] |= UINT64_C(1);
    } else {
        state->subtrahend = state->value.words[0] >> 1U;
    }
}

/** @brief Initializes one framework source state from validated metadata. */
static benchmark_adapter_status_t initialize(void *opaque, uint64_t sequence_index,
                                              const benchmark_workload_t *workload,
                                              void *context)
{
    (void)context;
    if (opaque == NULL || workload == NULL ||
        bignum_sub_u64_benchmark_validate_workload(workload) !=
            BIGNUM_SUB_U64_BENCHMARK_SUCCESS) {
        return BENCHMARK_ADAPTER_STATUS_INPUT_ERROR;
    }
    fill_state((sub_u64_benchmark_state_t *)opaque, workload, sequence_index);
    return BENCHMARK_ADAPTER_STATUS_SUCCESS;
}

/** @brief Executes one safe in-place subtraction on the mutable state. */
static benchmark_adapter_status_t operation(void *opaque, uint64_t iteration,
                                             const benchmark_workload_t *workload,
                                             void *context)
{
    sub_u64_benchmark_state_t *state = opaque;
    (void)iteration;
    (void)workload;
    (void)context;
    if (state == NULL || bignum_sub_u64(&state->value, &state->value,
                                        state->subtrahend) != BIGNUM_SUB_U64_OK) {
        return BENCHMARK_ADAPTER_STATUS_OPERATION_ERROR;
    }
    return BENCHMARK_ADAPTER_STATUS_SUCCESS;
}

/** @brief Hashes the complete post-operation state for observability. */
static uint64_t checksum(const void *opaque, uint64_t iteration, void *context)
{
    const sub_u64_benchmark_state_t *state = opaque;
    uint64_t hash = SUB_U64_FNV_OFFSET;
    (void)context;
    if (state == NULL) return 0U;
    for (size_t index = 0U; index < BIGNUM_CAPACITY; ++index) {
        hash ^= state->value.words[index];
        hash *= SUB_U64_FNV_PRIME;
    }
    hash ^= (uint64_t)state->value.len;
    hash *= SUB_U64_FNV_PRIME;
    hash ^= state->subtrahend;
    return (hash * SUB_U64_FNV_PRIME) ^ iteration;
}

bignum_sub_u64_benchmark_status_t bignum_sub_u64_benchmark_validate_workload(
    const benchmark_workload_t *workload)
{
    static const char *const inputs[] = {"zero", "nonzero", "mixed", NULL};
    static const char *const operations[] = {"zero", "default", "one", "borrow", "max", NULL};
    static const char *const measures[] = {"end-to-end", "kernel-only", NULL};
    static const char *const sizes[] = {"tiny", "small", "medium", "large", "variable", NULL};
    static const char *const capacities[] = {"normal", "near-capacity", NULL};

    if (workload == NULL) return BIGNUM_SUB_U64_BENCHMARK_NULL_ARGUMENT;
    if (!allowed(workload->input_kind, inputs) ||
        !allowed(workload->operation_kind, operations) ||
        !allowed(workload->measure_mode, measures) ||
        !allowed(workload->size_profile, sizes) ||
        !allowed(workload->capacity_profile, capacities)) {
        return BIGNUM_SUB_U64_BENCHMARK_INVALID_PROFILE;
    }
    return BIGNUM_SUB_U64_BENCHMARK_SUCCESS;
}

bignum_sub_u64_benchmark_status_t bignum_sub_u64_benchmark_adapter_init(
    benchmark_adapter_t *adapter)
{
    if (adapter == NULL) return BIGNUM_SUB_U64_BENCHMARK_NULL_ARGUMENT;
    *adapter = (benchmark_adapter_t){
        .benchmark_name = "bignum_sub_u64",
        .state_size = sizeof(sub_u64_benchmark_state_t),
        .success_code = BENCHMARK_ADAPTER_STATUS_SUCCESS,
        .adapter_context = NULL,
        .initialize = initialize,
        .operation = operation,
        .checksum = checksum
    };
    return BIGNUM_SUB_U64_BENCHMARK_SUCCESS;
}
