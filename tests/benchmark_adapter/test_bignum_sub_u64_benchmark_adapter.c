/**
 * @file test_bignum_sub_u64_benchmark_adapter.c
 * @brief Deterministic tests for the bignum-sub-u64 benchmark adapter.
 */
#include "bignum_sub_u64_benchmark_adapter.h"

#include <stdint.h>
#include <string.h>

/** @brief Creates one valid workload descriptor for adapter tests. */
static benchmark_workload_t make_workload(void)
{
    return (benchmark_workload_t){
        .data_mode = "custom",
        .input_kind = "nonzero",
        .operation_kind = "borrow",
        .measure_mode = "kernel-only",
        .size_profile = "medium",
        .capacity_profile = "normal",
        .seed = UINT64_C(0x123456789abcdef0),
        .warmup = 2U,
        .data_count = 4U
    };
}

/** @brief Checks accepted and rejected workload vocabulary. */
static int test_validation(void)
{
    benchmark_workload_t workload = make_workload();
    if (bignum_sub_u64_benchmark_validate_workload(&workload) !=
        BIGNUM_SUB_U64_BENCHMARK_SUCCESS) return 0;
    workload.operation_kind = "negative-result";
    if (bignum_sub_u64_benchmark_validate_workload(&workload) !=
        BIGNUM_SUB_U64_BENCHMARK_INVALID_PROFILE) return 0;
    return bignum_sub_u64_benchmark_validate_workload(NULL) ==
        BIGNUM_SUB_U64_BENCHMARK_NULL_ARGUMENT;
}

/** @brief Checks initialization, borrow operation and checksum observability. */
static int test_callbacks(void)
{
    benchmark_workload_t workload = make_workload();
    benchmark_adapter_t adapter;
    sub_u64_benchmark_state_t state;
    uint64_t before;
    if (bignum_sub_u64_benchmark_adapter_init(&adapter) !=
        BIGNUM_SUB_U64_BENCHMARK_SUCCESS) return 0;
    if (bignum_sub_u64_benchmark_adapter_init(NULL) !=
        BIGNUM_SUB_U64_BENCHMARK_NULL_ARGUMENT) return 0;
    if (adapter.state_size != sizeof(state) || adapter.initialize == NULL ||
        adapter.operation == NULL || adapter.checksum == NULL) return 0;
    if (adapter.initialize(&state, 0U, &workload, adapter.adapter_context) !=
        BENCHMARK_ADAPTER_STATUS_SUCCESS) return 0;
    before = adapter.checksum(&state, 0U, adapter.adapter_context);
    if (adapter.operation(&state, 0U, &workload, adapter.adapter_context) !=
        BENCHMARK_ADAPTER_STATUS_SUCCESS) return 0;
    return adapter.checksum(&state, 1U, adapter.adapter_context) != before &&
           state.value.len <= BIGNUM_CAPACITY;
}

int main(void)
{
    return test_validation() && test_callbacks() ? 0 : 1;
}
