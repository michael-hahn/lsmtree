//
//  tree.hpp
//  lsmtree
//
//  Created by Michael Hahn on 3/15/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef tree_hpp
#define tree_hpp
#pragma once

#include <stdio.h>
#include <string>
#include <map>
#include "Definitions.hpp"
#include "BPlusTree.hpp"
#include "bloom_filter.hpp"

class Tree {
public:
    Tree(int estimate_number_insertion, double false_pos_prob);
    
    bool in_tree (int key);
    
    void insert_or_update (int key, long value);
    
    std::string get_value_or_blank (int key);
    
    std::string range (int lower, int upper);
    
    void efficient_range (int lower, int upper, std::map<int, long>& result);
    
    void delete_key (int key);
    
    std::pair<unsigned long, std::string> tree_dump ();
    
private:
    BPlusTree btree;
    
    bloom_filter tree_filter;
    
    void construct_tree_filter (int estimate_number_insertion, double false_pos_prob);
};

#include "tree.cpp"
#endif /* tree_hpp */
