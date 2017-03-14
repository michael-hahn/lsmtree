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



void Db::insert_or_update (int key, int value) {
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

std::string Db::get_value_or_blank (int key) {
    std::string rtn = "";
    std::map<int, int>::iterator it;
    it = this->database.find(key);
    
    if (it != this->database.end()) {
        std::stringstream out;
        out << it->second;
        rtn = out.str();
    }
    
    return rtn;
}

std::string Db::range (int lower, int upper) {
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
}

void Db::delete_key (int key) {
    this->database.erase(key);
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

















