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
#include "Definitions.hpp"
#include "BPlusTree.hpp"

class Tree {
public:
    void insert_or_update (int key, int value);
    
    std::string get_value_or_blank (int key);
    
    std::string range (int lower, int upper);
    
    void delete_key (int key);
    
    std::pair<unsigned long, std::string> tree_dump ();
    
private:
    BPlusTree btree;
};

#include "tree.cpp"
#endif /* tree_hpp */
