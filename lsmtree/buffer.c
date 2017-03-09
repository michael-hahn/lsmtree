//
//  buffer.c
//  lsmtree
//
//  Created by Michael Hahn on 3/6/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"

unsigned _next_power(unsigned x)
{
    --x;
    x |= x >> 0x01;
    x |= x >> 0x02;
    x |= x >> 0x04;
    x |= x >> 0x08;
    x |= x >> 0x10;
    return ++x;
}

void _buffer_extendby(struct buffer *b, int len)
{
    char* buffer;
    
    len += b->NUL;
    if (len <= b->buflen)
        return;
        
    b->buflen = _next_power(len);
    buffer = realloc(b->buf, b->buflen);
    b->buf = buffer;
}


struct buffer *buffer_new(size_t reserve)
{
    struct buffer *b=malloc(sizeof(struct buffer));
    b->buf = NULL;
    b->NUL = 0;
    b->buflen = 0;
    
    if (reserve)
        _buffer_extendby(b, reserve + 1);
    
    return b;
}

void buffer_free(struct buffer *b)
{
    if (b->buf)
        free(b->buf);
    free(b);
}

void buffer_clear(struct buffer *b)
{
    b->NUL = 0;
    if (b->buf)
        b->buf[b->NUL] = '\0';
}

void buffer_putstr(struct buffer *b, const char *str)
{
    size_t len = strlen(str);
    
    _buffer_extendby(b, len + 1);
    memcpy(&b->buf[b->NUL], str, len);
    b->NUL += len;
    b->buf[b->NUL] = '\0';
}

void buffer_putnstr(struct buffer *b, const char *str,size_t n)
{
    _buffer_extendby(b, n + 1);
    memcpy(&b->buf[b->NUL], str, n);
    b->NUL += n;
    b->buf[b->NUL] = '\0';
}

void buffer_putc(struct buffer *b, const char c)
{
    _buffer_extendby(b, 2);
    b->buf[b->NUL++] = c;
    b->buf[b->NUL] = '\0';
}

void buffer_putint(struct buffer *b,int i)
{
    unsigned int value=i;
    _buffer_extendby(b,sizeof(int));
    b->buf[b->NUL++] = (char)(value);
    b->buf[b->NUL++] = (char)(value >> 0x08);
    b->buf[b->NUL++] = (char)(value >> 0x10);
    b->buf[b->NUL++] = (char)(value >> 0x18);
}

char * buffer_detach(struct buffer *b)
{
    char *buffer = b->buf;
    b->NUL = 0;
    b->buflen = 0;
    return buffer;
}

void buffer_dump(struct buffer *b)
{
    int i;
    printf("--buffer dump:buflen<%d>,pos<%d>\n",b->buflen,b->NUL);
    
    for(i=0;i<b->NUL;i++)
        printf("	[%d] %c\n",i,b->buf[i]);
}