//
//  memmapped3.cpp
//  lsmtree
//
//  Created by Michael Hahn on 3/28/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#include "memmapped3.hpp"
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

//initialize a file for persistant last-level storage
//keep the file open until the whole database is closed unless error occurs
Memmapped3::Memmapped3(std::string file_path){
    this->cur_array_num = 0;
    
    this->page_num = INITIAL_PAGE_NUM;//initialize so tentative
    
    construct_mm3_filter();

    
    int fd, result;
    fd = open(file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
        perror("Error opening a file in this level");
        exit(EXIT_FAILURE);
    }
    this->fd = fd;
    
    result = lseek(this->fd, sysconf(_SC_PAGE_SIZE) * this->page_num - 1, SEEK_SET);
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

void Memmapped3::construct_mm3_filter () {
    bloom_parameters parameters;
    parameters.projected_element_count = ESTIMATE_NUMBER_INSERTION;
    parameters.false_positive_probability = FALSE_POS_PROB;
    
    if (!parameters) {
        std::cout << "LOGFATAL:\t\t" << "Invalid mm3 bloom filter parameters" << std::endl;
        assert(parameters);
    }
    
    parameters.compute_optimal_parameters();
    
    for (int i = 0; i < this->page_num; i++) {
        bloom_filter filter(parameters);
        this->mm3_filter.push_back(filter);
    }
    return;
}

bool Memmapped3::in_mm3 (int key, int num) {
    if (num >= this->page_num) {
        perror("Error using bloom filter");
    }
    if (this->mm3_filter[num].contains(key) > 0) {
        //std::cout << "LOGINFO:\t\t" << "Cache bloom filter contains " << key << std::endl;
        return true;
    } else {
        //std::cout << "LOGINFO:\t\t" << "Cache bloom filter does NOT contain " << key << std::endl;
        return false;
    }
}

void Memmapped3::insert(std::map<int, long> pairs) {
    //when a new page of a file is needed
    if (this->cur_array_num >= this->page_num) {
        this->page_num = this->page_num + INCREMENTAL;
        if (ftruncate(this->fd, sysconf(_SC_PAGE_SIZE) * this->page_num) == -1) {
            perror("Error expanding the file");
            close(this->fd);
            exit(EXIT_FAILURE);
        }
        
        //construct new bloom filters
        bloom_parameters parameters;
        parameters.projected_element_count = ESTIMATE_NUMBER_INSERTION;
        parameters.false_positive_probability = FALSE_POS_PROB;
        
        if (!parameters) {
            std::cout << "LOGFATAL:\t\t" << "Invalid mm3 bloom filter parameters" << std::endl;
            assert(parameters);
        }
        
        parameters.compute_optimal_parameters();
        
        for (int i = 0; i < INCREMENTAL; i++) {
            bloom_filter filter(parameters);
            this->mm3_filter.push_back(filter);
        }
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
        if (!in_mm3(it->first, this->cur_array_num)){
            this->mm3_filter[this->cur_array_num].insert(it->first);
        }
        it++;
    }
    //unmap to free memory
    if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
        perror("Error unmapping the file");
        close(this->fd);
        exit(EXIT_FAILURE);
    }
    
    this->fenses.push_back(std::pair<int, int>(pairs.begin()->first, pairs.rbegin()->first));
    //std::cout << "MM3: fense " << cur_array_num << ": " << pairs.begin()->first << " - " << pairs.rbegin()->first << std::endl;
    this->elt_size.push_back(pairs.size());
    //std::cout << "MM3: Page " << cur_array_num << " size: " << pairs.size() << std::endl;
    this->cur_array_num++;
    return;
}

std::string Memmapped3::get_value_or_blank(int key) {
    std::string rtn = "";
    for (int i = this->cur_array_num - 1; i >= 0; i--) {
        if (key >= this->fenses[i].first && key <= this->fenses[i].second) {
            if (in_mm3(key, i)) {
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
    return rtn;
}

void Memmapped3::efficient_range(int lower, int upper, std::map<int, long>& result) {
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
    return;
}