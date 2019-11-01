#include "sim.h"
#include "logging.h"

int main(int argc, char* argv[]) {
    // Files
    FILE *pfInput= fopen(argv[1], "r");
    FILE *pfOutput = fopen(argv[2], "a");
    NVRAM_SIZE = stoi(argv[3]);
    DRAM_SIZE = stoi(argv[4]);
    TOTAL_SIZE = NVRAM_SIZE + DRAM_SIZE;

    unordered_map<uintmax_t, queue<uintmax_t> *> nextRequest;

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

        if (nextRequest.count(uPage) == 0) {
            nextRequest[uPage] = new queue<uintmax_t>();
        }

        if (u8OP == READ) {
            nextRequest[uPage]->push(gTimeStamp);
        }

        gTimeStamp++;
    }

    for (auto i = nextRequest.begin(); i != nextRequest.end(); i++) {
        i->second->push(gTimeStamp);
    }

    fseek(pfInput, 0, SEEK_SET);
    gTimeStamp = 0;

    while (!feof(pfInput)) {
        uintmax_t uPage = 0;
        uint8_t u8OP = 0;

        // Read trace
        fscanf(pfInput, "%" SCNuMAX " %" SCNu8, &uPage, &u8OP);
        gTotal++;

        // Check if it's in the cache
        if (sysMap.count(uPage) != 0) {
            // In the cache
            if (u8OP == READ) {
                PageInfo *found = sysMap[uPage];
                found->getList()->remove(uPage);
                found->setTimeStamp(gTimeStamp);
                // Update heap
                assert(nextRequest[uPage]->front() == gTimeStamp);
                found->getQueue()->pop();
                gHeap.pageHit(found->getIndex());
                gHit++;
                found->getList()->addBack(new Node(found));
            } else {
                gFlush++;
            }
        } else {
            // Cache miss, prepare new page
            PageInfo *newPage = new PageInfo((OPType)u8OP, uPage, gTimeStamp);
            gMiss++;

            if (dram.getSize() == DRAM_SIZE) {
                // Remove one based on max heap
                PageInfo *tmp = gHeap.top();

                dram.remove(tmp->getPageNumber());
                sysMap.erase(tmp->getPageNumber());
                gHeap.pop();

                free(tmp);
            }

            // Update page
            newPage->setQueue(nextRequest[uPage]);

            // Update queue
            if (u8OP == READ) {
                nextRequest[uPage]->pop();
            } else {
                gFlush++;
            }

            // Push into max heap
            gHeap.insert(newPage);

            // Put into system map
            sysMap.insert(make_pair(uPage, newPage));

            newPage->setList(&dram);
            newPage->getList()->addBack(new Node(newPage));
        }
    }

    fprintf(pfOutput, "%" SCNuMAX " %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", gMiss, gHit, gFlush, gTotal);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
