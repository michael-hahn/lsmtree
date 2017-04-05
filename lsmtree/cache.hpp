//
//  cache.hpp
//  lsmtree
//
//  Created by Michael Hahn on 3/9/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef cache_hpp
#define cache_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <set>
#include "database.hpp"
#include "bloom_filter.hpp"
#include "tree.hpp"
#include "memmapped.hpp"
#include "memmapped2.hpp"
#include "memmappedL.hpp"
#include "tree.hpp"
#include "comp.h"
#include <pthread.h>

#define MAXCACHESIZE 4

struct kv_compare {
    bool operator() (const std::pair<int, long> elt1, const std::pair<int, long> elt2) {
        return (elt1.first < elt2.first);
    }
};

class Cache {
private:
    std::vector<std::pair<int, long>> cache;
    
    bloom_filter cache_filter;
    
    void construct_cache_filter (int estimate_number_insertion, double false_pos_prob);
        
public:
    Cache(int estimate_number_insertion, double false_pos_prob);
    
    bool in_cache (int key);
    
    std::vector<std::pair<int, long>> sanitize();
    
    void insert (int key, long value, Memmapped* mm1, Memmapped2* mm2, MemmappedL* mml, Tree* tree);
    
    std::string get_value_or_blank (int key, Memmapped* mm1, Memmapped2* mm2, MemmappedL* mml, Tree* tree);
    
    void* get_value_or_blank_pthread (void* thread_data);
    
    void efficient_range(int lower, int upper, Memmapped* mm1, Memmapped2* mm2, MemmappedL* mml, Tree* tree, std::map<int, long>& result);
    
    void* efficient_range_pthread (void* thread_data);
    
    void delete_key (int key, Memmapped* mm1, Memmapped2* mm2, MemmappedL* mml, Tree* tree);
    
    std::pair<std::string, int> cache_dump (std::set<std::pair<int, bool>, set_compare>& found_once);
    
    std::pair<std::set<int>, std::set<int>> all_keys ();
    
};

#include "cache.cpp"
#endif /* cache_hpp */
