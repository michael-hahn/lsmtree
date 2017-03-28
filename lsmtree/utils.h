//
//  utils.h
//  lsmtree
//
//  Created by Michael Hahn on 2/14/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include <stdio.h>
#include "cache.hpp"
#include "database.hpp"
#include "tree.hpp"
#include "memmapped.hpp"
#include "memmapped2.hpp"

/**
 * trim away extra whitespace and leave only one
 */
void remove_extra_whitespace(char* input_str);

/**
 * check if the input string can possibly be converted to an int
 * strings such as 012 and -012 are acceptable and will be interpreted to int as 12 and -12
 * strings such as abc, 1as, 1-9, 9u8 are not
 * returns -1 if incorrect and 0 if correct
 * THIS CHECK CAN BE DISABLED IF ASSUME CORRECT INPUTS ALWAYS
 */
int check_valid_int(char* input_str);

/**
 * convert a signed integer character string to signed integer without using atoi
 */
int to_int(char int_string[]);

/**
 * read a binary file that contains key-value pairs
 */
int read_binary_file (std::string file_path, Cache* cache, Db* database);

/**
 * This is for debugging only
 */
int write_binary_file ();

//#include "utils.cpp"
#endif /* utils_h */
