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

// Recency real cache
LinkedList rrCache;
// Frequency real cache
LinkedList frCache;
// Recency ghost cache
LinkedList rgCache;
// Frequency ghost cache
LinkedList fgCache;

uint32_t nvmCnt = 0;
uint32_t dramCnt = 0;

uint32_t NVRAM_SIZE = 0;
uint32_t DRAM_SIZE = 0;
uint32_t TOTAL_SIZE = 0;

double TARGET_R_SIZE = 0.0;

