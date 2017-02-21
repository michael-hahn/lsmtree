//
//  utils.c
//  lsmtree
//
//  Created by Michael Hahn on 2/14/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#include <ctype.h>

#include "utils.h"

void remove_extra_whitespace(char* input_str) {
    int i, x;
    for (i=x=0; input_str[i]; ++i) {
        if (!isspace(input_str[i]) || (i > 0 && !isspace(input_str[i-1])))
            input_str[x++] = input_str[i];
    }
    input_str[x] = '\0';
    return;
}

int check_valid_int(char* input_str) {
    int i;
    if (input_str[0] == '-') {
        for (i = 1; input_str[i] != '\0' && input_str[i] != '\n'; i++) {
            if (input_str[i] != '0' && input_str[i] != '1' && input_str[i] != '2' && input_str[i] != '3'
                && input_str[i] != '4' && input_str[i] != '5' && input_str[i] != '6' && input_str[i] != '7'
                && input_str[i] != '8' && input_str[i] != '9')
                return -1;
        }
    } else {
        for (i = 0; input_str[i] != '\0' && input_str[i] != '\n'; i++) {
            if (input_str[i] != '0' && input_str[i] != '1' && input_str[i] != '2' && input_str[i] != '3'
                && input_str[i] != '4' && input_str[i] != '5' && input_str[i] != '6' && input_str[i] != '7'
                && input_str[i] != '8' && input_str[i] != '9')
                return -1;
        }
    }
    return 0;
}

int to_int(char* int_string) {
    int c, sign, offset, n;
    // Handle negative integers
    if (int_string[0] == '-')
        sign = -1;
    // Set starting position to convert
    if (sign == -1)
        offset = 1;
    else
        offset = 0;
    n = 0;
    for (c = offset; int_string[c] != '\0' && int_string[c] != '\n'; c++)
        n = n * 10 + int_string[c] - '0';
    if (sign == -1)
        n = -n;
    return n;
}