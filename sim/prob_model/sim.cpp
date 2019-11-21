#include "sim.h"
#include "logging.h"

int LFUSize() {
    int total = 0;

    for (int i = 0; i < MAX_FREQ; i++) {
        total += lfu[i]->getSize();
    }

    return total;
}

void Evict() {
    // Return if eviction is not necessary
    if (lru.getSize() != LFUSize()) {
        cout << lru.getSize() << ": " << LFUSize() << endl;

        cout << "LRU: " << endl;

        while (lru.getSize()) {
            cout << lru.getTop()->page->getPageNumber() << " ";
            lru.removeTop();
        }

        cout << endl;
        cout << "LFU: " << endl;

        for (int i = 0; i < MAX_FREQ; i++) {
            while (lfu[i]->getSize()) {
                cout << lfu[i]->getTop()->page->getPageNumber() << " ";
                lfu[i]->removeTop();
            }
        }

        cout << endl;
        cout << "Ghost: " << endl;

        while (ghost.getSize()) {
            cout << ghost.getTop()->page->getPageNumber() << " ";
            ghost.removeTop();
        }
    }
    assert(lru.getSize() == LFUSize());

    if (lru.getSize() < DRAM_SIZE) {
        return;
    }

    PageInfo *vicPage;
    Node *vicNode;
    int dist = 0;

    // Remove ghost if full
    if (ghost.getSize() == DRAM_SIZE) {
        PageInfo *victim = ghost.getTop()->page;
        ghost.removeTop();

        // Remove the corresponding candidate
        if (candiMap.count(victim->getCandidate())) {
            candiMap[victim->getCandidate()]->remove(victim->getPageNumber());
        }

        if (candiMap[victim->getCandidate()]->getSize() == 0) {
            free(candiMap[victim->getCandidate()]);
            candiMap.erase(victim->getCandidate());
        }

        sysMap.erase(victim->getPageNumber());
        free(victim);
    }

    // Select to evict from LFU or LRU
    if (distribution(generator) > rate) {
        // LFU

        for (int i = 0; i < MAX_FREQ; i++) {
            if (lfu[i]->getSize()) {
                vicPage = lfu[i]->getTop()->page;
                lfu[i]->removeTop();
                break;
            }
        }

        vicNode = lru.getTop();

        if (vicNode->page->getPageNumber() == vicPage->getPageNumber()) {
            // Remove directly without going into ghost, we don't know how to adjust based on this one
            PageInfo *victim = vicNode->page;

            lru.removeTop();
            sysMap.erase(victim->getPageNumber());

            free(victim);

            return;
        }

        vicPage->setCandidate(vicNode->page->getPageNumber());
        if (candiMap.count(vicNode->page->getPageNumber()) == 0) {
            candiMap.insert(make_pair(vicNode->page->getPageNumber(), new LinkedList()));
        }
        candiMap[vicNode->page->getPageNumber()]->addBack(new Node(vicPage));

        while (vicNode->page->getPageNumber() != vicPage->getPageNumber()) {
            vicNode = vicNode->next;
            dist++;
        }

        vicPage->setPageType(F);
        vicPage->setCacheType(GHOST);
        vicPage->setDist(dist);

        // Remove from  LRU
        lru.remove(vicPage->getPageNumber());

        ghost.addBack(new Node(vicPage));
    } else {
        // LRU

        vicPage = lru.getTop()->page;
        lru.removeTop();

        for (int i = 0; i < MAX_FREQ; i++) {
            if (lfu[i]->getTop()) {
                vicNode = lfu[i]->getTop();

                if (vicNode->page->getPageNumber() == vicPage->getPageNumber()) {
                    // Remove directly without going into ghost, we don't know how to adjust based on this one
                    PageInfo *victim = vicNode->page;

                    lfu[i]->removeTop();
                    sysMap.erase(victim->getPageNumber());

                    free(victim);

                    return;
                }

                vicPage->setCandidate(vicNode->page->getPageNumber());

                if (candiMap.count(vicNode->page->getPageNumber()) == 0) {
                    candiMap.insert(make_pair(vicNode->page->getPageNumber(), new LinkedList()));
                }
                candiMap[vicNode->page->getPageNumber()]->addBack(new Node(vicPage));

                break;
            }
        }

        for (int i = 0; i < MAX_FREQ; i++) {
            vicNode = lfu[i]->getTop();

            while (vicNode) {
                if (vicNode->page->getPageNumber() == vicPage->getPageNumber()) {
                    lfu[i]->remove(vicPage->getPageNumber());
                    break;
                } else {
                    vicNode = vicNode->next;
                    dist++;
                }
            }

            if (vicNode) {
                break;
            }
        }

        vicPage->setPageType(R);
        vicPage->setCacheType(GHOST);
        vicPage->setDist(dist);

        ghost.addBack(new Node(vicPage));
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

    // Initialize LFU vector
    for (int i = 0; i < MAX_FREQ; i++) {
        lfu.push_back(new LinkedList());
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

            if (found->getCacheType() == REAL) {
                // Real cache hit
                if (u8OP == WRITE) {
                    gFlush++;
                    continue;
                }

                gHit++;

                // Check if this is in the candidate list
                if (candiMap.count(uPage)) {
                    Node *cur = candiMap[uPage]->getTop();

                    while (cur) {
                        PageInfo *victim = cur->page;

                        int dis = victim->getDist();
                        uintmax_t pos = victim->getGhost();

                        double adj = (double) dis / (double) (ghost.getTop()->page->getGhost() - pos) / DRAM_SIZE;

                        if (victim->getPageType() == R) {
                            rate = max(0.0, rate -adj);
                        } else {
                            rate = min(1.0, rate + adj);
                        }

                        ghost.remove(victim->getPageNumber());
                        sysMap.erase(victim->getPageNumber());

                        free(victim);

                        cur = cur->next;
                    }

                    free(candiMap[uPage]);
                    candiMap.erase(uPage);
                }

                // Remove from LRU and LFU
                lfu[found->getFreq() - 1]->remove(uPage);
                lru.remove(uPage);

                // Add back to LRU and LFU
                found->pageHit(MAX_FREQ);
                lfu[found->getFreq() - 1]->addBack(new Node(found));
                lru.addBack(new Node(found));
            } else {
                // Ghost cache hit
                if (u8OP == WRITE) {
                    // Do nothing for write requests?
                    continue;
                }

                // Update weight
                int dis = found->getDist();
                uintmax_t pos = found->getGhost();

                double adj = (double) dis / (double) (ghost.getTop()->page->getGhost() - pos) / DRAM_SIZE;

                if (found->getPageType() == R) {
                    rate = min(1.0, rate + adj);
                } else {
                    rate = max(0.0, rate - adj);
                }

                // Remove the old page
                PageInfo *victim = sysMap[uPage];
                sysMap.erase(uPage);
                ghost.remove(uPage);

                // Remove the corresponding candidate
                if (candiMap.count(victim->getCandidate())) {
                    candiMap[victim->getCandidate()]->remove(uPage);
                }

                if (candiMap[victim->getCandidate()]->getSize() == 0) {
                    free(candiMap[victim->getCandidate()]);
                    candiMap.erase(victim->getCandidate());
                }

                free(victim);

                // Check if we need to evict one
                Evict();

                // Prepare new page
                PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);
                newPage->setCacheType(REAL);

                // Put into system map
                sysMap.insert(make_pair(uPage, newPage));

                // Add to both LFU and LRU
                lfu[newPage->getFreq() - 1]->addBack(new Node(newPage));
                lru.addBack(new Node(newPage));
            }
        } else {
            gMiss++;

            if (u8OP == WRITE) {
                gFlush++;
            }

            // Check if we need to evict one
            Evict();

            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);
            newPage->setCacheType(REAL);

            // Put into system map
            sysMap.insert(make_pair(uPage, newPage));

            // Add to both LFU and LRU
            lfu[newPage->getFreq() - 1]->addBack(new Node(newPage));
            lru.addBack(new Node(newPage));
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", gTotal, gMiss, gHit, gFlush);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
