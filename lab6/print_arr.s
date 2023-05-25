
section .data
    numbers dq 8, 3, 7, 4
    n dq 4
    b equ $-n
    next_line db 0x0a
    len equ $-next_line
    deb dq 0,0,0,0

section .text
    global _start


RECURSION:

    cmp rsi, rdx
    jl CONT
    ret
CONT:
    push rdx
    push rsi
    mov rax, [rdi + rsi*8]

OUTER_LOOP:
    cmp rsi, rdx
    je TO_NEXT
GET_SMALL:
    cmp rsi, rdx
    jge GET_BIG
    mov rbx, [rdi + rdx*8]
    cmp rbx, rax
    jle GET_BIG
    sub rdx, 1
    jmp GET_SMALL
GET_BIG:
    cmp rsi, rdx
    jge TAIL
    mov rbx, [rdi + rsi*8]
    cmp rbx, rax
    jg TAIL
    add rsi, 1
    jmp GET_BIG
TAIL:

    cmp rsi, rdx
    jge OUTER_LOOP
    mov rbx, [rdi + rsi*8]
    xchg rbx, [rdi + rdx*8]
    xchg rbx, [rdi + rsi*8]


    jmp OUTER_LOOP

TO_NEXT:

    pop r8
    mov rax, [rdi + rsi*8]
    xchg rax, [rdi + r8*8]
    xchg rax, [rdi + rsi*8]
    push rsi

    lea rdx, [rsi-1]
    mov rsi, r8
    call RECURSION
    pop rsi 
    pop rdx 
    add rsi, 1
    call RECURSION
    ret

_start:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov rdi, numbers
    mov rsi, 0
    mov rdx, [n]
    sub rdx, 1
    call RECURSION

    jmp END
print_arr:
    push rax
    push rdi
    push rdx
    push rsi
    push rbx
    push rbp
    mov rbp, rsp
    sub rsp, 8

    mov rax, 1
    mov rdi, 1
    mov rdx, [rbx]
    add rdx, '0'
    mov [rbp-8], rdx
    lea rsi, [rbp-8]
    mov rdx, b
    syscall
    add rbx, 8
    
    mov rax, 1
    mov rdi, 1
    mov rdx, [rbx]
    add rdx, '0'
    mov [rbp-8], rdx
    lea rsi, [rbp-8]
    mov rdx, b
    syscall
    add rbx, 8

    mov rax, 1
    mov rdi, 1
    mov rdx, [rbx]
    add rdx, '0'
    mov [rbp-8], rdx
    lea rsi, [rbp-8]
    mov rdx, b
    syscall
    add rbx, 8

    mov rax, 1
    mov rdi, 1
    mov rdx, [rbx]
    add rdx, '0'
    mov [rbp-8], rdx
    lea rsi, [rbp-8]
    mov rdx, b
    syscall
    add rbx, 8

    mov rax, 1
    mov rdi, 1
    mov rsi, next_line
    mov rdx, len
    syscall

    add rsp, 8
    pop rbp
    pop rbx
    pop rsi
    pop rdx
    pop rdi
    pop rax
    ; pop rbx
    
    ret
END:
    mov rax, 60
	xor rdi, rdi
	syscall