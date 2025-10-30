#!/bin/bash

TRACES=("gcc.trace" "swim.trace" "read01.trace" "read02.trace" "read03.trace" "write01.trace" "write02.trace" "small.trace")

echo -e "\nBASIC FUNCTIONS"

# Test Direct-Mapped Cache
echo -e "\nDirect-Mapped Cache Tests"
./csim 1 1 4 write-allocate write-through lru < small.trace
./csim 4 1 16 write-allocate write-back lru < read01.trace

# Test Set-Associative Cache  
echo -e "\n Set-Associative Cache Tests"
./csim 128 2 16 write-allocate write-back lru < gcc.trace
./csim 64 4 32 write-allocate write-through fifo < swim.trace

# Test Fully-Associative Cache
echo -e "\n Fully-Associative Cache Tests"
./csim 1 256 16 write-allocate write-back lru < read02.trace

echo -e "\n COMBINATION TESTS"

# Test Write Policy Combs
echo -e "\n Write Policy Combinations (on gcc.trace)"
echo "WA-WB-LRU:"
./csim 256 4 16 write-allocate write-back lru < gcc.trace | tail -1
echo "WA-WT-LRU:"
./csim 256 4 16 write-allocate write-through lru < gcc.trace | tail -1
echo "NWA-WT-LRU:"
./csim 256 4 16 no-write-allocate write-through lru < gcc.trace | tail -1

# Test Eviction Policy Tests
echo -e "\nEviction Policy Comparison (on swim.trace)"
echo "LRU:"
./csim 256 4 16 write-allocate write-back lru < swim.trace | tail -1
echo "FIFO:"
./csim 256 4 16 write-allocate write-back fifo < swim.trace | tail -1

echo -e "\n CACHE SIZE TEST"

# Test Varying Cache Sizes
echo -e "\nCache Size Scaling (on gcc.trace)"
for size in 16 64 256 1024; do
    echo "Sets: $size"
    ./csim $size 1 4 write-allocate write-back lru < gcc.trace | tail -1
done

# Test Block Size Impact
echo -e "\nBlock Size Impact (on swim.trace)"
for block_size in 4 16 64 128; do
    echo "Block Size: $block_size"
    ./csim 256 4 $block_size write-allocate write-back lru < swim.trace | tail -1
done

echo -e "\nASSOCIATIVITY"

# Test Associativity Scaling
echo -e "\nAssociativity Scaling (on gcc.trace)"
./csim 256 1 16 write-allocate write-back lru < gcc.trace | tail -1
./csim 128 2 16 write-allocate write-back lru < gcc.trace | tail -1
./csim 64 4 16 write-allocate write-back lru < gcc.trace | tail -1
./csim 32 8 16 write-allocate write-back lru < gcc.trace | tail -1
./csim 1 256 16 write-allocate write-back lru < gcc.trace | tail -1

echo -e "\n TRACE-SPECIFIC BEHAVIOR"

# Test different trace types
echo -e "\n Read Heavy Traces"
for trace in read01.trace read02.trace read03.trace; do
    echo "Trace: $trace"
    ./csim 256 4 16 write-allocate write-back lru < $trace | tail -1
done

echo -e "\n Write-Heavy Traces"
for trace in write01.trace write02.trace; do
    echo "Trace: $trace"
    ./csim 256 4 16 write-allocate write-back lru < $trace | tail -1
done


# Final comprehensive test of best candidates
echo -e "\nTop Configs Comparison - across all traces"

CONFIGURATIONS=(
    "256 4 16 write-allocate write-back lru"
    "256 4 16 write-allocate write-through lru"
    "128 8 32 write-allocate write-back lru" 
    "1 1024 16 write-allocate write-back lru"
    "512 2 16 write-allocate write-back lru"
)

for config in "${CONFIGURATIONS[@]}"; do
    echo -e "\nConfiguration: $config"
    echo "----------------------"
    total_cycles=0
    for trace in "${TRACES[@]}"; do
        cycles=$(./csim $config < $trace 2>/dev/null | grep "Total cycles" | awk '{print $3}')
        if [ ! -z "$cycles" ]; then
            echo "$trace: $cycles cycles"
            total_cycles=$((total_cycles + cycles))
        fi
    done
    echo "TOTAL CYCLES: $total_cycles"
done

echo -e "\nTesting complete!"