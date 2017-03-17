//
//  tree.cpp
//  lsmtree
//
//  Created by Michael Hahn on 3/15/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//
#include <sstream>
#include <cassert>
#include "tree.hpp"

Tree::Tree(int estimate_number_insertion, double false_pos_prob) {
    std::cout << "LOGINFO:\t\t" << "Constructing tree bloom filter..." << std::endl;
    construct_tree_filter(estimate_number_insertion, false_pos_prob);
}

void Tree::construct_tree_filter (int estimate_number_insertion, double false_pos_prob) {
    bloom_parameters parameters;
    parameters.projected_element_count = estimate_number_insertion;
    parameters.false_positive_probability = false_pos_prob;
    
    if (!parameters) {
        std::cout << "LOGFATAL:\t\t" << "Invalid tree bloom filter parameters" << std::endl;
        assert(parameters);
    }
    
    parameters.compute_optimal_parameters();
    
    bloom_filter filter(parameters);
    this->tree_filter = filter;
    return;
}

bool Tree::in_tree (int key) {
    if (this->tree_filter.contains(key) > 0) {
        std::cout << "LOGINFO:\t\t" << "Tree bloom filter contains " << key << std::endl;
        return true;
    } else {
        std::cout << "LOGINFO:\t\t" << "Tree bloom filter does NOT contain " << key << std::endl;
        return false;
    }
}

void Tree::insert_or_update (int key, int value) {
    if (!in_tree(key)) {
        this->tree_filter.insert(key);
        std::cout << "LOGINFO:\t\t" << "Insertion to tree bloom filter succeeded." << std::endl;
    } else std::cout << "LOGINFO:\t\t" << "Tree bloom filter already contains this key: " << key << std::endl;
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
    std::string no_entry = "";
    if (in_tree(key)) {
        return this->btree.getValue(key);
    } else {
        std::cout << "LOGINFO:\t\t" << "No match found in tree according to tree bloom filter. No entry." << std::endl;
        return no_entry;
    }
}

std::string Tree::range (int lower, int upper) {
    return this->btree.getRange(lower, upper);
}

void Tree::delete_key (int key) {
    if (in_tree(key)) {
        this->btree.remove(key);
    }
}

std::pair<unsigned long, std::string> Tree::tree_dump () {
    return this->btree.print_leaves_string();
}