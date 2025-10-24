#include <iostream>
#include <vector>    
#include <cstdint>   
#include <sstream>
#include <cmath>


using std::vector;
using std::string;
using std::cerr;
using std::stoi;
using std::exit;
using std::cin;
using std::istringstream; 
using std::stoul;
using std::cout;
using std::endl;

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
    string cache_type;
};

struct cacheStats {
    uint64_t total_loads;
    uint64_t total_stores;
    uint64_t load_hits;
    uint64_t load_misses;
    uint64_t store_hits;
    uint64_t store_misses;
    uint64_t total_cycles;
};

bool power_of_two(int x) {
    return x > 0 && (x & (x - 1)) == 0;
}

configParameters parse(int argc, char **argv) {
    if (argc != 7) {
        cerr << "Incorrect number of arguments!";
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

    if (!power_of_two(params.num_sets) || !power_of_two(params.blocks_in_set)) {
        cerr << "Error: Number of sets and blocks in sets must both be powers of 2.\n";
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

    if (params.num_sets > 1 && params.block_size == 1) {
      params.cache_type = "direct";
    } else if (params.num_sets > 1 && params.block_size > 1) {
      params.cache_type = "set-associative";
    } else if (params.num_sets == 1 && params.block_size > 1) {
      params.cache_type = "fully-associative";
    } else {
      cerr << "Error: Invalid combinations of num_sets and block_size. Does not correspond to any cache type.\n";
      exit(1);
    }
    return params; 
}

void simulate_direct(cache &cach, const configParameters param, cacheStats &result ) {
    string line;
    string address;
    char operation;
    unsigned extra;
    
    int offset_bits = log2(param.block_size);
    int index_bits  = log2(param.num_sets);

    while (getline(cin, line)) {
      if (line.empty()) {
        continue;
      }
      istringstream str(line);
      str >> operation; 
      str >> address;
      str >> extra;

      uint32_t address_int = stoul(address, nullptr, 16);
      uint32_t index = (address_int >> offset_bits) & ((1 << index_bits) - 1);
      uint32_t tag = address_int >> (offset_bits + index_bits);

      block &b = cach.sets[index].blocks[0];

      if (b.valid && b.tag == tag) {
        if (operation == 'l') {
          result.load_hits++;
          result.total_cycles += 1;
        } else if(operation == 's') {
          result.store_hits++;
          
          if (param.write_rule == "write-through") {
            result.total_cycles += 101;
          } else if (param.write_rule == "write-back") {
            result.total_cycles += 1;
            b.dirty = true;
          }
        }
      } else {
        if (operation == 's') {
          result.store_misses++;

          if (param.write_allocate == "write-allocate") {
            result.total_cycles += 101;
            b.tag = tag;
            b.valid = true;

            if (param.write_rule == "write-through") {
              result.total_cycles += 100;
            } else if (param.write_rule == "write-back") {
              b.dirty = true;
            }
          }
        } else if (operation == 'l') {
          result.total_cycles += 101;
          result.load_misses++;
          b.tag = tag;
          b.valid = true;
          b.dirty = false;
        }
      }
    }
}


void simulate_set_associative(cache &c, const configParameters &params, cacheStats &stats) {
  // TODO: implement
}

void simulate_fully_associative(cache &c, const configParameters &params, cacheStats &stats) {
  // TODO: implement
}

void run_simulation(cache &c, const configParameters &params, cacheStats &stats) {
  // TODO: implement
  if (params.cache_type == "direct") { simulate_direct(c, params, stats); }
  else if (params.cache_type == "set-associative") { simulate_set_associative(c, params, stats); }
  else if (params.cache_type == "fully-associative") { simulate_fully_associative(c, params, stats);} 
  else {
    cerr << "Error:unknown cache type in run_simulation()" << std::endl;
    exit(1);
  }
}


cache initialize_cache(const configParameters params) {
  cache c;
  c.sets = vector<set>(params.num_sets);
  for (int i = 0; i < params.num_sets; i++) {
    c.sets[i].blocks = vector<block>(params.blocks_in_set);
    for (int j = 0; j < params.blocks_in_set; j++) {
      c.sets[i].blocks[j].valid = false;
      c.sets[i].blocks[j].dirty = false;
      c.sets[i].blocks[j].tag = 0;
      c.sets[i].blocks[j].timestamp = 0;
    }   
  }
  return c;
}


cacheStats initialize_stats() {
  cacheStats s = {0, 0, 0, 0, 0, 0, 0};
  return s;
}

void print_stats(const cacheStats &stats) {
  cout << "Total loads: " << stats.total_loads << endl;
  cout << "Total stores: " << stats.total_stores << endl;
  cout << "Load hits: " << stats.load_hits << endl;
  cout << "Load misses: " << stats.load_misses << endl;
  cout << "Store hits: " << stats.store_hits << endl;
  cout << "Store misses: " << stats.store_misses << endl;
}

int main( int argc, char **argv ) {
  // TODO: implement
  configParameters params = parse(argc, argv);
  cacheStats stats = initialize_stats();
  cache cache = initialize_cache(params);

  run_simulation(cache, params, stats);

  print_stats(stats);


  return 0;
}
