#include "sim.h"
#include "logging.h"

void EvictFrom(LinkedList *list) {
    // Evict one
    PageInfo *victim = list->getTop()->page;
    list->removeTop();

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

                if (found->getList() != &nvm && nvm.getSize() == NVRAM_SIZE) {
                    EvictFrom(&nvm);
                    found->setPageType(DIRTY);
                    found->setList(&nvm);
                }

                found->getList()->addBack(new Node(found));
            } else if (u8OP == READ) {
                // READ
                gRTotal++;
                gRHit++;

                found->getList()->addBack(new Node(found));
            }
        } else {
            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);

            // Put into system map
            sysMap.insert(make_pair(uPage, newPage));

            if (u8OP == WRITE) {
                // WRITE
                gWTotal++;
                gWMiss++;

                if (nvm.getSize() == NVRAM_SIZE) {
                    EvictFrom(&nvm);
                }

                newPage->setPageType(DIRTY);
                newPage->setList(&nvm);
                newPage->getList()->addBack(new Node(newPage));
            } else if (u8OP == READ) {
                // READ
                gRTotal++;
                gRMiss++;

                if (dram.getSize() == DRAM_SIZE) {
                    EvictFrom(&dram);
                }

                newPage->setPageType(CLEAN);
                newPage->setList(&dram);
                newPage->getList()->addBack(new Node(newPage));
            }
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", gWMiss, gRMiss, gWHit, gRHit, gWTotal, gRTotal);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
