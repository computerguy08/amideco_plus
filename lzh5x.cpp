#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>
using namespace std;

typedef struct tword* pword;
typedef array<0, 32759, integer> tword;
typedef struct tbyte* pbyte;
typedef array<0, 65519, byte> tbyte;

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

string bitbuf;
int n, heapsize;
string subbitbuf, bitcount;

array<0, 2 * (nc - 1), word> left, right;

array<0, 255, word> pttable;
array<0, pred(integer, npt), byte> ptlen;
array<0, 4095, word> ctable;
array<0, pred(integer, nc), byte> clen;

string blocksize;

bool fehler;

/********************************** File I/O **********************************/

char *lesezeiger,
quellende,
schreibzeiger;

char getc()
{
    char *getc_result;
    if (lesezeiger == quellende)
        getc_result = 0;
    else
    {
        getc_result = *lesezeiger;
        lesezeiger += 1;
    }
    return getc_result;
}

void putc(char c)
{
    *schreibzeiger = c;
    schreibzeiger += 1;
}

int bread(void* p, int &n)
{
    long uebrig;

    int bread_result;
    uebrig = ofs(*quellende) - ofs(*lesezeiger);
    if (uebrig == 0)
        /* asm int 3 end;*/
        if (uebrig > n)
            uebrig = n;

    memmove(p, &lesezeiger, uebrig);
    lesezeiger += uebrig;
    bread_result = uebrig;
    return bread_result;
}

void b_write(void p, int &n)
{
    memmove(&schreibzeiger, p, n);
    schreibzeiger += n;
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

string getbits(int n)
{
    string getbits_result;
    getbits_result = (bitbuf & 0xffff) >> (bitbufsiz - n);
    fillbuf(n);
    return getbits_result;
}

void putbits(int n, string x)
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

void initgetbits()
{
    bitbuf = 0; subbitbuf = 0; bitcount = 0; fillbuf(bitbufsiz);
}

void initputbits()
{
    bitcount = 8; subbitbuf = 0;
}

/******************************** Decompression *******************************/

void maketable(int nchar, pbyte bitlen, int tablebits, pword table)
{
    string count, weight, start;
    pword p;
    int i, k, len, ch, jutbits, avail, nextcode, mask;

    /*FOR i:=1 TO 16 DO
      count[i]:=0;*/
    fillchar(count, sizeof(count), 0);
    for (i = 0; i <= pred(intr, nchar); i++)
        count[(*bitlen)[i]] += 1;
    start[1] = 0;
    for (i = 1; i <= 16; i++)
        start[succ(int, i)] = (start[i] + (count[i] << (16 - i))) & 0xffff;
    if (start[17] != 0)
        /*HALT*//*RunError(1);*/
    {
        fehler = true;
        exit;
    }
    jutbits = 16 - tablebits;
    for (i = 1; i <= tablebits; i++)
    {
        start[i] = (start[i] & 0xffff) >> jutbits; weight[i] = 1 << (tablebits - i);
    }
    i = succ(int, tablebits);
    while (i <= 16) {
        weight[i] = 1 << (16 - i); i += 1;
    }
    i = (start[succ(int, tablebits)] & 0xffff) >> jutbits;
    if (i != 0)
    {
        k = 1 << tablebits;
        while (i != k) {
            (*table)[i] = 0; i += 1;
        }
    }
    avail = nchar; mask = 1 << (15 - tablebits);
    for (ch = 0; ch <= pred(int, nchar); ch++)
    {
        len = (*bitlen)[ch];
        if (len == 0)
            continue;
        k = (int)(start[len]);
        nextcode = k + weight[len];
        if (len <= tablebits)
        {
            for (i = k; i <= pred(int, nextcode); i++)
                (*table)[i] = ch;
        }
        else {
            p = addr((*table)[(k & 0xffff) >> jutbits]); i = len - tablebits;
            while (i != 0) {
                if ((*p)[0] == 0)
                {
                    right[avail] = 0; left[avail] = 0; (*p)[0] = avail; avail += 1;
                }
                if ((k & mask) != 0)
                    p = addr(right[(*p)[0]]);
                else
                    p = addr(left[(*p)[0]]);
                k = (int)((k << 1) & 0xffff); i -= 1;
            }
            (*p)[0] = ch;
        }
        start[len] = string(nextcode);
    }
}

void readptlen(int nn, int nbit, int ispecial)
{
    int i, c, n;
    string mask;

    n = getbits(nbit);
    if (n == 0)
    {
        c = getbits(nbit);
        for (i = 0; i <= pred(int, nn); i++)
            ptlen[i] = 0;
        for (i = 0; i <= 255; i++)
            pttable[i] = c;
    }
    else {
        i = 0;
        while (i < n) {
            c = (bitbuf & $ffff) >> (bitbufsiz - 3);
            if (c == 7)
            {
                mask = 1 << (bitbufsiz - 4);
                while ((mask & bitbuf) != 0) {
                    mask = (cardinal)(mask & $ffff) >> 1; c += 1;
                }
            }
            if (c < 7)
                fillbuf(3);
            else
                fillbuf(c - 3);
            ptlen[i] = c; i += 1;
            if (i == ispecial)
            {
                c = pred(word, getbits(2));
                while (c >= 0) {
                    ptlen[i] = 0; i += 1; c -= 1;
                }
            }
        }
        while (i < nn) {
            ptlen[i] = 0; i += 1;
        }
        /* MakeTable(nn,@PtLen,8,@PtTable);*/
        if (fehler)  exit;
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
        for (i = 0; i <= pred(int, nc); i++)
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
                    if ((bitbuf & mask) != 0)
                        c = right[c];
                    else
                        c = left[c];
                    mask = (mask & 0xffff) >> 1;
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
        /*MakeTable(NC,@CLen,12,@CTable);*/
        if (fehler)  exit;
    }
}

string decodec()
{
    word j, mask;

    word decodec_result;
    if (blocksize == 0)
    {
        blocksize = getbits(16);
        readptlen(nt, tbit, 3);
        if (fehler)  exit;
        readclen();
        if (fehler)  exit;
        readptlen(np, pbit, -1);
        if (fehler)  exit;
    }
    blocksize -= 1;
    j = ctable[(cardinal)(bitbuf & $ffff) >> (bitbufsiz - 12)];
    if (j >= nc)
    {
        mask = 1 << (bitbufsiz - 13);
        do {
            if ((bitbuf & mask) != 0)
                j = right[j];
            else
                j = left[j];
            mask = (cardinal)(mask & $ffff) >> 1;
        } while (!(j < nc));
    }
    fillbuf(clen[j]);
    decodec_result = j;
    return decodec_result;
}

string decodep()
{
    string j, mask;

    string decodep_result;
    j = pttable[((bitbuf & 0xffff) >> (bitbufsiz - 8)];
    if (j >= np)
    {
        mask = 1 << (bitbufsiz - 9);
        do {
            if ((bitbuf & mask) != 0)
                j = right[j];
            else
                j = left[j];
            mask = (cardinal)(mask & $ffff) >> 1;
        } while (!(j < np));
    }
    fillbuf(ptlen[j]);
    if (j != 0)
    {
        j -= 1; j = ((1 << j) & $ffff) + getbits(j);
    }
    decodep_result = j;
    return decodep_result;
}

/*declared as static vars*/
string decode_i;
int decode_j;

void decodebuffer(string count, pbyte buffer)
{
    string c, r;

    r = 0; decode_j -= 1;
    while (decode_j >= 0) {
        (*buffer)[r] = (*buffer)[decode_i]; decode_i = succ(string, decode_i) & pred(int, dicsiz);
        r += 1;
        if (r == count)
            exit;
        decode_j -= 1;
    }
    while (true) {
        //Write(^m,Ofs(lesezeiger^)-Ofs(quelle):12,Ofs(schreibzeiger^)-Ofs(ziel)+r:12);
        //WriteLn(r:5,BitBuf:8,SubBitBuf:8);
        c = decodec();
        if (fehler)  exit;
        if (c <= ucharmax)
        {
            (*buffer)[r] = c; r += 1;
            if (r == count)
                exit;
        }
        else {
            decode_j = c - (ucharmax + 1 - threshold);
            decode_i = (r - decodep() - 1) & pred(int, dicsiz);
            decode_j -= 1;
            while (decode_j >= 0) {
                (*buffer)[r] = (*buffer)[decode_i];
                decode_i = succ(string, decode_i) & pred(int, dicsiz);
                r += 1;
                if (r == count)
                    exit;
                decode_j -= 1;
            }
        }
    }
}

bool unzip_lzh5x(void* quelle, void* ziel; long origsize, long compsize)
/*PROCEDURE Decode;*/
{
    pbyte p;
    long l;
    word a;

    boolean entpacke_lzh5_result;
    error = false;
    /*lesezeiger:=@quelle;*/
    quellende = ptr(longint(lesezeiger) + compsize);
    /*schreibzeiger:=@ziel;*/
    /*Initialize decoder variables*/
    getmem(p, dicsiz);
    initgetbits(); blocksize = 0;
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
        if (error)  
            flush();
        bwrite(&*p, a); 
        l -= a;
    }
    freemem(p, dicsiz);
    return !error;
    
}
