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
#include <limits>
#include <cmath>
#include <map>
#include <algorithm>

Cache::Cache(int estimate_number_insertion, double false_pos_prob){
    //std::cout << "LOGINFO:\t\t" << "Constructing cache bloom filter..." << std::endl;
    construct_cache_filter(estimate_number_insertion, false_pos_prob);
}

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
    if (this->cache_filter.contains(key) > 0) {
        //std::cout << "LOGINFO:\t\t" << "Cache bloom filter contains " << key << std::endl;
        return true;
    } else {
        //std::cout << "LOGINFO:\t\t" << "Cache bloom filter does NOT contain " << key << std::endl;
        return false;
    }
}

//sanitize the vector for insertion into the next level
//only the most updated value will be push to the next level
//including delete entry.
std::vector<std::pair<int, long>> Cache::sanitize() {
    std::vector<std::pair<int, long>> rtn;
    std::set<std::pair<int, long>, kv_compare> sanitized_set;
    for (std::vector<std::pair<int, long>>::iterator it = this->cache.begin(); it != this->cache.end(); it++) {
        sanitized_set.insert(*it);
    }
    for (std::set<std::pair<int, long>, kv_compare>::iterator it = sanitized_set.begin(); it != sanitized_set.end(); it++) {
        rtn.push_back(*it);
    }
    return rtn;
}

//Insertion just inserts to the beginning of the vector
//Not update needed
//Always insert at the front if there is space
//If cache is full, flush all of the cache to the next level before insertion
void Cache::insert (int key, long value, Memmapped* mm1, Memmapped2* mm2, Memmapped3* mm3) {
    std::pair<int, long> entry (key, value);
    //if (value == LONG_MAX) std::cout << "LOGINFO:\t\t" << "Deletion entry is inserted into the cache." << std::endl;
    if (this->cache.size() < MAXCACHESIZE) {
        std::vector<std::pair<int, long>>::iterator it = this->cache.begin();
        if (!in_cache(key)) {
            this->cache_filter.insert(key);
        }
        this->cache.insert(it, entry);
        //std::cout << "LOGINFO:\t\t" << "Insertion to cache succeeded with no flush to disk." << std::endl;
        //std::cout << "LOGINFO:\t\t" << "Cache entries: " << this->cache.size() << std::endl;
        return;
    } else {
        mm1->insert(this->sanitize(), mm2, mm3);
        //std::cout << "LOGINFO:\t\t" << "Insertion to cache causes flush " << remove_from_cache.first << " : " << remove_from_cache.second << " to disk." << std::endl;
        this->cache.clear();
        this->cache_filter.clear();
        std::vector<std::pair<int, long>>::iterator it = this->cache.begin();
        if (!in_cache(key)) {
            this->cache_filter.insert(key);
        }
        this->cache.insert(it, entry);
        //std::cout << "LOGINFO:\t\t" << "Insertion to cache succeeded after flush entries to disk." << std::endl;
        //std::cout << "LOGINFO:\t\t" << "Cache entries: " << this->cache.size() << std::endl;
        return;
    }
}

//if cache bloom filter indicates an existance of key in cache, search cache
//Always start to search from the beginning of the vector since newer value is located at the beginning
//Otherwise search the database
std::string Cache::get_value_or_blank (int key, Memmapped* mm1, Memmapped2* mm2, Memmapped3* mm3) {
    std::string rtn = "";
    if (in_cache(key)) {
    //if (true) {
        for (std::vector<std::pair<int, long>>::iterator it = this->cache.begin(); it != this->cache.end(); it++) {
            if (it->first == key) {
                //std::cout << "LOGINFO:\t\t" << "Find entry in cache: " << it->first << " : " << it->second << std::endl;
                if (it->second == LONG_MAX) {
                    //std::cout << "LOGINFO:\t\t" << "Cache finds the deletion entry in key: " << key << std::endl;
                    return rtn;
                } else {
                    std::stringstream out;
                    out << it->second;
                    rtn = out.str();
                    break;
                }
            }
        }
        if (rtn == "") {
            //std::cout << "LOGINFO:\t\t" << "Cache counting bloom filter returns false positive. Searching the database..." << std::endl;
            rtn = mm1->get_value_or_blank(key, mm2, mm3);
        }
    }
    else {
        //std::cout << "LOGINFO:\t\t" << "No match found in cache according to cache bloom filter. Searching the database..." << std::endl;
        rtn = mm1->get_value_or_blank(key, mm2, mm3);
    }
    return rtn;
}

void Cache::efficient_range(int lower, int upper, Memmapped* mm1, Memmapped2* mm2, Memmapped3* mm3, std::map<int, long>& result) {
    for (std::vector<std::pair<int, long>>::iterator it = this->cache.begin(); it != this->cache.end(); it++) {
        if (it->first >= lower && it->first < upper) {
            result.insert(*it); //only the most updated key-value pairs will be inserted
        }
    }
    mm1->efficient_range(lower, upper, mm2, mm3, result);
    return;
    
}

//We insert the deletion into the cache. The key is marked "delete": we implement as LONG_MAX (valid values are int-sized only).
void Cache::delete_key (int key, Memmapped* mm1, Memmapped2* mm2, Memmapped3* mm3) {
    //std::cout << "LOGINFO:\t\t" << "Remove " << key << " from the database" << std::endl;
    insert(key, LONG_MAX, mm1, mm2, mm3);
}

//Only shows the most updated key-value pair
//also returns the total valid entries in the cache
std::pair<std::string, int> Cache::cache_dump (std::set<std::pair<int, bool>, set_compare>& found_once) {
    int total_valid = 0;
    std::string rtn = "";
    std::set<int>::iterator set_it;
    std::pair<std::set<std::pair<int, bool>, set_compare>::iterator, bool> set_rtn;
    
    for (std::vector<std::pair<int, long>>::iterator it = this->cache.begin(); it != this->cache.end(); it++) {
        if (it->second == LONG_MAX) {
            set_rtn = found_once.insert(std::pair<int, bool>(it->first, false));
        } else {
            set_rtn = found_once.insert(std::pair<int, bool>(it->first, true));
        }
        if (set_rtn.second == true) {
            if (it->second != LONG_MAX) {
                std::stringstream first_ss;
                first_ss << it->first;
                std::stringstream second_ss;
                second_ss << it->second;
                rtn += first_ss.str() + ":" + second_ss.str() + ":" + "L1" + " ";
                total_valid++;
            }
        } //else
            //std::cout << "LOGINFO:\t\t" << "Entry " << it->first << " : " << it->second << " is old." << std::endl;
    }
    return std::pair<std::string, int> (rtn, total_valid);
    
}

//same as cache_dump but returns a set that holds all valid keys instead of a string
std::pair<std::set<int>, std::set<int>> Cache::all_keys () {
    std::set<int> found_once;
    std::set<int> to_delete;
    std::pair<std::set<int>::iterator, bool> set_rtn;
    
    for (std::vector<std::pair<int, long>>::iterator it = this->cache.begin(); it != this->cache.end(); it++) {
        set_rtn = found_once.insert(it->first);
        if (set_rtn.second == true) {
            if (it->second == LONG_MAX) {
                //std::cout << "LOGINFO:\t\t" << "Entry " << it->first << " : " << it->second << " is to be deleted." << std::endl;
                to_delete.insert(it->first);
            }
            //std::cout << "LOGINFO:\t\t" << "Entry " << it->first << " : " << it->second << " counts into total count." << std::endl;
        } //else std::cout << "LOGINFO:\t\t" << "Entry " << it->first << " : " << it->second << " is old." << std::endl;
    }
    for (std::set<int>::iterator set_it = to_delete.begin(); set_it != to_delete.end(); set_it++) {
        found_once.erase(*set_it);
    }
    return std::pair<std::set<int>, std::set<int>> (found_once, to_delete);
}







