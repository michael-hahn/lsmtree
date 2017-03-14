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
#include "database.hpp"
#include "bloom_filter.hpp"

#define MAXCACHESIZE 20

class Cache {
private:
    std::vector<std::pair<int, int>> cache;
    
    bloom_filter cache_filter;
    
    /**
     * false_pos_prob: (0, 1)
     * deletion of an element in cache will not have an effect on cache bloom filter
     * therefore, false positive rate may be higher
     */
    void construct_cache_filter (int estimate_number_insertion, double false_pos_prob);
    
public:
    bool in_cache (int key);
    
    void insert (int key, int value, Db* database);
    
    std::string get_value_or_blank (int key, Db* database);
    
    std::string range (int lower, int upper, Db* database);
    
    void delete_key (int key, Db* database);
    
    std::pair<std::string, int> cache_dump ();
    
};

#include "cache.cpp"
#endif /* cache_hpp */
