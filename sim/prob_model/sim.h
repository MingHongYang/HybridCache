#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

using namespace std;

#include "pageinfo.h"
#include "../linkedlist/LinkedList.h"

#define DEBUG false

#define MAX_FREQ 20

// Global time stamp
uintmax_t gTimeStamp = 0;

// Ghost cache counter
uintmax_t gGhostCounter = 0;

// Global hash map for all the pages in the cache, including ghost caches
unordered_map<uintmax_t, PageInfo *> sysMap;

vector<LinkedList *> lfu;
LinkedList lru;
LinkedList ghost;

// Candidate map for mapping page ID to the pages where it is a candidate
unordered_map<uintmax_t, LinkedList *> candiMap;

// Random number generator
default_random_engine generator;
uniform_real_distribution<double> distribution(0.0,1.0);

double rate = 0.5;

uint32_t NVRAM_SIZE = 0;
uint32_t DRAM_SIZE = 0;
uint32_t TOTAL_SIZE = 0;

uint32_t TARGET_READ_SIZE = 0;
uint32_t TARGET_WRITE_SIZE = 0;
uint32_t TARGET_RR_SIZE = 0;
uint32_t TARGET_RF_SIZE = 0;
uint32_t TARGET_WR_SIZE = 0;
uint32_t TARGET_WF_SIZE = 0;

