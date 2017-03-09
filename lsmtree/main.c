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
#include "utils.h"

#define MAX_STDIN_BUFFER_SIZE 1024

#define CHECKED_NEEDED 1


extern int errno;

int main(int argc, const char * argv[]) {

    
//    //if the input is from the terminal, output an interactive marker at the beginning of the command
//    char* control_message = "";
//    if (isatty(fileno(stdin))) {
//        control_message = "lsmtree_control > ";
//    }
//    
//    //receive a command from the user
//    char* command_message = NULL;
//    char command_buffer[MAX_STDIN_BUFFER_SIZE];
//    int errnum;
//    while (printf("%s", control_message),
//           command_message = fgets(command_buffer, MAX_STDIN_BUFFER_SIZE, stdin),
//           !feof(stdin)) {
//        if (command_message == NULL) {
//            errnum = errno;
//            fprintf(stderr, "fgets failed with error number: %d\n", errno);
//            break;
//        }
//        
//        /** six commands are supported
//         *  put: p key:int value:int
//         *  ** duplicates are not supported; repeated put of a key updates the value
//         *  ** no outputs
//         *
//         *  get: g key:int
//         *  ** returns the current value associated with the key; blank if key not in the tree
//         *
//         *  range: r key1:int key2:int
//         *  ** key1 inclusive, key2 exclusive; blank line if no keys in the range; otherwise key:value key:value
//         *
//         *  delete: d key:int
//         *  ** deletion of non-existing key has no effect; no output generated
//         *
//         *  load: l "path/to/file"
//         *
//         *  print status: s
//         *  ** Total Pairs: count
//         *  ** LVL1: count, LVL2: count, ..., LVLN: count
//         *  ** key:value:level key:value:level ...
//         *
//         */
//        //command must at least be one character long
//        if (strlen(command_buffer) > 0) {
//            remove_extra_whitespace(command_buffer);
//            //process user command and make sure they are well-formed
//            char * token;
//            token = strtok(command_buffer, " ");
//            fprintf(stdout, "The command is: %s\n", token);
//            //the command must be a single character (p, g, r, d, l, or s) with either nothing or a space appended afterwards
//            if (token[1] != '\0' && token[1] != '\n')
//                fprintf(stderr, "Command is a single character. See usage...\n");
//            //process single character command
//            else {
//                if (strncmp(token, "p", 1) == 0) {
//                    fprintf(stdout, "Put command received...\n");
//                    token = strtok(NULL, " ");
//                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need key value pair. Please try again...\n");
//                    else {
//                        char* key = token;
//                        if (CHECKED_NEEDED) {
//                            if (check_valid_int(key) == -1)
//                                fprintf(stderr, "Invalid key value. Please try again...\n");
//                            else {
//                                int int_key = to_int(key);
//                                fprintf(stdout, "Key: %d received...\n", int_key);
//                                token = strtok(NULL, " ");
//                                if (token == NULL) fprintf(stderr, "Not enough arguments. Need value. Please try again...\n");
//                                else {
//                                    char* value = token;
//                                    if (check_valid_int(value) == -1)
//                                        fprintf(stderr, "Invalid value value. Please try again...\n");
//                                    else {
//                                        int int_value = to_int(value);
//                                        fprintf(stdout, "Value: %d received...\n", int_value);
//                                        token = strtok(NULL, " ");
//                                        if (token != NULL) fprintf(stderr, "Too many arguments. Insertion discarded...\n");
//                                        else {
//                                            fprintf(stdout, "INSERT key-value pair: %d %d to the database...\n", int_key, int_value);
//                                            //TODO: insert to the database here
//                                        }
//                                    }
//                                }
//                            }
//                        } else {
//                            int int_key = to_int(key);
//                            fprintf(stdout, "Key: %d received...\n", int_key);
//                            token = strtok(NULL, " ");
//                            if (token == NULL) fprintf(stderr, "Not enough arguments. Need value. Please try again...\n");
//                            else {
//                                char* value = token;
//                                int int_value = to_int(value);
//                                fprintf(stdout, "Value: %d received...\n", int_value);
//                                token = strtok(NULL, " ");
//                                if (token != NULL) fprintf(stderr, "Too many arguments. Insertion discarded...\n");
//                                else {
//                                    fprintf(stdout, "INSERT key-value pair: %d %d to the database...\n", int_key, int_value);
//                                    //TODO: insert to the database here
//                                }
//                            }
//                        }
//                    }
//                } else if (strncmp(token, "g", 1) == 0) {
//                    fprintf(stdout, "Get command received...\n");
//                    token = strtok(NULL, " ");
//                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need key. Please try again...\n");
//                    else {
//                        char* key = token;
//                        if (CHECKED_NEEDED) {
//                            if (check_valid_int(key) == -1)
//                                fprintf(stderr, "Invalid key value. Please try again...\n");
//                            else {
//                                int int_key = to_int(key);
//                                fprintf(stdout, "Key: %d received...\n", int_key);
//                                token = strtok(NULL, " ");
//                                if (token != NULL) fprintf(stderr, "Too many arguments. Retrieval discarded...\n");
//                                else {
//                                    fprintf(stdout, "GET value from the key: %d if key is in the database...\n", int_key);
//                                    //TODO: get from the database here
//                                }
//                            }
//                        } else {
//                            int int_key = to_int(key);
//                            fprintf(stdout, "Key: %d received...\n", int_key);
//                            token = strtok(NULL, " ");
//                            if (token != NULL) fprintf(stderr, "Too many arguments. Retrieval discarded...\n");
//                            else {
//                                fprintf(stdout, "GET value from the key: %d if key is in the database...\n", int_key);
//                                //TODO: get from the database here
//                            }
//                        }
//                    }
//                } else if (strncmp(token, "r", 1) == 0) {
//                    fprintf(stdout, "Range command received...\n");
//                    token = strtok(NULL, " ");
//                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need two key ranges. Please try again...\n");
//                    else {
//                        char* from = token;
//                        if (CHECKED_NEEDED) {
//                            if (check_valid_int(from) == -1)
//                                fprintf(stderr, "Invalid from value. Please try again...\n");
//                            else {
//                                int int_from = to_int(from);
//                                fprintf(stdout, "From range: %d received...\n", int_from);
//                                token = strtok(NULL, " ");
//                                if (token == NULL) fprintf(stderr, "Not enough arguments. Need to range. Please try again...\n");
//                                else {
//                                    char* to = token;
//                                    if (check_valid_int(to) == -1)
//                                        fprintf(stderr, "Invalid to value. Please try again...\n");
//                                    else {
//                                        int int_to = to_int(to);
//                                        fprintf(stdout, "To range: %d received...\n", int_to);
//                                        token = strtok(NULL, " ");
//                                        if (token != NULL) fprintf(stderr, "Too many arguments. Range request discarded...\n");
//                                        else {
//                                            if (int_from <= int_to) {
//                                                fprintf(stdout, "GET values from key range: %d - %d in the database...\n", int_from, int_to);
//                                                //TODO: get from the database here
//                                            } else fprintf(stderr, "From value must be smaller than or equal to to value. Range request discarded...\n");
//                                        }
//                                    }
//                                }
//                            }
//                        } else {
//                            int int_from = to_int(from);
//                            fprintf(stdout, "From range: %d received...\n", int_from);
//                            token = strtok(NULL, " ");
//                            if (token == NULL) fprintf(stderr, "Not enough arguments. Need to range. Please try again...\n");
//                            else {
//                                char* to = token;
//                                int int_to = to_int(to);
//                                fprintf(stdout, "To range: %d received...\n", int_to);
//                                token = strtok(NULL, " ");
//                                if (token != NULL) fprintf(stderr, "Too many arguments. Range request discarded...\n");
//                                else {
//                                    if (int_from <= int_to) {
//                                        fprintf(stdout, "GET values from key range: %d - %d in the database...\n", int_from, int_to);
//                                        //TODO: get from the database here
//                                    } else fprintf(stderr, "From value must be smaller than or equal to to value. Range request discarded...\n");
//                                }
//                            }
//                        }
//                    }
//                } else if (strncmp(token, "d", 1) == 0) {
//                    fprintf(stdout, "Delete command received...\n");
//                    token = strtok(NULL, " ");
//                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need key. Please try again...\n");
//                    else {
//                        char* key = token;
//                        if (CHECKED_NEEDED) {
//                            if (check_valid_int(key) == -1)
//                                fprintf(stderr, "Invalid key value. Please try again...\n");
//                            else {
//                                int int_key = to_int(key);
//                                fprintf(stdout, "Key: %d received...\n", int_key);
//                                token = strtok(NULL, " ");
//                                if (token != NULL) fprintf(stderr, "Too many arguments. Deletion discarded...\n");
//                                else {
//                                    fprintf(stdout, "DELETE value from the key: %d if key is in the database...\n", int_key);
//                                    //TODO: delete from the database here
//                                }
//                            }
//                        } else {
//                            int int_key = to_int(key);
//                            fprintf(stdout, "Key: %d received...\n", int_key);
//                            token = strtok(NULL, " ");
//                            if (token != NULL) fprintf(stderr, "Too many arguments. Deletion discarded...\n");
//                            else {
//                                fprintf(stdout, "DELETE value from the key: %d if key is in the database...\n", int_key);
//                                //TODO: delete from the database here
//                            }
//                        }
//                    }
//                } else if (strncmp(token, "l", 1) == 0) {
//                    fprintf(stdout, "Load command received...\n");
//                    token = strtok(NULL, " ");
//                    if (token == NULL) fprintf(stderr, "Not enough arguments. Need file path. Please try again...\n");
//                    else {
//                        char* path = token;
//                        fprintf(stdout, "File path: %s received...\n", path);
//                        token = strtok(NULL, " ");
//                        if (token != NULL) fprintf(stderr, "Too many arguments. File load discarded...\n");
//                        else {
//                            unsigned long path_length = strlen(path);
//                            if (path[path_length - 1] == '\n') path[path_length - 1] = '\0';
//                            fprintf(stdout, "LOAD file from %s to the database...\n", path);
//                            FILE *fh = fopen(path, "r");
//                            //TODO: load file to the database here
//                        }
//                    }
//                } else if (strncmp(token, "s", 1) == 0) {
//                    fprintf(stdout, "Print status command received...\n");
//                    token = strtok(NULL, " ");
//                    if (token != NULL) fprintf(stderr, "Too many arguments. Print status discarded...\n");
//                    else {
//                        fprintf(stdout, "STATUS is retrieving from the database...\n");
//                        //TODO: print status here
//                    }
//                } else {
//                    fprintf(stderr, "Invalid command received. Please try again...\n");
//                }
//            }
//        }
//    }
    
    return 0;
}
