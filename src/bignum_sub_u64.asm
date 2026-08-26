; -----------------------------------------------------------------------------
; @file    bignum_sub_u64.asm
; @author  git@bayborodov.com
; @version 1.0.0
; @date    29.07.2026
;
; @brief   Экстремально оптимизированная реализация вычитания 64-битного числа.
;
; @details
;   Использует System V AMD64 ABI.
;   Оптимизации:
;   - Branchless Overlap Check (проверка перекрытия без ветвлений)
;   - CF Preservation (сохранение флага переноса через lea/dec)
;   - Early Exit (мгновенное копирование остатка через SSE, если borrow = 0)
; -----------------------------------------------------------------------------

section .text
global bignum_sub_u64

; --- Константы ---
BIGNUM_CAPACITY         equ 32
BIGNUM_OFFSET_LEN       equ 256
BUF_SIZE                equ 264

BIGNUM_SUB_U64_OK                    equ  0
BIGNUM_SUB_U64_ERR_NULL_PTR          equ -1
BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT   equ -2
BIGNUM_SUB_U64_ERR_BUFFER_OVERLAP    equ -3
BIGNUM_SUB_U64_ERR_BAD_LENGTH        equ -4

align 16
bignum_sub_u64:
    ; Аргументы:
    ; rdi = bignum_t *result
    ; rsi = const bignum_t *a
    ; rdx = uint64_t b

    push    rbp
    mov     rbp, rsp
    push    r14
    push    r15

    ; 1. Проверка на NULL
    test    rdi, rdi
    je      .err_null
    test    rsi, rsi
    je      .err_null

    ; 2. Проверка длины a->len
    mov     r8, qword [rsi + BIGNUM_OFFSET_LEN]
    cmp     r8, BIGNUM_CAPACITY
    ja      .err_cap

    ; 3. Branchless проверка перекрытия буферов (разрешаем in-place rdi == rsi)
    cmp     rdi, rsi
    je      .overlap_ok
    mov     rax, rdi
    sub     rax, rsi
    mov     rcx, rax
    sar     rcx, 63
    xor     rax, rcx
    sub     rax, rcx        ; rax = abs(result - a)
    cmp     rax, BUF_SIZE
    jb      .err_overlap
.overlap_ok:

    ; 4. Fast path: вычитание нуля (b == 0)
    test    rdx, rdx
    jnz     .b_not_zero
    cmp     rdi, rsi
    je      .success        ; Если in-place и b==0, ничего делать не нужно

    ; Копируем a в result
    mov     rcx, r8
    mov     r14, rsi
    mov     r15, rdi
    jmp     .fast_copy      ; Используем SSE-копирование из основного алгоритма

.b_not_zero:
    ; 5. Проверка a->len == 0 (a = 0, b > 0 -> отрицательный результат)
    test    r8, r8
    jz      .err_negative

    ; 6. Fast path: a->len == 1
    cmp     r8, 1
    jne     .generic_sub
    mov     rax, [rsi]
    cmp     rax, rdx
    jb      .err_negative   ; a < b
    sub     rax, rdx
    mov     [rdi], rax

    ; Устанавливаем длину (0 если rax == 0, иначе 1)
    test    rax, rax
    setnz   cl
    movzx   rcx, cl
    mov     qword [rdi + BIGNUM_OFFSET_LEN], rcx
    jmp     .normalize      ; Хвост не является частью значения

.generic_sub:
    ; 7. Основной цикл вычитания (a->len > 1)
    ; Результат гарантированно положительный, так как a >= 2^64, а b < 2^64
    mov     rax, [rsi]
    sub     rax, rdx        ; Вычитаем b из младшего слова, устанавливаем CF
    mov     [rdi], rax

    mov     rcx, r8
    dec     rcx             ; rcx = количество оставшихся слов (a->len - 1)
    lea     r14, [rsi + 8]  ; Указатель на чтение
    lea     r15, [rdi + 8]  ; Указатель на запись

    align 16
.sub_loop:
    ; Аппаратная магия x86: инструкции JNC, LEA и DEC НЕ ПОРТЯТ флаг CF!
    ; Поэтому мы можем передавать borrow между итерациями без setc/shr.
    jnc     .fast_copy      ; Если заёма (borrow) больше нет, мгновенно копируем остаток

    mov     rax, [r14]
    sbb     rax, 0          ; Вычитаем borrow
    mov     [r15], rax

    lea     r14, [r14 + 8]
    lea     r15, [r15 + 8]
    dec     rcx
    jnz     .sub_loop
    jmp     .sub_done

    ; --- Блок быстрого копирования (Early Exit) ---
    align 16
.fast_copy:
    test    rcx, rcx
    jz      .sub_done
    mov     rax, rcx
    shr     rax, 1          ; rax = количество 16-байтных блоков
    jz      .fast_copy_odd

    align 16
.fast_copy_sse:
    movdqu  xmm0, [r14]
    movdqu  [r15], xmm0
    lea     r14, [r14 + 16]
    lea     r15, [r15 + 16]
    dec     rax
    jnz     .fast_copy_sse

.fast_copy_odd:
    test    rcx, 1
    jz      .sub_done
    mov     rax, [r14]
    mov     [r15], rax

    ; 8. Установка начальной длины
.sub_done:
    mov     qword [rdi + BIGNUM_OFFSET_LEN], r8

    ; 9. Нормализация результата (удаление ведущих нулей)
.normalize:
    mov     rcx, qword [rdi + BIGNUM_OFFSET_LEN]
    test    rcx, rcx
    jz      .success

    align 16
.norm_loop:
    mov     rax, [rdi + rcx*8 - 8]
    test    rax, rax
    jnz     .norm_found
    dec     rcx
    jnz     .norm_loop

.norm_found:
    mov     qword [rdi + BIGNUM_OFFSET_LEN], rcx

.success:
    mov     eax, BIGNUM_SUB_U64_OK
    jmp     .epilogue

.err_null:
    mov     eax, BIGNUM_SUB_U64_ERR_NULL_PTR
    jmp     .epilogue

.err_cap:
    mov     eax, BIGNUM_SUB_U64_ERR_BAD_LENGTH
    jmp     .epilogue

.err_overlap:
    mov     eax, BIGNUM_SUB_U64_ERR_BUFFER_OVERLAP
    jmp     .epilogue

.err_negative:
    mov     eax, BIGNUM_SUB_U64_ERR_NEGATIVE_RESULT

.epilogue:
    pop     r15
    pop     r14
    pop     rbp
    ret
