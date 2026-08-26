# Documentation Quality Gate Report — bignum-sub-u64

**Review status:** PASS with one documented upstream warning.

**Reviewed revision:** working tree after the C11 documentation repair, benchmark-framework migration and YASM optimization. Protected CI workflow and Makefile were not modified.

## Scope and evidence

The review covers the public header, C11 implementation, YASM implementation, deterministic tests, randomized tests, MT test, distribution runner, benchmark adapter header/source, ST/MT benchmark entry points, standard/full JSON manifests, JSON companions and README. JSON syntax was checked with `jq`; benchmark protocol was checked by ST and MT smoke runs; correctness was checked with C11/YASM tests and an independent 10,000-case C11/YASM differential harness.

## Artifact checklist

| Artifact | QG result | Evidence |
|---|---|---|
| `include/bignum_sub_u64.h` | PASS | File/type/status/function Doxygen; one public operation; ownership, aliasing, transactional errors, thread-safety and complexity are explicit. |
| `src/bignum_sub_u64.c` | PASS | Reference algorithm and every internal helper documented; validation, borrow propagation and normalization rationale are local. |
| `src/bignum_sub_u64.asm` | PASS | ABI registers, offsets, status codes, preserved registers, borrow-flag flow and fast paths are documented. |
| `tests/test_bignum_sub_u64.c` | PASS | Deterministic edge cases include NULL, bad length, partial overlap, zero input, zero subtrahend, exact zero, maximum word, borrow chain and transactional preservation. |
| `tests/test_bignum_sub_u64_extra.c` | PASS | Randomized robustness artifact with 10,000 iterations and invariant checks. |
| `tests/test_bignum_sub_u64_mt.c` | PASS | Dynamic pthread reentrancy test with independent thread-local records. |
| `tests/test_bignum_sub_u64_runner.c` | PASS | Distribution integration smoke test. |
| `benchmarks/adapter/bignum_sub_u64_benchmark_adapter.h` | PASS | Adapter status type, state lifecycle and public adapter functions documented. |
| `benchmarks/adapter/bignum_sub_u64_benchmark_adapter.c` | PASS | Static helpers and callbacks documented; deterministic safe workload generation and checksum rationale are explicit. |
| `benchmarks/bench_bignum_sub_u64.c` | PASS | ST entry point documents framework ownership and exit mapping. |
| `benchmarks/bench_bignum_sub_u64_mt.c` | PASS | MT entry point documents independent worker state and protocol behavior. |
| `benchmarks/profiles/bignum_sub_u64_standard.json` | PASS | Valid schema and accepted adapter vocabulary; adjacent `.json.md` command guide exists. |
| `benchmarks/profiles/bignum_sub_u64_full.json` | PASS | Covers size, operation, timing and capacity axes; adjacent `.json.md` command guide exists. |
| `libs/benchmark-framework/dist` | PASS | Compatibility distribution contains framework header, static archive, tools, profiles and docs. |
| `README.md` | PASS | English template-style guide covers scope, invariants, API, build/test, coverage, benchmark workflow, installation, ABI and release. |

## Functional and performance evidence

The C11 suite completed with `0 / 4` failing binaries; deterministic tests report 17/17 passing. AddressSanitizer and UBSan runs completed with zero sanitizer issues. Manual gcov evidence for `src/bignum_sub_u64.c` is 100.00% line coverage (40/40), 100.00% branch coverage (32/32) and 100.00% call coverage (1/1).

The independent differential harness compared logical result words, normalized lengths and error statuses for 10,000 deterministic randomized cases. It also compared the complete destination bytes on error paths. The harness reported `10000 differential cases passed`.

The benchmark-framework standard matrix produced 48 samples for each implementation: eight profiles, ST and MT modes, and three repetitions. The optimized YASM candidate passed the configured 5% statistics gate against the C11 baseline. Representative median speedups were 1.72x for medium borrow ST, 2.53x for large max MT, 1.88x for variable default ST and 2.27x for near-capacity borrow ST. Small paths remain framework-overhead dominated.

## Known warning and recommendation

Linking the legacy `bignum-common` assembly object emits a warning because it lacks a `.note.GNU-stack` section. This is an upstream assembly hygiene issue and is not masked by this module. The recommended follow-up is an upstream patch adding the non-executable-stack note and a CI check for executable-stack regressions.

## Final decision

The module satisfies the documentation QG for the reviewed artifacts. The benchmark adapter uses the actual installed compatibility header `benchmark_framework.h`; the v1.0.0 framework source documentation identifies the corresponding core API. No undocumented project-owned contract changes remain.
