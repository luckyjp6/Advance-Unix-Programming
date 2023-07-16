# Advance-Unix-Programming
此repo收錄由黃俊穎老師開設的高等Unix課程的lab。  

- Lab以類似CTF的形式進行，按spec要求和題目機互動或上傳code解題來獲取flag（題目機使用proof-of-work機制避免伺服器崩潰），大量使用python pwntool套件，此repo收錄各lab上傳的code或是直接和題目機互動的pwntool程式碼。
- 作業部分皆為完整、可獨立展示的專案。

## Lab1 -- Solve Equations
- 目標：熟悉pwntool和base64 encode/decode，皆為大數運算並有時間限制。
- 實作內容：使用pwntool的recvuntil取得待解等式總量，以及後續各等式，解析等式再用base64 encode計算結果並輸出，題目機確認答案正確性後回傳flag。

## Lab2 -- Traverse the Directory
- 目標：練習使用C語言的dirnet.h，透過資料夾的路徑遍歷資料夾。
- 實作內容：程式執行時透過參數傳入指定的資料夾名稱和想尋找的字串，程式遍歷給定的資料夾及其下所有子資料夾，尋找內容包含指定字串的檔案，並輸出該檔案路徑，題目機確認答案正確性後回傳flag。

## Lab3 -- Alter Globle Offset Table (GOT)
- 目標：練習使用.so檔取代原有執行檔(chals)的函式定義，練習讀取並解析static執行檔的GOT以及使用dlfunc.h的函式變更GOT。
- 前情提要：
    - 目標執行檔為chals，本次lab將嘗試變更chals的GOT以獲取flag。
    - chals引用libpoem.h之中的init()和code_xxx()等函式。（xxx是0~1476之間的任意數字）
    - chals中使用code_xxx()的順序並非正確，將透過更改GOT的方式使code_xxx()能正確被呼叫。其正確對應關係定義在shuffle.h中。
    - 實作部分在solver.py和libsolver.c。
    - 因GOT中的code_xxx()函式並未依照順序排放，將使用solver.py獲取其順序。
    - 將libsolver.c編譯成share library的.so檔，取代chals中init()的定義。
- 實作內容（init() in libsolver.c）：
    - 讀取並解析GOT以獲取程式的起始位置。
    - 使用dlopen()和dlsym()來存取code_xxx各函式的所在位置。
    - 使用mprotect開啟GOT的寫入權限（位置需align成一個page的大小）。
    - 改動GOT，依對應關係調整為題目希望的順序。
    - 還原GOT讀寫權限。
- 心得：很多轉換，頗複雜。

## Lab4 -- Buffer Overflow
- 目標：練習撰寫assembly code，並學習buffer overflow的嚴重性及如何利用此漏洞。
- 前情提要：
    - 題目機執行remoteguess.c，隨機產生一個數字，再fork出子程式並以sandbox限制各種system call然後執行學生上傳的程式碼，最後要求猜出一開始隨機產生的數字。
    - 學生上傳一個函式，接收一個參數printf pointer，本次lab使用assembly撰寫，參見submit.py Line 17，大致C語言對應參見solver.c。
        ```c
        typedef int (*printf_ptr_t)(const char *format, ...);
        void solver(printf_ptr_t fptr)
        ```
    - 此次實作包括上傳函式（以asm撰寫）及後續buffer overflow的操作，參見submit.py。
- 實作內容：隨機產生的數字無法預測，但可利用remoteguess.c的漏洞：使用read()讀取超出buffer大小的輸入。可利用stack的特性，強制複寫正確答案，但此舉會破壞stack上的return addrass、rbp和cannary，所以在學生上傳的函式中將上述三者的值leak出來，並放置在輸入的對應位置一併傳送，其中return address需解析remoteguess執行檔進行調整。
    ```
           |  .....                           |
           |  (previous func's stack frame)   |
           +----------------------------------+
           |  additional parameters (if any)  | (higher address)
           |  .....                           |
           +----------------------------------+
           |  return address (64-bit)         |
           +----------------------------------+
    rbp -> |  (pushed) rbp (64-bit)           |
           +----------------------------------+
           |  canary (64-bit, if required)    |
           +----------------------------------+
           |  local variables (if any)        |
    rsp -> |  .....                           | (lower address)
           +----------------------------------+
    ```
- 心得：第一次寫assembly，花了三天才完成，最大的關卡式leak stack資訊不能直接使用system call，而是使用參數傳入的printf pointer，非常新手不友好，還好堅持走過來了。

## Lab5 -- Kernel Module
- 目標：練習撰寫kernel module。
- 實作內容：
    - 創建8個device，使用kzalloc()將device對應到kernel memory space 4KB的空間。
    - 可安全卸載kernel module，正確使用kfree()釋放記憶體，不造成系統崩潰。
    - 可由讀取```/proc/kshram```獲取shared memory file的大小。
    - 實作mmap在kernel module對應的函式。
    - 實作ioctl command
        - KSHRAM_GETSLOTS：回傳module中可用的slot數量。
        - KSHRAM_GETSIZE：回傳當前打開的device對應的shared memory空間大小。
        - KSHRAM_SETSIZE：設定當前打開的device對應的shared memory空間大小。
- 心得：It's a whole new world! 花了很多時間看document，了解kernel module的機制及運作方式，過程中虛擬機crash好幾次哈哈哈。

## Lab6 -- Sorting with Assembly
- 目標：熟練使用assembly。
- 前情提要：
    - 題目機fork出子程式執行學生實作的函式，傳入陣列指標和陣列大小。
        ```
        typedef void (*sort_funcptr_t)(long *numbers, int n);
        ```
    - 現時120秒之內完成，bubble sort不可行。
    - 此次lab有計時競賽。
- 實作內容：Assembly版的quick sort。
- 心得：挑戰自己照著C code寫出對應的assembly code，後來老師讓大家分享作法才知道大部分人是直接用現有的好工具將C code轉成assembly，沒什麼人認真在刻asm。值得一提的是，有位碩班學長耗時無數個日夜用「排序正確的數量」將題目機的程式碼leak出來，每次上傳都要解pow但只能獲得兩個byte，重組再反組譯後發現老師擋得很嚴實沒什麼漏洞，最後駭了將答題結果傳到記分板的指令，將自己移到第一名並留下耐人尋味的數字諧音笑話。

## Lab7 -- ROP
- 目標：練習Return-Oriented Programming (ROP)並熟悉使用assembly中一般的system call。
- 前情提要：
    - 題目機source code參見ropshell.c
    - 題目機以當下時間為seed隨機生成一大段資料（稱為***code***），並用mprotect限制***code***寫入權限，接著輸出作為seed的時間和***code***存放的位置，隨後stack接收學生上傳的內容，學生需根據seed生成***code***並在其中尋找所需的片段，如assemble後的```pop rax \nret```為```b'X\xc3'```，在***code***中找到其位置並放在return address，便可以ROP實現流程控制。以上是老師設計此實驗預設給同學的作法。
    - 此lab任務有三：
        - 讀取指定檔案"/FLAG"獲得flag
        - 讀取shared memory內容獲得flag
        - 連線至指定網站獲得flag
- 實作內容：由於實驗設計是隨機生成資料內容，難以確保次次皆能達成此lab三項任務，因此此次作業我採用不同的方式，首先以ROP方式使用四次system call：
    1. mprotect開啟***code***寫入權限，從***code***起始位置開始開一個page的寫入權限。
    2. read讀入task 1檔案檔名，將檔名放置在***code***起始位置，方便後續讀取。
    3. read讀入完成餘下所有操作的asm code，並將寫入asm code的位置放在return address，使程式跳轉至asm code執行。
    4. 最後收尾，以state 37呼叫exit結束程式
    
    asm code的部分即assembly版的開檔讀檔、讀取shared memory和connect。因以assembly撰寫，有許多細節處裡：於statck寫入資訊時需注意修飾字（如WORD, DWORD, QWORD等）；實作使用大量syscall，需注意參數傳遞的方式（value or pointer）；填sys_connect參數時需轉換成big endian。
- 心得：這次lab之後和asm熟悉很多，對於stack堆疊資料的方式也更加得心應手。一路彎彎繞繞，好感動又過了一關。

## Lab8 -- Ptrace
- 目標：練習使用ptrace、學習運用中斷點0xcc及讀寫子程序的暫存器。
- 前情提要：實作程式在solver.c，用於測試的執行檔內容參見sample1.c，只有正確的magic number可以使程式輸出"Bingo"並回傳0，其他情況則會回傳-1。
- 實作內容：測試執行檔中有多處中斷點，solver.c在合適時機使用PTRACE_GETREGS取得子程序的rip，紀錄為存檔點；使用PTRACE_POKETEXT改變magic number的值，以暴力法嘗試所有可能，藉由子程序的回傳值判斷是否已找到對的magic number，失敗便使用PTRACE_SETREGS設定子程序rip回到存檔點，成功便結束程式。