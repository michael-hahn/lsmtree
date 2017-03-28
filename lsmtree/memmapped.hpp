//
//  memmapped.hpp
//  lsmtree
//
//  Created by Michael Hahn on 3/27/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef memmapped_hpp
#define memmapped_hpp
#include <utility>
#include <string>
#include <vector>
#include <map>
#include <string>
#include "memmapped2.hpp"
#include "memmapped3.hpp"
#include "bloom_filter.hpp"

#define ARRAY_NUM 2
#define FILESIZE (sysconf(_SC_PAGE_SIZE) * ARRAY_NUM)

class Memmapped {
private:
    std::pair<int, int> fenses[ARRAY_NUM];
    
    size_t elt_size[ARRAY_NUM];
    
    int fd;
    
    int cur_array_num;
    
    //a bloom filter for each page
    bloom_filter mm1_filter[ARRAY_NUM];
    
    void construct_mm1_filter (int estimate_number_insertion, double false_pos_prob);
    
public:
    Memmapped(std::string file_path, int estimate_number_insertion, double false_pos_prob);
    
    bool in_mm1 (int key, int num);
    
    std::map<int, long> consolidate();
    
    void insert(std::vector<std::pair<int, long>> cache, Memmapped2* mm2, Memmapped3* mm3);
    
    std::string get_value_or_blank(int key, Memmapped2* mm2, Memmapped3* mm3);
    
    void efficient_range(int lower, int upper, Memmapped2* mm2, Memmapped3* mm, std::map<int, long>& result);
};



#include "memmapped.cpp"
#endif /* memmapped_hpp */
