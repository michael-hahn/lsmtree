//
//  database.hpp
//  lsmtree
//
//  Created by Michael Hahn on 3/9/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef database_hpp
#define database_hpp

#include <stdio.h>
#include <map>
#include <string>
#include <set>
#include "tree.hpp"
#include "bloom_filter.hpp"

#define MAXDATABASESIZE 100

class Db {
private:
    std::map<int, long> database;
    
    bloom_filter database_filter;
    
    void construct_database_filter (int estimate_number_insertion, double false_pos_prob);
    
public:
    Db (int estimate_number_insertion, double false_pos_prob);
    
    bool in_database (int key);
    
    void insert_or_update (int key, long value, Tree* btree);
    
    std::string get_value_or_blank (int key, Tree* btree);
    
    std::string range (int lower, int upper, Tree* btree);
    
    void efficient_range (int lower, int upper, Tree* btree, std::map<int, long>& result);
    
    void delete_key (int key, Tree* btree);
    
    std::pair<std::string, int> db_dump ();
    
    std::pair<std::set<int>, std::set<int>> all_keys ();
};

#include "database.cpp"
#endif /* database_hpp */
