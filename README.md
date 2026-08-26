# bignum-sub-u64

[![C/ASM CI](https://github.com/kirill-bayborodov/bignum-sub-u64/actions/workflows/ci.yml/badge.svg)](https://github.com/kirill-bayborodov/bignum-sub-u64/actions/workflows/ci.yml)
[![Latest release](https://img.shields.io/github/v/release/kirill-bayborodov/bignum-sub-u64?label=release)](https://github.com/kirill-bayborodov/bignum-sub-u64/releases/latest)

`bignum-sub-u64` subtracts one unsigned 64-bit word from a non-negative arbitrary-precision integer represented by `bignum_t`. The repository contains an independently testable C11 reference implementation and an x86-64 YASM implementation selected by the build configuration.

## Distribution

The module is maintained as a standalone repository and is also consumed by the [`bignum-lib`](https://github.com/kirill-bayborodov/bignum-lib) aggregate distribution. The operation has one public function, does not allocate memory, and uses caller-owned input and output records.

## Features

The API supports exact in-place operation, rejects partial buffer overlap, preserves the destination on every error, propagates borrow across arbitrary word lengths, and normalizes the reported result length. The C11 source is the correctness reference for coverage and baseline measurements. The YASM source uses the System V AMD64 ABI, a fast one-word path, borrow-preserving control flow, and a linear multi-word loop.

## Scope and invariants

The value is stored little-endian in `bignum_t.words`, with `words[0]` as the least significant word and `len` as the number of significant words. A zero value has `len == 0`. Valid input satisfies `0 <= a->len <= BIGNUM_CAPACITY`; a result is normalized by removing leading zero words from its logical length. Physical words beyond `len` are not part of the value and are not required to be modified.

The operation accepts `result == a` as the in-place form. Distinct records may not partially overlap. If validation fails or `a < b`, the function returns a named error status and leaves every byte of `result` unchanged.

## Dependencies

The build requires GNU Make, GCC, YASM and the standard POSIX pthread environment. Static analysis uses `cppcheck` when available. The `bignum-core` submodule supplies `bignum_t` and `BIGNUM_CAPACITY`; initialize it with `git submodule update --init --recursive`.

The reproducible benchmark workflow uses the v1.0.0 distribution of [`benchmark-framework`](https://github.com/kirill-bayborodov/benchmark-framework). Its compatibility bundle is installed under `libs/benchmark-framework/dist` and provides the `benchmark_framework.h` adapter API, `libbenchmark_framework.a`, matrix tools and JSON profiles.

## API

The public header declares one operation:

```c
bignum_sub_u64_status_t bignum_sub_u64(
    bignum_t *result, const bignum_t *a, uint64_t b);
```

`result` is a caller-allocated destination and `a` is a borrowed immutable input. On `BIGNUM_SUB_U64_OK`, `result` contains the normalized value `a - b`. On `BIGNUM_SUB_U64_ERR_NULL_PTR`, `BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT`, `BIGNUM_SUB_U64_ERR_BUFFER_OVERLAP` or `BIGNUM_SUB_U64_ERR_BAD_LENGTH`, `result` is unchanged.

## Build and test

Clone with the required dependency and select the reference or assembly implementation explicitly:

```bash
git clone --recurse-submodules https://github.com/kirill-bayborodov/bignum-sub-u64.git
cd bignum-sub-u64
make build CONFIG=debug USE_ASM=no
make test CONFIG=debug USE_ASM=no
make test CONFIG=release USE_ASM=yes
```

The standard test target runs deterministic unit tests, randomized robustness tests, the multithreaded reentrancy test and the distribution runner. Sanitizer checks are available through:

```bash
make test_sanitize SAN=address CONFIG=debug USE_ASM=no
make test_sanitize SAN=undefined CONFIG=debug USE_ASM=no
```

## C11 coverage

The reference translation unit is instrumented with GCC gcov flags and executed by the deterministic, randomized and MT test artifacts. The reviewed run reached 100.00% line coverage (39/39), 100.00% branch coverage (36/36) and 100.00% call coverage (1/1). The suite includes NULL, invalid length, partial overlap, zero input, zero subtrahend, one-word underflow, exact zero, maximum word, borrow propagation and transactional preservation scenarios.

A reproducible manual coverage command is:

```bash
mkdir -p /tmp/sub_u64_cov/bin
gcc -std=c11 -Wall -Wextra -pedantic -O0 -g --coverage \
  -Iinclude -Ilibs/bignum-core/include \
  -c src/bignum_sub_u64.c -o /tmp/sub_u64_cov/bignum_sub_u64.o
# Compile and run tests/test_bignum_sub_u64.c,
# tests/test_bignum_sub_u64_extra.c and tests/test_bignum_sub_u64_mt.c
# against the instrumented object, then run:
gcov -b -c -o /tmp/sub_u64_cov /tmp/sub_u64_cov/bignum_sub_u64.o
```

## Benchmark-framework workflow

The project-owned adapter is in `benchmarks/adapter/`. The ST and MT entry points delegate dataset lifecycle, timing and protocol output to benchmark-core. The domain-specific manifests are `benchmarks/profiles/bignum_sub_u64_standard.json` and `benchmarks/profiles/bignum_sub_u64_full.json`; each has an adjacent `.json.md` companion.

Build the benchmark binaries after building the selected implementation:

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -Iinclude -Ilibs/bignum-core/include \
  -Ilibs/benchmark-framework/dist -Ibenchmarks \
  benchmarks/bench_bignum_sub_u64.c \
  benchmarks/adapter/bignum_sub_u64_benchmark_adapter.c \
  build/bignum_sub_u64.o libs/bignum-core/build/bignum_core.o libs/bignum-init/build/bignum_init.o libs/bignum-init-u64/build/bignum_init_u64.o libs/bignum-init-from-array/build/bignum_init_from_array.o libs/bignum-normalize/build/bignum_normalize.o \
  libs/benchmark-framework/dist/libbenchmark_framework.a \
  -pthread -lm -o bin/bench_bignum_sub_u64_st

cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic -DBENCHMARK_MODE_MT \
  -Iinclude -Ilibs/bignum-core/include \
  -Ilibs/benchmark-framework/dist -Ibenchmarks \
  benchmarks/bench_bignum_sub_u64_mt.c \
  benchmarks/adapter/bignum_sub_u64_benchmark_adapter.c \
  build/bignum_sub_u64.o libs/bignum-core/build/bignum_core.o libs/bignum-init/build/bignum_init.o libs/bignum-init-u64/build/bignum_init_u64.o libs/bignum-init-from-array/build/bignum_init_from_array.o libs/bignum-normalize/build/bignum_normalize.o \
  libs/benchmark-framework/dist/libbenchmark_framework.a \
  -pthread -lm -o bin/bench_bignum_sub_u64_mt
```

Run the standard matrix and create a baseline summary:

```bash
libs/benchmark-framework/dist/tools/bench_matrix \
  --manifest benchmarks/profiles/bignum_sub_u64_standard.json \
  --output benchmarks/reports/c11_standard_matrix.json \
  --st-binary bin/bench_bignum_sub_u64_st \
  --mt-binary bin/bench_bignum_sub_u64_mt \
  --repetitions 3 --iterations 20000 --mt-total-iterations 40000 \
  --threads 2 --warmup 100 --data-count 64 --seed 12345

libs/benchmark-framework/dist/tools/benchmark_stats \
  --input benchmarks/reports/c11_standard_matrix.json \
  --output benchmarks/reports/c11_standard_summary.json \
  --allow-regressions
```

The candidate matrix uses the same manifest and parameters and passes the C11 matrix through `--baseline`. Successful programs publish one `benchmark=...` line followed by `Benchmark finished.`. The reviewed optimized YASM run showed up to 2.53x median speedup on selected multi-word profiles and no regression under the configured 5% gate; small one-word and zero-subtrahend paths are expected to be dominated by call and framework overhead.

## C/ASM boundary

Both implementations use the System V AMD64 ABI. `result`, `a` and `b` arrive in `rdi`, `rsi` and `rdx`; the named status is returned in `eax`. The YASM function preserves `rbp`, `r14` and `r15`, uses no calls and therefore requires no internal call-site stack alignment, and treats `bignum_t` as a contiguous array of `uint64_t` words followed by `size_t len` at byte offset 256. Validation and all error exits occur before destination writes.

## Installation and linking

The distribution target creates a generated single header and static archive directly under `dist/`. A consumer can link the archive as follows:

```bash
make dist CONFIG=release USE_ASM=yes
gcc application.c dist/libbignum_sub_u64.a \
  -Idist -no-pie -o application
```

For aggregate `bignum-lib` integration, prefer its generated single header and static archive. The integration runner is `tests/test_bignum_sub_u64_runner.c`.

## Performance recommendations

The YASM implementation already uses fast paths for zero subtrahend and one-word inputs, preserves the borrow flag across the multi-word loop, and skips work after borrow clears. Further gains should be measured rather than assumed. Candidate work includes specialized short-length paths, reducing framework preparation overhead in kernel-only measurements, and upstreaming a `.note.GNU-stack` section for the shared upstream assembly object.

## Contributing and release

Contributions should preserve the single-operation public API, transactional error contract, deterministic tests, randomized oracle coverage, adapter/profile companions and the C11/YASM differential harness. Before a release, run the C11 and YASM suites, sanitizers, coverage, benchmark matrix, JSON validation, static analysis and a clean working-tree review. CI workflows are the source of release automation.

## License

This project is released under the MIT License. See [LICENSE](LICENSE).
