//
//  utils.c
//  lsmtree
//
//  Created by Michael Hahn on 2/14/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#include <ctype.h>
#include <string>
#include <fstream>
#include <iostream>
#include <set>
#include "cache.hpp"
#include "database.hpp"
#include "tree.hpp"
//#include "utils.h"

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

int read_binary_file (std::string file_path, Cache* cache, Db* database, Tree* btree) {
    int key;
    int value;
    
    std::ifstream file;
    file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    file.open (file_path, std::ifstream::binary);
    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        int size = (int) file.tellg();
        file.seekg(0, std::ios::beg);

        while ((int) file.tellg() < size) {
            file.read((char*)&key, 4);
            
            file.read((char*)&value, 4);
            
            std::cout << "LOGINFO:\t\t" << "Inserting " << key << " : " << value << " from binary file..." << std::endl;
            cache->insert(key, value, database, btree);
        }
        std::cout << "LOGINFO:\t\t" << "Insertion from binary file is done..." << std::endl;
        file.close();
    } else {
        std::cout << "LOGFATAL:\t\t" << "Opening binary file failed..." << std::endl;
    }
    return 0;
}

unsigned long total_size (Cache* cache, Db* database, Tree* btree) {
    std::set<int> found_once;
    std::set<int>::iterator set_it;
    std::pair<std::set<int>::iterator, bool> set_rtn;
    
    std::set<int> cache_keys = cache->all_keys();
    std::set<int> database_keys = database->all_keys();
    
    for (std::set<int>::iterator cache_it = cache_keys.begin(); cache_it != cache_keys.end(); cache_it++) {
        found_once.insert(*cache_it);
    }
    
    for (std::set<int>::iterator database_it = database_keys.begin(); database_it != database_keys.end(); database_it++) {
        found_once.insert(*database_it);
    }
    
    unsigned long less = 0;
    for (std::set<int>::iterator fo_it = found_once.begin(); fo_it != found_once.end(); fo_it++) {
        if (btree->get_value_or_blank(*fo_it) != "") {
            std::cout << "LOGINFO:\t\t" << "Found key: " << *fo_it <<  " in the B+ tree." << std::endl;
            less++;
        }
    }
    unsigned long total_valid_keys = btree->tree_dump().first;
    total_valid_keys += found_once.size();
    total_valid_keys -= less;
    return total_valid_keys;
}


//For debugging only. File path is hardcoded.
int write_binary_file () {
    std::ofstream file ("/Users/Michael/Documents/Harvard/g1/cs265/xcode/binary.dat", std::ofstream::binary);
    for (int i = 0; i < 100; i++) {
        file.write((char*) &i, sizeof(i));
    }
    file.close();
    return 0;
}
