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
    if (LFUSize() < DRAM_SIZE) {
        return;
    }
    
    for (int i = 0; i < MAX_FREQ; i++) {
        if (lfu[i]->getSize()) {
            PageInfo *victim = lfu[i]->getTop()->page;
            lfu[i]->removeTop();

            sysMap.erase(victim->getPageNumber());

            free(victim);
            break;
        }
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

        if (u8OP == WRITE) {
            // Skip writes
            continue;
        }

        gTotal++;

        // Check if it's in the cache
        if (sysMap.count(uPage) != 0) {
            if (u8OP == READ) {
                gHit++;
            } else {
                // We ignore write hits on pages and does not count into frequency
                gFlush++;
                continue;
            }

            // In the cache
            PageInfo *found = sysMap[uPage];
            found->getList()->remove(uPage);

            found->pageHit(MAX_FREQ);
            found->setList(lfu[found->getFreq() - 1]);
            found->getList()->addBack(new Node(found));
        } else {
            gMiss++;

            if (u8OP == WRITE) {
                gFlush++;
            }

            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);

            // Put into system map
            sysMap.insert(make_pair(uPage, newPage));

            newPage->setList(lfu[0]);
            newPage->getList()->addBack(new Node(newPage));

            Evict();
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", gTotal, gMiss, gHit, gFlush);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
