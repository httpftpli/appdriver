#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "MyDll.h"
#include "stdafx.h"



#define EXIT_OK 0
#define EXIT_FAILED -1
///////////�������Ͷ���
typedef unsigned char BOOLEAN;
typedef unsigned char INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed char INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                 /* Unsigned 16 bit quantity                           */
typedef signed short INT16S;                 /* Signed   16 bit quantity                           */
typedef unsigned long INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed long INT32S;                   /* Signed   32 bit quantity                           */
typedef float FP32;                     /* Single precision floating point                    */
typedef double FP64;                     /* Double precision floating point                    */

typedef unsigned int OS_STK;                   /* Each stack entry is 16-bit wide                    */
typedef unsigned int OS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */

#define BYTE           INT8S                     /* Define data types for backward compatibility ...   */
#define UBYTE          INT8U                     /* ... to uC/OS V1.xx.  Not actually needed for ...   */
#define WORD           INT16S                    /* ... uC/OS-II.                                      */
#define UWORD          INT16U
#define LONG           INT32S
#define ULONG          INT32U
///////////////////////


FILE *infile, *outfile;
INT32U textsize = 0, codesize = 0, printcount = 0;

void Error(char *message) {
    printf("\n%s\n", message);
    exit(EXIT_FAILED);
}

/* LZSS Parameters */

#define N       4096    /* Size of string buffer */
#define F       60  /* Size of look-ahead buffer */
#define THRESHOLD   2
#define NIL     N   /* End of tree's node  */

INT8U
text_buf[N + F - 1];
INT16S match_position, match_length,
lson[N + 1], rson[N + 257], dad[N + 1];

void InitTree(void) {  /* Initializing tree */
    INT16S i;

    for (i = N + 1; i <= N + 256; i++) rson[i] = NIL;          /* root */
    for (i = 0; i < N; i++) dad[i] = NIL;           /* node */
}

void InsertNode(INT16S r) {  /* Inserting node to the tree */
    INT16S i, p, cmp;
    INT8U *key;
    INT16U c;

    cmp = 1;
    key = &text_buf[r];
    p = N + 1 + key[0];
    rson[r] = lson[r] = NIL;
    match_length = 0;
    for (;;) {
        if (cmp >= 0) {
            if (rson[p] != NIL) p = rson[p];
            else {
                rson[p] = r;
                dad[r] = p;
                return;
            }
        } else {
            if (lson[p] != NIL) p = lson[p];
            else {
                lson[p] = r;
                dad[r] = p;
                return;
            }
        }
        for (i = 1; i < F; i++) if ((cmp = key[i] - text_buf[p + i]) != 0) break;
        if (i > THRESHOLD) {
            if (i > match_length) {
                match_position = ((r - p) & (N - 1)) - 1;
                if ((match_length = i) >= F) break;
            }
            if (i == match_length) {
                if ((c = ((r - p) & (N - 1)) - 1) < match_position) {
                    match_position = c;
                }
            }
        }
    }
    dad[r] = dad[p];
    lson[r] = lson[p];
    rson[r] = rson[p];
    dad[lson[p]] = r;
    dad[rson[p]] = r;
    if (rson[dad[p]] == p) rson[dad[p]] = r;
    else lson[dad[p]] = r;
    dad[p] = NIL;  /* remove p */
}

void DeleteNode(INT16S p) {  /* Deleting node from the tree */
    INT16S q;

    if (dad[p] == NIL) return;         /* unregistered */
    if (rson[p] == NIL) q = lson[p];
    else if (lson[p] == NIL) q = rson[p];
    else {
        q = lson[p];
        if (rson[q] != NIL) {
            do {
                q = rson[q];
            } while (rson[q] != NIL);
            rson[dad[q]] = lson[q];
            dad[lson[q]] = dad[q];
            lson[q] = lson[p];
            dad[lson[p]] = q;
        }
        rson[q] = rson[p];
        dad[rson[p]] = q;
    }
    dad[q] = dad[p];
    if (rson[dad[p]] == p) rson[dad[p]] = q;
    else lson[dad[p]] = q;
    dad[p] = NIL;
}

/* Huffman coding parameters */

#define N_CHAR      (256 - THRESHOLD + F)
/* character code (= 0..N_CHAR-1) */
#define T       (N_CHAR * 2 - 1)    /* Size of table */
#define R       (T - 1)         /* root position */
#define MAX_FREQ    0x8000
/* update when cumulative frequency */
/* reaches to this value */

typedef INT8U uchar;

/*  
 * Tables for encoding/decoding upper 6 bits of  
 * sliding dictionary pointer  
 */
/* encoder table */
uchar p_len[64] = {
    0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

uchar p_code[64] = {
    0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
    0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9C,
    0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
    0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,
    0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
    0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

/* decoder table */
uchar d_code[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
    0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
    0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
    0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
    0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
    0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
    0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
    0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
    0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
    0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
    0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

uchar d_len[256] = {
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

INT16U freq[T + 1]; /* cumulative freq table */

/*  
 * pointing parent nodes.  
 * area [T..(T + N_CHAR - 1)] are pointers for leaves  
 */
INT16S prnt[T + N_CHAR];

/* pointing children nodes (son[], son[] + 1)*/
INT16S son[T];

INT16U getbuf = 0;
INT8U getlen = 0;

INT16S GetBit(void) { /* get one bit */
    INT16S i;

    while (getlen <= 8) {
        if ((i = getc(infile)) < 0) i = 0;
        printf(" Infile Positon: %ld \n", ftell(infile));
        getbuf |= i << (8 - getlen);
        getlen += 8;
    }
    i = getbuf;
    getbuf <<= 1;
    getlen--;
    return(i < 0);
}

INT16S GetByte(void) {    /* get a byte */
    INT16U i;

    while (getlen <= 8) {
        if ((i = getc(infile)) < 0) i = 0;
        printf(" Infile Positon: %ld \n", ftell(infile));
        getbuf |= i << (8 - getlen);
        getlen += 8;
    }
    i = getbuf;
    getbuf <<= 8;
    getlen -= 8;
    return i >> 8;
}

INT16U putbuf = 0;
uchar putlen = 0;

void Putcode(INT16S l, INT16U c) {        /* output c bits */
    putbuf |= c >> putlen;
    if ((putlen += l) >= 8) {
        putc(putbuf >> 8, outfile);
        printf("Positon: %ld \n", ftell(outfile));
        if ((putlen -= 8) >= 8) {
            putc(putbuf, outfile);
            printf("Positon: %ld \n", ftell(outfile));
            codesize += 2;
            putlen -= 8;
            putbuf = c << (l - putlen);

        } else {
            putbuf <<= 8;
            codesize++;
        }
    }
}


/* initialize freq tree */

void StartHuff() {
    INT16S i, j;

    for (i = 0; i < N_CHAR; i++) {
        freq[i] = 1;
        son[i] = i + T;
        prnt[i + T] = i;
    }
    i = 0; j = N_CHAR;
    while (j <= R) {
        freq[j] = freq[i] + freq[i + 1];
        son[j] = i;
        prnt[i] = prnt[i + 1] = j;
        i += 2; j++;
    }
    freq[T] = 0xffff;
    prnt[R] = 0;
}


/* reconstruct freq tree */

void reconst() {
    INT16S i, j, k;
    INT16U f, l;

    /* halven cumulative freq for leaf nodes */
    j = 0;
    for (i = 0; i < T; i++) {
        if (son[i] >= T) {
            freq[j] = (freq[i] + 1) / 2;
            son[j] = son[i];
            j++;
        }
    }
    /* make a tree : first, connect children nodes */
    for (i = 0, j = N_CHAR; j < T; i += 2, j++) {
        k = i + 1;
        f = freq[j] = freq[i] + freq[k];
        for (k = j - 1; f < freq[k]; k--) ;
        k++;
        l = (j - k) * 2;

        /* movmem() is Turbo-C dependent  
           rewritten to memmove() by Kenji */

        /* movmem(&freq[k], &freq[k + 1], l);*/
        (void)memmove(&freq[k + 1], &freq[k], l);
        freq[k] = f;
        /* movmem(&son[k], &son[k + 1], l); */
        (void)memmove(&son[k + 1], &son[k], l);
        son[k] = i;
    }
    /* connect parent nodes */
    for (i = 0; i < T; i++) {
        if ((k = son[i]) >= T) {
            prnt[k] = i;
        } else {
            prnt[k] = prnt[k + 1] = i;
        }
    }
}


/* update freq tree */

void update(INT16S c) {
    INT16S i, j, k, l;

    if (freq[R] == MAX_FREQ) {
        reconst();
    }
    c = prnt[c + T];
    do {
        k = ++freq[c];

        /* swap nodes to keep the tree freq-ordered */
        if (k > freq[l = c + 1]) {
            while (k > freq[++l]);
            l--;
            freq[c] = freq[l];
            freq[l] = k;

            i = son[c];
            prnt[i] = l;
            if (i < T) prnt[i + 1] = l;

            j = son[l];
            son[l] = i;

            prnt[j] = c;
            if (j < T) prnt[j + 1] = c;
            son[c] = j;

            c = l;
        }
    } while ((c = prnt[c]) != 0);   /* do it until reaching the root */
}

INT16U code, len;

void EncodeChar(INT16U c) {
    INT16U i;
    INT16S j, k;

    i = 0;
    j = 0;
    k = prnt[c + T];

    /* search connections from leaf node to the root */
    do {
        i >>= 1;

        /*  
        if node's address is odd, output 1  
        else output 0  
        */
        if (k & 1) i += 0x8000;

        j++;
    } while ((k = prnt[k]) != R);
    Putcode(j, i);
    code = i;
    len = j;
    update(c);
}

void EncodePosition(INT16U c) {
    INT16U i;

    /* output upper 6 bits with encoding */
    i = c >> 6;
    Putcode(p_len[i], (unsigned)p_code[i] << 8);

    /* output lower 6 bits directly */
    Putcode(6, (c & 0x3f) << 10);
}

void EncodeEnd() {
    if (putlen) {
        putc(putbuf >> 8, outfile);
        printf("Positon: %ld \n", ftell(outfile));
        codesize++;
    }
}

INT16S DecodeChar() {
    INT16U c;

    c = son[R];

    /*  
     * start searching tree from the root to leaves.  
     * choose node #(son[]) if input bit == 0  
     * else choose #(son[]+1) (input bit == 1)  
     */
    while (c < T) {
        c += GetBit();
        c = son[c];
    }
    c -= T;
    update(c);
    return c;
}

INT16S DecodePosition() {
    INT16U i, j, c;

    /* decode upper 6 bits from given table */
    i = GetByte();
    c = (unsigned)d_code[i] << 6;
    j = d_len[i];

    /* input lower 6 bits directly */
    j -= 2;
    while (j--) {
        i = (i << 1) + GetBit();
    }
    return c | i & 0x3f;
}

/* Compression */

void Encode(void) {  /* Encoding/Compressing */
    INT16S i, c, len, r, s, last_match_length;

    fseek(infile, 0L, 2);
    textsize = ftell(infile);
    if (fwrite(&textsize, sizeof(textsize), 1, outfile) < 1) Error("Unable to write");   /* write size of original text */
    if (textsize == 0) return;
    rewind(infile);
    textsize = 0;           /* rewind and rescan */
    StartHuff();
    InitTree();
    s = 0;
    r = N - F;
    for (i = s; i < r; i++) text_buf[i] = ' ';
    for (len = 0; len < F && (c = getc(infile)) != EOF; len++) {
        printf(" Infile Positon: %ld \n", ftell(infile));
        text_buf[r + len] = c;
    }
    textsize = len;
    for (i = 1; i <= F; i++) InsertNode(r - i);
    InsertNode(r);
    do {
        if (match_length > len) match_length = len;
        if (match_length <= THRESHOLD) {
            match_length = 1;
            EncodeChar(text_buf[r]);
        } else {
            EncodeChar(255 - THRESHOLD + match_length);
            EncodePosition(match_position);
        }
        last_match_length = match_length;
        for (i = 0; i < last_match_length && (c = getc(infile)) != EOF; i++) {
            printf(" Infile Positon: %ld \n", ftell(infile));
            DeleteNode(s);
            text_buf[s] = c;
            if (s < F - 1) text_buf[s + N] = c;
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            InsertNode(r);
        }
        if ((textsize += i) > printcount) {
            printf("%12ld\r", textsize);
            printcount += 1024;
        }
        while (i++ < last_match_length) {
            DeleteNode(s);
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            if (--len) InsertNode(r);
        }
    } while (len > 0);
    EncodeEnd();
    printf("input: %ld bytes\n", textsize);
    printf("output: %ld bytes\n", codesize);
    printf("output/input: %.3f\n", (double)codesize / textsize);
}

void Decode(void) {  /* Decoding/Uncompressing */
    INT16S i, j, k, r, c;
    INT32U count;

    if (fread(&textsize, sizeof textsize, 1, infile) < 1) Error("Unable to read");  /* read size of original text */
    if (textsize == 0) return;
    StartHuff();
    for (i = 0; i < N - F; i++) text_buf[i] = ' ';
    r = N - F;
    for (count = 0; count < textsize;) {
        c = DecodeChar();
        if (c < 256) {
            putc(c, outfile);
            printf("Positon: %ld \n", ftell(outfile));
            text_buf[r++] = c;
            r &= (N - 1);
            count++;
        } else {
            i = (r - DecodePosition() - 1) & (N - 1);
            j = c - 255 + THRESHOLD;
            for (k = 0; k < j; k++) {
                c = text_buf[(i + k) & (N - 1)];
                putc(c, outfile);
                printf("Positon: %ld \n", ftell(outfile));
                text_buf[r++] = c;
                r &= (N - 1);
                count++;
            }
        }
        if (count > printcount) {
            printf("%12ld\r", count);
            printcount += 1024;
        }
    }
    printf("%12ld\n", count);
}
//extern "C"  int __stdcall  eDecode(char * inpath,char * outpath,int checkcode)
int __stdcall eDecode(char *inpath, char *outpath, int checkcode) {

    if (checkcode == 26816) {
        textsize = 0;
        codesize = 0;
        printcount = 0;
        getbuf = 0;
        getlen = 0;
        putbuf = 0;
        putlen = 0;
        infile = fopen(inpath, "rb");
        outfile = fopen(outpath, "wb");

        /*Encode();*/
        Decode();
        fclose(infile); fclose(outfile);
        return 1;

    }
    return checkcode;
}
//extern "C" __declspec(dllexport) int   eEncode(char * inpath,char * outpath,int checkcode)
int __stdcall eEncode(char *inpath, char *outpath, int checkcode) {

    if (checkcode == 22568) {
        textsize = 0;
        codesize = 0;
        printcount = 0;
        getbuf = 0;
        getlen = 0;
        putbuf = 0;
        putlen = 0;
        infile = fopen(inpath, "rb");
        outfile = fopen(outpath, "wb");

        Encode();
        //Decode();
        fclose(infile); fclose(outfile);
        return 1;

    }
    return checkcode;
}


int eEncode1(char *inpath, char *outpath) {

    if (checkcode == 22568) {
        textsize = 0;
        codesize = 0; 
        printcount = 0;
        getbuf = 0;
        getlen = 0;
        putbuf = 0;
        putlen = 0;
        infile = fopen(inpath, "rb");
        outfile = fopen(outpath, "wb");

        Encode();
        //Decode();
        fclose(infile); 
        fclose(outfile);
        return 1;

    }
    return checkcode;
}


//extern "C" __declspec(dllexport) int __stdcall test(int t)
int __stdcall test(int t) {
    int x;
    x = t + 10;
    return x;

}