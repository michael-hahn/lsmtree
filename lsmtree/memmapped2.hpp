//
//  memmapped2.hpp
//  lsmtree
//
//  Created by Michael Hahn on 3/27/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef memmapped2_hpp
#define memmapped2_hpp

#include <utility>
#include <string>
#include <vector>
#include <map>
#include "bloom_filter.hpp"
#include "memmappedL.hpp"
#include "tree.hpp"
#include "comp.h"
#include <pthread.h>

#define ARRAY_NUM_2 4
#define FILESIZE_2 (sysconf(_SC_PAGE_SIZE) * ARRAY_NUM_2)


class Memmapped2 {
private:
    std::pair<int, int> fenses[ARRAY_NUM_2];
    
    size_t elt_size[ARRAY_NUM_2];
    
    int fd;
    
    int cur_array_num;
    
    std::pair<int, long>* mapped_addr[ARRAY_NUM_2];
    
    //a bloom filter for each page
    bloom_filter mm2_filter[ARRAY_NUM_2];
    
    void construct_mm2_filter (int estimate_number_insertion, double false_pos_prob);

    
public:
    Memmapped2(std::string file_path, int estimate_number_insertion, double false_pos_prob);
    
    bool in_mm2 (int key, int num);
    
    void free_mem();
    
    std::map<int, long> consolidate();
    
    void insert(std::map<int, long> pairs, MemmappedL* mml, Tree* tree);
    
    std::string get_value_or_blank(int key, MemmappedL* mml, Tree* tree);
    
    void* get_value_or_blank_pthread (void* thread_data);
    
    void efficient_range(int lower, int upper, MemmappedL* mml, Tree* tree, std::map<int, long>& result);
    
    void* efficient_range_pthread (void* thread_data);
    
    std::pair<std::string, int> mm2_dump (std::set<std::pair<int, bool>, set_compare>& found_once);
};



#include "memmapped2.cpp"

#endif /* memmapped2_hpp */
