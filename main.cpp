#include <iostream>
#include <vector>    
#include <cstdint>   

struct Block {
    bool valid;
    bool dirty;        
    uint32_t tag;
    uint64_t timestamp;
};

struct Set {
    std::vector<Block> blocks;
};

struct Cache {
    std::vector<Set> sets;
};





int main( int argc, char **argv ) {
  // TODO: implement

  return 0;
}
