#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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

// Hash map to store the access sequence of each page
unordered_map<uintmax_t, queue<uintmax_t> *> nextRequest;

// Write frequency ghost cache
LinkedList dram;
LinkedList nvm;

uint32_t NVRAM_SIZE = 0;
uint32_t DRAM_SIZE = 0;
uint32_t TOTAL_SIZE = 0;

uint32_t TARGET_READ_SIZE = 0;
uint32_t TARGET_WRITE_SIZE = 0;
uint32_t TARGET_RR_SIZE = 0;
uint32_t TARGET_RF_SIZE = 0;
uint32_t TARGET_WR_SIZE = 0;
uint32_t TARGET_WF_SIZE = 0;

struct MaxHeap {
    std::vector<PageInfo *> data;

    MaxHeap() {
        data.push_back(NULL);
    }
    
    void insert(PageInfo *page) {
        data.push_back(page);
        int current = data.size()-1;

        // Set index
        page->setIndex(current);

        while(current != 1) {
            if(data[current]->getQueue()->front() > data[current >> 1]->getQueue()->front()) {
                assert(data[current]->getIndex() == current);
                assert(data[current >> 1]->getIndex() == current >> 1);
                
                data[current]->setIndex(current >> 1);
                data[current >> 1]->setIndex(current);

                std::swap(data[current], data[current >> 1]);
                current >>= 1;
            } else {
                break;
            }
        }
    }

    void pop() {
        if(data.size() == 1) return;

        std::swap(data[1], data[data.size()-1]);
        data.pop_back();
        data[1]->setIndex(1);
        int current = 1;

        while(current < data.size()) {
            int largest = current;
            if((current << 1) < data.size() && data[current << 1]->getQueue()->front() > data[largest]->getQueue()->front()) {
                largest = current << 1;
            }

            if((current << 1) + 1 < data.size() && data[(current << 1) + 1]->getQueue()->front() > data[largest]->getQueue()->front()) {
                largest = (current << 1) + 1;
            }

            if(largest != current) {
                data[current]->setIndex(largest);
                data[largest]->setIndex(current);
                std::swap(data[current], data[largest]);
                current = largest;
            } else {
                break;
            }
        }
    }

    PageInfo *top() {
        return data[1];
    }

    bool empty() {
        return data.size() <= 1;
    }

    void pageHit(int index) {
        max_heapify(index);
    }

    private:
    void max_heapify(int index) {
        int largest = index;
        if((index << 1) < data.size() && data[largest]->getQueue()->front() < data[index << 1]->getQueue()->front()) {
            largest = index << 1;
        }

        if((index << 1) + 1 < data.size() && data[largest]->getQueue()->front() < data[(index << 1) + 1]->getQueue()->front()) {
            largest = (index << 1) + 1;
        }

        if(index != largest) {
            data[index]->setIndex(largest);
            data[largest]->setIndex(index);
            std::swap(data[index], data[largest]);
            max_heapify(largest);
        }
    }
};

MaxHeap gHeap;
