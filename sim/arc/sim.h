#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

#include "pageinfo.h"
#include "../linkedlist/LinkedList.h"

#define DEBUG false

// Global time stamp
uintmax_t gTimeStamp = 0;

// Global hash map for all the pages in the cache, including ghost caches
unordered_map<uintmax_t, PageInfo *> sysMap;

// Read recency real cache
LinkedList rrrCache;
// Read recency ghost cache
LinkedList rrgCache;
// Read frequency real cache
LinkedList rfrCache;
// Read frequency ghost cache
LinkedList rfgCache;
// Write recency real cache
LinkedList wrrCache;
// Write recency ghost cache
LinkedList wrgCache;
// Write frequency real cache
LinkedList wfrCache;
// Write frequency ghost cache
LinkedList wfgCache;

uint32_t nvmCnt = 0;
uint32_t dramCnt = 0;

uint32_t NVRAM_SIZE = 0;
uint32_t DRAM_SIZE = 0;
uint32_t TOTAL_SIZE = 0;

uint32_t TARGET_READ_SIZE = 0;
uint32_t TARGET_WRITE_SIZE = 0;
uint32_t TARGET_RR_SIZE = 0;
uint32_t TARGET_RF_SIZE = 0;
uint32_t TARGET_WR_SIZE = 0;
uint32_t TARGET_WF_SIZE = 0;
