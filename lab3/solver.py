from pwn import *
elf = ELF('./chals')
# elf = ELF('/proc/self/maps')
print("main =", hex(elf.symbols['main']))
print("{:<12s} {:<8s} {:<8s}".format("Func", "GOT", "Address"))
l = 0
flg = 0
for g in elf.got:
    # if "init" in g:
    #     print("{:<12s} {:<8x} {:<8x}".format(g, elf.got[g], elf.symbols[g]))
    if "code_" in g:
        flg = 1
    if flg:
        # print("{:<12s}".format(g[5:]))
        # l.append(g[5:])
        l += 1
        if "code_" not in g:
            print("{:<12s} {:<8x} {:<8x}".format(g, elf.got[g], elf.symbols[g]))
# print(len(l))
print(l)