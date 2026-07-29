/**
 * @file    bignum_sub_u64.h
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    29.07.2026
 *
 * @brief   Публичный заголовочный файл для модуля вычитания 64-битного числа из большого числа.
 *
 * @details
 *   Определяет API для функции bignum_sub_u64, включая типы данных,
 *   коды состояния и прототипы функций.
 *
 * @see     bignum.h
 * @since   1.0.0
 *
 * @history
 *   - rev. 0 (29.07.2026): Первоначальное создание API по аналогии с bignum_div_u64.
 */

#ifndef BIGNUM_SUB_U64_H
#define BIGNUM_SUB_U64_H

#include <bignum.h>
#include <stddef.h>
#include <stdint.h>

// Проверка на наличие определения BIGNUM_CAPACITY из общего заголовка
#ifndef BIGNUM_CAPACITY
#  error "bignum.h must define BIGNUM_CAPACITY"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Коды состояния для функции bignum_sub_u64.
 */
typedef enum {
    BIGNUM_SUB_U64_OK                    =  0,
    BIGNUM_SUB_U64_ERR_NULL_PTR          = -1,
    BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT   = -2, /**< Результат отрицательный (a < b). */
    BIGNUM_SUB_U64_ERR_BUFFER_OVERLAP    = -3, /**< Обнаружено перекрытие буферов result и a. */
    BIGNUM_SUB_U64_ERR_BAD_LENGTH        = -4  /**< Ошибка: длина входного числа a->len превышает BIGNUM_CAPACITY. */
} bignum_sub_u64_status_t;

/**
 * @brief Выполняет вычитание 64-битного числа из большого беззнакового целого числа.
 *
 * @details
 *   ### Алгоритм
 *   1.  **Валидация:** Проверяются входные указатели `result` и `a` на `NULL`,
 *       а также буферы `result` и `a` на недопустимое перекрытие.
 *   2.  **Проверка длины:** Проверяется, что `a->len` не превышает `BIGNUM_CAPACITY`.
 *   3.  **Проверка знака:** Если `a` равно 0 (т.е. `a->len == 0`), а `b > 0`, 
 *       возвращается ошибка `BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT`.
 *   4.  **Вычитание:** Выполняется вычитание `b` из младшего слова `a` с 
 *       распространением заимствования (borrow) на старшие слова при необходимости.
 *   5.  **Нормализация:** Длина результата `result->len` устанавливается корректно,
 *       удаляя ведущие нули, если старшие слова обнулились.
 *
 * @param[out] result Указатель на структуру `bignum_t` для записи разности.
 * @param[in]  a      Указатель на `bignum_t`, представляющую уменьшаемое.
 * @param[in]  b      64-битное вычитаемое.
 *
 * @return bignum_sub_u64_status_t Код состояния операции.
 * @retval BIGNUM_SUB_U64_OK                    Успешное выполнение.
 * @retval BIGNUM_SUB_U64_ERR_NULL_PTR          Один из входных указателей `NULL`.
 * @retval BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT   Результат отрицательный (`a < b`).
 * @retval BIGNUM_SUB_U64_ERR_BUFFER_OVERLAP    Обнаружено перекрытие буферов `result` и `a`.
 * @retval BIGNUM_SUB_U64_ERR_BAD_LENGTH        Длина `a->len` превышает `BIGNUM_CAPACITY`.
 */
bignum_sub_u64_status_t bignum_sub_u64(bignum_t *result, const bignum_t *a, const uint64_t b);


#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_SUB_U64_H */
