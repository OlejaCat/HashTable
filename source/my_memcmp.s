global myMemcmp
section .text



myMemcmp:
    mov ecx, edx
    mov eax, 1
    shl eax, cl
    dec eax

    vmovdqu ymm0, [rdi]
    vmovdqu ymm1, [rsi]
    vpcmpeqb ymm2, ymm0, ymm1
    vpmovmskb edx, ymm2

    and edx, eax
    cmp edx, eax
    sete al
    movzx eax, al
    ret
;myMemcmp:
;.loop_4_bytes:
;    cmp rdx, 8
;    jb .reminder
;    mov rax, [rsi]
;    mov rcx, [rdi]
;    cmp rax, rcx
;    jne .not_equal
;    add rsi, 8
;    add rdi, 8
;    sub rdx, 8
;    jmp .loop_4_bytes
;
;.reminder:
;    test rdx, rdx
;    jz .equal
;
;    mov al, [rsi]
;    mov cl, [rdi]
;    cmp cl, al
;    jne .not_equal
;    inc rsi
;    inc rdi
;    dec rdx
;    jnz .reminder
;
;.equal:
;    xor eax, eax
;    inc eax
;    ret
;
;.not_equal:
;    xor rax, rax
;    ret

