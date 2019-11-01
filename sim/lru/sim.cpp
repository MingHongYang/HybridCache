#include "sim.h"
#include "logging.h"

void Evict() {
    // Return if eviction is not necessary
    if (lru.getSize() < DRAM_SIZE) {
        return;
    }

    // Evict LRU page
    PageInfo *victim = lru.getTop()->page;
    lru.removeTop();

    sysMap.erase(victim->getPageNumber());
    free(victim);
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

    while (!feof(pfInput)) {
        uintmax_t uPage = 0;
        uint8_t u8OP = 0;

        // Read trace
        fscanf(pfInput, "%" SCNuMAX " %" SCNu8, &uPage, &u8OP);

        gTotal++;

        // Check if it's in the cache
        if (sysMap.count(uPage) != 0) {
            if (u8OP == WRITE) {
                // We do not change the priority of pages for page writes
                gFlush++;
                continue;
            } else {
                gHit++;
            }

            // In the cache
            PageInfo *found = sysMap[uPage];
            found->setTimeStamp(gTimeStamp);
            found->getList()->remove(uPage);
            found->getList()->addBack(new Node(found));
        } else {
            gMiss++;
            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);

            // Put into system map
            sysMap.insert(make_pair(uPage, newPage));

            if (u8OP == WRITE) {
                gFlush++;
            }
            newPage->setList(&lru);
            newPage->getList()->addBack(new Node(newPage));
            Evict();
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", gMiss, gHit, gTotal, gFlush);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
