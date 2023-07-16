"""
Used to get the GOT of the executable 'chals', 
the functions code_xxx() is not in order 
so we'll have to get the correct order of the functions
"""

from pwn import *
elf = ELF('./chals')
# elf = ELF('/proc/self/maps')
print("main =", hex(elf.symbols['main']))
print("{:<12s} {:<8s} {:<8s}".format("Func", "GOT", "Address"))
start = 0
for g in elf.got:
    if "code_" in g:
        start = 1
    if start:
        if "code_" not in g:
            print("{:<12s} {:<8x} {:<8x}".format(g, elf.got[g], elf.symbols[g]))