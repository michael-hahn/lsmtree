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
#include "utils.cpp"
#include "database.hpp"
#include "cache.hpp"
#include "tree.hpp"

#define MAX_STDIN_BUFFER_SIZE 1024

#define CHECKED_NEEDED 1

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

int main(int argc, const char * argv[]) {
    
    //initialize cache
    Cache cache(20, 0.001);

    //initialize database
    Db database(200, 0.001);
    
    //write a binary file
    //for debugging only
    //write_binary_file();
    
    //initialize B+ tree
    Tree btree(100000000, 0.01);
    
    //timer
    timespec timer_start, timer_end;
//    double average_insert_nano_time = 0.0;
//    long max_insert_nano_time = 0;
//    int insert_counter = 0;
    
    double average_get_nano_time = 0.0;
    long max_get_nano_time = 0;
    int get_counter = 0;
    
 
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
                                            
//                                            clock_serv_t cclock;
//                                            mach_timespec_t mts;
//                                            host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
//                                            clock_get_time(cclock, &mts);
//                                            mach_port_deallocate(mach_task_self(), cclock);
//                                            timer_start.tv_sec = mts.tv_sec;
//                                            timer_start.tv_nsec = mts.tv_nsec;
                                            
                                            cache.insert(int_key, int_value, &database, &btree);
                                            
//                                            clock_serv_t cclock2;
//                                            mach_timespec_t mts2;
//                                            host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock2);
//                                            clock_get_time(cclock2, &mts2);
//                                            mach_port_deallocate(mach_task_self(), cclock2);
//                                            timer_end.tv_sec = mts2.tv_sec;
//                                            timer_end.tv_nsec = mts2.tv_nsec;
//                                            
//                                            long insert_time = diff(timer_start, timer_end).tv_nsec;
//                                            if (insert_time > max_insert_nano_time)
//                                                max_insert_nano_time = insert_time;
//                                            insert_counter++;
//                                            average_insert_nano_time += (insert_time - average_insert_nano_time) / insert_counter;
//                                            
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
                                    cache.insert(int_key, int_value, &database, &btree);
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
                                    
                                    clock_serv_t cclock;
                                    mach_timespec_t mts;
                                    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
                                    clock_get_time(cclock, &mts);
                                    mach_port_deallocate(mach_task_self(), cclock);
                                    timer_start.tv_sec = mts.tv_sec;
                                    timer_start.tv_nsec = mts.tv_nsec;
                                    
                                    std::cout << cache.get_value_or_blank(int_key, &database, &btree) << std::endl;
                                    
                                    clock_serv_t cclock2;
                                    mach_timespec_t mts2;
                                    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock2);
                                    clock_get_time(cclock2, &mts2);
                                    mach_port_deallocate(mach_task_self(), cclock2);
                                    timer_end.tv_sec = mts2.tv_sec;
                                    timer_end.tv_nsec = mts2.tv_nsec;
                                    
                                    long get_time = diff(timer_start, timer_end).tv_nsec;
                                    if (get_time > max_get_nano_time)
                                        max_get_nano_time = get_time;
                                    get_counter++;
                                    average_get_nano_time += (get_time - average_get_nano_time) / get_counter;
                                    

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
                                std::cout << cache.get_value_or_blank(int_key, &database, &btree) << std::endl;
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
                                                std::cout << cache.range(int_from, int_to, &database, &btree) << std::endl;
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
                                        std::cout << cache.range(int_from, int_to, &database, &btree) << std::endl;
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
                                    cache.delete_key(int_key, &database, &btree);
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
                                cache.delete_key(int_key, &database, &btree);
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
                            read_binary_file(path_str, &cache, &database, &btree);
                        }
                    }
                } else if (strncmp(token, "s", 1) == 0) {
                    //fprintf(stdout, "Print status command received...\n");
                    token = strtok(NULL, " ");
                    if (token != NULL) fprintf(stderr, "Too many arguments. Print status discarded...\n");
                    else {
                        //fprintf(stdout, "STATUS is retrieving from the database...\n");
                        //TODO: print status here
                        std::cout << "LOGINFO:\t\t" << "Total Pairs: " << total_size(&cache, &database, &btree) << std::endl;
                        std::cout << "LOGINFO:\t\t" << "LVL1: "<< cache.cache_dump().second << std::endl;
                        std::cout << "LOGINFO:\t\t" << cache.cache_dump().first << std::endl;
                        std::cout << "LOGINFO:\t\t" << "LVL2: "<< database.db_dump().second << std::endl;
                        std::cout << "LOGINFO:\t\t" << database.db_dump().first << std::endl;
                        std::cout << "LOGINFO:\t\t" << "LVL3: "<< btree.tree_dump().first << std::endl;
                        std::cout << "LOGINFO:\t\t" << btree.tree_dump().second << std::endl;
                    }
                } else if (strncmp(token, "q", 1) == 0) {
                    break;
                } else {
                    fprintf(stderr, "Invalid command received. Please try again...\n");
                }
            }
        }
    }
//    std::cout << "Maximum insertion time (nanoseconds): " << max_insert_nano_time << std::endl;
//    std::cout << "Average insertion time (nanoseconds): " << average_insert_nano_time << std::endl;
    
    std::cout << "Maximum get time (nanoseconds): " << max_get_nano_time << std::endl;
    std::cout << "Average get time (nanoseconds): " << average_get_nano_time << std::endl;
    
    return 0;
}
