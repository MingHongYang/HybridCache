#include "sim.h"
#include "logging.h"

void Evict() {
    // Evict one page based on rw LRU
    PageInfo *victim = rw.getTop()->page;
    rw.removeTop();

    // Update associated counts
    if (victim->getMemType() == DRAM) {
        dramCnt--;
    } else {
        nvmCnt--;
    }

    // Update flush counts
    if (victim->getPageStatus() == DIRTY) {
        gFlush++;
    }

    sysMap.erase(victim->getPageNumber());
    free(victim);
}

void Migrate() {
    // Migrate a page from NVM to DRAM
    PageInfo *victim;
    Node *vicNode = wo.getTop();

    while (vicNode->page->getMemType() != NVRAM) {
        vicNode = vicNode->next;
    }

    victim = vicNode->page;

    // Put this page to DRAM
    victim->setMemType(DRAM);

    if (victim->getPageStatus() == DIRTY) {
        gFlush++;
        victim->setPageStatus(CLEAN);
    }

    nvmCnt--;
    dramCnt++;
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
            PageInfo *found = sysMap[uPage];

            rw.remove(uPage);
            rw.addBack(new Node(found));

            if (u8OP == WRITE) {
                // Update the write LRU
                wo.remove(uPage);
                wo.addBack(new Node(found));

                // Check if the page should be flushed and update page status
                if (found->getMemType() == NVRAM) {
                    // NVRAM can cache dirty pages
                    if (found->getPageStatus() == DIRTY) {
                        gWHit++;
                    } else {
                        found->setPageStatus(DIRTY);
                    }
                } else {
                    // Write on DRAM doesn't count as hit
                    gFlush++;
                }
            } else {
                // Reads are always hit
                gRHit++;
            }
        } else {
            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);

            // Put into system map
            sysMap.insert(make_pair(uPage, newPage));

            // Check if cache is full, new page always goes to NVRAM
            if (nvmCnt == NVRAM_SIZE) {
                // Migrate one from NVM to DRAM
                if (dramCnt == DRAM_SIZE) {
                    // Evict one from rw LRU first
                    Evict();
                }
                
                // Migrate one to DRAM
                if (nvmCnt == NVRAM_SIZE) {
                    Migrate();
                }
            }

            if (u8OP == WRITE) {
                newPage->setPageStatus(DIRTY);
            } else {
                newPage->setPageStatus(CLEAN);
            }

            rw.addBack(new Node(newPage));
            wo.addBack(new Node(newPage));
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", gWHit, gRHit, gTotal, gFlush);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
