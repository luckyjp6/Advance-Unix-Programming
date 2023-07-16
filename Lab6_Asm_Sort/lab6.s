_start:
    lea rdx, [rsi-1]
    mov rsi, 0
    call RECURSION

RECURSION:
    cmp rsi, rdx
    jl CONT
    ret
CONT:
    push rdx
    push rsi
    mov rax, qword ptr[rdi + rsi*8]

OUTER_LOOP:
    cmp rsi, rdx
    je TO_NEXT
GET_SMALL:
    cmp rsi, rdx
    jge GET_BIG
    mov rbx, qword ptr[rdi + rdx*8]
    cmp rbx, rax
    jle GET_BIG
    sub rdx, 1
    jmp GET_SMALL
GET_BIG:
    cmp rsi, rdx
    jge TAIL
    mov rbx, qword ptr[rdi + rsi*8]
    cmp rbx, rax
    jg TAIL
    add rsi, 1
    jmp GET_BIG
TAIL:
    cmp rsi, rdx
    jge OUTER_LOOP
    mov rbx, qword ptr[rdi + rsi*8]
    xchg rbx, qword ptr[rdi + rdx*8]
    xchg rbx, qword ptr[rdi + rsi*8]

    jmp OUTER_LOOP

TO_NEXT:
    pop r8
    mov rax, qword ptr[rdi + rsi*8]
    xchg rax, qword ptr[rdi + r8*8]
    xchg rax, qword ptr[rdi + rsi*8]
    push rsi

    lea rdx, [rsi-1]
    mov rsi, r8
    call RECURSION
    pop rsi 
    pop rdx 
    add rsi, 1
    call RECURSION
    ret