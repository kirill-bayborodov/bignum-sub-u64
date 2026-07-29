/**
 * @file    bignum_sub_u64.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    29.07.2026
 *
 * @brief   Реализация модуля вычитания 64-битного числа из большого числа.
 */

#include "bignum_sub_u64.h"

/**
 * @brief Внутренняя функция для проверки недопустимого перекрытия буферов.
 *
 * @param[in] res Указатель на буфер результата.
 * @param[in] a   Указатель на входной буфер.
 * @return 1 если есть частичное перекрытие, 0 если всё в порядке или это in-place.
 */
static inline int check_buffer_overlap(const bignum_t *res, const bignum_t *a) {
    if (res == a) {
        return 0; // In-place операция (result == a) разрешена
    }
    
    const unsigned char *p_res = (const unsigned char *)res;
    const unsigned char *p_a   = (const unsigned char *)a;

    // Проверяем пересечение диапазонов памяти
    if ((p_res < p_a + sizeof(bignum_t)) && (p_a < p_res + sizeof(bignum_t))) {
        return 1;
    }
    return 0;
}

bignum_sub_u64_status_t bignum_sub_u64(bignum_t *result, const bignum_t *a, const uint64_t b) {
    if (!result || !a) {
        return BIGNUM_SUB_U64_ERR_NULL_PTR;
    }

    if (a->len > BIGNUM_CAPACITY) {
        return BIGNUM_SUB_U64_ERR_BAD_LENGTH;
    }

    if (check_buffer_overlap(result, a)) {
        return BIGNUM_SUB_U64_ERR_BUFFER_OVERLAP;
    }

    // Быстрый путь: вычитание нуля
    if (b == 0) {
        if (result != a) {
            for (size_t i = 0; i < a->len; i++) {
                result->words[i] = a->words[i];
            }
            result->len = a->len;
        }
        return BIGNUM_SUB_U64_OK;
    }

    // Обработка случая, когда a = 0 (а b > 0, так как b == 0 отсеяно выше)
    if (a->len == 0) {
        return BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT;
    }

    // Обработка случая, когда a состоит из одного слова
    if (a->len == 1) {
        if (a->words[0] < b) {
            return BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT;
        }
        result->words[0] = a->words[0] - b;
        result->len = (result->words[0] == 0) ? 0 : 1;
        return BIGNUM_SUB_U64_OK;
    }

    // a->len > 1, следовательно a >= 2^64, и результат точно будет положительным.
    // Выполняем вычитание первого слова и вычисляем заимствование (borrow).
    uint64_t borrow = (a->words[0] < b) ? 1 : 0;
    result->words[0] = a->words[0] - b;

    // Проходим по остальным словам
    for (size_t i = 1; i < a->len; i++) {
        uint64_t word = a->words[i];
        result->words[i] = word - borrow;
        // Заимствование возникает только если текущее слово меньше borrow (т.е. word == 0 и borrow == 1)
        borrow = (word < borrow) ? 1 : 0;
    }

    // Нормализация результата (удаление ведущих нулей)
    size_t new_len = a->len;
    while (new_len > 0 && result->words[new_len - 1] == 0) {
        new_len--;
    }
    result->len = new_len;

    return BIGNUM_SUB_U64_OK;
}

