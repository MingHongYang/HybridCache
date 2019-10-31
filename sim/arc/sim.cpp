#include "sim.h"
#include "logging.h"

void CheckSize(void) {
    int read = rrrCache.getSize() + rfrCache.getSize();
    int write = wrrCache.getSize() + wfrCache.getSize();

    assert(read <= TOTAL_SIZE);
    assert(write <= NVRAM_SIZE);
}

void UpdateTargetSize(CacheType type, OPType op) {
    // Update the target size when hitting on type cache

    switch (type) {
        case RRR:
            return;
        case RRG:
            if (op == READ) {
                TARGET_READ_SIZE = min((int)TARGET_READ_SIZE + 1, (int)TOTAL_SIZE - 2);
                TARGET_WRITE_SIZE = TOTAL_SIZE - TARGET_READ_SIZE;
                TARGET_RR_SIZE = min((int)TARGET_RR_SIZE + 1, (int)TARGET_READ_SIZE - 1);
                TARGET_RF_SIZE = TARGET_READ_SIZE - TARGET_RR_SIZE;
            }
            return;
        case RFR:
            return;
        case RFG:
            if (op == READ) {
                TARGET_READ_SIZE = min((int)TARGET_READ_SIZE + 1, (int)TOTAL_SIZE - 2);
                TARGET_WRITE_SIZE = TOTAL_SIZE - TARGET_READ_SIZE;
                TARGET_RF_SIZE = min((int)TARGET_RF_SIZE + 1, (int)TARGET_READ_SIZE - 1);
                TARGET_RR_SIZE = TARGET_READ_SIZE - TARGET_RF_SIZE;
            }
            return;
        case WRR:
            return;
        case WRG:
            if (op == WRITE) {
                TARGET_WRITE_SIZE = min((int)TARGET_WRITE_SIZE + 1, (int)NVRAM_SIZE);
                TARGET_READ_SIZE = TOTAL_SIZE - TARGET_WRITE_SIZE;
                TARGET_WR_SIZE = min((int)TARGET_WR_SIZE + 1, (int)TARGET_WRITE_SIZE - 1);
                TARGET_WF_SIZE = TARGET_WRITE_SIZE - TARGET_WR_SIZE;
            }
            return;
        case WFR:
            return;
        case WFG:
            if (op == WRITE) {
                TARGET_WRITE_SIZE = min((int)TARGET_WRITE_SIZE + 1, (int)NVRAM_SIZE);
                TARGET_READ_SIZE = TOTAL_SIZE - TARGET_WRITE_SIZE;
                TARGET_WF_SIZE = min((int)TARGET_WF_SIZE + 1, (int)TARGET_WRITE_SIZE - 1);
                TARGET_WR_SIZE = TARGET_WRITE_SIZE - TARGET_WF_SIZE;
            }
            return;
    }
}

int TargetSize(CacheType type) {
    switch (type) {
        case RRR:
            return (int)TARGET_RR_SIZE;
        case RRG:
            return (int)TARGET_RF_SIZE;
        case RFR:
            return (int)TARGET_RF_SIZE;
        case RFG:
            return (int)TARGET_RR_SIZE;
        case WRR:
            return (int)TARGET_WR_SIZE;
        case WRG:
            return (int)TARGET_WF_SIZE;
        case WFR:
            return (int)TARGET_WF_SIZE;
        case WFG:
            return (int)TARGET_WR_SIZE;
    }

    return 0;
}

void EvictReal(LinkedList *real, LinkedList *ghost, CacheType type) {
    assert(real->getTop() != NULL);

    // Remove from real cache
    PageInfo *victim = real->getTop()->page;

    real->removeTop();

    // Add to ghost
    victim->setCacheType(type);
    victim->setList(ghost);
    victim->getList()->addBack(new Node(victim));
}

void EvictGhost(LinkedList *list) {
    assert(list->getTop() != NULL);

    // Remove from ghost cache
    PageInfo *victim = list->getTop()->page;

    list->removeTop();

    // Remove from system
    sysMap.erase(victim->getPageNumber());

    // Remove page instance
    free(victim);
}

void Evict(void) {
    int readSize = rrrCache.getSize() + rfrCache.getSize();
    int writeSize = wrrCache.getSize() + wfrCache.getSize();

    //CheckSize();

#if DEBUG
    cout << "R: " << readSize << " W: " << writeSize << endl;
    cout << "rrr: " << rrrCache.getSize() << " rfr: " << rfrCache.getSize() << " wrr: " << wrrCache.getSize() << " wfr: " << wfrCache.getSize() << endl;
    cout << TARGET_READ_SIZE << " " << TARGET_RR_SIZE << " " << TARGET_WR_SIZE << endl;
#endif

    if (readSize < TARGET_READ_SIZE) {
        // Evict from write
        if (wrrCache.getSize() < TargetSize(WRR) && wfrCache.getSize()) {
            // Evict from frequency
            if (wfgCache.getSize() > TargetSize(WFG))
                EvictGhost(&wfgCache);

            EvictReal(&wfrCache, &wfgCache, WFG);
        } else {
            // Evict from recency
            if (wrgCache.getSize() > TargetSize(WRG))
                EvictGhost(&wrgCache);

            EvictReal(&wrrCache, &wrgCache, WRG);
        }
    } else {
        // Evict from read
        if (rrrCache.getSize() < TargetSize(RRR) && rfrCache.getSize()) {
            // Evict from frequency
            if (rfgCache.getSize() > TargetSize(RFG))
                EvictGhost(&rfgCache);

            EvictReal(&rfrCache, &rfgCache, RFG);
        } else {
            // Evict from recency
            if (rrgCache.getSize() > TargetSize(RRG))
                EvictGhost(&rrgCache);

            EvictReal(&rrrCache, &rrgCache, RRG);
        }
    }
}

int main(int argc, char* argv[]) {
    // Files
    FILE *pfInput= fopen(argv[1], "r");
    FILE *pfOutput = fopen(argv[2], "a");
    FILE *pfStat = fopen(argv[3], "a");
    NVRAM_SIZE = stoi(argv[4]);
    DRAM_SIZE = stoi(argv[5]);
    TOTAL_SIZE = NVRAM_SIZE + DRAM_SIZE;

    // Initialize all the sizes to half of the physical space
    TARGET_READ_SIZE = TOTAL_SIZE / 2;
    TARGET_WRITE_SIZE = TOTAL_SIZE / 2;
    TARGET_RR_SIZE = TARGET_READ_SIZE / 2;
    TARGET_RF_SIZE = TARGET_READ_SIZE / 2;
    TARGET_WR_SIZE = TARGET_WRITE_SIZE / 2;
    TARGET_WF_SIZE = TARGET_WRITE_SIZE / 2;

    if (!pfInput || !pfOutput || !pfStat) {
        // Error opening files
        printf("File name error\n");

        return ENOENT;
    }

    while (!feof(pfInput)) {
        uintmax_t uPage = 0;
        uint8_t u8OP = 0;

        // Read trace
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

#if DEBUG
                cout << "Write hit: " << found->getCacheType() << endl;
#endif

                switch(found->getCacheType()) {
                    case RRR:
                        found->setCacheType(WRR);
                        found->setList(&wrrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RRG:
                        // TODO: When write hits on clean pages, should we move them to NVM immediately? Now we move to NVM immediately
                        UpdateTargetSize(RRG, WRITE);
                        Evict();

                        found->setCacheType(WRR);
                        found->setList(&wrrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RFR:
                        found->setCacheType(WRR);
                        found->setList(&wrrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RFG:
                        UpdateTargetSize(RFG, WRITE);
                        Evict();

                        found->setCacheType(WRR);
                        found->setList(&wrrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WRR:
                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WRG:
                        UpdateTargetSize(WRG, WRITE);
                        Evict();

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WFR:
                        found->getList()->addBack(new Node(found));
                        break;

                    case WFG:
                        UpdateTargetSize(WFG, WRITE);
                        Evict();

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    default:
                        break;
                }

            } else if (u8OP == READ) {
                // READ
                gRTotal++;
                gRHit++;

                found->setOP(READ);

#if DEBUG
                cout << "Read hit: " << found->getCacheType() << endl;
#endif

                switch(found->getCacheType()) {
                    case RRR:
                        found->setCacheType(RFR);
                        found->setList(&rfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RRG:
                        UpdateTargetSize(RRG, READ);
                        Evict();

                        found->setCacheType(RFR);
                        found->setList(&rfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RFR:
                        found->getList()->addBack(new Node(found));
                        break;

                    case RFG:
                        UpdateTargetSize(RFG, READ);
                        Evict();

                        found->setCacheType(RFR);
                        found->setList(&rfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WRR:
                        found->getList()->addBack(new Node(found));
                        break;

                    case WRG:
                        // TODO: When read hits on dirty pages in ghost, should we move them to DRAM? We move them to DRAM for now.
                        UpdateTargetSize(WRG, READ);
                        Evict();

                        found->setCacheType(RRR);
                        found->setList(&rrrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WFR:
                        found->getList()->addBack(new Node(found));
                        break;

                    case WFG:
                        UpdateTargetSize(WFG, READ);
                        Evict();

                        found->setCacheType(RRR);
                        found->setList(&rrrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    default:
                        break;
                }
            }
        } else {
            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);

            // Put into system map
            sysMap.insert(make_pair(uPage, newPage));

            // Evict one page
            if (rrrCache.getSize() + rfrCache.getSize() + wrrCache.getSize() + wfrCache.getSize() == TOTAL_SIZE)
                Evict();

            if (u8OP == WRITE) {
                // WRITE
                gWTotal++;
                gWMiss++;

                newPage->setPageType(DIRTY);
                newPage->setMemType(NVRAM);
                newPage->setCacheType(WRR);
                newPage->setList(&wrrCache);

                newPage->getList()->addBack(new Node(newPage));
            } else if (u8OP == READ) {
                // READ
                gRTotal++;
                gRMiss++;

                newPage->setPageType(CLEAN);
                newPage->setMemType(DRAM);
                newPage->setCacheType(RRR);
                newPage->setList(&rrrCache);

                newPage->getList()->addBack(new Node(newPage));
            }
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", gWMiss, gRMiss, gWHit, gRHit, gWTotal, gRTotal);
    fprintf(pfStat, "%d %d %d %d %d %d\n", TARGET_READ_SIZE, TARGET_WRITE_SIZE, TARGET_RR_SIZE, TARGET_RF_SIZE, TARGET_WR_SIZE, TARGET_WF_SIZE);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);
    fclose(pfStat);

    return true;
}
