# bignum_sub_u64 standard benchmark profile

The standard manifest is a deterministic smoke and regression matrix for the zero-subtrahend, one-word, borrow-chain, variable-length and near-capacity paths. Every profile is safe for the operation: no profile intentionally requests a negative result.

Build the two benchmark entry points with the selected implementation:

```bash
gcc -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -Iinclude -Ilibs/bignum-core/include \
  -Ilibs/benchmark-framework/dist -Ibenchmarks \
  benchmarks/bench_bignum_sub_u64.c \
  benchmarks/adapter/bignum_sub_u64_benchmark_adapter.c \
  build/bignum_sub_u64.o libs/bignum-core/build/bignum_core.o libs/bignum-init/build/bignum_init.o libs/bignum-init-u64/build/bignum_init_u64.o libs/bignum-init-from-array/build/bignum_init_from_array.o libs/bignum-normalize/build/bignum_normalize.o \
  libs/benchmark-framework/dist/libbenchmark_framework.a \
  -pthread -lm -o bin/bench_bignum_sub_u64_st

gcc -std=c11 -O2 -Wall -Wextra -Werror -pedantic -DBENCHMARK_MODE_MT \
  -Iinclude -Ilibs/bignum-core/include \
  -Ilibs/benchmark-framework/dist -Ibenchmarks \
  benchmarks/bench_bignum_sub_u64_mt.c \
  benchmarks/adapter/bignum_sub_u64_benchmark_adapter.c \
  build/bignum_sub_u64.o libs/bignum-core/build/bignum_core.o libs/bignum-init/build/bignum_init.o libs/bignum-init-u64/build/bignum_init_u64.o libs/bignum-init-from-array/build/bignum_init_from_array.o libs/bignum-normalize/build/bignum_normalize.o \
  libs/benchmark-framework/dist/libbenchmark_framework.a \
  -pthread -lm -o bin/bench_bignum_sub_u64_mt
```

Run a short matrix with three repetitions:

```bash
libs/benchmark-framework/dist/tools/bench_matrix \
  --manifest benchmarks/profiles/bignum_sub_u64_standard.json \
  --output benchmarks/reports/sub_u64_standard_matrix.json \
  --st-binary bin/bench_bignum_sub_u64_st \
  --mt-binary bin/bench_bignum_sub_u64_mt \
  --repetitions 3 --iterations 20000 --mt-total-iterations 40000 \
  --threads 2 --warmup 100 --data-count 64 --seed 12345

libs/benchmark-framework/dist/tools/benchmark_stats \
  --input benchmarks/reports/sub_u64_standard_matrix.json \
  --output benchmarks/reports/sub_u64_standard_summary.json \
  --allow-regressions
```

For candidate comparison, pass the reviewed C11 matrix as `--baseline` and use `--threshold-pct 5`. The output protocol must contain one `benchmark=...` line followed by `Benchmark finished.`.
