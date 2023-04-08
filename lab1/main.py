# #!/usr/bin/env python3
# # -*- coding: utf-8 -*-

import base64
import hashlib
import time
import struct
from pwn import *

#r = remote('localhost', 10011)
r = remote('up23.zoolab.org', 10363)

def solve_pow():
    prefix = r.recvline().decode().split("'")[1]
    print(time.time(), "solving pow ...")
    solved = b''
    for i in range(1000000000):
        h = hashlib.sha1((prefix + str(i)).encode()).hexdigest()
        if h[:6] == '000000':
            solved = str(i).encode()
            print("solved =", solved)
            break
    print(time.time(), "done.")

    r.sendlineafter(b'string S: ', base64.b64encode(solved))

solve_pow()
# r.interactive()
r.recvuntil("Please complete the ")
times = int(r.recvuntil(" "))
# times = 1
print("times", times)

for i in range(times):
    r.recvuntil(": ")
    calcu = r.recvuntil(" =").decode()

    # calcu = "9982713419868 // 35719794 ="
    # print(calcu[0:-2], "##")
    # print(base64.b64encode(struct.pack("<q",int(eval(calcu[0:-2])))))
    ans = int(eval(calcu[0:-2]))
    # print("ans", ans)

    nn, remain = divmod(ans.bit_length(), 8)
    if remain: nn = nn +1

    r.sendline(base64.b64encode(ans.to_bytes(nn, byteorder = 'little')))

flg = r.recvline().decode()
print(flg)
# r.interactive()
r.close()

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :
