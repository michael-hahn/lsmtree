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

//bloom filter implementation

#define GOLDEN_RATIO_64 0x61C8864680B583EBUL
static inline uint32_t b_hash(uint64_t val)
{
    return (val * GOLDEN_RATIO_64) >> (64 - 8);
}

#define K_HASH 7
#define M_BITS 256
#define N_BYTES (M_BITS / 8)
#define BYTE_INDEX(a) (a / 8)
#define BIT_INDEX(a) (a % 8)

static inline void bloom_add(uint8_t bloom[N_BYTES], uint64_t val)
{
    uint8_t i;
    uint32_t pos;
    
    for (i = 0; i < K_HASH; i++) {
        pos = b_hash(val + i) % M_BITS;
        bloom[BYTE_INDEX(pos)] |= 1 << BIT_INDEX(pos);
    }
}

/* element in set belong to super */
static inline bool bloom_match(const uint8_t super[N_BYTES], const uint8_t set[N_BYTES])
{
    uint8_t i;
    
    for (i = 0; i < N_BYTES; i++)
        if ((super[i] & set[i]) != set[i])
            return false;
    return true;
}

static inline bool bloom_in(const uint8_t bloom[N_BYTES], uint64_t val)
{
    uint8_t tmp[N_BYTES];
    
    memset(tmp, 0, N_BYTES);
    bloom_add(tmp, val);
    return bloom_match(bloom, tmp);
}

static inline bool bloom_empty(const uint8_t bloom[N_BYTES])
{
    uint8_t i;
    
    for (i = 0; i < N_BYTES; i++)
        if (bloom[i] != 0)
            return false;
    return true;
}

#endif /* comp_h */