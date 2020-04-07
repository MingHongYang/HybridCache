#include "sim.h"
#include "logging.h"

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

void Replace(CacheType type) {
    int rSize = rrCache.getSize();

    if (rSize != 0 && (rSize > TARGET_R_SIZE || (rSize == TARGET_R_SIZE && type == FG))) {
        EvictReal(&rrCache, &rgCache, RG);
    } else {
        EvictReal(&frCache, &fgCache, FG);
    }
}

void Evict() {
    int rSize = rrCache.getSize();
    int fSize = frCache.getSize();

    if (rSize + fSize < TOTAL_SIZE) {
        return;
    }

    rSize += rgCache.getSize();
    fSize += fgCache.getSize();

#if DEBUG
    cout << "R: " << readSize << " W: " << writeSize << endl;
    cout << "rrr: " << rrrCache.getSize() << " rfr: " << rfrCache.getSize() << " wrr: " << wrrCache.getSize() << " wfr: " << wfrCache.getSize() << endl;
    cout << TARGET_READ_SIZE << " " << TARGET_RR_SIZE << " " << TARGET_WR_SIZE << endl;
#endif

    if (rSize == TOTAL_SIZE) {
        if (rrCache.getSize() < TOTAL_SIZE) {
            EvictGhost(&rgCache);
            Replace(RG);
        } else {
            EvictReal(&rrCache, &rgCache, RG);
        }
    } else if (rrCache.getSize() + frCache.getSize() == TOTAL_SIZE) {
        if (rSize + fSize == 2 * TOTAL_SIZE) {
            // Remove from FG
            EvictGhost(&fgCache);
        }
            
        // Evict one, not from frequency ghost
        Replace(RG);        
    }
}

int main(int argc, char* argv[]) {
    // Files
    FILE *pfInput= fopen(argv[1], "r");
    FILE *pfOutput = fopen(argv[2], "a");
    NVRAM_SIZE = stoi(argv[3]);
    DRAM_SIZE = stoi(argv[4]);
    TOTAL_SIZE = NVRAM_SIZE + DRAM_SIZE;

    // Initialize all the sizes to half of the physical space
    TARGET_R_SIZE = 0;

    if (!pfInput || !pfOutput) {
        // Error opening files
        printf("File name error\n");

        return ENOENT;
    }

    while (!feof(pfInput)) {
        uintmax_t uPage = 0;
        uint8_t u8OP = 0;

        // Read trace
        fscanf(pfInput, "%" SCNuMAX " %" SCNu8, &uPage, &u8OP);
        gTotal++;

        // Check if it's in the cache
        if (sysMap.count(uPage) != 0) {
            // In the cache
            PageInfo *found = sysMap[uPage];

            if (found->getCacheType() == RG) {
                // Ghost hit, adjust boundary
                double delta;

                if (rgCache.getSize() < fgCache.getSize()) {
                    delta = fgCache.getSize() / rgCache.getSize();
                } else {
                    delta = 1.0;
                }

                TARGET_R_SIZE = min((double)TOTAL_SIZE, TARGET_R_SIZE + delta);

                found->getList()->remove(uPage);
                found->setCacheType(FR);
                found->setList(&frCache);
                found->getList()->addBack(new Node(found));

                Replace(RG);
            } else if (found->getCacheType() == FG) {
                // Ghost hit, adjust boundary
                double delta;

                if (rgCache.getSize() > fgCache.getSize()) {
                    delta = rgCache.getSize() / fgCache.getSize();
                } else {
                    delta = 1.0;
                }

                TARGET_R_SIZE = max(0.0, TARGET_R_SIZE - delta);

                found->getList()->remove(uPage);
                found->setCacheType(FR);
                found->setList(&frCache);
                found->getList()->addBack(new Node(found));

                Replace(FG);
            } else {
                // Real hit
                if (u8OP == WRITE) {
                    // Real write hit does not change the priority
                    gFlush++;
                    continue;
                } else {
                    // Read hit
                    gHit++;
                    PageInfo *found = sysMap[uPage];
                    found->getList()->remove(uPage);
                    found->setCacheType(FR);
                    found->setList(&frCache);
                    found->getList()->addBack(new Node(found));
                }
            }
        } else {
            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);

            int rSize = rrCache.getSize() + rgCache.getSize();

            if (rSize == TOTAL_SIZE) {
                if (rrCache.getSize() < TOTAL_SIZE) {
                    EvictGhost(&rgCache);
                    Replace(RG);
                } else {
                    EvictReal(&rrCache, &rgCache, RG);
                    EvictGhost(&rgCache);
                }
            } else if (rSize < TOTAL_SIZE) {
                int fSize = frCache.getSize() + fgCache.getSize();

                if (rSize + fSize == 2 * TOTAL_SIZE) {
                    // Remove from FG
                    EvictGhost(&fgCache);
                }

                // Evict one, not from frequency ghost
                if (rSize + fSize >= TOTAL_SIZE) {
                    Replace(RG); 
                }
            }

            if (u8OP == WRITE) {
                // WRITE
                gFlush++;
            } else if (u8OP == READ) {
                // READ
                gMiss++;
            }

            // Put into system map
            sysMap.insert(make_pair(uPage, newPage));

            newPage->setCacheType(RR);
            newPage->setList(&rrCache);
            newPage->getList()->addBack(new Node(newPage));
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", gMiss, gHit, gFlush, gTotal);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
