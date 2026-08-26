# bignum_sub_u64 full benchmark profile

The full manifest covers all supported operation tokens and timing boundaries across tiny, small, medium, variable, large and near-capacity records. `zero` input is paired only with a zero subtrahend; `borrow`, `one`, `max` and `default` construct non-negative successful operations.

The adapter interprets the workload axes as follows:

| Axis | Values | Meaning |
|---|---|---|
| `input_kind` | `zero`, `nonzero`, `mixed` | Canonical empty input, populated input, or deterministic mixed dataset metadata. |
| `operation_kind` | `zero`, `one`, `borrow`, `max`, `default` | Zero subtrahend, ordinary one-word subtraction, borrow propagation, maximal subtrahend, or seed-derived subtrahend. |
| `measure_mode` | `kernel-only`, `end-to-end` | Core timing boundary; the adapter does not reinterpret this field. |
| `size_profile` | `tiny`, `small`, `medium`, `large`, `variable` | Logical input length selected by the adapter. |
| `capacity_profile` | `normal`, `near-capacity` | Normal capacity or a full-capacity record. |

Build and execute the full matrix with the same commands as the standard profile, replacing the manifest path and increasing repetitions/iterations as appropriate for the host. A reviewed C11 matrix must be generated before an ASM candidate matrix so that the baseline is not replaced automatically.

```bash
libs/benchmark-framework/dist/tools/bench_matrix \
  --manifest benchmarks/profiles/bignum_sub_u64_full.json \
  --output benchmarks/reports/sub_u64_full_matrix.json \
  --st-binary bin/bench_bignum_sub_u64_st \
  --mt-binary bin/bench_bignum_sub_u64_mt \
  --repetitions 7 --iterations 200000 \
  --mt-total-iterations 400000 --threads 2 \
  --warmup 1000 --data-count 4096 --seed 12345

libs/benchmark-framework/dist/tools/benchmark_stats \
  --input benchmarks/reports/sub_u64_full_matrix.json \
  --output benchmarks/reports/sub_u64_full_summary.json \
  --baseline benchmarks/reports/sub_u64_c11_full_matrix.json \
  --threshold-pct 5
```

The matrix tool records profile metadata, process output, status and timing samples. The statistics tool reports median, mean, standard deviation and MAD for each profile/mode group. MT total iterations must be divisible by the thread count.
