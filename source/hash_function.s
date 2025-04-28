;section .rodata
;remain_jump_table:
;    dq out
;    dq one_byte
;    dq two_bytes
;    dq three_bytes
; faster without jump table

; ------------------------------------------------------------------------------

section .text
global crc_hash

crc_hash:
    mov r8, rdi          
    lea r9, [rdi + rsi]  
    mov eax, 0xFFFFFFFF  

    lea r10, [r9 - 3]    
    cmp r8, r10
    jae .remain

.loop_4_bytes:
    mov edx, [r8]       
    ; sse4 crc32 hash function
    crc32 eax, edx        
    add r8, 4           
    cmp r8, r10
    jb  .loop_4_bytes

.remain:
    mov rdx, r9
    sub rdx, r8         
    test rdx, rdx
    jz out
    ;jmp [remain_jump_table + rdx * 8]

;three_bytes:
    movzx ecx, byte [r8]
    crc32 eax, cl
    inc r8

    cmp rdx, 2
    jb out

;two_bytes:
    movzx ecx, byte [r8]
    crc32 eax, cl
    inc r8

    cmp rdx, 3
    jb out

;one_byte:
    movzx ecx, byte [r8]
    crc32 eax, cl

out:
    not eax             
    and eax, 0xFFF
    ret
