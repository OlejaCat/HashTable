global myMemcpy
section .text

myMemcpy
    mov [rdi], [rsi]
    mov [rdi + 1], [rsi + 1]
    mov [rdi + 2], [rsi + 2]
    mov [rdi + 3], [rsi + 3]
    ret

