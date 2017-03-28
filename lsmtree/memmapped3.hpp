//
//  memmapped3.hpp
//  lsmtree
//
//  Created by Michael Hahn on 3/28/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef memmapped3_hpp
#define memmapped3_hpp

#include <utility>
#include <string>
#include <vector>
#include <map>
#include "bloom_filter.hpp"

#define ESTIMATE_NUMBER_INSERTION 200
#define FALSE_POS_PROB 0.001
#define INCREMENTAL 8
#define INITIAL_PAGE_NUM 16

class Memmapped3 {
private:
    int page_num;
    
    std::vector<std::pair<int, int>> fenses;
    
    std::vector<size_t> elt_size;
    
    int fd;
    
    int cur_array_num;
    
    //a bloom filter for each page
    std::vector<bloom_filter> mm3_filter;
    
    void construct_mm3_filter();
    
    
public:
    Memmapped3(std::string file_path);
    
    bool in_mm3 (int key, int num);
    
    void insert(std::map<int, long> pairs);
    
    std::string get_value_or_blank(int key);
    
    void efficient_range(int lower, int upper, std::map<int, long>& result);
};

#include "memmapped3.cpp"
#endif /* memmapped3_hpp */
