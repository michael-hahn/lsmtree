//
//  comp.h
//  lsmtree
//
//  Created by Michael Hahn on 3/29/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef comp_h
#define comp_h


bool get_stop = false;


struct set_compare {
    bool operator() (const std::pair<int, bool> elt1, const std::pair<int, bool> elt2) {
        return (elt1.first < elt2.first);
    }
};


typedef struct thread_data_get {
    int key;
    std::string rtn;
} thread_data_get;


typedef struct thread_data_range {
    int lower;
    int upper;
    std::map<int, long> result;
} thread_data_range;


#endif /* comp_h */