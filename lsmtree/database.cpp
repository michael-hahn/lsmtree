//
//  database.cpp
//  lsmtree
//
//  Created by Michael Hahn on 3/9/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#include "database.hpp"

#include <iostream>
#include <sstream>
#include <cassert>

Db::Db (int estimate_number_insertion, double false_pos_prob) {
    std::cout << "LOGINFO:\t\t" << "Constructing database bloom filter..." << std::endl;
    construct_database_filter(estimate_number_insertion, false_pos_prob);
}

void Db::construct_database_filter (int estimate_number_insertion, double false_pos_prob) {
    bloom_parameters parameters;
    parameters.projected_element_count = estimate_number_insertion;
    parameters.false_positive_probability = false_pos_prob;
    
    if (!parameters) {
        std::cout << "LOGFATAL:\t\t" << "Invalid database bloom filter parameters" << std::endl;
        assert(parameters);
    }
    
    parameters.compute_optimal_parameters();
    
    bloom_filter filter(parameters);
    this->database_filter = filter;
    return;
}

bool Db::in_database(int key) {
    if (this->database_filter.contains(key)) {
        std::cout << "LOGINFO:\t\t" << "Database bloom filter contains " << key << std::endl;
        return true;
    } else {
        std::cout << "LOGINFO:\t\t" << "Database bloom filter does NOT contain " << key << std::endl;
        return false;
    }
}


void Db::insert_or_update (int key, int value, Tree* btree) {
    this->database_filter.insert(key);
    if (this->database.size() >= MAXDATABASESIZE) {
        for (int i = 0; i < MAXDATABASESIZE / 2; i++) {
            std::map<int, int>::iterator it = this->database.begin();
            btree->insert_or_update(it->first, it->second);
            this->database.erase(it);
            std::cout << "LOGINFO:\t\t" << "Insertion to database causes flush " << it->first << " : " << it->second << " to B+ Tree." << std::endl;
        }
    }
    std::pair<std::map<int,int>::iterator,bool> ret;
    ret = this->database.insert ( std::pair<int,int>(key,value) );
    if (ret.second==false) {
        std::cout << "LOG_INFO:\t\t" << key << " is already in the database ( " << ret.first->second << " )" << std::endl;
        if (ret.first->second != value) {
            std::cout << "LOG_INFO:\t\t" << "Updating... " << std::endl;
            ret.first->second = value;
            std::cout << "LOG_INFO:\t\t" << key << " is now ( " << ret.first->second << " )" << std::endl;
        }
        else std::cout << "LOG_INFO:\t\t" << "Same value provided. " << std::endl;
    }
}

std::string Db::get_value_or_blank (int key, Tree* btree) {
    std::string rtn = "";
    if (in_database(key)) {
        std::map<int, int>::iterator it;
        it = this->database.find(key);
    
        if (it != this->database.end()) {
            std::stringstream out;
            out << it->second;
            rtn = out.str();
        } else {
            std::cout << "LOGINFO:\t\t" << "Database bloom filter returns false positive. Searching B+ tree..." << std::endl;
            rtn = btree->get_value_or_blank(key);
        }
    } else {
        std::cout << "LOGINFO:\t\t" << "No match found in database according to database bloom filter. Searching B+ tree..." << std::endl;
        rtn = btree->get_value_or_blank(key);
    }
    return rtn;
}

// We flush all the values in database to btree when we receive range query
std::string Db::range (int lower, int upper, Tree* btree) {
    /**
     * The following code is for range query in database
     * With the third level btree we do not need to perform it 
     ********************
     
    std::string rtn = "";
    std::map<int, int>::iterator itlower, itupper;
    
    if (lower == upper)
        return rtn;
    
    if (this->database.begin()->first >= upper)
        return rtn;
    
    if (this->database.rbegin()->first < lower)
        return rtn;
    
    if (this->database.rbegin()->first == lower) {
        std::stringstream end_first;
        end_first << this->database.rbegin()->first;
        std::stringstream end_second;
        end_second << this->database.rbegin()->second;
        rtn += end_first.str() + ":" + end_second.str();
        return rtn;
    }
    
    if (this->database.begin()->first >= lower) {
        itlower = this->database.begin();
        itupper = this->database.upper_bound(upper);
        if (itupper == this->database.end()) {
            if (this->database.rbegin()->first == upper) {
                itupper--;
            }
        } else {
            if ((itupper--)->first != upper) {
                itupper++;
            }
        }
    }
    if (this->database.begin()->first < lower) {
        itlower = this->database.lower_bound(lower);
        itupper = this->database.upper_bound(upper);
        if (itupper == this->database.end()) {
            if (this->database.rbegin()->first == upper) {
                itupper--;
            }
        } else {
            if ((itupper--)->first != upper) {
                itupper++;
            }
        }
    }
    for (std::map<int, int>::iterator it = itlower; it != itupper; ++it) {
        std::stringstream first_ss;
        first_ss << it->first;
        std::stringstream second_ss;
        second_ss << it->second;
        rtn += first_ss.str() + ":" + second_ss.str() + " ";
    }
    
    return rtn;
     */
    for (std::map<int, int>::iterator it = this->database.begin(); it != this->database.end(); it++) {
        btree->insert_or_update(it->first, it->second);
        std::cout << "LOGINFO:\t\t" << "Range query causes flush " << it->first << " : " << it->second << " to btree." << std::endl;
    }
    this->database.clear();
    std::cout << "LOGINFO:\t\t" << "Clear out database." << std::endl;
    this->database_filter.clear();
    std::cout << "LOGINFO:\t\t" << "Clear out database bloom filter." << std::endl;
    return btree->range(lower, upper);
}

// We delete the key in database and in btree
void Db::delete_key (int key, Tree* btree) {
    if (in_database(key)) {
        std::cout << "LOGINFO:\t\t" << "Remove entry in the database (key is): " << key << std::endl;
        this->database.erase(key);
    }
    std::cout << "LOGINFO:\t\t" << "Remove B+ tree key..." << std::endl;
    btree->delete_key(key);
    return;
}

std::string Db::db_dump () {
    std::string rtn = "";
    
    for (std::map<int, int>::iterator it = this->database.begin(); it != this->database.end(); ++it) {
        std::stringstream first_ss;
        first_ss << it->first;
        std::stringstream second_ss;
        second_ss << it->second;
        rtn += first_ss.str() + ":" + second_ss.str() + ":" + "L2" + " ";
    }
    
    return  rtn;
}

unsigned long Db::get_db_size () {
    return this->database.size();
}

















