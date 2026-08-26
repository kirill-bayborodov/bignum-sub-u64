/**
 * @file bench_bignum_sub_u64_mt.c
 * @brief Multi-thread benchmark entry point for bignum_sub_u64.
 * @details benchmark-core assigns independent mutable states to workers and
 *          publishes one aggregate machine-readable protocol result.
 */
#include "benchmark_framework.h"
#include "adapter/bignum_sub_u64_benchmark_adapter.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
    benchmark_adapter_t adapter;
    if (bignum_sub_u64_benchmark_adapter_init(&adapter) !=
        BIGNUM_SUB_U64_BENCHMARK_SUCCESS) {
        return EXIT_FAILURE;
    }
    benchmark_core_status_t status = benchmark_core_run_mt(argc, argv, &adapter);
    return (status == BENCHMARK_CORE_STATUS_SUCCESS ||
            status == BENCHMARK_CORE_STATUS_HELP) ? EXIT_SUCCESS : EXIT_FAILURE;
}
