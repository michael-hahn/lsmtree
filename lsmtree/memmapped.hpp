//
//  memmapped.hpp
//  lsmtree
//
//  Created by Michael Hahn on 3/27/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef memmapped_hpp
#define memmapped_hpp
#include <utility>
#include <string>
#include <vector>
#include <map>
#include <string>
#include <set>
#include "memmapped2.hpp"
#include "memmappedL.hpp"
#include "tree.hpp"
#include "bloom_filter.hpp"
#include "comp.h"
#include <pthread.h>

#define ARRAY_NUM 2
#define FILESIZE (sysconf(_SC_PAGE_SIZE) * ARRAY_NUM)

class Memmapped {
private:
    std::pair<int, int> fenses[ARRAY_NUM];
    
    size_t elt_size[ARRAY_NUM];
    
    int fd;
    
    int cur_array_num;
    
    std::pair<int, long>* mapped_addr[ARRAY_NUM];
    
    //a bloom filter for each page
    bloom_filter mm1_filter[ARRAY_NUM];
    
    void construct_mm1_filter (int estimate_number_insertion, double false_pos_prob);
    
public:
    Memmapped(std::string file_path, int estimate_number_insertion, double false_pos_prob);
    
    bool in_mm1 (int key, int num);
    
    void free_mem();
    
    std::map<int, long> consolidate();
    
    void insert(std::vector<std::pair<int, long>> cache, Memmapped2* mm2, MemmappedL* mm, Tree* tree);
    
    std::string get_value_or_blank(int key, Memmapped2* mm2, MemmappedL* mml, Tree* tree);
    
    void* get_value_or_blank_pthread (void* thread_data);
    
    void efficient_range(int lower, int upper, Memmapped2* mm2, MemmappedL* mml, Tree* tree, std::map<int, long>& result);
    
    void* efficient_range_pthread (void* thread_data);
    
    std::pair<std::string, int> mm1_dump (std::set<std::pair<int, bool>, set_compare>& found_once);
};



#include "memmapped.cpp"
#endif /* memmapped_hpp */
