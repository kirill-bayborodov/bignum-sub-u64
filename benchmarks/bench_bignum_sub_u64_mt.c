/**
 * @file    bench_bignum_sub_u64_mt.c
 * @brief   Многопоточный микробенчмарк для профилирования bignum_sub_u64.
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    29.07.2026
 *
 * @details
 *   Для чистоты измерений все случайные данные генерируются заранее
 *   в основном потоке и передаются в рабочие потоки. Каждый поток
 *   выполняет свой набор вызовов bignum_sub_u64, используя
 *   общий пул предварительно сгенерированных данных.
 *
 * # Сборка
 * gcc -g -O2 -I include -I libs/bignum-common/include -no-pie -fno-omit-frame-pointer -pthread \
 *   benchmarks/bench_bignum_sub_u64_mt.c build/bignum_sub_u64.o \
 *   -o bin/bench_bignum_sub_u64_mt
 *
 * # Запуск perf
 * /usr/local/bin/perf record -F 9999 -o benchmarks/reports/report_bench_bignum_sub_u64_mt -g -- \
 *   bin/bench_bignum_sub_u64_mt
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <bignum.h>
#include "bignum_sub_u64.h"

// --- Локальные определения для компиляции ---
#define BIGNUM_CAPACITY 32

#ifndef ITER_PER_THREAD
#  define ITER_PER_THREAD (20000000u * 20)
#endif

#ifndef THREAD_COUNT
#  define THREAD_COUNT 4
#endif

#define PREGEN_DATA_COUNT 8192

// Структура для передачи данных в поток
typedef struct {
    unsigned thread_id;
    unsigned iters;
    const bignum_t* a;       // Указатель на общий пул исходных чисел
    const uint64_t* b;       // Указатель на общий пул 64-битных вычитаемых
    unsigned data_count;     // Размер пула
} thread_arg_t;

/** Инициализация случайного bignum_t */
static void init_random_bignum(bignum_t *num) {
    int used = (rand() % BIGNUM_CAPACITY) + 1;
    num->len = used;
    for (int i = 0; i < used; ++i) {
        num->words[i] = ((uint64_t)rand() << 32) | rand();
    }
    for (int i = used; i < BIGNUM_CAPACITY; ++i) {
        num->words[i] = 0;
    }
}

/** Функция, исполняемая каждым потоком */
static void* thread_func(void *arg) {
    const thread_arg_t *t = arg;

    // Кэшируем значения локально для быстрого доступа
    const unsigned thread_id = t->thread_id;
    const unsigned iters = t->iters;
    const unsigned data_count = t->data_count;
    const bignum_t *a_pool = t->a;
    const uint64_t *b_pool = t->b;

    // Настройки прогресс-бара
    uint32_t progress_step = (iters >= 100) ? (iters / 100) : 1;
    uint32_t next_progress = progress_step;
    int percent = 0;

    for (unsigned i = 0; i < iters; ++i) {
        unsigned data_idx = (i + thread_id) % data_count;

        // Выделяем память под результат на стеке.
        bignum_t res_dst;

        // Вызов целевой функции
        bignum_sub_u64(&res_dst, &a_pool[data_idx], b_pool[data_idx]);

        // Проверяем длину РЕЗУЛЬТАТА, чтобы компилятор не вырезал вызов функции
        if (res_dst.len == 0xDEADBEEF) {
            return (void*)1;
        }

        /* Отрисовка прогресс-бара ТОЛЬКО нулевым потоком */
        if (thread_id == 0 && i + 1 == next_progress) {
            percent++;
            int pos = percent / 2;
            char bar[51];
            for (int j = 0; j < 50; ++j) {
                if (j < pos) bar[j] = '=';
                else if (j == pos && percent < 100) bar[j] = '>';
                else bar[j] = ' ';
            }
            bar[50] = '\0';
            printf("\r[%s] %d%%", bar, percent);
            fflush(stdout);
            next_progress += progress_step;
        }
    }

    if (thread_id == 0) {
        printf("\n");
    }

    return NULL;
}

int main(void) {
    // --- Фаза 1: Предварительная генерация данных в основном потоке ---
    printf("Pregenerating %u data sets for %u threads...\n", PREGEN_DATA_COUNT, THREAD_COUNT);

    bignum_t* a = malloc(sizeof(bignum_t) * PREGEN_DATA_COUNT);
    uint64_t* b = malloc(sizeof(uint64_t) * PREGEN_DATA_COUNT);

    if (!a || !b) {
        perror("Failed to allocate memory for test data");
        return 1;
    }

    srand((unsigned)time(NULL));
    for (unsigned i = 0; i < PREGEN_DATA_COUNT; ++i) {
        init_random_bignum(&a[i]);
        b[i] = ((uint64_t)rand() << 32) | rand();
        
        if (a[i].len == 1 && a[i].words[0] < b[i]) {
            a[i].words[0] = b[i] + (rand() % 1000);
        }
    }

    // --- Фаза 2: Запуск потоков и профилирование ---
    printf("Starting benchmark with %u threads, %u iterations each...\n", THREAD_COUNT, ITER_PER_THREAD);
    pthread_t threads[THREAD_COUNT];
    thread_arg_t args[THREAD_COUNT];

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        args[i].thread_id  = i;
        args[i].iters      = ITER_PER_THREAD;
        args[i].a          = a;
        args[i].b          = b;
        args[i].data_count = PREGEN_DATA_COUNT;
        if (pthread_create(&threads[i], NULL, thread_func, &args[i]) != 0) {
            perror("pthread_create");
            free(a);
            free(b);
            return 1;
        }
    }

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        void *res;
        pthread_join(threads[i], &res);
        if (res != NULL) {
            fprintf(stderr, "Error in thread %u\n", i);
        }
    }

    printf("Benchmark finished.\n");

    // --- Фаза 3: Очистка ---
    free(a);
    free(b);

    return 0;
}
