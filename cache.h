#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <string>
#include <cstdint>

using std::vector;
using std::string;

struct block {
    bool valid;
    bool dirty;
    uint32_t tag;
    uint64_t timestamp;
};

struct set {
    vector<block> blocks;
};

struct cache {
    vector<set> sets;
};

struct configParameters {
    int num_sets;
    int blocks_in_set;
    int block_size;
    string write_allocate;
    string write_rule;
    string eviction_rule;
};


#endif