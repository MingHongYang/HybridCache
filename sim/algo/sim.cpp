#include "sim.h"
#include "logging.h"

void EvictPage(LinkedList *real, LinkedList *ghost, CacheType type) {
    // Remove a page from ghost list and add a page from real to ghost with specific cache type

    // Remove from ghost first
    PageInfo *victim = ghost.getTop()->page;
    ghost.removeTop();

    // Remove from system
    sysMap.erase(victim->getPageNumber());

    // Remove page instance
    free(victim);

    // Remove from real cache
    victim = read.getTop()->page;
    real.removeTop();

    // Add to ghost
    victim->setCacheType(type);
    victim->setList(ghost);
    victim->getList()->addBack(Node(victim));
}

void Evict(void) {
    int readSize = rrrCache.getSize() + rfrCache.getSize();
    int writeSize = wrrCache.getSize() + wfrCache.getSize();

    assert(writeSize <= NVRAM_SIZE);
    assert(readSize < TOTAL_SIZE);

    if (readSize < TARGET_READ_SIZE) {
        // Evict from write
        if (wrrCache.getSize() < TARGET_WR_SIZE) {
            // Evict from frequency
            EvictPage(&wfrCache, &wfgCache, WFG);
        } else {
            // Evict from recency
            EvictPage(&wrrCache, &wrgCache, WRG);
        }
    } else {
        // Evict from read
        if (rrrCache.getSize() < TARGET_RR_SIZE) {
            // Evict from frequency
            EvictPage(&rfrCache, &rfgCache, RFG);
        } else {
            // Evict from recency
            EvictPage(&rrrCache, &rrgCache, RRG);
        }
    }
}

int main(int argc, char* argv[]) {
    // Files
    FILE *pfInput= fopen(argv[1], "r");
    FILE *pfOutput = fopen(argv[2], "w");
    NVRAM_SIZE = stoi(argv[3]);
    DRAM_SIZE = stoi(argv[4]);
    TOTAL_SIZE = NVRAM_SIZE + DRAM_SIZE;

    // Initialize all the sizes to half of the physical space
    TARGET_READ_SIZE = TOTAL_SIZE / 2;
    TARGET_RR_SIZE = TARGET_READ_SIZE / 2;
    TARGET_WR_SIZE = TARGET_RR_SIZE;

    if (!pfInput || !pfOutput) {
        // Error opening files
        printf("File name error\n");

        return ENOENT;
    }

    while (!feof(pfInput)) {
        uintmax_t uPage = 0;
        uint8_t u8OP = 0;

        // Read the log
        fscanf(pfInput, "%" SCNuMAX " %" SCNu8, &uPage, &u8OP);

        // Check if it's in the cache
        if (sysMap.count(uPage) != 0) {
            // In the cache
            PageInfo *found = sysMap[uPage];
            found->setTimeStamp(gTimeStamp);
            found->getList()->remove(uPage);

            if (u8OP == WRITE) {
                // WRITE
                gWTotal++;
                gWHit++;

                found->setOP(WRITE);
                found->setPageType(DIRTY);

                switch(found->getCacheType()) {
                    case RRR:
                        found->setCacheType(WRR);
                        found->setList(&wrrCache);
                        found->getList()->addBack(Node(found));
                        break;

                    case RRG:
                        // TODO: When write hits on clean pages, should we move them to NVM immediately? Now we move to NVM immediately
                        found->setCacheType(WRR);
                        found->setList(&wrrCache);
                        found->getList()->addBack(Node(found));

                        // Update target size
                        TARGET_READ_SIZE = min(TARGET_READ_SIZE + 1, TOTAL_SIZE - 1);
                        TARGET_RR_SIZE = min(TARGET_RR_SIZE + 1, TARGET_READ_SIZE - 1);
                        break;

                    case RFR:
                        found->setCacheType(WRR);
                        found->setList(&wrrCache);
                        found->getList()->addBack(Node(found));
                        break;

                    case RFG:
                        found->setCacheType(WRR);
                        found->setList(&wrrCache);
                        found->getList()->addBack(Node(found));

                        // Update target size
                        TARGET_READ_SIZE = min(TARGET_READ_SIZE + 1, TOTAL_SIZE - 1);
                        TARGET_RR_SIZE = max(TARGET_RR_SIZE - 1, 1);
                        break;

                    case WRR:
                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(Node(found));
                        break;

                    case WRG:
                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(Node(found));

                        //Update target size
                        TARGET_READ_SIZE = max(TARGET_READ_SIZE - 1, DRAM_SIZE);
                        TARGET_WR_SIZE = min(TARGET_WR_SIZE + 1, NVRAM_SIZE - 1);
                        break;

                    case WFR:
                        found->getList->addBack(Node(found));
                        break;

                    case WFG:
                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(Node(found));

                        //Update target size
                        TARGET_READ_SIZE = max(TARGET_READ_SIZE - 1, DRAM_SIZE);
                        TARGET_WR_SIZE = max(TARGET_WR_SIZE - 1, 1);
                        break;

                    default:
                        break;
                }

            } else if (u8OP == READ) {
                // READ
                gRTotal++;
                gRHit++;

                found->setOP(READ);

                switch(found->getCacheType()) {
                    case RRR:
                        found->setCacheType(RFR);
                        found->setList(&rfrCache);
                        found->getList()->addBack(Node(found));
                        break;

                    case RRG:
                        found->setCacheType(RFR);
                        found->setList(&rfrCache);
                        found->getList()->addBack(Node(found));

                        //Update target size
                        TARGET_READ_SIZE = min(TARGET_READ_SIZE + 1, DRAM_SIZE);
                        TARGET_RR_SIZE = min(TARGET_RR_SIZE + 1, TARGET_READ_SIZE - 1);
                        break;

                    case RFR:
                        list->addBack(Node(found));
                        break;

                    case RFG:
                        found->setCacheType(RFR);
                        found->setList(&rfrCache);
                        found->getList()->addBack(Node(found));
                        
                        //Update target size
                        TARGET_READ_SIZE = min(TARGET_READ_SIZE + 1, DRAM_SIZE);
                        TARGET_RR_SIZE = max(TARGET_RR_SIZE - 1, 1);
                        break;

                    case WRR:
                        found->getList()->addBack(Node(found));
                        break;

                    case WRG:
                        // TODO: When read hits on dirty pages in ghost, should we move them to DRAM? We move them to DRAM for now.
                        found->setCacheType(RRR);
                        found->setList(&rrrCache);
                        found->getList()->addBack(Node(found));
                        
                        //Update target size
                        TARGET_READ_SIZE = max(TARGET_READ_SIZE - 1, DRAM_SIZE);
                        TARGET_WR_SIZE = min(TARGET_WR_SIZE + 1, NVRAM - 1);
                        break;

                    case WFR:
                        found->getList()->addBack(Node(found));
                        break;

                    case WFG:
                        found->setCacheType(RRR);
                        found->setList(&rrrCache);
                        found->getList()->addBack(Node(found));
                        
                        //Update target size
                        TARGET_READ_SIZE = max(TARGET_READ_SIZE - 1, DRAM_SIZE);
                        TARGET_WR_SIZE = max(TARGET_WR_SIZE - 1, 1);
                        break;

                    default:
                        break;
                }
            }
        } else {
            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);

            // Evict one page
            Evict();

            if (u8OP == WRITE) {
                // WRITE
                gWTotal++;
                gWMisis++;

                newPage->setPageType(DIRTY);
                newPage->setMemType(NVRAM);
                newPage->setCacheType(WRR);
                newPage->setList(wrrCache);

                newPage->getList()->addBack(Node(newPage));
            } else if (u8OP == READ) {
                // READ
                gRTotal++;
                gRMiss++;

                newPage->setPageType(CLEAN);
                newPage->setMemType(DRAM);
                newPage->setCacheType(RRR);
                newPage->setList(rrrCache);

                newPage->getList()->addBack(Node(newPage));
            }
        }
    }

    fprintf(pfOutput, "%" SCNuMAX "\t%" SCNuMAX "\t%" SCNuMAX "\t%" SCNuMAX "\t%" SCNuMAX "\t%" SCNuMAX "\n", gWMiss, gRMiss, gWHit, gRHit, gWTotal, gRTotal);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
