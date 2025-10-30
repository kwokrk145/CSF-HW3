TODO: names of team members and their contributions to the project
MS 1: ensure Makefile works and can build csim executable

MS2: 
Ryan: direct mapped and set associative 
Madhu: fully associative

Both worked on general workflow implementations

TODO (for MS3): best cache report

observations:
- Write-back vs Write thru
write back is better (ex: 9344483 WBack vs 25327519 for WThru cycles)
because write-back reduces memory traffic by only writing dirty blocks back when evicted, while write-thru writes to mem on every stores

-  eviction policy
ex: LRU vs FIFO 9009593 cycles for LRU vs 9655593 for FIFO
lru keeps recently used blocks - works good for programs w/temporal locality

- as cache size increases, performance better bc larger caches reduce capacity misses significantly

- block size impact (interesting finding, could require more testing)
4B: 5304493 cycles
16B: 9009593 cycles
64B: 9591193 cycles
128B: 8757593 cycles
very small blocks perform best, possible bc traces have poor spatial locality so larger blocks waste by fetching unsused data

- most benefits come from 1 to 4 way associativity
1-way: 20312483 cycles
2-way:13425283 cycles (34% improvement)
4-way:11414883 cycles 
8-way: 11068483 cycles 
Full: 10802483 cycles (2% impr..)
notice the decreasing % returns

overall best: 
1. 1 1024 16 WA-WB-LRU: 18202537 cycles (best)
2. 256 4 16 WA-WB-LRU: 18363737 cycles (very close second, best practical: write-allocate write-back LRU)
3. 512 2 16 WA-WB-LRU: 18697737 cycles

raw data:

BASIC FUNCTIONS

Direct-Mapped Cache Tests
Total loads: 15
Total stores: 7
Load hits: 1
Load misses: 14
Store hits: 0
Store misses: 7
Total cycles: 2822
Total loads: 5
Total stores: 0
Load hits: 2
Load misses: 3
Store hits: 0
Store misses: 0
Total cycles: 1205

 Set-Associative Cache Tests
Total loads: 318197
Total stores: 197486
Load hits: 309037
Load misses: 9160
Store hits: 187099
Store misses: 10387
Total cycles: 13425283
Total loads: 220668
Total stores: 82525
Load hits: 217756
Load misses: 2912
Store hits: 76294
Store misses: 6231
Total cycles: 15876324

 Fully-Associative Cache Tests
Total loads: 10
Total stores: 0
Load hits: 9
Load misses: 1
Store hits: 0
Store misses: 0
Total cycles: 410

 COMBINATION TESTS

 Write Policy Combinations (on gcc.trace)
WA-WB-LRU:
Total cycles: 9344483
WA-WT-LRU:
Total cycles: 25327519
NWA-WT-LRU:
Total cycles: 22897883

Eviction Policy Comparison (on swim.trace)
LRU:
Total cycles: 9009593
FIFO:
Total cycles: 9655593

 CACHE SIZE TEST

Cache Size Scaling (on gcc.trace)
Sets: 16
Total cycles: 53698983
Sets: 64
Total cycles: 23913883
Sets: 256
Total cycles: 16048083
Sets: 1024
Total cycles: 9233383

Block Size Impact (on swim.trace)
Block Size: 4
Total cycles: 5304493
Block Size: 16
Total cycles: 9009593
Block Size: 64
Total cycles: 9591193
Block Size: 128
Total cycles: 8757593

ASSOCIATIVITY

Associativity Scaling (on gcc.trace)
Total cycles: 20312483
Total cycles: 13425283
Total cycles: 11414883
Total cycles: 11068483
Total cycles: 10802483

 TRACE-SPECIFIC BEHAVIOR

 Read Heavy Traces
Trace: read01.trace
Total cycles: 1205
Trace: read02.trace
Total cycles: 410
Trace: read03.trace
Total cycles: 1609

 Write-Heavy Traces
Trace: write01.trace
Total cycles: 1205
Trace: write02.trace
Total cycles: 410

Top Configs Comparison - across all traces

Configuration: 256 4 16 write-allocate write-back lru
----------------------
gcc.trace: 9344483 cycles
swim.trace: 9009593 cycles
read01.trace: 1205 cycles
read02.trace: 410 cycles
read03.trace: 1609 cycles
write01.trace: 1205 cycles
write02.trace: 410 cycles
small.trace: 4822 cycles
TOTAL CYCLES: 18363737

Configuration: 256 4 16 write-allocate write-through lru
----------------------
gcc.trace: 25327519 cycles
swim.trace: 13258262 cycles
read01.trace: 1205 cycles
read02.trace: 410 cycles
read03.trace: 1609 cycles
write01.trace: 1708 cycles
write02.trace: 1411 cycles
small.trace: 5527 cycles
TOTAL CYCLES: 38597651

Configuration: 128 8 32 write-allocate write-back lru
----------------------
gcc.trace: 9452483 cycles
swim.trace: 9571193 cycles
read01.trace: 2405 cycles
read02.trace: 810 cycles
read03.trace: 2409 cycles
write01.trace: 2405 cycles
write02.trace: 810 cycles
small.trace: 7222 cycles
TOTAL CYCLES: 19039737

Configuration: 1 1024 16 write-allocate write-back lru
----------------------
gcc.trace: 9226883 cycles
swim.trace: 8965993 cycles
read01.trace: 1205 cycles
read02.trace: 410 cycles
read03.trace: 1609 cycles
write01.trace: 1205 cycles
write02.trace: 410 cycles
small.trace: 4822 cycles
TOTAL CYCLES: 18202537

Configuration: 512 2 16 write-allocate write-back lru
----------------------
gcc.trace: 9588883 cycles
swim.trace: 9099193 cycles
read01.trace: 1205 cycles
read02.trace: 410 cycles
read03.trace: 1609 cycles
write01.trace: 1205 cycles
write02.trace: 410 cycles
small.trace: 4822 cycles
TOTAL CYCLES: 18697737

Testing complete!
