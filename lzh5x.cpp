#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>
using namespace std;

/*
NOTE :
   The following constants are set to the values used by LHArc.
   You can change three of them as follows :

   DICBIT : Lempel-Ziv dictionnary size.
   Lowering this constant can lower the compression efficiency a lot !
   But increasing it (on a 32 bit platform only, i.e. Delphi 2) will not yield
   noticeably better results.
   If you set DICBIT to 15 or more, set PBIT to 5; and if you set DICBIT to 19
   or more, set NPT to NP, too.

   WINBIT : Sliding window size.
   The compression ratio depends a lot of this value.
   You can increase it to 15 to get better results on large files.
   I recommend doing this if you have enough memory, except if you want that
   your compressed data remain compatible with LHArc.
   On a 32 bit platform, you can increase it to 16. Using a larger value will
   only waste time and memory.

   BUFBIT : I/O Buffer size. You can lower it to save memory, or increase it
   to reduce disk access.
*/

const int bitbufsiz = 16;
const int ucharmax = 255;

const int dicbit = 13;
const int dicsiz = 1 << dicbit;

const int matchbit = 8;
const int maxmatch = 1 << matchbit;
const int threshold = 3;
const int percflag = 0x8000;

const int nc = (ucharmax + maxmatch + 2 - threshold);
const int cbit = 9;
const int codebit = 16;

const int np = dicbit + 1;
const int nt = codebit + 3;
const int pbit = 4; /*Log2(NP)*/
const int tbit = 5; /*Log2(NT)*/
const int npt = nt; /*Greater from NP and NT*/

const int nul = 0;
const int maxhashval = (3 * dicsiz + ((dicsiz >> 9) + 1) * ucharmax);

const int winbit = 14;
const int windowsize = 1 << winbit;

const int bufbit = 13;
const int bufsize = 1 << bufbit;

string  pttable, ctable, right = "X";
int n, heapsize, bitcount, bitbuf, subbitbuf, blocksize, tword[32759], decode_i, decode_j;
char ptlen[npt], clen[nc], tbyte[65519], left[1024], * r_pointer, * increase, * w_pointer;
bool error;
long compsize;

/********************************** File I/O **********************************/

char getc() {
    if (r_pointer == increase)
        return 0;
    else {
        r_pointer += 1;
        return *(r_pointer);
    }
}

void putc(char c) {
    *(w_pointer) = c;
    w_pointer += 1;
}

int bread(void* p, int& n)
{
    long left;
    left = increase - r_pointer;
    if (left == 0)
        __asm int 3;
    if (left > n)
        left = n;

    memmove(p, &r_pointer, left);
    r_pointer += left;
    return left;
}

void b_write(void* p, int& n)
{
    memmove(&w_pointer, p, n);
    w_pointer += n;
}

/**************************** Bit handling routines ***************************/

void fillbuf(int n)
{
    bitbuf = (bitbuf << n) & 0xffff;
    while (n > bitcount) {
        n -= bitcount;
        bitbuf = bitbuf | ((subbitbuf << n) & 0xffff);
        if (compsize != 0)
        {
            compsize -= 1; subbitbuf = getc();
        }
        else
            subbitbuf = 0;
        bitcount = 8;
    }
    bitcount -= n;
    bitbuf = bitbuf | ((subbitbuf & 0xffff) >> bitcount);
}

int getbits(int n)
{
    fillbuf(n);
    return (bitbuf & 0xffff) >> (bitbufsiz - n);
}

void putbits(int n, int x)
{
    if (n < bitcount)
    {
        bitcount -= n;
        subbitbuf = subbitbuf | ((x << bitcount) & 0xffff);
    }
    else {
        n -= bitcount;
        putc(subbitbuf | ((x & 0xffff) >> n)); compsize += 1;
        if (n < 8)
        {
            bitcount = 8 - n; subbitbuf = (x << bitcount) & 0xffff;
        }
        else {
            putc((x & 0xffff) >> (n - 8)); compsize += 1;
            bitcount = 16 - n; subbitbuf = (x << bitcount) & 0xffff;
        }
    }
}


void initputbits()
{
    bitcount = 8; subbitbuf = 0;
}

/******************************** Decompression *******************************/

void maketable(int nchar, char* bitlen, int tablebits, string table)
{
    string count = 0, weight, start;
    char* p;
    int i, k, len, ch, jutbits, avail, nextcode, mask;

    /*FOR i:=1 TO 16 DO
      count[i]:=0;*/
    for (i = 0; i < nchar; i++)
        count[bitlen[i]] += 1;
    start[1] = 0;
    for (i = 1; i <= 16; i++)
        start[i + 1] = (start[i] + (count[i] << (16 - i))) & 0xffff;
    if (start[17] != 0) {
        /*HALT*//*RunError(1);*/
        error = true;
        exit;
    }
    jutbits = 16 - tablebits;
    for (i = 1; i <= tablebits; i++)
        start[i] = (start[i] & 0xffff) >> jutbits; weight[i] = 1 << (tablebits - i);
    i = tablebits + 1;
    while (i <= 16)
        weight[i] = 1 << (16 - i); i += 1;
    i = (start[tablebits + 1] & 0xffff) >> jutbits;
    if (i != 0) {
        k = 1 << tablebits;
        while (i != k) {
            table[i] = 0; i += 1;
        }
    }
    avail = nchar; mask = 1 << (15 - tablebits);
    for (ch = 0; ch < nchar; ch++)
    {
        len = bitlen[ch];
        if (len == 0)
            continue;
        k = (int)(start[len]);
        nextcode = k + weight[len];
        if (len <= tablebits)
        {
            for (i = k; i < nextcode; i++)
                table[i] = ch;
        }
        else {
            p = &table[(k & 0xffff) >> jutbits];
            i = len - tablebits;
            while (i != 0) {
                if (p[0] == 0) {
                    //right[avail] = 0; 
                    //left[avail] = 0; 
                    p[0] = avail;
                    avail += 1;
                }
                /*if ((k & mask) != 0)
                    p = &right[p[0]];
                else
                    p = &left[p[0]];*/
                k = (int)((k << 1) & 0xffff); i -= 1;
            }
            p[0] = ch;
        }
        start[len] = nextcode;
    }
}

void readptlen(int nn, int nbit, int ispecial)
{
    int i, c, n;
    int mask;

    n = getbits(nbit);
    if (n == 0)
    {
        c = getbits(nbit);
        for (i = 0; i < nn; i++)
            ptlen[i] = 0;
        for (i = 0; i <= 255; i++)
            pttable[i] = c;
    }
    else {
        i = 0;
        while (i < n) {
            c = (bitbuf & 0xffff) >> (bitbufsiz - 3);
            if (c == 7)
            {
                mask = 1 << (bitbufsiz - 4);
                while ((mask & bitbuf) != 0) {
                    mask = mask & 0xffff >> 1; c += 1;
                }
            }
            if (c < 7)
                fillbuf(3);
            else
                fillbuf(c - 3);
            ptlen[i] = c; i += 1;
            if (i == ispecial)
            {
                c = getbits(2) - 1;
                while (c >= 0) {
                    ptlen[i] = 0; i += 1; c -= 1;
                }
            }
        }
        while (i < nn) {
            ptlen[i] = 0; i += 1;
        }
        maketable(nn, ptlen, 8, pttable);
        if (error)  exit;
    }
}

void readclen()
{
    int i, c, n;
    string mask;

    n = getbits(cbit);
    if (n == 0)
    {
        c = getbits(cbit);
        for (i = 0; i < nc; i++)
            clen[i] = 0;
        for (i = 0; i <= 4095; i++)
            ctable[i] = c;
    }
    else {
        i = 0;
        while (i < n) {
            c = pttable[(bitbuf & 0xffff) >> (bitbufsiz - 8)];
            if (c >= nt)
            {
                mask = 1 << (bitbufsiz - 9);
                do {
                    /*if ((bitbuf & mask) != 0)
                        c = right[c];
                    else
                        c = left[c];
                    mask = (mask & 0xffff) >> 1;*/
                } while (!(c < nt));
            }
            fillbuf(ptlen[c]);
            if (c <= 2)
            {
                if (c == 1)
                    c = 2 + getbits(4);
                else
                    if (c == 2)
                        c = 19 + getbits(cbit);
                while (c >= 0) {
                    clen[i] = 0; i += 1; c -= 1;
                }
            }
            else {
                clen[i] = c - 2; i += 1;
            }
        }
        while (i < nc) {
            clen[i] = 0; i += 1;
        }
        maketable(nc, clen, 12, ctable);
        if (error)  exit;
    }
}

int decodec()
{
    int j, mask;
    if (blocksize == 0)
    {
        blocksize = getbits(16);
        readptlen(nt, tbit, 3);
        if (error)  exit;
        readclen();
        if (error)  exit;
        readptlen(np, pbit, -1);
        if (error)  exit;
    }
    blocksize -= 1;
    j = ctable[(bitbuf & 0xffff) >> (bitbufsiz - 12)];
    if (j >= nc)
    {
        mask = 1 << (bitbufsiz - 13);
        /*do {
            if ((bitbuf & mask) != 0)
                j = right[j];
            else
                j = left[j];
            mask = (mask & 0xffff) >> 1;
        } while (!(j < nc));*/
    }
    fillbuf(clen[j]);
    return j;
}

int decodep()
{
    int j, mask;

    string decodep_result;
    j = pttable[((bitbuf & 0xffff) >> (bitbufsiz - 8))];
    if (j >= np)
    {
        mask = 1 << (bitbufsiz - 9);
        /*do {
            if ((bitbuf & mask) != 0)
                j = right[j];
            else
                j = left[j];
            mask = (mask & 0xffff) >> 1;
        } while (!(j < np));*/
    }
    fillbuf(ptlen[j]);
    if (j != 0) {
        j -= 1;
        j = ((1 << j) & 0xffff) + getbits(j);
    }
    return j;
}

void decodebuffer(int count, char* buffer)
{
    int c, r;

    r = 0; decode_j -= 1;
    while (decode_j >= 0) {
        buffer[r] = buffer[decode_i];
        decode_i = (decode_i + 1) & (dicsiz - 1);
        r += 1;
        if (r == count)
            exit;
        decode_j -= 1;
    }
    while (true) {
        //Write(^m,Ofs(lesezeiger^)-Ofs(quelle):12,Ofs(schreibzeiger^)-Ofs(ziel)+r:12);
        //WriteLn(r:5,BitBuf:8,SubBitBuf:8);
        c = decodec();
        if (error)  exit;
        if (c <= ucharmax)
        {
            buffer[r] = c; r += 1;
            if (r == count)
                exit;
        }
        else {
            decode_j = c - (ucharmax + 1 - threshold);
            decode_i = (r - decodep() - 1) & (dicsiz - 1);
            decode_j -= 1;
            while (decode_j >= 0) {
                buffer[r] = buffer[decode_i];
                decode_i = (decode_i + 1) & (dicsiz - 1);
                r += 1;
                if (r == count)
                    exit;
                decode_j -= 1;
            }
        }
    }
}

bool unzip_lzh5x(char source, char target, long origsize, long compsize1) {
    char* p;
    long l;
    int a;
    bool entpacke_lzh5_result;
    compsize = compsize1;
    error = false;
    r_pointer = &source;
    *increase = (long(r_pointer) + compsize);
    w_pointer = &target;
    /*Initialize decoder variables*/
    p = (char*)malloc(dicsiz);

    bitbuf = 0;
    subbitbuf = 0;
    bitcount = 0;
    fillbuf(bitbufsiz);

    blocksize = 0;
    decode_j = 0;
    /*skip file size*/
    l = origsize; compsize -= 4;
    /*unpacks the file*/
    while (l > 0) {
        if (l > dicsiz)
            a = dicsiz;
        else
            a = l;
        decodebuffer(a, p);
        b_write(p, a);
        l -= a;
    }
    free(p);
    return !error;
}