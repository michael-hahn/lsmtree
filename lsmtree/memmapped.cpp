//
//  memmapped.cpp
//  lsmtree
//
//  Created by Michael Hahn on 3/27/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#include "memmapped.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cache.hpp"
#include <stdio.h>
#include <cassert>
#include <sstream>
#include <limits>


//initialize a file for persistant second-level storage
//keep the file open until the whole database is closed unless error occurs
Memmapped::Memmapped(std::string file_path, int estimate_number_insertion, double false_pos_prob){
    construct_mm1_filter(estimate_number_insertion, false_pos_prob);
    
    this->cur_array_num = 0;
    
    int fd, result;
    fd = open(file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
        perror("Error opening a file in this level");
        exit(EXIT_FAILURE);
    }
    this->fd = fd;
    
    result = lseek(this->fd, FILESIZE - 1, SEEK_SET);
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
    
    //memory map each page
    for (int i = 0; i < ARRAY_NUM; i++) {
        std::pair<int, long>* map = (std::pair<int, long>*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, sysconf(_SC_PAGE_SIZE) * i);
        if (map == MAP_FAILED) {
            close(this->fd);
            perror("Error mapping the file for consolidation");
            exit(EXIT_FAILURE);
        }
        this->mapped_addr[i] = map;
    }
}

void Memmapped::construct_mm1_filter (int estimate_number_insertion, double false_pos_prob) {
    bloom_parameters parameters;
    parameters.projected_element_count = estimate_number_insertion;
    parameters.false_positive_probability = false_pos_prob;
    
    if (!parameters) {
        std::cout << "LOGFATAL:\t\t" << "Invalid mm1 bloom filter parameters" << std::endl;
        assert(parameters);
    }
    
    parameters.compute_optimal_parameters();
    
    for (int i = 0; i < ARRAY_NUM; i++) {
        bloom_filter filter(parameters);
        this->mm1_filter[i] = filter;
    }
    return;
}

bool Memmapped::in_mm1 (int key, int num) {
    if (num >= ARRAY_NUM) {
        perror("Error using bloom filter");
    }
    if (this->mm1_filter[num].contains(key) > 0) {
        //std::cout << "LOGINFO:\t\t" << "Cache bloom filter contains " << key << std::endl;
        return true;
    } else {
        //std::cout << "LOGINFO:\t\t" << "Cache bloom filter does NOT contain " << key << std::endl;
        return false;
    }
}

void Memmapped::free_mem() {
    for (int i = 0; i < ARRAY_NUM; i++) {
        if (munmap(this->mapped_addr[i], sysconf(_SC_PAGE_SIZE)) == -1) {
            perror("Error unmapping the file");
            close(this->fd);
            exit(EXIT_FAILURE);
        }
    }
    close(this->fd);
    return;
}

//same functionality as sanitize function in first-level
//consolidate all pages, only keep the most updated value of each key
//including deletion keys
std::map<int, long> Memmapped::consolidate(){
    std::map<int, long> rtn;
    for (int i = ARRAY_NUM - 1; i >= 0; i--) {
        std::pair<int, long>* map = this->mapped_addr[i];
        for (int j = 0; j < this->elt_size[i]; j++) {
            rtn.insert(map[j]);
        }
    }
    return rtn;
}
void Memmapped::insert(std::vector<std::pair<int, long>> cache, Memmapped2* mm2, MemmappedL* mml, Tree* tree) {
    //when flushing to the next level is needed
    if (this->cur_array_num >= ARRAY_NUM) {
        this->cur_array_num = 0;
        for (int i = 0; i < ARRAY_NUM; i++) {
            this->mm1_filter[i].clear();
        }
        std::map<int, long> consolidated = this->consolidate();
        mm2->insert(consolidated, mml, tree);
    }
    std::pair<int, long>* map = this->mapped_addr[this->cur_array_num];
    for (int i = 0; i < cache.size(); i++) {
        map[i] = cache[i];
        if (!in_mm1(cache[i].first, this->cur_array_num)){
            this->mm1_filter[this->cur_array_num].insert(cache[i].first);
        }
    }
    this->fenses[cur_array_num] = std::pair<int, int>(cache[0].first, cache[cache.size() - 1].first);
    //std::cout << "MM1: fense " << cur_array_num << ": " << cache[0].first << " - " << cache[cache.size() - 1].first << std::endl;
    this->elt_size[cur_array_num] = cache.size();
    //std::cout << "MM1: Page " << cur_array_num << " size: " << cache.size() << std::endl;
    this->cur_array_num++;
    return;
}

std::string Memmapped::get_value_or_blank(int key, Memmapped2* mm2, MemmappedL* mml, Tree* tree) {
    std::string rtn = "";
    for (int i = this->cur_array_num - 1; i >= 0; i--) {
        if (key >= this->fenses[i].first && key <= this->fenses[i].second) {
            if (in_mm1(key, i)) {
                std::pair<int, long>* map = this->mapped_addr[i];
                size_t left = 0;
                size_t right = this->elt_size[i] - 1;
                size_t mid;
                while (left <= right) {
                    mid = (left + right) / 2;
                    if (map[mid].first == key) {
                        if (map[mid].second == LONG_MAX) {
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
                if (rtn != "")
                    break;
            }
        }
    }
    if (rtn == "")
        rtn = mm2->get_value_or_blank(key, mml, tree);
    return rtn;
}

void* Memmapped::get_value_or_blank_pthread (void* thread_data) {
    std::string rtn = "";
    thread_data_get* search_key = (thread_data_get*) thread_data;
    for (int i = this->cur_array_num - 1; i >= 0; i--) {
#ifdef SYNC
        if (get_stop){
            pthread_exit(NULL);
        }
#endif
        if (search_key->key >= this->fenses[i].first && search_key->key <= this->fenses[i].second) {
            if (in_mm1(search_key->key, i)) {
                std::pair<int, long>* map = this->mapped_addr[i];
                size_t left = 0;
                size_t right = this->elt_size[i] - 1;
                size_t mid;
                while (left <= right) {
#ifdef SYNC
                    if (get_stop){
                        pthread_exit(NULL);
                    }
#endif
                    mid = (left + right) / 2;
                    if (map[mid].first == search_key->key) {
#ifdef SYNC
                        get_stop = true;
#endif
                        if (map[mid].second == LONG_MAX) {
                            std::stringstream out_long;
                            out_long << LONG_MAX;
                            rtn = out_long.str();
                            search_key->rtn = rtn;
                            pthread_exit(NULL);
                        } else {
                            std::stringstream out;
                            out << map[mid].second;
                            rtn = out.str();
                            break;
                        }
                    } else if (search_key->key > map[mid].first) {
                        left = mid + 1;
                    } else {
                        right = mid - 1;
                    }
                }
                if (rtn != "")
                    break;
            }
        }
    }
    search_key->rtn = rtn;
    pthread_exit(NULL);
}

void Memmapped::efficient_range(int lower, int upper, Memmapped2* mm2, MemmappedL* mml, Tree* tree, std::map<int, long>& result) {
    for (int i = this->cur_array_num - 1; i >= 0; i--) {
        if (lower > this->fenses[i].second || upper <= this->fenses[i].first)
            ;
        else {
            std::pair<int, long>* map = this->mapped_addr[i];
            for (int j = 0; j < this->elt_size[i]; j++) {
                if (map[j].first >= lower && map[j].first < upper) {
                    result.insert(map[j]);
                }
            }
        }
    }
    mm2->efficient_range(lower, upper, mml, tree, result);
    return;
}

void* Memmapped::efficient_range_pthread (void* thread_data) {
    thread_data_range* search_key = (thread_data_range*) thread_data;
    for (int i = this->cur_array_num - 1; i >= 0; i--) {
        if (search_key->lower > this->fenses[i].second || search_key->upper <= this->fenses[i].first)
            ;
        else {
            std::pair<int, long>* map = this->mapped_addr[i];
            for (int j = 0; j < this->elt_size[i]; j++) {
                if (map[j].first >= search_key->lower && map[j].first < search_key->upper) {
                    search_key->result.insert(map[j]);
                }
            }
        }
    }
    pthread_exit(NULL);
}

std::pair<std::string, int> Memmapped::mm1_dump (std::set<std::pair<int, bool>, set_compare>& found_once) {
    int total_valid = 0;
    std::string rtn = "";
    std::pair<std::set<std::pair<int, bool>, set_compare>::iterator, bool> set_rtn;
    
    for (int i = this->cur_array_num - 1; i >= 0; i--) {
        std::pair<int, long>* map = this->mapped_addr[i];
        for (int j = 0; j < this->elt_size[i]; j++) {
            if (map[j].second == LONG_MAX) {
                set_rtn = found_once.insert(std::pair<int, bool>(map[j].first, false));
            } else {
                set_rtn = found_once.insert(std::pair<int, bool>(map[j].first, true));
            }
            if (set_rtn.second) {
                if (map[j].second != LONG_MAX) {
                    std::stringstream first_ss;
                    first_ss << map[j].first;
                    std::stringstream second_ss;
                    second_ss << map[j].second;
                    rtn += first_ss.str() + ":" + second_ss.str() + ":" + "L1" + " ";
                    total_valid++;
                }
            }
        }
    }
    return std::pair<std::string, int> (rtn, total_valid);
}
