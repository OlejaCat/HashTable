global myMemcmp
section .text

myMemcmp:
.loop_4_bytes:
    cmp rdx, 4
    jb .reminder
    mov eax, [rsi]
    mov ecx, [rdi]
    cmp eax, ecx
    jne .not_equal
    add rsi, 4
    add rdi, 4
    sub rdx, 4
    jmp .loop_4_bytes

.reminder:
    test rdx, rdx
    jz .equal

    mov al, [rsi]
    mov cl, [rdi]
    cmp cl, al
    jne .not_equal
    inc rsi
    inc rdi
    dec rdx
    jnz .reminder

.equal:
    xor eax, eax
    inc eax
    ret

.not_equal:
    xor rax, rax
    ret
