//
//  comp.h
//  lsmtree
//
//  Created by Michael Hahn on 3/29/17.
//  Copyright © 2017 Michael Hahn. All rights reserved.
//

#ifndef comp_h
#define comp_h

struct set_compare {
    bool operator() (const std::pair<int, bool> elt1, const std::pair<int, bool> elt2) {
        return (elt1.first < elt2.first);
    }
};

#endif /* comp_h */
