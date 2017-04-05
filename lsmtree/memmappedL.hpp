//
//  memmappedL.hpp
//  lsmtree
//
//  Created by Michael Hahn on 4/2/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef memmappedL_hpp
#define memmappedL_hpp

#include <utility>
#include <string>
#include <vector>
#include <map>
#include "bloom_filter.hpp"
#include "comp.h"
#include "tree.hpp"
#include <pthread.h>

#define ARRAY_NUM_L 8
#define FILE_SIZE_L (sysconf(_SC_PAGE_SIZE) * ARRAY_NUM_L)

class MemmappedL {
private:
    std::pair<int, int> fenses[ARRAY_NUM_L];
    
    size_t elt_size[ARRAY_NUM_L];
    
    int fd;
    
    int cur_array_num;
    
    std::pair<int, long>* mapped_addr[ARRAY_NUM_L];
    
    bloom_filter mml_filter[ARRAY_NUM_L];
    
    void construct_mml_filter (int estimate_number_insertion, double false_pos_prob);
    
public:
    MemmappedL(std::string file_path, int estimate_number_insertion, double false_pos_prob);
    
    bool in_mml (int key, int num);
    
    void free_mem();
    
    std::map<int, long> consolidate();
    
    void insert(std::map<int, long> pairs, Tree* tree);
    
    std::string get_value_or_blank(int key, Tree* tree);
    
    void* get_value_or_blank_pthread (void* thread_data);
    
    void efficient_range(int lower, int upper, Tree* tree, std::map<int, long>& result);
    
    void* efficient_range_pthread (void* thread_data);
    
    std::pair<std::string, int> mml_dump (std::set<std::pair<int, bool>, set_compare>& found_once);
};

#include "memmappedL.cpp"

#endif /* memmappedL_hpp */
