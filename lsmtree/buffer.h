//
//  buffer.h
//  lsmtree
//
//  Created by Michael Hahn on 3/6/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef buffer_h
#define buffer_h

#include <stdio.h>

struct buffer {
    char *buf;
    int NUL;
    int buflen;
};

struct buffer *buffer_new(size_t reserve);
void buffer_free(struct buffer *b);

void buffer_clear(struct buffer *b);
char *buffer_detach(struct buffer *b);

void buffer_putc(struct buffer *b, const char c);
void buffer_putstr(struct buffer *b, const char *str);
void buffer_putnstr(struct buffer *b,const char *str,size_t n);
void buffer_putint(struct buffer *b,int i);

void buffer_dump(struct buffer *b);

#endif /* buffer_h */
