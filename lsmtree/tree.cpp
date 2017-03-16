//
//  tree.cpp
//  lsmtree
//
//  Created by Michael Hahn on 3/15/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//
#include <sstream>
#include "tree.hpp"

void Tree::insert_or_update (int key, int value) {
    std::string aValue = this->btree.getValue(key);
    std::stringstream value_ss;
    value_ss << value;
    if (aValue == "")
        this->btree.insert(key, value);
    else if (aValue != value_ss.str()) {
        this->btree.remove(key);
        this->btree.insert(key, value);
    }
    
}

std::string Tree::get_value_or_blank (int key) {
    return this->btree.getValue(key);
}

std::string Tree::range (int lower, int upper) {
    return this->btree.getRange(lower, upper);
}

void Tree::delete_key (int key) {
    this->btree.remove(key);
}

std::pair<unsigned long, std::string> Tree::tree_dump () {
    return this->btree.print_leaves_string();
}