//
//  main.c
//  lsmtree
//
//  Created by Michael Hahn on 2/14/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <limits>
#include "utils.cpp"
#include "database.hpp"
#include "cache.hpp"
#include "tree.hpp"
#include "memmapped.hpp"
#include "memmapped2.hpp"
#include "memmapped3.hpp"
#include "memmappedL.hpp"
#include "comp.h"

#include <pthread.h>

#define MAX_STDIN_BUFFER_SIZE 1024

#define CHECKED_NEEDED 1

#define EFFICIENT_RANGE 1

#define NUM_THREADS 5

extern int errno;

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

timespec diff(timespec start, timespec end) {
    timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

//initialize cache
Cache cache(MAXCACHESIZE, 0.001);

//initialize second level
Memmapped mm1("/Users/Michael/Documents/Harvard/g1/cs265/xcode/mm1.dat", MAXCACHESIZE * 2, 0.001);

//initialize third level
Memmapped2 mm2("/Users/Michael/Documents/Harvard/g1/cs265/xcode/mm2.dat", MAXCACHESIZE * 8, 0.001);

//initialize fourth level
MemmappedL mml("/Users/Michael/Documents/Harvard/g1/cs265/xcode/mml.dat", MAXCACHESIZE * 64, 0.001);

//write a binary file
//for debugging only
//write_binary_file();

//initialize B+ tree
Tree btree("/Users/Michael/Documents/Harvard/g1/cs265/xcode/tree.dat", MAXCACHESIZE * 128, 0.001);


void* cache_get_value_pthread_wrapper(void* thread_data) {
    return cache.get_value_or_blank_pthread(thread_data);
}

void* memmapped_get_value_pthread_wrapper(void* thread_data) {
    return mm1.get_value_or_blank_pthread(thread_data);
}

void* memmapped2_get_value_pthread_wrapper(void* thread_data) {
    return mm2.get_value_or_blank_pthread(thread_data);
}

void* memmappedl_get_value_pthread_wrapper(void* thread_data) {
    return mml.get_value_or_blank_pthread(thread_data);
}

void* tree_get_value_pthread_wrapper(void* thread_data) {
    return btree.get_value_or_blank_pthread(thread_data);
}

void* cache_range_pthread_wrapper(void* thread_data) {
    return cache.efficient_range_pthread(thread_data);
}

void* memmapped_range_pthread_wrapper(void* thread_data) {
    return mm1.efficient_range_pthread(thread_data);
}

void* memmapped2_range_pthread_wrapper(void* thread_data) {
    return mm2.efficient_range_pthread(thread_data);
}

void* memmappedl_range_pthread_wrapper(void* thread_data) {
    return mml.efficient_range_pthread(thread_data);
}

void* tree_range_pthread_wrapper(void* thread_data) {
    return btree.efficient_range_pthread(thread_data);
}

int main(int argc, const char * argv[]) {
    
    //timer
    timespec timer_start, timer_end;
//    double average_put_nano_time = 0.0;
//    double average_put_sec_time = 0.0;
//    long max_put_nano_time = 0;
//    long max_put_sec_time = 0;
//    int put_counter = 0;

    double average_get_nano_time = 0.0;
    double average_get_sec_time = 0.0;
    long max_get_nano_time = 0;
    long max_get_sec_time = 0;
    int get_counter = 0;
    
//    double average_range_nano_time = 0.0;
//    double average_range_sec_time = 0.0;
//    long max_range_nano_time = 0;
//    long max_range_sec_time = 0;
//    int range_counter = 0;
    
 
    //if the input is from the terminal, output an interactive marker at the beginning of the command
    char* control_message = (char*)"";
    if (isatty(fileno(stdin))) {
        control_message = (char*)"lsmtree_control > ";
    }
    
    //receive a command from the user
    char* command_message = NULL;
    char command_buffer[MAX_STDIN_BUFFER_SIZE];
    int errnum;
    while (printf("%s", control_message),
           command_message = fgets(command_buffer, MAX_STDIN_BUFFER_SIZE, stdin),
           !feof(stdin)) {
        if (command_message == NULL) {
            errnum = errno;
            fprintf(stderr, "fgets failed with error number: %d\n", errno);
            break;
        }
        
        //command must at least be one character long
        if (strlen(command_buffer) > 0) {
            remove_extra_whitespace(command_buffer);
            //process user command and make sure they are well-formed
            char * token;
            token = strtok(command_buffer, " ");
            //fprintf(stdout, "The command is: %s\n", token);
            //the command must be a single character (p, g, r, d, l, or s) with either nothing or a space appended afterwards
            if (token[1] != '\0' && token[1] != '\n')
                fprintf(stderr, "Command is a single character. See usage...\n");
            //process single character command
            else {
                if (strncmp(token, "p", 1) == 0) {
                    //fprintf(stdout, "Put command received...\n");
                    token = strtok(NULL, " ");
                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need key value pair. Please try again...\n");
                    else {
                        char* key = token;
                        if (CHECKED_NEEDED) {
                            if (check_valid_int(key) == -1)
                                fprintf(stderr, "Invalid key value. Please try again...\n");
                            else {
                                int int_key = to_int(key);
                                //fprintf(stdout, "Key: %d received...\n", int_key);
                                token = strtok(NULL, " ");
                                if (token == NULL) fprintf(stderr, "Not enough arguments. Need value. Please try again...\n");
                                else {
                                    char* value = token;
                                    if (check_valid_int(value) == -1)
                                        fprintf(stderr, "Invalid value value. Please try again...\n");
                                    else {
                                        int int_value = to_int(value);
                                        //fprintf(stdout, "Value: %d received...\n", int_value);
                                        token = strtok(NULL, " ");
                                        if (token != NULL) fprintf(stderr, "Too many arguments. Insertion discarded...\n");
                                        else {
                                            //fprintf(stdout, "INSERT key-value pair: %d %d to the database...\n", int_key, int_value);
                                            //TODO: insert to the database here
                                            //database.insert_or_update(int_key, int_value);
//                                            
//                                            clock_serv_t cclock;
//                                            mach_timespec_t mts;
//                                            host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
//                                            clock_get_time(cclock, &mts);
//                                            mach_port_deallocate(mach_task_self(), cclock);
//                                            timer_start.tv_sec = mts.tv_sec;
//                                            timer_start.tv_nsec = mts.tv_nsec;
                                            
                                            cache.insert(int_key, int_value, &mm1, &mm2, &mml, &btree);
                                            
//                                            clock_serv_t cclock2;
//                                            mach_timespec_t mts2;
//                                            host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock2);
//                                            clock_get_time(cclock2, &mts2);
//                                            mach_port_deallocate(mach_task_self(), cclock2);
//                                            timer_end.tv_sec = mts2.tv_sec;
//                                            timer_end.tv_nsec = mts2.tv_nsec;
//                                            
//                                            
//                                            long put_nsec_time = diff(timer_start, timer_end).tv_nsec;
//                                            long put_sec_time = diff(timer_start, timer_end).tv_sec;
//                                            if (put_sec_time > max_put_sec_time) {
//                                                max_put_sec_time = put_sec_time;
//                                                max_put_nano_time = put_nsec_time;
//                                            } else {
//                                                if (put_nsec_time > max_put_nano_time)
//                                                    max_put_nano_time = put_nsec_time;
//                                            }
//                                            put_counter++;
//                                            average_put_nano_time += (put_nsec_time - average_put_nano_time) / put_counter;
//                                            average_put_sec_time += (put_sec_time - average_put_sec_time) / put_counter;

                                        }
                                    }
                                }
                            }
                        } else {
                            int int_key = to_int(key);
                            //fprintf(stdout, "Key: %d received...\n", int_key);
                            token = strtok(NULL, " ");
                            if (token == NULL) fprintf(stderr, "Not enough arguments. Need value. Please try again...\n");
                            else {
                                char* value = token;
                                int int_value = to_int(value);
                                //fprintf(stdout, "Value: %d received...\n", int_value);
                                token = strtok(NULL, " ");
                                if (token != NULL) fprintf(stderr, "Too many arguments. Insertion discarded...\n");
                                else {
                                    //fprintf(stdout, "INSERT key-value pair: %d %d to the database...\n", int_key, int_value);
                                    //TODO: insert to the database here
                                    //database.insert_or_update(int_key, int_value);
                                    cache.insert(int_key, int_value, &mm1, &mm2, &mml, &btree);
                                }
                            }
                        }
                    }
                } else if (strncmp(token, "g", 1) == 0) {
                    //fprintf(stdout, "Get command received...\n");
                    token = strtok(NULL, " ");
                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need key. Please try again...\n");
                    else {
                        char* key = token;
                        if (CHECKED_NEEDED) {
                            if (check_valid_int(key) == -1)
                                fprintf(stderr, "Invalid key value. Please try again...\n");
                            else {
                                int int_key = to_int(key);
                                //fprintf(stdout, "Key: %d received...\n", int_key);
                                token = strtok(NULL, " ");
                                if (token != NULL) fprintf(stderr, "Too many arguments. Retrieval discarded...\n");
                                else {
                                    //fprintf(stdout, "GET value from the key: %d if key is in the cache/database...\n", int_key);
                                    //TODO: get from the database here
                                    //std::cout << "LOGINFO:\t\t" << database.get_value_or_blank(int_key) << std::endl;
                                    
                                    pthread_t threads[NUM_THREADS];
                                    thread_data_get tdg[NUM_THREADS];
                                    for (int i = 0; i < NUM_THREADS; i++) {
                                        tdg[i].key = int_key;
                                        tdg[i].rtn = "";
                                    }
                                    pthread_attr_t attr;
                                    void* status;
                                    int rc;
                                    
                                    pthread_attr_init(&attr);
                                    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
                                    
                                    std::stringstream out_long;
                                    out_long << LONG_MAX;
                                    std::string longmax = out_long.str();
                                    
                    
                                    clock_serv_t cclock;
                                    mach_timespec_t mts;
                                    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
                                    clock_get_time(cclock, &mts);
                                    mach_port_deallocate(mach_task_self(), cclock);
                                    timer_start.tv_sec = mts.tv_sec;
                                    timer_start.tv_nsec = mts.tv_nsec;
                                    
                                    //std::cout << cache.get_value_or_blank(int_key, &mm1, &mm2, &mml, &btree) << std::endl;
                                    rc = pthread_create(&threads[0], &attr, cache_get_value_pthread_wrapper, (void*)&tdg[0]);
                                    if (rc){
                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                        exit(-1);
                                    }
                                    rc = pthread_create(&threads[1], &attr, memmapped_get_value_pthread_wrapper, (void*)&tdg[1]);
                                    if (rc){
                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                        exit(-1);
                                    }
                                    rc = pthread_create(&threads[2], &attr, memmapped2_get_value_pthread_wrapper, (void*)&tdg[2]);
                                    if (rc){
                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                        exit(-1);
                                    }
                                    rc = pthread_create(&threads[3], &attr, memmappedl_get_value_pthread_wrapper, (void*)&tdg[3]);
                                    if (rc){
                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                        exit(-1);
                                    }
                                    rc = pthread_create(&threads[4], &attr, tree_get_value_pthread_wrapper, (void*)&tdg[4]);
                                    if (rc){
                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                        exit(-1);
                                    }
                                    
                                    pthread_attr_destroy(&attr);
                                    for(int i=0; i < NUM_THREADS; i++ ){
                                        rc = pthread_join(threads[i], &status);
                                        
                                        if (rc){
                                            std::cout << "Error:unable to join," << rc << std::endl;
                                            exit(-1);
                                        }
                                    }
                                    
                                    
                                    if (tdg[0].rtn != "") {
                                        if (tdg[0].rtn != longmax)
                                            std::cout << tdg[0].rtn << std::endl;
                                        else
                                            std::cout << "" << std::endl;
                                    } else {
                                        if (tdg[1].rtn != "") {
                                            if (tdg[1].rtn != longmax)
                                                std::cout << tdg[1].rtn << std::endl;
                                            else
                                                std::cout << "" << std::endl;
                                        } else {
                                            if (tdg[2].rtn != "") {
                                                if (tdg[2].rtn != longmax)
                                                    std::cout << tdg[2].rtn << std::endl;
                                                else
                                                    std::cout << "" << std::endl;
                                            } else {
                                                if (tdg[3].rtn != "") {
                                                    if (tdg[3].rtn != longmax)
                                                        std::cout << tdg[3].rtn << std::endl;
                                                    else
                                                        std::cout << "" << std::endl;
                                                } else {
                                                    if (tdg[4].rtn != "") {
                                                        if (tdg[4].rtn != longmax)
                                                            std::cout << tdg[4].rtn << std::endl;
                                                        else
                                                            std::cout << "" << std::endl;
                                                    } else {
                                                        std::cout << "" << std::endl;
                                                    }
                                                }
                                            }
                                        }
                                    }
#ifdef SYNC
                                    get_stop = false;
#endif
                                    
                                    clock_serv_t cclock2;
                                    mach_timespec_t mts2;
                                    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock2);
                                    clock_get_time(cclock2, &mts2);
                                    mach_port_deallocate(mach_task_self(), cclock2);
                                    timer_end.tv_sec = mts2.tv_sec;
                                    timer_end.tv_nsec = mts2.tv_nsec;
                                    
                                    
                                    long get_nsec_time = diff(timer_start, timer_end).tv_nsec;
                                    long get_sec_time = diff(timer_start, timer_end).tv_sec;
                                    if (get_sec_time > max_get_sec_time) {
                                        max_get_sec_time = get_sec_time;
                                        max_get_nano_time = get_nsec_time;
                                    } else {
                                        if (get_nsec_time > max_get_nano_time)
                                            max_get_nano_time = get_nsec_time;
                                    }
                                    get_counter++;
                                    average_get_nano_time += (get_nsec_time - average_get_nano_time) / get_counter;
                                    average_get_sec_time += (get_sec_time - average_get_sec_time) / get_counter;
                                }
                            }
                        } else {
                            int int_key = to_int(key);
                            //fprintf(stdout, "Key: %d received...\n", int_key);
                            token = strtok(NULL, " ");
                            if (token != NULL) fprintf(stderr, "Too many arguments. Retrieval discarded...\n");
                            else {
                                //fprintf(stdout, "GET value from the key: %d if key is in the database...\n", int_key);
                                //TODO: get from the database here
                                //std::cout << "LOGINFO:\t\t" << database.get_value_or_blank(int_key) << std::endl;
                                std::cout << cache.get_value_or_blank(int_key, &mm1, &mm2, &mml, &btree) << std::endl;
                            }
                        }
                    }
                } else if (strncmp(token, "r", 1) == 0) {
                    //fprintf(stdout, "Range command received...\n");
                    token = strtok(NULL, " ");
                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need two key ranges. Please try again...\n");
                    else {
                        char* from = token;
                        if (CHECKED_NEEDED) {
                            if (check_valid_int(from) == -1)
                                fprintf(stderr, "Invalid from value. Please try again...\n");
                            else {
                                int int_from = to_int(from);
                                //fprintf(stdout, "From range: %d received...\n", int_from);
                                token = strtok(NULL, " ");
                                if (token == NULL) fprintf(stderr, "Not enough arguments. Need to range. Please try again...\n");
                                else {
                                    char* to = token;
                                    if (check_valid_int(to) == -1)
                                        fprintf(stderr, "Invalid to value. Please try again...\n");
                                    else {
                                        int int_to = to_int(to);
                                        //fprintf(stdout, "To range: %d received...\n", int_to);
                                        token = strtok(NULL, " ");
                                        if (token != NULL) fprintf(stderr, "Too many arguments. Range request discarded...\n");
                                        else {
                                            if (int_from <= int_to) {
                                                //fprintf(stdout, "GET values from key range: %d - %d in the database...\n", int_from, int_to);
                                                //TODO: get from the database here
                                                //std::cout << "LOGINFO:\t\t" << database.range(int_from, int_to) << std::endl;
                                                if (EFFICIENT_RANGE) {
                                                    pthread_t threads[NUM_THREADS];
                                                    thread_data_range tdr[NUM_THREADS];
                                                    for (int i = 0; i < NUM_THREADS; i++) {
                                                        tdr[i].lower = int_from;
                                                        tdr[i].upper = int_to;
                                                    }
                                                    pthread_attr_t attr;
                                                    void* status;
                                                    int rc;
                                                    
                                                    pthread_attr_init(&attr);
                                                    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
                                                    
//                                                    clock_serv_t cclock;
//                                                    mach_timespec_t mts;
//                                                    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
//                                                    clock_get_time(cclock, &mts);
//                                                    mach_port_deallocate(mach_task_self(), cclock);
//                                                    timer_start.tv_sec = mts.tv_sec;
//                                                    timer_start.tv_nsec = mts.tv_nsec;

                                                    
                                                    rc = pthread_create(&threads[0], &attr, cache_range_pthread_wrapper, (void*)&tdr[0]);
                                                    if (rc){
                                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                                        exit(-1);
                                                    }
                                                    rc = pthread_create(&threads[1], &attr, memmapped_range_pthread_wrapper, (void*)&tdr[1]);
                                                    if (rc){
                                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                                        exit(-1);
                                                    }
                                                    rc = pthread_create(&threads[2], &attr, memmapped2_range_pthread_wrapper, (void*)&tdr[2]);
                                                    if (rc){
                                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                                        exit(-1);
                                                    }
                                                    rc = pthread_create(&threads[3], &attr, memmappedl_range_pthread_wrapper, (void*)&tdr[3]);
                                                    if (rc){
                                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                                        exit(-1);
                                                    }
                                                    rc = pthread_create(&threads[4], &attr, tree_range_pthread_wrapper, (void*)&tdr[4]);
                                                    if (rc){
                                                        std::cout << "Error:unable to create thread," << rc << std::endl;
                                                        exit(-1);
                                                    }
                                                    
                                                    pthread_attr_destroy(&attr);
                                                    for(int i=0; i < NUM_THREADS; i++ ){
                                                        rc = pthread_join(threads[i], &status);
                                                        
                                                        if (rc){
                                                            std::cout << "Error:unable to join," << rc << std::endl;
                                                            exit(-1);
                                                        }
                                                    }

                                                    std::map<int, long> map_to_print;
                                                    for (int i = 0; i < NUM_THREADS; i++) {
                                                        for (std::map<int, long>::iterator it = tdr[i].result.begin(); it != tdr[i].result.end(); it++) {
                                                            map_to_print.insert(*it);
                                                        }
                                                    }
                                                    
//                                                    clock_serv_t cclock2;
//                                                    mach_timespec_t mts2;
//                                                    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock2);
//                                                    clock_get_time(cclock2, &mts2);
//                                                    mach_port_deallocate(mach_task_self(), cclock2);
//                                                    timer_end.tv_sec = mts2.tv_sec;
//                                                    timer_end.tv_nsec = mts2.tv_nsec;
//                                                    
//                                                    long range_nsec_time = diff(timer_start, timer_end).tv_nsec;
//                                                    long range_sec_time = diff(timer_start, timer_end).tv_sec;
//                                                    if (range_sec_time > max_range_sec_time) {
//                                                        max_range_sec_time = range_sec_time;
//                                                        max_range_nano_time = range_nsec_time;
//                                                    } else {
//                                                        if (range_nsec_time > max_range_nano_time)
//                                                            max_range_nano_time = range_nsec_time;
//                                                    }
//                                                    range_counter++;
//                                                    average_range_nano_time += (range_nsec_time - average_range_nano_time) / range_counter;
//                                                    average_range_sec_time += (range_sec_time - average_range_sec_time) / range_counter;


                                                    
//                                                    cache.efficient_range(int_from, int_to, &mm1, &mm2, &mml, &btree, map_to_print);
                                                    for (std::map<int, long>::iterator it = map_to_print.begin(); it != map_to_print.end(); it++) {
                                                        if (it->second != LONG_MAX) {
                                                            std::cout << it->first << ":" << it->second << " ";
                                                        }
                                                    }
                                                    std::cout << std::endl;
                                                } else
                                                    ;
                                            } else fprintf(stderr, "From value must be smaller than or equal to to value. Range request discarded...\n");
                                        }
                                    }
                                }
                            }
                        } else {
                            int int_from = to_int(from);
                            //fprintf(stdout, "From range: %d received...\n", int_from);
                            token = strtok(NULL, " ");
                            if (token == NULL) fprintf(stderr, "Not enough arguments. Need to range. Please try again...\n");
                            else {
                                char* to = token;
                                int int_to = to_int(to);
                                //fprintf(stdout, "To range: %d received...\n", int_to);
                                token = strtok(NULL, " ");
                                if (token != NULL) fprintf(stderr, "Too many arguments. Range request discarded...\n");
                                else {
                                    if (int_from <= int_to) {
                                        //fprintf(stdout, "GET values from key range: %d - %d in the database...\n", int_from, int_to);
                                        //TODO: get from the database here
                                        //std::cout << "LOGINFO:\t\t" << database.range(int_from, int_to) << std::endl;
                                        if (EFFICIENT_RANGE) {
                                            std::map<int, long> map_to_print;
                                            cache.efficient_range(int_from, int_to, &mm1, &mm2, &mml, &btree, map_to_print);
                                            for (std::map<int, long>::iterator it = map_to_print.begin(); it != map_to_print.end(); it++) {
                                                if (it->second != LONG_MAX) {
                                                    std::cout << it->first << ":" << it->second << " ";
                                                }
                                            }
                                            std::cout << std::endl;
                                        } else
                                            ;
                                    } else fprintf(stderr, "From value must be smaller than or equal to to value. Range request discarded...\n");
                                }
                            }
                        }
                    }
                } else if (strncmp(token, "d", 1) == 0) {
                    //fprintf(stdout, "Delete command received...\n");
                    token = strtok(NULL, " ");
                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need key. Please try again...\n");
                    else {
                        char* key = token;
                        if (CHECKED_NEEDED) {
                            if (check_valid_int(key) == -1)
                                fprintf(stderr, "Invalid key value. Please try again...\n");
                            else {
                                int int_key = to_int(key);
                                //fprintf(stdout, "Key: %d received...\n", int_key);
                                token = strtok(NULL, " ");
                                if (token != NULL) fprintf(stderr, "Too many arguments. Deletion discarded...\n");
                                else {
                                    //fprintf(stdout, "DELETE value from the key: %d if key is in the database...\n", int_key);
                                    //TODO: delete from the database here
                                    //database.delete_key(int_key);
                                    cache.delete_key(int_key, &mm1, &mm2, &mml, &btree);
                                }
                            }
                        } else {
                            int int_key = to_int(key);
                            //fprintf(stdout, "Key: %d received...\n", int_key);
                            token = strtok(NULL, " ");
                            if (token != NULL) fprintf(stderr, "Too many arguments. Deletion discarded...\n");
                            else {
                                //fprintf(stdout, "DELETE value from the key: %d if key is in the database...\n", int_key);
                                //TODO: delete from the database here
                                //database.delete_key(int_key);
                                cache.delete_key(int_key, &mm1, &mm2, &mml, &btree);
                            }
                        }
                    }
                } else if (strncmp(token, "l", 1) == 0) {
                    //fprintf(stdout, "Load command received...\n");
                    token = strtok(NULL, " ");
                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need file path. Please try again...\n");
                    else {
                        char* path = token;
                        //fprintf(stdout, "File path: %s received...\n", path);
                        token = strtok(NULL, " ");
                        if (token != NULL) fprintf(stderr, "Too many arguments. File load discarded...\n");
                        else {
                            unsigned long path_length = strlen(path);
                            std::string path_str(path);
                            if (path_str[path_length - 1] == '\n') path_str[path_length - 1] = '\0';
                            path_str.erase(std::remove(path_str.begin(), path_str.end(), '"'), path_str.end());
                            //std::cout << "LOGINFO:\t\t" << "LOAD file from " << path_str << " to the database...\n" << std::endl;
                            //TODO: load file to the database here
                            read_binary_file(path_str, &cache, &mm1, &mm2, &mml, &btree);
                        }
                    }
                } else if (strncmp(token, "s", 1) == 0) {
                    //fprintf(stdout, "Print status command received...\n");
                    token = strtok(NULL, " ");
                    if (token != NULL) fprintf(stderr, "Too many arguments. Print status discarded...\n");
                    else {
                        //fprintf(stdout, "STATUS is retrieving from the database...\n");
                        //TODO: print status here
                        std::set<std::pair<int, bool>, set_compare> total_pairs;
                        std::pair<std::string, int> cache_result = cache.cache_dump(total_pairs);
                        std::pair<std::string, int> mm1_result = mm1.mm1_dump(total_pairs);
                        std::pair<std::string, int> mm2_result = mm2.mm2_dump(total_pairs);
                        std::pair<std::string, int> mml_result = mml.mml_dump(total_pairs);
                        std::pair<unsigned long, std::string> tree_result = btree.tree_dump(total_pairs);
                        int unique_count = 0;
                        for (std::set<std::pair<int, bool>, set_compare>::iterator it = total_pairs.begin(); it != total_pairs.end(); it++) {
                            if (it->second)
                                unique_count++;
                        }
                        std::cout << "Total Pairs: " << unique_count << std::endl;
                        std::cout << "LVL1: "<< cache_result.second << std::endl;
                        std::cout << cache_result.first << std::endl;
                        std::cout << "LVL2: "<< mm1_result.second << std::endl;
                        std::cout << mm1_result.first << std::endl;
                        std::cout << "LVL3: "<< mm2_result.second << std::endl;
                        std::cout << mm2_result.first << std::endl;
                        std::cout << "LVL4: "<< mml_result.second << std::endl;
                        std::cout << mml_result.first << std::endl;
                        std::cout << "LVL5: "<< tree_result.first << std::endl;
                        std::cout << tree_result.second << std::endl;
                    }
                } else if (strncmp(token, "q", 1) == 0) {
                    break;
                } else {
                    fprintf(stderr, "Invalid command received. Please try again...\n");
                }
            }
        }
    }
//    std::cout << "Maximum put time (seconds): " << max_put_sec_time << std::endl;
//    std::cout << "Average put time (seconds): " << average_put_sec_time << std::endl;
//    std::cout << "Maximum put time (nanoseconds): " << max_put_nano_time << std::endl;
//    std::cout << "Average put time (nanoseconds): " << average_put_nano_time << std::endl;
    
    std::cout << "Maximum get time (seconds): " << max_get_sec_time << std::endl;
    std::cout << "Average get time (seconds): " << average_get_sec_time << std::endl;
    std::cout << "Maximum get time (nanoseconds): " << max_get_nano_time << std::endl;
    std::cout << "Average get time (nanoseconds): " << average_get_nano_time << std::endl;
   
//    std::cout << "Maximum range time (seconds): " << max_range_sec_time << std::endl;
//    std::cout << "Average range time (seconds): " << average_range_sec_time << std::endl;
//    std::cout << "Maximum range time (nanoseconds): " << max_range_nano_time << std::endl;
//    std::cout << "Average range time (nanoseconds): " << average_range_nano_time << std::endl;

    
    mm1.free_mem();
    mm2.free_mem();
    mml.free_mem();
    btree.free_mem();
    
    return 0;
}
