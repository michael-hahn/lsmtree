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

class Db {
private:
    std::map<int, int> database;
    
public:
    void insert_or_update (int key, int value);
    
    std::string get_value_or_blank (int key);
    
    std::string range (int lower, int upper);
    
    void delete_key (int key);
    
    std::string db_dump ();
    
    unsigned long get_db_size ();
};

#include "database.cpp"
#endif /* database_hpp */
