//
//  memmapped2.cpp
//  lsmtree
//
//  Created by Michael Hahn on 3/27/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#include "memmapped2.hpp"
#include "memmapped.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cache.hpp"

//initialize a file for persistant second-level storage
//keep the file open until the whole database is closed unless error occurs
Memmapped2::Memmapped2(std::string file_path, int estimate_number_insertion, double false_pos_prob){
    construct_mm2_filter(estimate_number_insertion, false_pos_prob);
    
    this->cur_array_num = 0;
    
    int fd, result;
    fd = open(file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
        perror("Error opening a file in this level");
        exit(EXIT_FAILURE);
    }
    this->fd = fd;
    
    result = lseek(this->fd, FILESIZE_2 - 1, SEEK_SET);
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
}

void Memmapped2::construct_mm2_filter (int estimate_number_insertion, double false_pos_prob) {
    bloom_parameters parameters;
    parameters.projected_element_count = estimate_number_insertion;
    parameters.false_positive_probability = false_pos_prob;
    
    if (!parameters) {
        std::cout << "LOGFATAL:\t\t" << "Invalid mm2 bloom filter parameters" << std::endl;
        assert(parameters);
    }
    
    parameters.compute_optimal_parameters();
    
    for (int i = 0; i < ARRAY_NUM_2; i++) {
        bloom_filter filter(parameters);
        this->mm2_filter[i] = filter;
    }
    return;
}

bool Memmapped2::in_mm2 (int key, int num) {
    if (num >= ARRAY_NUM_2) {
        perror("Error using bloom filter");
    }
    if (this->mm2_filter[num].contains(key) > 0) {
        //std::cout << "LOGINFO:\t\t" << "Cache bloom filter contains " << key << std::endl;
        return true;
    } else {
        //std::cout << "LOGINFO:\t\t" << "Cache bloom filter does NOT contain " << key << std::endl;
        return false;
    }
}

//same functionality as sanitize function in first-level
//consolidate all pages, only keep the most updated value of each key
//including deletion keys
std::map<int, long> Memmapped2::consolidate(){
    std::map<int, long> rtn;
    for (int i = ARRAY_NUM_2 - 1; i >= 0; i--) {
        std::pair<int, long>* map = (std::pair<int, long>*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, sysconf(_SC_PAGE_SIZE) * i);
        if (map == MAP_FAILED) {
            close(this->fd);
            perror("Error mapping the file for consolidation");
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < this->elt_size[i]; j++) {
            rtn.insert(map[j]);
        }
        if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
            perror("Error unmapping the file");
            close(this->fd);
            exit(EXIT_FAILURE);
        }
    }
    return rtn;
}

void Memmapped2::insert(std::map<int, long> pairs, Memmapped3* mm3) {
    //when flushing to the next level is needed
    if (this->cur_array_num >= ARRAY_NUM_2) {
        this->cur_array_num = 0;
        for (int i = 0; i < ARRAY_NUM_2; i++) {
            this->mm2_filter[i].clear();
        }
        std::map<int, long> consolidated = this->consolidate();
        mm3->insert(consolidated);
    }
    std::pair<int, long>* map = (std::pair<int, long>*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, sysconf(_SC_PAGE_SIZE) * this->cur_array_num);
    if (map == MAP_FAILED) {
        close(this->fd);
        perror("Error mmapping the file for insertion");
        exit(EXIT_FAILURE);
    }
    
    std::map<int, long>::iterator it = pairs.begin();
    for (int i = 0; i < pairs.size(); i++) {
        map[i] = std::pair<int, long>(it->first, it->second);
        if (!in_mm2(it->first, this->cur_array_num)){
            this->mm2_filter[this->cur_array_num].insert(it->first);
        }
        it++;
    }
    //unmap to free memory
    if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
        perror("Error unmapping the file");
        close(this->fd);
        exit(EXIT_FAILURE);
    }
    
    this->fenses[cur_array_num] = std::pair<int, int>(pairs.begin()->first, pairs.rbegin()->first);
    //std::cout << "MM2: fense " << cur_array_num << ": " << pairs.begin()->first << " - " << pairs.rbegin()->first << std::endl;
    this->elt_size[cur_array_num] = pairs.size();
    //std::cout << "MM2: Page " << cur_array_num << " size: " << pairs.size() << std::endl;
    this->cur_array_num++;
    return;
}

std::string Memmapped2::get_value_or_blank(int key, Memmapped3* mm3) {
    std::string rtn = "";
    for (int i = this->cur_array_num - 1; i >= 0; i--) {
        if (key >= this->fenses[i].first && key <= this->fenses[i].second) {
            if (in_mm2(key, i)) {
                std::pair<int, long>* map = (std::pair<int, long>*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ, MAP_SHARED, this->fd, sysconf(_SC_PAGE_SIZE) * i);
                if (map == MAP_FAILED) {
                    close(this->fd);
                    perror("Error mmapping the file for insertion");
                    exit(EXIT_FAILURE);
                }
                size_t left = 0;
                size_t right = this->elt_size[i] - 1;
                size_t mid;
                while (left <= right) {
                    mid = (left + right) / 2;
                    if (map[mid].first == key) {
                        if (map[mid].second == LONG_MAX) {
                            if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
                                perror("Error unmapping the file");
                                close(this->fd);
                                exit(EXIT_FAILURE);
                            }
                            return "";
                        } else {
                            std::stringstream out;
                            out << map[mid].second;
                            rtn = out.str();
                            break;
                        }
                    } else if (key > map[mid].first) {
                        left = mid + 1;
                    } else {
                        right = mid - 1;
                    }
                }
                if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
                    perror("Error unmapping the file");
                    close(this->fd);
                    exit(EXIT_FAILURE);
                }
                if (rtn != "")
                    break;
            }
        }
    }
    if (rtn == "")
        rtn = mm3->get_value_or_blank(key);
    return rtn;
}

void Memmapped2::efficient_range(int lower, int upper, Memmapped3* mm3, std::map<int, long>& result) {
    for (int i = this->cur_array_num - 1; i >= 0; i--) {
        if (lower > this->fenses[i].second || upper <= this->fenses[i].first)
            ;
        else {
            std::pair<int, long>* map = (std::pair<int, long>*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ, MAP_SHARED, this->fd, sysconf(_SC_PAGE_SIZE) * i);
            if (map == MAP_FAILED) {
                close(this->fd);
                perror("Error mmapping the file for insertion");
                exit(EXIT_FAILURE);
            }
            for (int j = 0; j < this->elt_size[i]; j++) {
                if (map[j].first >= lower && map[j].first < upper) {
                    result.insert(map[j]);
                }
            }
            if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
                perror("Error unmapping the file");
                close(this->fd);
                exit(EXIT_FAILURE);
            }
        }
    }
    mm3->efficient_range(lower, upper, result);
    return;
}
