#include <iostream>
#include <vector>    
#include <cstdint>   

using std::vector;
using std::string;
using std::cerr;
using std::stoi;
using std::exit;

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


configParameters parse(int argc, char **argv) {
    if (argc != 7) {
        cerr << "Arguments are missing!";
        exit(1);
    }

    configParameters params;
    params.num_sets       = stoi(argv[1]);
    params.blocks_in_set = stoi(argv[2]);
    params.block_size     = stoi(argv[3]);
    params.write_allocate = argv[4];
    params.write_rule   = argv[5];
    params.eviction_rule = argv[6];

    if (params.num_sets <= 0 || params.blocks_in_set <= 0 || params.block_size < 4) {
      cerr << "Error: Invalid cache parameters\n";
      exit(1);
    }

    if (params.write_rule == "write-back" && params.write_allocate == "no-write-allocate") {
        cerr << "Error: Cannot combine write-back with no-write-allocate.\n";
        exit(1);
    }

    if (params.eviction_rule != "lru" && params.eviction_rule != "fifo") {
        cerr << "Error: Invalid eviction rule.\n";
        exit(1);
    }
    return params;

}


int main( int argc, char **argv ) {
  // TODO: implement
  configParameters params = parse(argc, argv);
  return 0;
}
