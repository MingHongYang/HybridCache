#include "sim.h"
#include "logging.h"

void CheckSize(FILE *pFile) {
    size_t read = rrrCache.getSize() + rfrCache.getSize();
    size_t write = wrrCache.getSize() + wfrCache.getSize();

    assert(read + write <= TOTAL_SIZE);

    fprintf(pFile, "%d %d %d %d %d %d %d %d %d %d\n", TARGET_WRITE_SIZE, TARGET_WF_SIZE, TARGET_WR_SIZE, TARGET_READ_SIZE, TARGET_RF_SIZE, TARGET_RR_SIZE, wfrCache.getSize(), wrrCache.getSize(), rfrCache.getSize(), rrrCache.getSize());

#if DEBUG
    cout << "***" << endl;
    cout << TARGET_WRITE_SIZE << " " << TARGET_WF_SIZE << " " << TARGET_WR_SIZE << endl;
    cout << TARGET_READ_SIZE << " " << TARGET_RF_SIZE << " " << TARGET_RR_SIZE << endl;
    cout << write << " " << wrgCache.getSize() << " " << wfgCache.getSize() << endl;
    cout << read << " " << rrgCache.getSize() << " " << rfgCache.getSize() << endl;
#endif
}

int AdjustedValue(CacheType type, OPType op, MemType loc) {
    switch(op) {
        case READ:
            if (loc == DRAM) {
                return DRAM_R;
            } else {
                return NVRAM_R;
            }
        case WRITE:
            if (loc == DRAM) {
                return DRAM_W;
            } else {
                return NVRAM_W;
            }
            break;
        default:
            assert(false);
            break;
    }

    return 0;
}

void UpdateTargetSize(CacheType type, OPType op, MemType loc) {
    // Update the target size when hitting on type cache

    int val = AdjustedValue(type, op, loc);
    if (val == 0) {
        return;
    }

    switch (type) {
        case RRR:
            break;
        case RRG:
            if (val > 0) {
                TARGET_READ_SIZE = min((int)TARGET_READ_SIZE + val, (int)TOTAL_SIZE - 1);
                TARGET_RR_SIZE = min((int)TARGET_RR_SIZE + val, (int)TARGET_READ_SIZE - 1);
            } else {
                TARGET_READ_SIZE = max((int)TARGET_READ_SIZE + val, 1);
                TARGET_RR_SIZE = max((int)TARGET_RR_SIZE + val, 1);
            }
            TARGET_WRITE_SIZE = max((int)(TOTAL_SIZE - TARGET_READ_SIZE), 1);
            TARGET_RF_SIZE = max((int)(TARGET_READ_SIZE - TARGET_RR_SIZE), 1);
            TARGET_WR_SIZE = max((int)(TARGET_WRITE_SIZE - TARGET_WF_SIZE), 1);
            break;
        case RFR:
            break;
        case RFG:
            if (val > 0) {
                TARGET_READ_SIZE = min((int)TARGET_READ_SIZE + val, (int)TOTAL_SIZE - 1);
                TARGET_RF_SIZE = min((int)TARGET_RF_SIZE + val, (int)TARGET_READ_SIZE - 1);
            } else {
                TARGET_READ_SIZE = max((int)TARGET_READ_SIZE + val, 1);
                TARGET_RF_SIZE = max((int)TARGET_RF_SIZE + val, 1);
            }
            TARGET_WRITE_SIZE = max((int)(TOTAL_SIZE - TARGET_READ_SIZE), 1);
            TARGET_RR_SIZE = max((int)(TARGET_READ_SIZE - TARGET_RF_SIZE), 1);
            TARGET_WR_SIZE = max((int)(TARGET_WRITE_SIZE - TARGET_WF_SIZE), 1);
            break;
        case WRR:
            break;
        case WRG:
            if (val > 0) {
                TARGET_WRITE_SIZE = min((int)TARGET_WRITE_SIZE + val, (int)TOTAL_SIZE - 1);
                TARGET_WR_SIZE = min((int)TARGET_WR_SIZE + val, (int)TARGET_WRITE_SIZE - 1);
            } else {
                TARGET_WRITE_SIZE = max((int)TARGET_WRITE_SIZE + val, (int)TOTAL_SIZE - 1);
                TARGET_WR_SIZE = max((int)TARGET_WR_SIZE + val, (int)TARGET_WRITE_SIZE - 1);
            }
            TARGET_READ_SIZE = max((int)(TOTAL_SIZE - TARGET_WRITE_SIZE), 1);
            TARGET_WF_SIZE = max((int)(TARGET_WRITE_SIZE - TARGET_WR_SIZE), 1);
            TARGET_RR_SIZE = max((int)(TARGET_READ_SIZE - TARGET_RF_SIZE), 1);
            break;
        case WFR:
            break;
        case WFG:
            if (val > 0) {
                TARGET_WRITE_SIZE = min((int)TARGET_WRITE_SIZE + val, (int)TOTAL_SIZE - 1);
                TARGET_WF_SIZE = min((int)TARGET_WF_SIZE + val, (int)TARGET_WRITE_SIZE - 1);
            } else {
                TARGET_WRITE_SIZE = max((int)TARGET_WRITE_SIZE + val, 1);
                TARGET_WF_SIZE = max((int)TARGET_WF_SIZE + val, 1);
            }
            TARGET_READ_SIZE = max((int)(TOTAL_SIZE - TARGET_WRITE_SIZE), 1);
            TARGET_WR_SIZE = max((int)(TARGET_WRITE_SIZE - TARGET_WF_SIZE), 1);
            TARGET_RR_SIZE = max((int)(TARGET_READ_SIZE - TARGET_RF_SIZE), 1);
            break;
    }

    return;
}

int TargetSize(CacheType type) {
    switch (type) {
        case RRR:
            return (int)TARGET_RR_SIZE;
        case RRG:
            return (int)max((int)(TARGET_WRITE_SIZE * TARGET_RF_SIZE / TARGET_READ_SIZE), 1);
        case RFR:
            return (int)TARGET_RF_SIZE;
        case RFG:
            return max((int)TARGET_WRITE_SIZE - TargetSize(RRG), 1);
        case WRR:
            return (int)TARGET_WR_SIZE;
        case WRG:
            return (int)max((int)(TARGET_READ_SIZE * TARGET_WF_SIZE / TARGET_WRITE_SIZE), 1);
        case WFR:
            return (int)TARGET_WF_SIZE;
        case WFG:
            return max((int)TARGET_READ_SIZE - TargetSize(WRG), 1);
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

    // Adjust the corresponding number of pages cached on physical memory
    if (victim->getMemType() == DRAM) {
        dramCnt--;
    } else {
        nvramCnt--;
    }
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
    size_t readSize = rrrCache.getSize() + rfrCache.getSize();
    size_t writeSize = wrrCache.getSize() + wfrCache.getSize();

    if (readSize + writeSize < TOTAL_SIZE) {
        return;
    }

    if (TARGET_READ_SIZE == 0) {
        // Initialize
        TARGET_READ_SIZE = readSize;
        TARGET_WRITE_SIZE = writeSize;
        TARGET_RR_SIZE = (int)(readSize / 2);
        TARGET_RF_SIZE = readSize - TARGET_RR_SIZE;
        TARGET_WR_SIZE = (int)(writeSize / 2);
        TARGET_WF_SIZE = writeSize - TARGET_WR_SIZE;
    }

#if DEBUG
    cout << "R: " << readSize << endl;
    cout << "rrr: " << rrrCache.getSize() << " rfr: " << rfrCache.getSize() << " wrr: " << wrrCache.getSize() << " wfr: " << wfrCache.getSize() << endl;
    cout << TARGET_READ_SIZE << " " << TARGET_RR_SIZE << " " << TARGET_WR_SIZE << endl;
#endif

    if (readSize <= TARGET_READ_SIZE) {
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
#if DEBUG
            cout << "W: " << wrrCache.getSize() << " " << TargetSize(WRR) << " " << wfrCache.getSize() << endl;
#endif
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
#if DEBUG
            cout << "R: " << rrrCache.getSize() << " " << TargetSize(RRR) << " " << rfrCache.getSize() << endl;
            cout << rrgCache.getSize() << " " << rfgCache.getSize() << endl;
#endif
            EvictReal(&rrrCache, &rrgCache, RRG);
        }
    }
}

int main(int argc, char* argv[]) {
    // Read config file path
    //ConfigFile cFile(argv[3]);

    // Files
    FILE *pfInput= fopen(argv[1], "r");
    FILE *pfOutput = fopen(argv[2], "a");
    FILE *pfStat = fopen(argv[3], "a");
    NVRAM_SIZE = stoi(argv[4]);
    DRAM_SIZE = stoi(argv[5]);
    DRAM_R = stoi(argv[6]);
    DRAM_W = stoi(argv[7]);
    NVRAM_R = stoi(argv[8]);
    NVRAM_W = stoi(argv[9]);
    TOTAL_SIZE = NVRAM_SIZE + DRAM_SIZE;

    uintmax_t lines = 0;
    // Initialize all the sizes to 0
    /*
       TARGET_READ_SIZE = TOTAL_SIZE / 2;
       TARGET_WRITE_SIZE = TOTAL_SIZE / 2;
       TARGET_RR_SIZE = TARGET_READ_SIZE / 2;
       TARGET_RF_SIZE = TARGET_READ_SIZE / 2;
       TARGET_WR_SIZE = TARGET_WRITE_SIZE / 2;
       TARGET_WF_SIZE = TARGET_WRITE_SIZE / 2;
       */

    TARGET_READ_SIZE = 0;
    TARGET_WRITE_SIZE = 0;
    TARGET_RR_SIZE = 0;
    TARGET_RF_SIZE = 0;
    TARGET_WR_SIZE = 0;
    TARGET_WF_SIZE = 0;

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

        // Print every 50 lines
        if (lines % 50 == 0)
            CheckSize(pfStat);
        lines++;

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
                        // Real cache hit
                        gRRRHitW++;

                        // Check whether this is a valid hit
                        if (found->getMemType() == DRAM) {
                            gNumWonDRAM++;
                            gNumFlush++;
                        } else {
                            gNumWonNVRAM++;
                        }

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RRG:
                        // TODO: When write hits on clean pages, should we move them to NVM immediately? Now we move to NVM immediately
                        UpdateTargetSize(RRG, WRITE, found->getMemType());
                        Evict();

                        // Ghost cache hit
                        gRRGHitW++;

                        // Check where this page is located
                        if (nvramCnt < NVRAM_SIZE) {
                            found->setMemType(NVRAM);
                        } else {
                            found->setMemType(DRAM);
                        }

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RFR:
                        // Real cache hit
                        gRFRHitW++;

                        // Check whether this is a valid hit
                        if (found->getMemType() == DRAM) {
                            gNumWonDRAM++;
                            gNumFlush++;
                        } else {
                            gNumWonNVRAM++;
                        }

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RFG:
                        UpdateTargetSize(RFG, WRITE, found->getMemType());

                        Evict();

                        // Ghost cache hit
                        gRFGHitW++;

                        // Check where this page is located
                        if (nvramCnt < NVRAM_SIZE) {
                            found->setMemType(NVRAM);
                        } else {
                            found->setMemType(DRAM);
                        }

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WRR:
                        // Real cache hit
                        gWRRHitW++;

                        // Check whether this is a valid hit
                        if (found->getMemType() == DRAM) {
                            gNumWonDRAM++;
                            gNumFlush++;
                        } else {
                            gNumWonNVRAM++;
                        }

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WRG:
                        UpdateTargetSize(WRG, WRITE, found->getMemType());

                        Evict();

                        // Ghost cache hit
                        gWRGHitW++;

                        // Check where this page is located
                        if (nvramCnt < NVRAM_SIZE) {
                            found->setMemType(NVRAM);
                        } else {
                            found->setMemType(DRAM);
                        }

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WFR:
                        // Real cache hit
                        gWFRHitW++;

                        // Check whether this is a valid hit
                        if (found->getMemType() == DRAM) {
                            gNumWonDRAM++;
                            gNumFlush++;
                        } else {
                            gNumWonNVRAM++;
                        }

                        found->getList()->addBack(new Node(found));
                        break;

                    case WFG:
                        UpdateTargetSize(WFG, WRITE, found->getMemType());

                        Evict();

                        // Ghost cache hit
                        gWFGHitW++;

                        // Check where this page is located
                        if (nvramCnt < NVRAM_SIZE) {
                            found->setMemType(NVRAM);
                        } else {
                            found->setMemType(DRAM);
                        }

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
                        // Real cache hit
                        gRRRHitR++;

                        found->setCacheType(RFR);
                        found->setList(&rfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RRG:
                        UpdateTargetSize(RRG, READ, found->getMemType());

                        Evict();

                        // Ghost cache hit
                        gRRGHitR++;

                        // Check where this page is located
                        if (nvramCnt < NVRAM_SIZE) {
                            found->setMemType(NVRAM);
                        } else {
                            found->setMemType(DRAM);
                        }

                        found->setCacheType(RFR);
                        found->setList(&rfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case RFR:
                        // Real cache hit
                        gRFRHitR++;

                        found->getList()->addBack(new Node(found));
                        break;

                    case RFG:
                        UpdateTargetSize(RFG, READ, found->getMemType());

                        Evict();

                        // Ghost cache hit
                        gRFGHitR++;

                        // Check where this page is located
                        if (nvramCnt < NVRAM_SIZE) {
                            found->setMemType(NVRAM);
                        } else {
                            found->setMemType(DRAM);
                        }

                        found->setCacheType(RFR);
                        found->setList(&rfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WRR:
                        // Real cache hit
                        gWRRHitR++;

                        found->getList()->addBack(new Node(found));
                        break;

                    case WRG:
                        // TODO: When read hits on dirty pages in ghost, should we move them to DRAM? Keep them in WFR as they are dirty pages.
                        UpdateTargetSize(WRG, READ, found->getMemType());

                        Evict();

                        // Ghost cache hit
                        gWRGHitR++;

                        // Check where this page is located
                        if (nvramCnt < NVRAM_SIZE) {
                            found->setMemType(NVRAM);
                        } else {
                            found->setMemType(DRAM);
                        }

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
                        found->getList()->addBack(new Node(found));
                        break;

                    case WFR:
                        // Real cache hit
                        gWFRHitR++;

                        found->getList()->addBack(new Node(found));
                        break;

                    case WFG:
                        UpdateTargetSize(WFG, READ, found->getMemType());

                        Evict();

                        // Ghost cache hit
                        gWFGHitR++;

                        // Check where this page is located
                        if (nvramCnt < NVRAM_SIZE) {
                            found->setMemType(NVRAM);
                        } else {
                            found->setMemType(DRAM);
                        }

                        found->setCacheType(WFR);
                        found->setList(&wfrCache);
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
            Evict();

            if (u8OP == WRITE) {
                // WRITE
                gWTotal++;
                gWMiss++;

                if (nvramCnt < NVRAM_SIZE) {
                    newPage->setPageType(DIRTY);
                    newPage->setMemType(NVRAM);

                    nvramCnt++;
                } else {
                    newPage->setPageType(CLEAN);
                    newPage->setMemType(DRAM);

                    dramCnt++;
                    gNumFlush++;
                }
                newPage->setCacheType(WRR);
                newPage->setList(&wrrCache);
                newPage->getList()->addBack(new Node(newPage));
            } else if (u8OP == READ) {
                // READ
                gRTotal++;
                gRMiss++;

                if (dramCnt < DRAM_SIZE) {
                    newPage->setPageType(CLEAN);
                    newPage->setMemType(DRAM);

                    dramCnt++;
                } else {
                    newPage->setPageType(CLEAN);
                    newPage->setMemType(NVRAM);

                    nvramCnt++;
                }
                newPage->setCacheType(RRR);
                newPage->setList(&rrrCache);
                newPage->getList()->addBack(new Node(newPage));
            }
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX  " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX  "%" SCNuMAX " %" SCNuMAX "\n", gNumWonNVRAM, gNumWonDRAM, gNumFlush, gWFRHitR + gWRRHitR + gRFRHitR + gRRRHitR, gWTotal, gRTotal, gWFGHitR, gWRGHitR, gRFGHitR, gRRGHitR, gWFGHitW, gWRGHitW, gRFGHitW, gRRGHitW, gWFRHitR, gWRRHitR, gRFRHitR, gRRRHitR, gWFRHitW, gWRRHitW, gRFRHitW, gRRRHitW);
    //fprintf(pfStat, "%d %d %d %d %d %d\n", TARGET_READ_SIZE, TARGET_WRITE_SIZE, TARGET_RR_SIZE, TARGET_RF_SIZE, TARGET_WR_SIZE, TARGET_WF_SIZE);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);
    fclose(pfStat);

    return true;
}
