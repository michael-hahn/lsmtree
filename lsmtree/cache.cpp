//
//  cache.cpp
//  lsmtree
//
//  Created by Michael Hahn on 3/9/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#include "cache.hpp"
#include <iostream>
#include <sstream>
#include <set>
#include <cassert>

void Cache::construct_cache_filter (int estimate_number_insertion, double false_pos_prob) {
    bloom_parameters parameters;
    parameters.projected_element_count = estimate_number_insertion;
    parameters.false_positive_probability = false_pos_prob;
    
    if (!parameters) {
        std::cout << "LOGFATAL:\t\t" << "Invalid cache bloom filter parameters" << std::endl;
        assert(parameters);
    }
    
    parameters.compute_optimal_parameters();
    
    bloom_filter filter(parameters);
    this->cache_filter = filter;
    return;
}

bool Cache::in_cache (int key) {
    if (this->cache_filter.contains(key)) {
        std::cout << "LOGINFO:\t\t" << "Cache bloom filter contains " << key << std::endl;
        return true;
    } else {
        std::cout << "LOGINFO:\t\t" << "Cache bloom filter does NOT contain " << key << std::endl;
        return false;
    }
}

//Insertion just inserts to the beginning of the vector
//Not update needed
//Always insert at the front if there is space
//If cache is full, flush half of the cache to disk before insertion
//also insert to bloom filter
void Cache::insert (int key, int value, Db* database) {
    std::pair<int, int> entry (key, value);
    this->cache_filter.insert(key);
    std::cout << "LOGINFO:\t\t" << "Insertion to cache bloom filter succeeded." << std::endl;
    if (this->cache.size() < MAXCACHESIZE) {
        std::vector<std::pair<int, int>>::iterator it = this->cache.begin();
        this->cache.insert(it, entry);
        std::cout << "LOGINFO:\t\t" << "Insertion to cache succeeded with no flush to disk." << std::endl;
        std::cout << "LOGINFO:\t\t" << "Cache entries: " << this->cache.size() << std::endl;
        return;
    } else {
        for (int i = 0; i < MAXCACHESIZE / 2; i++) {
            std::pair<int, int> remove_from_cache = this->cache.back();
            database->insert_or_update(remove_from_cache.first, remove_from_cache.second);
            this->cache.pop_back();
            std::cout << "LOGINFO:\t\t" << "Insertion to cache causes flush " << remove_from_cache.first << " : " << remove_from_cache.second << " to disk." << std::endl;
        }
        std::vector<std::pair<int, int> >::iterator it = this->cache.begin();
        this->cache.insert(it, entry);
        std::cout << "LOGINFO:\t\t" << "Insertion to cache succeeded after flush entries to disk." << std::endl;
        std::cout << "LOGINFO:\t\t" << "Cache entries: " << this->cache.size() << std::endl;
        return;
    }
}

//if cache bloom filter indicates an existance of key in cache, search cache
//Always start to search from the beginning of the vector since newer value is located at the beginning
//Otherwise search the database
std::string Cache::get_value_or_blank (int key, Db* database) {
    std::string rtn = "";
    if (in_cache(key)) {
        for (std::vector<std::pair<int, int>>::iterator it = this->cache.begin(); it != this->cache.end(); it++) {
            if (it->first == key) {
                std::cout << "LOGINFO:\t\t" << "Find entry in cache: " << it->first << " : " << it->second << std::endl;
                std::stringstream out;
                out << it->second;
                rtn = out.str();
                break;
            }
        }
    }
    else {
        std::cout << "LOGINFO:\t\t" << "No match found in cache according to cache bloom filter. Searching the database..." << std::endl;
        rtn = database->get_value_or_blank(key);
    }
    return rtn;
}

//We flush the whole vector to the database when receiving a range query
//we also flush the bloom filter
std::string Cache::range (int lower, int upper, Db* database) {
    while (!this->cache.empty()) {
        std::pair<int, int> remove_from_cache = this->cache.back();
        database->insert_or_update(remove_from_cache.first, remove_from_cache.second);
        this->cache.pop_back();
        std::cout << "LOGINFO:\t\t" << "Range query causes flush " << remove_from_cache.first << " : " << remove_from_cache.second << " to disk." << std::endl;
    }
    this->cache_filter.clear();
    std::cout << "LOGINFO:\t\t" << "Clear out cache bloom filter." << std::endl;
    return database->range(lower, upper);
}

//We need to delete all instances of the key in cache and the database
void Cache::delete_key (int key, Db* database) {
    std::vector<std::pair<int, int>>::iterator it = this->cache.begin();
    while (it != this->cache.end()) {
        if (it->first == key) {
            std::cout << "LOGINFO:\t\t" << "Remove entry in the cache: " << it->first << " : " << it->second << std::endl;
            this->cache.erase(it);
        } else
            it++;
    }
    std::cout << "LOGINFO:\t\t" << "Remove database key..." << std::endl;
    database->delete_key(key);
    return;
}

//Only shows the most updated key-value pair
//also returns the total valid entries in the cache
std::pair<std::string, int> Cache::cache_dump () {
    int total_valid = 0;
    std::string rtn = "";
    std::set<int> found_once;
    std::set<int>::iterator set_it;
    std::pair<std::set<int>::iterator, bool> set_rtn;
    
    for (std::vector<std::pair<int, int>>::iterator it = this->cache.begin(); it != this->cache.end(); it++) {
        set_rtn = found_once.insert(it->first);
        if (set_rtn.second == true) {
            std::stringstream first_ss;
            first_ss << it->first;
            std::stringstream second_ss;
            second_ss << it->second;
            rtn += first_ss.str() + ":" + second_ss.str() + ":" + "L1" + " ";
            total_valid++;
        } else
            std::cout << "LOGINFO:\t\t" << "Entry " << it->first << " : " << it->second << " is old." << std::endl;
    }
    return std::pair<std::string, int> (rtn, total_valid);
    
}







