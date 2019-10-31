#include "sim.h"
#include "logging.h"

void Evict() {
    // Return if eviction is not necessary
    if (lru.getSize() < DRAM_SIZE) {
        return;
    }

    if (evictF < 0) {
        // Find the LFU page to be evicted
        for (int i = 0; i < MAX_FREQ; i++) {
            if (lfu[i]->getSize()) {
                PageInfo *victim = lfu[i]->getTop()->page;
                lfu[i]->removeTop();
                lru.remove(victim->getPageNumber());

                victim->setCacheType(GHOST);
                victim->setPageType(F);
                victim->setList(&ghost);
                victim->getList()->addBack(new Node(victim));

                break;
            }
        }
    } else {
        // Evict LRU page
        PageInfo *victim = lru.getTop()->page;
        lru.removeTop();
        victim->getListF()->remove(victim->getPageNumber());

        // Add to ghost
        victim->setCacheType(GHOST);
        victim->setPageType(R);
        victim->setList(&ghost);
        victim->getList()->addBack(new Node(victim));
    }

    // Check if ghost is full
    if (ghost.getSize() >= DRAM_SIZE) {
        // Evict one from ghost
        PageInfo *victim = ghost.getTop()->page;
        ghost.removeTop();

        sysMap.erase(victim->getPageNumber());
        free(victim);
    }
}

int main(int argc, char* argv[]) {
    // Files
    FILE *pfInput= fopen(argv[1], "r");
    FILE *pfOutput = fopen(argv[2], "a");
    NVRAM_SIZE = stoi(argv[3]);
    DRAM_SIZE = stoi(argv[4]);
    TOTAL_SIZE = NVRAM_SIZE + DRAM_SIZE;

    if (!pfInput || !pfOutput) {
        // Error opening files
        printf("File name error\n");

        return ENOENT;
    }

    // Initialize LFU vectors
    for (int i = 0; i < MAX_FREQ; i++) {
        lfu.push_back(new LinkedList());
    }

    while (!feof(pfInput)) {
        uintmax_t uPage = 0;
        uint8_t u8OP = 0;

        // Read trace
        fscanf(pfInput, "%" SCNuMAX " %" SCNu8, &uPage, &u8OP);

        if (u8OP == WRITE) {
            // Skip writes
            continue;
        }

        gTotal++;

        // Check if it's in the cache
        if (sysMap.count(uPage) != 0) {
            // In the cache
            PageInfo *found = sysMap[uPage];
            found->setTimeStamp(gTimeStamp);

            if (found->getCacheType() == REAL) {
                // Remove from both lists
                found->getListR()->remove(uPage);
                found->getListF()->remove(uPage);

                gHitR++;
                if (u8OP == WRITE) {
                    // WRITE
                } else if (u8OP == READ) {
                    // READ

                    found->pageHit(MAX_FREQ);
                    found->getListR()->addBack(new Node(found));
                    found->setListF(lfu[found->getFreq() - 1]);
                    found->getListF()->addBack(new Node(found));
                }
            } else {
                // Remove from ghost list
                found->getList()->remove(uPage);

                // Ghost cache hit
                if (found->getPageType() == F) {
                    // Evicted from LFU
                    gHitGF++;
                    evictF++;
                } else {
                    // Evicted from LRU
                    gHitGR++;
                    evictF--;
                }

                // Move it to real cache
                found->setCacheType(REAL);

                found->setListR(&lru);
                found->setListF(lfu[0]);
                found->getListR()->addBack(new Node(found));
                found->getListF()->addBack(new Node(found));

                Evict();
            }
        } else {
            gMiss++;

            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);

            // Put into system map
            sysMap.insert(make_pair(uPage, newPage));

            if (u8OP == WRITE) {
            } else if (u8OP == READ) {
                // READ

                Evict();

                newPage->setCacheType(REAL);

                newPage->setListR(&lru);
                newPage->setListF(lfu[0]);
                newPage->getListR()->addBack(new Node(newPage));
                newPage->getListF()->addBack(new Node(newPage));

            }
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", gHitR, gHitGR, gHitGF, gMiss, gTotal);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
