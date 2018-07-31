#include <algorithm>
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

enum OPType {WRITE, READ};
enum MemType {DRAM, NVRAM, SYSTEM};
enum PageStatus {DIRTY, CLEAN};

//#define NVRAM_SIZE 5
//#define DRAM_SIZE 5

#define DEBUG false
#define CHECK_CACHE false
#define MAX_CON_READS 3
#define CLEAN_PAGE_OFFSET 1

class PageInfo
{
    uintmax_t pageNumber;
    uintmax_t timeStamp;
    uint8_t u8Reads;
    OPType eOPType;
    MemType eMemType;
    PageStatus eDirty;

    public:
    
    bool operator < (const PageInfo& page) const
    {
        // The page with greater consecutive reads will be placed at the back of the vector
        // The page with smaller time stamp will be placed at the back of the vector
        switch (eMemType) {
            case DRAM:
                return (timeStamp > page.timeStamp);

                break;
            case NVRAM:
                if (u8Reads > page.u8Reads) {
                    // Compare number of consecutive reads first
                    return false;
                } else if (u8Reads == page.u8Reads) {
                    // Clean pages have higher priority when evicting
                    if (eDirty == page.eDirty) {
                        // Compare time stamp
                        if (timeStamp == page.timeStamp) {
                            cerr << "Time stamp collision! " << timeStamp << endl;
                        }
                        return (timeStamp > page.timeStamp); // what if they are the same?
                    } else if (eDirty == CLEAN) {
                        return false;
                    } else {
                        return true;
                    }
                } else {
                    return true;
                }

                break;
        }
    }

    bool operator = (const PageInfo& page) {
        pageNumber = page.pageNumber;
        timeStamp = page.timeStamp;
        u8Reads = page.u8Reads;
        eOPType = page.eOPType;
        eMemType = page.eMemType;
        eDirty = page.eDirty;
    }

    PageInfo(PageInfo &page) {
        pageNumber = page.pageNumber;
        timeStamp = page.timeStamp;
        u8Reads = page.u8Reads;
        eOPType = page.eOPType;
        eMemType = page.eMemType;
        eDirty = page.eDirty;
    }

    PageInfo(OPType opType, uintmax_t pageID, MemType type, uintmax_t& curTimeStamp) : pageNumber(pageID), eMemType(type), eOPType(opType) {
        curTimeStamp++;
        timeStamp = curTimeStamp;

        if (eOPType == WRITE) {
            eDirty = DIRTY;
        } else {
            eDirty = CLEAN;
        }

        u8Reads = 0;
    }

    void setu8Reads(uint8_t reads) {
        u8Reads = reads;
    }

    uint8_t getu8Reads() {
        return u8Reads;
    }

    void incReads(void) {
        if (u8Reads < MAX_CON_READS - 1) {
            u8Reads = u8Reads + 1;
        }
    }

    void clrReads(void) {
        u8Reads = 0;
    }

    uintmax_t getPageNumber() {
        return pageNumber;
    }

    OPType getOP() {
        return eOPType;
    }

    void setOP(OPType op) {
        eOPType = op;
    }

    uintmax_t getTimeStamp() {
        return timeStamp;
    }

    void setTimeStamp(uintmax_t& curTimeStamp) {
        curTimeStamp++;
        timeStamp = curTimeStamp;
    }

    MemType getMemType() {
        return eMemType;
    }

    void setMemType(MemType newType) {
        eMemType = newType;
    }

    PageStatus getDirty() {
        return eDirty;
    }

    void setDirty(PageStatus dirty) {
        eDirty = dirty;
    }
};
