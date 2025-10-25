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

//represetns a single cache block with metadata
struct block {
    bool valid;      // Indicates if block contains valid data
    bool dirty;     // indicates if block has been modified (write-back only)
    uint32_t tag;       // Tag portion of the address
    uint64_t timestamp;   //  for LRU/FIFO eviction policies
};

// represents a set of blocks in the cache
struct set {
    vector<block> blocks;
};

//Represents the entire cache structure
struct cache {
    vector<set> sets;
};

//  Configuration parameters for cache simulation
struct configParameters {
    int num_sets; 
    int blocks_in_set; //associativity
    int block_size;
    string write_allocate; // "write-allocate" or "no-write-allocate"
    string write_rule; // "write-through" or "write-back"
    string eviction_rule; // "lru" or "fifo"
    string cache_type; // "direct", "set-associative" or "fully-associative"
};

// statistics collected during cache simulation
struct cacheStats {
    uint64_t total_loads;
    uint64_t total_stores;
    uint64_t load_hits;
    uint64_t load_misses;
    uint64_t store_hits;
    uint64_t store_misses;
    uint64_t total_cycles;
};

/**
 * Checks if a number is a power of two
 * @param x The number to check
 * @return true if x is a power of two, false otherwise
 */
bool power_of_two(int x) {
    return x > 0 && (x & (x - 1)) == 0;
}

/**
 * Parses command line arguments and validates cache configuration
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Validated configuration parameters
 */
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

    // validate parameters
    if (params.num_sets <= 0 || params.blocks_in_set <= 0 || params.block_size < 4) {
        cerr << "Error: Invalid cache parameters\n";
        exit(1);
    }

    // ensure we have powers of two
    if (!power_of_two(params.num_sets) || !power_of_two(params.blocks_in_set)) {
        cerr << "Error: Number of sets and blocks in sets must both be powers of 2.\n";
        exit(1);
    }

    // make sure we don't have the incorrect combination
    if (params.write_rule == "write-back" && params.write_allocate == "no-write-allocate") {
        cerr << "Error: Cannot combine write-back with no-write-allocate.\n";
        exit(1);
    }

    // make sure we have valid rules
    if (params.eviction_rule != "lru" && params.eviction_rule != "fifo") {
        cerr << "Error: Invalid eviction rule.\n";
        exit(1);
    }

    //  cache type based on configuration
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

/**
 * simulates a direct-mapped cache
 * Direct-mapped: Each memory block maps to exactly one cache location
 * @param cach The cache structure
 * @param param Cache configuration parameters
 * @param result Statistics structure to update
 */
void simulate_direct(cache &cach, const configParameters param, cacheStats &result ) {
    string line;
    string address;
    char operation;
    unsigned extra;
    
    int offset_bits = (int) log2(param.block_size);
    int index_bits  = (int) log2(param.num_sets);
    int adjustedSize = 100 * (param.block_size / 4);

    // read the lines from the file
    while (getline(cin, line)) {
      if (line.empty()) {
        continue;
      }
      istringstream str(line);
      str >> operation; 
      str >> address;
      str >> extra;

      // convert address into numerical values
      uint32_t address_int = stoul(address, nullptr, 16);
      uint32_t index = (address_int >> offset_bits) & ((1 << index_bits) - 1);
      uint32_t tag = address_int >> (offset_bits + index_bits);

      block &b = cach.sets[index].blocks[0];

      // if we are getting rid of a dirty block, we write it back to memory
      if (b.valid && b.dirty && b.tag != tag && param.write_rule == "write-back") {
        result.total_cycles += adjustedSize;
        b.dirty = false;
      }

      // following section is for hit
      if (b.valid && b.tag == tag) {
        if (operation == 'l') {
          //  load hit
          result.load_hits++;
          result.total_cycles += 1;
        } else if(operation == 's') {
          // store hit
          result.store_hits++;
          
          if (param.write_rule == "write-through") {
            result.total_cycles += 1 + 100;
          } else if (param.write_rule == "write-back") {
            result.total_cycles += 1;
            b.dirty = true;
          }
        }
        // following section is miss
      } else {
        if (operation == 's') {
          result.store_misses++;

          if (param.write_allocate == "write-allocate") {
            // bring the full block 
            result.total_cycles += adjustedSize;
            b.tag = tag;
            b.valid = true;

            if (param.write_rule == "write-through") {
              result.total_cycles += 1 + 100;
            } else if (param.write_rule == "write-back") {
              result.total_cycles++;
              b.dirty = true;
            }
          } else if (param.write_allocate == "no-write-allocate") {
            result.total_cycles += 1 + 100;
          }
        } else if (operation == 'l') {
          result.total_cycles += 1 + adjustedSize;
          result.load_misses++;
          b.tag = tag;
          b.valid = true;
          b.dirty = false;
        }
      }
    }
}


  /**
 * Chooses a block for replacement using LRU or FIFO policy
 * @param s The cache set to search
 * @param param Cache configuration parameters
 * @param stats Statistics structure (updated if dirty block evicted)
 * @return Pointer to the block to be replaced
 */
block* chooseBlock(set &s, const configParameters &param, cacheStats &stats) {
  // check if there's an empty space
  for (int i = 0; i < param.blocks_in_set; i++) {
    if (s.blocks[i].valid == false) {
      return &s.blocks[i];
    }
  }

  // find the lowest timestamp
  block *toEvict = &s.blocks[0];
  for (int i = 0; i < param.blocks_in_set; i++) {
    block &b = s.blocks[i];
    if (b.timestamp < toEvict->timestamp) {
      toEvict = &b;
    }
  }
  int adjustedSize = 100 * (param.block_size / 4);
  // write back the block before eviction
  if (toEvict->dirty && toEvict->valid && param.write_rule == "write-back") {
    stats.total_cycles += adjustedSize;
    toEvict->dirty = false;
  }

  return toEvict;
}

/**
 * Simulates a set-associative cache
 * Set-associative: Multiple blocks per set, memory blocks can map to any block in a set
 * @param c The cache structure
 * @param param Cache configuration parameters
 * @param stats Statistics structure to update
 */
void simulate_set_associative(cache &c, const configParameters &param, cacheStats &stats) {
  // TODO: implement
  string line;
  string address;
  char operation;
  unsigned extra;
  uint64_t current_time = 0;

  int offset_bits = log2(param.block_size);
  int index_bits = log2(param.num_sets);
  int adjustedSize = 100 * (param.block_size / 4);

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

    set &s = c.sets[index];
    current_time++;

    if (operation == 'l') {
      stats.total_loads++;
    } else if (operation == 's') {
      stats.total_stores++;
    }

    bool hit = false;
    // this following section is for hit
    for (int i = 0; i < param.blocks_in_set; i++) {
      if (s.blocks[i].valid && s.blocks[i].tag == tag) {
        hit = true;
        block &b = s.blocks[i];
        b.timestamp = current_time; // this is for LRU specifically

        if (operation == 'l') {
          stats.load_hits++;
          stats.total_cycles++;
        } else if (operation == 's') {
          stats.store_hits++;
          if (param.write_rule == "write-through") {
            stats.total_cycles += 1 + 100;
          } else if (param.write_rule == "write-back") {
            b.dirty = true;
            stats.total_cycles++;
          }
        }
        break;
      }
    }

    // following section is for miss
    if (!hit) {
      if (operation == 's') {
        stats.store_misses++;
      } else if (operation == 'l') {
        stats.load_misses++;
      }

      if (operation == 'l' || param.write_allocate == "write-allocate") {
        // find the block to evict
        block *toEvict = chooseBlock(s, param, stats);
        stats.total_cycles += 1 + adjustedSize;
        toEvict->valid = true;
        toEvict-> tag = tag;
        toEvict->timestamp = current_time;
        toEvict->dirty = false;

        if (operation == 's') {
          if (param.write_rule == "write-through") {
            stats.total_cycles += 100 + 1; // cache + memory
          } else if (param.write_rule == "write-back") {
            toEvict->dirty = true;
          }
        }
        // write directly to memory
      } else if (operation == 's' && param.write_allocate == "no-write-allocate") {
        stats.total_cycles += 1 + 100;

      }

    }


  }


}


/*
Fully-Associative Cache:
- The entire cache is a single set.
- All blocks are placed in the same set.
- The tag is the same for all blocks in the cache.
- The index is not used.
- The offset is the same for all blocks in the cache.
Behavior plan:
decode tag only (offset = log2(block_size))
search all blocks in the set for matching tag (and valid)
LRU: use block.timestamp as last-accessed timestamp (update timestamp on access)
FIFO: use block.timestamp as last-accessed timestamp (set when block is filled, do NOT update on access)
(etc..)
*/
void simulate_fully_associative(cache &c, const configParameters &params, cacheStats &stats) {
  // TODO: implement
  string line;
  char operation;
  string address_str;
  unsigned extra;

  uint64_t global_time = 0; //global timestamp updated each access for LRU and FIFI

  int offset_bits = (int)std::log2(params.block_size);

  int adjustedSize = 100 * (params.block_size / 4);


  //one set: 
  set &cache_set = c.sets[0]; 
    

  while (std::getline(cin, line)) { //each line from cin 
    if (line.empty()) {
      continue;
    }

    //extract operating, address, and extra from line
    istringstream str(line);
    str >> operation; 
    str >> address_str;
    str >> extra;

    uint32_t address = std::stoul(address_str, nullptr, 16); //convert hex string to unsigned decimal int
    uint32_t tag = address >> offset_bits; //offset bits rep bytes w/in block
    global_time++;

    bool hit = false; //init to miss
    int hit_index = -1;

    // check every block in single set; if valid and matching tag, hit
    for (int i = 0; i < params.blocks_in_set; i++) {
      if (cache_set.blocks[i].valid && cache_set.blocks[i].tag == tag) {
        hit = true;
        hit_index = i;
        break;
      }
    }
    if (operation == 'l') { // load
      stats.total_loads++;

      if (hit) {
        stats.load_hits++;

        stats.total_cycles += 1; // cache hit -> 1 cycle


        if (params.eviction_rule == "lru") {
          cache_set.blocks[hit_index].timestamp = global_time;
        }
      } 
      else { //miss
        stats.load_misses++;

        // Find empty or evict block
        int replace_index = -1;
            
        for (int i = 0; i < params.blocks_in_set; i++) {
          if (!cache_set.blocks[i].valid) {
              replace_index = i;
              break;
          }
        }

        if (replace_index == -1) {
          // evict based on policy
          uint64_t oldest = cache_set.blocks[0].timestamp;
          replace_index = 0;
          for (int i = 1; i < params.blocks_in_set; i++) {
            if (cache_set.blocks[i].timestamp < oldest) {
              oldest = cache_set.blocks[i].timestamp;
              replace_index = i;
            }
          }

          //write back if dirty and using write-back
          if (params.write_rule == "write-back" && cache_set.blocks[replace_index].dirty) {
            stats.total_cycles += adjustedSize; // write dirty block to memory
            cache_set.blocks[replace_index].dirty = false;
          }
        }

        // new block into cache
        cache_set.blocks[replace_index].valid = true;
        cache_set.blocks[replace_index].dirty = false;
        cache_set.blocks[replace_index].tag = tag;
        cache_set.blocks[replace_index].timestamp = global_time;

        stats.total_cycles += adjustedSize + 1; // memory read + cache write
      }
    }


    //store
    else if (operation == 's') {
      stats.total_stores++;

      if (hit) {
        stats.store_hits++;
        if (params.write_rule == "write-through") {
          stats.total_cycles += 1 + 100; // 1 for cache +100 for mem
        } 
        else if (params.write_rule == "write-back") {
          cache_set.blocks[hit_index].dirty = true;
          stats.total_cycles += 1;
        }

        if (params.eviction_rule == "lru") {
          cache_set.blocks[hit_index].timestamp = global_time;
        }
      } 

      else {
        stats.store_misses++;

        if (params.write_allocate == "write-allocate") {
                
          int replace_index = -1;
          for (int i = 0; i < params.blocks_in_set; i++) {
            if (!cache_set.blocks[i].valid) {
              replace_index = i;
              break;
            }
          }

          if (replace_index == -1) {
                    
            // evict - LRU/FIFO
                    
            uint64_t oldest = cache_set.blocks[0].timestamp;
                    
            replace_index = 0;
                    
            for (int i = 1; i < params.blocks_in_set; i++) {  
              if (cache_set.blocks[i].timestamp < oldest) {     
                oldest = cache_set.blocks[i].timestamp;
                replace_index = i;
              }
            }

            if (params.write_rule == "write-back" && cache_set.blocks[replace_index].dirty) {
              stats.total_cycles += adjustedSize; // write back dirty
              cache_set.blocks[replace_index].dirty = false;
            }
          }

          cache_set.blocks[replace_index].valid = true;
          cache_set.blocks[replace_index].tag = tag;
          cache_set.blocks[replace_index].timestamp = global_time;

          if (params.write_rule == "write-back") {
            cache_set.blocks[replace_index].dirty = true;
            stats.total_cycles += adjustedSize + 1; // load + write
          } 
          else {
            cache_set.blocks[replace_index].dirty = false;
            stats.total_cycles += adjustedSize + 1 + 100; // 100 load + 1 cache + 100 memory
          }

        } 
        else if (params.write_allocate == "no-write-allocate") {
          //  drectly to memory dont mod cache

          stats.total_cycles += 1 + 100; 
        }
      }
    }

  } //while loop brack


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
  cout << "Total cycles: " << stats.total_cycles << endl;
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
