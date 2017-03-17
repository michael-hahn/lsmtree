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

#define MAXCACHESIZE 10

class Cache {
private:
    std::vector<std::pair<int, int>> cache;
    
    bloom_filter cache_filter;
    
    void construct_cache_filter (int estimate_number_insertion, double false_pos_prob);
        
public:
    Cache(int estimate_number_insertion, double false_pos_prob);
    
    bool in_cache (int key);
    
    void insert (int key, int value, Db* database, Tree* btree);
    
    std::string get_value_or_blank (int key, Db* database, Tree* btree);
    
    std::string range (int lower, int upper, Db* database, Tree* btree);
    
    void delete_key (int key, Db* database, Tree* btree);
    
    std::pair<std::string, int> cache_dump ();
    
    std::set<int> all_keys ();
    
};

#include "cache.cpp"
#endif /* cache_hpp */
