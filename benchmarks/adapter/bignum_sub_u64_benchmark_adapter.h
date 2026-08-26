/**
 * @file bignum_sub_u64_benchmark_adapter.h
 * @brief Benchmark-framework adapter contract for bignum_sub_u64.
 * @details Maps generic workload metadata to deterministic subtraction states.
 * @version 1.0.0
 */
#ifndef BIGNUM_SUB_U64_BENCHMARK_ADAPTER_H
#define BIGNUM_SUB_U64_BENCHMARK_ADAPTER_H

#include "benchmark_framework.h"
#include "bignum.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum bignum_sub_u64_benchmark_status {
    BIGNUM_SUB_U64_BENCHMARK_SUCCESS = 0,
    BIGNUM_SUB_U64_BENCHMARK_NULL_ARGUMENT = 1,
    BIGNUM_SUB_U64_BENCHMARK_INVALID_PROFILE = 2
} bignum_sub_u64_benchmark_status_t;

typedef struct sub_u64_benchmark_state {
    bignum_t value;
    uint64_t subtrahend;
} sub_u64_benchmark_state_t;

/** @brief Initializes callbacks for the generic benchmark lifecycle. */
bignum_sub_u64_benchmark_status_t bignum_sub_u64_benchmark_adapter_init(
    benchmark_adapter_t *adapter);

/** @brief Validates workload vocabulary accepted by the subtraction adapter. */
bignum_sub_u64_benchmark_status_t bignum_sub_u64_benchmark_validate_workload(
    const benchmark_workload_t *workload);

#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_SUB_U64_BENCHMARK_ADAPTER_H */
