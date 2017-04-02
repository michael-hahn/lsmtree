//
//  tree.cpp
//  lsmtree
//
//  Created by Michael Hahn on 3/15/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//
#include <sstream>
#include <cassert>
#include <cmath>
#include <limits>
#include "tree.hpp"

Tree::Tree(std::string file_path, int estimate_number_insertion, double false_pos_prob) {
    //std::cout << "LOGINFO:\t\t" << "Constructing tree bloom filter..." << std::endl;
    construct_tree_filter(estimate_number_insertion, false_pos_prob);
    
    this->fd = open(file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
        perror("Error opening a file in this level");
        exit(EXIT_FAILURE);
    }
    
    int result = lseek(this->fd, sysconf(_SC_PAGE_SIZE) * INIT_PAGE_NUM - 1, SEEK_SET);
    if (result == -1) {
        close(this->fd);
        perror("Error using lseek() to stretch the file");
        exit(EXIT_FAILURE);
    }
    result = write(this->fd, "", 1);
    if (result == -1) {
        close(this->fd);
        perror("Error writing last byte of the file");
        exit(EXIT_FAILURE);
    }
    
    BPlusTree* new_btree =  new BPlusTree(this->fd);
    this->btree = new_btree;
}

void Tree::construct_tree_filter (int estimate_number_insertion, double false_pos_prob) {
    bloom_parameters parameters;
    parameters.projected_element_count = estimate_number_insertion;
    parameters.false_positive_probability = false_pos_prob;
    
    if (!parameters) {
        //std::cout << "LOGFATAL:\t\t" << "Invalid tree bloom filter parameters" << std::endl;
        assert(parameters);
    }
    
    parameters.compute_optimal_parameters();
    
    bloom_filter filter(parameters);
    this->tree_filter = filter;
    return;
}

bool Tree::in_tree (int key) {
    if (this->tree_filter.contains(key) > 0) {
        //std::cout << "LOGINFO:\t\t" << "Tree bloom filter contains " << key << std::endl;
        return true;
    } else {
        //std::cout << "LOGINFO:\t\t" << "Tree bloom filter does NOT contain " << key << std::endl;
        return false;
    }
}

void Tree::free_mem() {
    delete this->btree;
    close(this->fd);
    return;
}

void Tree::insert_or_update (int key, long value) {
    if (value == LONG_MAX) {
        //std::cout << "LOGINFO:\t\t" << key << " is to be deleted from the tree instead of inserted." << std::endl;
        delete_key(key);
        return;
    }
    if (!in_tree(key)) {
        this->tree_filter.insert(key);
        //std::cout << "LOGINFO:\t\t" << "Insertion to tree bloom filter succeeded." << std::endl;
    } //else std::cout << "LOGINFO:\t\t" << "Tree bloom filter already contains this key: " << key << std::endl;
    std::string aValue = this->btree->getValue(key);
    std::stringstream value_ss;
    value_ss << value;
    if (aValue == "") {
        this->btree->insert(key, value);
    }
    else if (aValue != value_ss.str()) {
        this->btree->remove(key);
        this->btree->insert(key, value);
    }
    
}

std::string Tree::get_value_or_blank (int key) {
    std::string no_entry = "";
    if (in_tree(key)) {
    //if (true) {
        return this->btree->getValue(key);
    } else {
        //std::cout << "LOGINFO:\t\t" << "No match found in tree according to tree bloom filter. No entry." << std::endl;
        return no_entry;
    }
}

std::string Tree::range (int lower, int upper) {
    return this->btree->getRange(lower, upper);
}

void Tree::efficient_range (int lower, int upper, std::map<int, long>& result){
    LeafNode* node = this->btree->find_first_leaf_node(this->btree->getRoot());
    if (!node)
        return;
    else {
        while (node) {
            for (auto mapping : node->get_mappings()) {
                if (mapping.first >= lower && mapping.first < upper) {
                    std::pair<int, int> addr = mapping.second->value();
                    long* map = (long*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, sysconf(_SC_PAGE_SIZE) * (addr.first - 1));
                    if (map == MAP_FAILED) {
                        close(fd);
                        perror("Error mmapping the file for insertion");
                        exit(EXIT_FAILURE);
                    }
                    long value = map[addr.second - 1];
                    if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
                        perror("Error unmapping the file");
                        close(fd);
                        exit(EXIT_FAILURE);
                    }
                    result.insert(std::pair<int, long>(mapping.first, value));
                }
            }
            node = node->next();
        }
    }
}

void Tree::delete_key (int key) {
    if (in_tree(key)) {
        this->btree->remove(key);
    }
}

//TODO: Dump Tree from Disk
std::pair<unsigned long, std::string> Tree::tree_dump (std::set<std::pair<int, bool>, set_compare>& found_once) {
    return this->btree->print_leaves_string(this->fd, found_once);
}