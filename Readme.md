# Advance-Unix-Programming
Labs of Advance Unix Programming  

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
- 心得：第一次寫assembly，花了三天才完成，最大的關卡式leak stack資訊不能直接使用system call，而是使用參數傳入的printf pointer，新手不友好，但還好我走過來了。
