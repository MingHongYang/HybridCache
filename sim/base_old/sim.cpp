#include "sim.h"
#include "../linkedlist/LinkedList.h"

// System Hashmap
unordered_map<uintmax_t, Node *> sysMap;
// NVRAM Hashmap
unordered_map<uintmax_t, Node *> nvramMap;
// DRAM Hashmap
unordered_map<uintmax_t, Node *> dramMap;

// System vector
vector<PageInfo> vSystem;
// NVRAM vector
vector< vector<PageInfo> > vNVRAM(MAX_CON_READS*2);
// DRAM vector
vector<PageInfo> vDRAM;

// System list
LinkedList lSystem;
// NVRAM list
LinkedList aNVRAM[MAX_CON_READS * 2];
// DRAM list
LinkedList lDRAM;

uintmax_t gNumEvict = 0;
uintmax_t gNumFlush = 0;

uintmax_t gNumRR = 0;
uintmax_t gNumRW = 0;
uintmax_t gNumWR = 0;
uintmax_t gNumWW = 0;

uintmax_t gNumRonDRAM = 0;
uintmax_t gNumRonNVRAM = 0;
uintmax_t gNumWonDRAM = 0;
uintmax_t gNumWonNVRAM = 0;

uint32_t NVRAM_SIZE = 0;
uint32_t DRAM_SIZE = 0;
uint32_t SYS_SIZE = 0;

uintmax_t gWMiss = 0;
uintmax_t gRMiss = 0;

uintmax_t gWHit = 0;
uintmax_t gRHit = 0;

uintmax_t gWTotal = 0;
uintmax_t gRTotal = 0;

uintmax_t gWMissEvictNVRAM = 0;
uintmax_t gWMissEvictDRAM = 0;

// Counter used as unique time stamp
uintmax_t gCounter = 0;

uintmax_t CheckSize(MemType type) {
    uintmax_t listSize = 0;
    switch (type) {
        case DRAM:
            listSize = lDRAM.getSize();
            break;
        case NVRAM:
            for (int index = MAX_CON_READS * 2 - 1; index >= 0; index--) {
                listSize = listSize + aNVRAM[index].getSize();
            }

            break;
        case SYSTEM:
            listSize = lSystem.getSize();
            break;
    }

    return listSize;
}

#if DEBUG
void CheckVector() {
    vector<PageInfo>::iterator it;

    cout << "System map:" << endl;

    if (sysMap.size() != lSystem.getSize()) {
        cout << "System map error!" << endl;
        cout << sysMap.size() << " " << lSystem.getSize() << endl;
    }

    for (auto it = sysMap.begin(); it != sysMap.end(); ++it) {
        cout << it->first << " ";
    }
    cout << endl;

    cout << "DRAM map:" << endl;
    for (auto it = dramMap.begin(); it != dramMap.end(); ++it) {
        cout << it->first << " ";
    }
    cout << endl;

    cout << "NVRAM map:" << endl;
    for (auto it = nvramMap.begin(); it != nvramMap.end(); ++it) {
        cout << it->first << " ";
    }
    cout << endl;

    if (lSystem.getSize() > NVRAM_SIZE + DRAM_SIZE) {
        cerr << "System list error!" << endl;
    }

    if (lSystem.getSize() != 0) {
        cout << "System list:" << endl;
        lSystem.printList();
    } else {
        cout << "Empty" << endl;
    }

    if (CheckSize(NVRAM) > NVRAM_SIZE) {
        cerr << "NVRAM list error!" << endl;
    }

    for (int index = MAX_CON_READS * 2 - 1; index >= 0; index--) {
        cout << "NVRAM list " << index << ":" << endl;
        if (aNVRAM[index].getSize() != 0) {
            aNVRAM[index].printList();
        } else {
            cout << "Empty" << endl;
        }
    }

    if (lDRAM.getSize() > DRAM_SIZE) {
        cerr << "DRAM list error!" << endl;
    }

    if (lDRAM.getSize() != 0) {
        cout << "DRAM list:" << endl;
        lDRAM.printList();
    } else {
        cout << "Empty" << endl;
    }
}
#endif

void RemoveFromMap(MemType type, uintmax_t pageID) {
    unordered_map<uintmax_t, Node *>::const_iterator found;

    switch (type) {
        case NVRAM:
            found = nvramMap.find(pageID);
            if (found == nvramMap.end()) {
                // Not found, error
#if DEBUG
                cout << "Error from nvram map, can not find corresponding record" << endl;
                cout << "NVRAM map, PageID: " << pageID << endl;
                for (auto it = nvramMap.begin(); it != nvramMap.end(); ++it) {
                    cout << it->first << " ";
                }
                cout << endl;
#endif
            } else {
                nvramMap.erase(found);
            }
            break;
        case DRAM:
            found = dramMap.find(pageID);
            if (found == dramMap.end()) {
                // Not found, error
#if DEBUG
                cout << "Error from dram map, can not find corresponding record" << endl;
                cout << "DRAM map, PageID: " << pageID << endl;
                for (auto it = dramMap.begin(); it != dramMap.end(); ++it) {
                    cout << it->first << " ";
                }
                cout << endl;
#endif
            } else {
                dramMap.erase(found);
            }
            break;
        case SYSTEM:
            found = sysMap.find(pageID);
            if (found == sysMap.end()) {
                // Not found, error
#if DEBUG
                cout << "Error from system map, can not find corresponding record" << endl;
#endif
            } else {
                sysMap.erase(found);
            }
            break;
    }
}

void UpdateMap(MemType type, uintmax_t pageID, Node *pNode) {
    unordered_map<uintmax_t, Node *>::iterator found;
#if DEBUG
    cout << "Update map: " << type << " ID: " << pageID << endl;
#endif
    switch (type) {
        case NVRAM:
            found = nvramMap.find(pageID);
            if (found == nvramMap.end()) {
                // Not found, add new one
                nvramMap.insert({pageID, pNode});
            } else {
                found->second = pNode;
            }
            break;
        case DRAM:
            found = dramMap.find(pageID);
            if (found == dramMap.end()) {
                // Not found, add new one
                dramMap.insert({pageID, pNode});
            } else {
                found->second = pNode;
            }
            break;
        case SYSTEM:
            found = sysMap.find(pageID);
            if (found == sysMap.end()) {
                // Not found, add new one
                sysMap.insert({pageID, pNode});
            } else {
                found->second = pNode;
            }
            break;
    }
}

Node* FindNode(MemType type, uintmax_t pageID) {
    unordered_map<uintmax_t, Node *>::const_iterator found;
    Node *foundNode = NULL;

    switch (type) {
        case NVRAM:
            found = nvramMap.find(pageID);
            if (found != nvramMap.end()) {
                foundNode = found->second;
            }
            break;
        case DRAM:
            found = dramMap.find(pageID);
            if (found != dramMap.end()) {
                foundNode = found->second;
            }
            break;
        case SYSTEM:
            found = sysMap.find(pageID);
            if (found != sysMap.end()) {
                foundNode = found->second;
            }
            break;
    }

#if DEBUG
    if (foundNode != NULL) {
        cout << "Found: " << found->first << " in " << found->second->page->getMemType() << " id: " << found->second->page->getPageNumber() << endl;
    }
#endif
    return foundNode;
}

bool MigratePage() {
    // This function migrates a page from NVRAM to DRAMr
    Node *ramNode, *newRamNode = new Node();
#if DEBUG
    CheckVector();
#endif
    for (int index = MAX_CON_READS * 2 - 1; index >= 0; index--) {
        if (aNVRAM[index].getSize() > 0) {
            ramNode = aNVRAM[index].getTop();
#if DEBUG
            cout << "Migrate Page: " << ramNode->page->getPageNumber() << endl;
#endif

            // Set the page
            ramNode->page->setMemType(DRAM);
            ramNode->page->clrReads();
            if (ramNode->page->getDirty() == DIRTY) {
                gNumFlush++;
                ramNode->page->setDirty(CLEAN);
            }

            newRamNode->page = ramNode->page;

            // Add new node to dram, update dram map
            lDRAM.addBack(*newRamNode);
            UpdateMap(DRAM, newRamNode->page->getPageNumber(), newRamNode);

            // Remove node from nvram, remove it from nvram map
            aNVRAM[index].removeTop();
            RemoveFromMap(NVRAM, newRamNode->page->getPageNumber());

            return true;
        }
    }

    return false;
}

MemType EvictPage(void) {
    // Evict one from system queue and return the type of evicted page
    MemType evictedPageType = lSystem.getTop()->page->getMemType();
    Node *found;
    // Update statistics
    if (lSystem.getTop()->page->getDirty() == DIRTY) {
        // Flush page
        gNumFlush++;
    }
    gNumEvict++;

#if DEBUG
    cout << "Page being evicted: " << lSystem.getTop()->page->getPageNumber() << " type: " << lSystem.getTop()->page->getMemType() << endl;
#endif

    // Remove it from corresponding queue
    if (evictedPageType == NVRAM) {
        found = FindNode(NVRAM, lSystem.getTop()->page->getPageNumber());
        if (lSystem.getTop()->page->getDirty() == DIRTY) {
            aNVRAM[lSystem.getTop()->page->getu8Reads() * 2].removeNode(*found);
        } else {
            aNVRAM[lSystem.getTop()->page->getu8Reads() * 2 + CLEAN_PAGE_OFFSET].removeNode(*found);
        }

        // Remove from nvram map
        RemoveFromMap(NVRAM, lSystem.getTop()->page->getPageNumber());
    } else if (evictedPageType == DRAM) {
        found = FindNode(DRAM, lSystem.getTop()->page->getPageNumber());
        lDRAM.removeNode(*found);

        // Remove from dram map
        RemoveFromMap(DRAM, lSystem.getTop()->page->getPageNumber());
    }

    // Remove from system map
    RemoveFromMap(SYSTEM, lSystem.getTop()->page->getPageNumber());
    // Remove it from system queue
    lSystem.removePage(*(lSystem.getTop()));
    lSystem.removeTop();

    return evictedPageType;
}

int main(int argc, char* argv[]) {
    // Open file
    FILE *pfInput= fopen(argv[1], "r");
    FILE *pfOutput = fopen(argv[2], "w");
    NVRAM_SIZE = stoi(argv[3]);
    DRAM_SIZE = stoi(argv[4]);
    SYS_SIZE = NVRAM_SIZE + DRAM_SIZE;

    if (!pfInput || !pfOutput) {
        // Error opening files
        printf("File name error\n");

        return ENOENT;
    }

    while (!feof(pfInput)) {
        uintmax_t uPage = 0;
        uint8_t u8OP = 0;
        vector<PageInfo>::iterator itSystem, itRAM;

        // Read the log
        fscanf(pfInput, "%" SCNuMAX " %" SCNu8, &uPage, &u8OP);

#if DEBUG
        cout << endl;
        cout << "-----" << endl;
        cout << "Page info:" << endl;
        cout << "OP: " << (bool)u8OP << " PageID: " << uPage << endl;
        cout << "System info:" << endl;
        cout << "S: " << lSystem.getSize() << " NVRAM: " << CheckSize(NVRAM) << " DRAM: " << lDRAM.getSize() << endl;
        CheckVector();
#endif

        if (u8OP == WRITE) {
            // Write pages
            gWTotal++;
            // Prepare new page
            PageInfo *newPage = new PageInfo(WRITE, uPage, NVRAM, gCounter);
            // Prepare new node
            Node *newSysNode = new Node();
            Node *newRamNode = new Node();

            newSysNode->page = newPage;
            newRamNode->page = newPage;

#if DEBUG
            cout << "Write page:" << endl;
#endif
            // Find corresponding node in system list
            Node *sysNode = FindNode(SYSTEM, newPage->getPageNumber());

            if (sysNode != NULL) {
                // Cache hit
                Node *ramNode;
#if DEBUG
                cout << "Cache hit (W)" << endl;
#endif
                // Statistics
                if (sysNode->page->getOP() == WRITE) {
                    gNumWW++;
                } else {
                    gNumRW++;
                }

                // Find corresponding page in ram list
                if (sysNode->page->getMemType() == DRAM) {
                    // Find from DRAM
                    ramNode = FindNode(DRAM, newPage->getPageNumber());

                    // Write miss
                    gWMiss++;

                    // Update statistics
                    gNumWonDRAM++;

                    if (CheckSize(NVRAM) == NVRAM_SIZE) {
                        // NVRAM full, evict the last one
                        Node *tempRamNode = FindNode(SYSTEM, aNVRAM[0].getTop()->page->getPageNumber());
                        RemoveFromMap(SYSTEM, tempRamNode->page->getPageNumber());
                        RemoveFromMap(NVRAM, tempRamNode->page->getPageNumber());
                        aNVRAM[0].removeTop();
                        lSystem.removePage(*tempRamNode);
                        lSystem.removeNode(*tempRamNode);

                        // Update statistics
                        gNumFlush++;
                        gNumEvict++;
                    }

                    // Remove from dram map
                    RemoveFromMap(DRAM, ramNode->page->getPageNumber());
                    // Remove from system list and dram list, remove page when removing from system list
                    lSystem.removePage(*sysNode);
                    lSystem.removeNode(*sysNode);
                    lDRAM.removeNode(*ramNode);

                    // Update system map
                    UpdateMap(SYSTEM, newSysNode->page->getPageNumber(), newSysNode);
                    // Put the new page to system list
                    lSystem.addBack(*newSysNode);
                    // Update nvram map
                    UpdateMap(NVRAM, newRamNode->page->getPageNumber(), newRamNode);
                    // Put the new page to nvram list
                    aNVRAM[0].addBack(*newRamNode);
                } else {
                    // Find from NVRAM
                    ramNode = FindNode(NVRAM, newPage->getPageNumber());

                    // Update statistics
                    gNumWonNVRAM++;

                    // Write hit
                    gWHit++;

                    // Remove from system list and nvram list, remove page when removing from system list
                    lSystem.removePage(*sysNode);
                    lSystem.removeNode(*sysNode);

                    // Remove from nvram list
                    aNVRAM[0].removeNode(*ramNode);

                    // Add back the new page to system list
                    lSystem.addBack(*newSysNode);
                    // Update system map
                    UpdateMap(SYSTEM, newSysNode->page->getPageNumber(), newSysNode);
                    // Add back the new page to nvram, 0 is for no consecutive read dirty pages
                    aNVRAM[0].addBack(*newRamNode);
                    // Update nvram map
                    UpdateMap(NVRAM, newRamNode->page->getPageNumber(), newRamNode);
                }
            } else {
                // Cache miss
#if DEBUG
                cout << "Cache miss (W)" << endl;
#endif
                // Update miss count
                gWMiss++;

                if (CheckSize(NVRAM) == NVRAM_SIZE) {
                    // NVRAM full, evict the last one
                    Node *tempRamNode = FindNode(SYSTEM, aNVRAM[0].getTop()->page->getPageNumber());
                    RemoveFromMap(SYSTEM, tempRamNode->page->getPageNumber());
                    RemoveFromMap(NVRAM, tempRamNode->page->getPageNumber());
                    aNVRAM[0].removeTop();
                    lSystem.removePage(*tempRamNode);
                    lSystem.removeNode(*tempRamNode);

                    // Update statistics
                    gNumFlush++;
                    gNumEvict++;
                }

                // Add the new page to system list
                lSystem.addBack(*newSysNode);
                // Update system map
                UpdateMap(SYSTEM, newSysNode->page->getPageNumber(), newSysNode);
                // Add the new page to nvram, 0 is for no consecutive read dirty pages
                aNVRAM[0].addBack(*newRamNode);
                // Update nvram map
                UpdateMap(NVRAM, newRamNode->page->getPageNumber(), newRamNode);
            }
        } else {
            // Read pages
            gRTotal++;
            // Prepare new page
            PageInfo *newPage = new PageInfo(READ, uPage, DRAM, gCounter);
            // Prepare new node
            Node *newSysNode = new Node();
            Node *newRamNode = new Node();

            newSysNode->page = newPage;
            newRamNode->page = newPage;
#if DEBUG
            cout << "Read page:" << endl;
#endif
            // Find corresponding node in system list
            Node *sysNode = FindNode(SYSTEM, newPage->getPageNumber());

            if (sysNode != NULL) {
                // Cache hit
                Node *ramNode;
#if DEBUG
                cout << "Cache hit (R)" << endl;
#endif
                // Statistics
                if (sysNode->page->getOP() == WRITE) {
                    gNumWR++;
                } else {
                    gNumRR++;
                }

                // Read hit
                gRHit++;

                // Find corresponding page in ram list
                if (sysNode->page->getMemType() == DRAM) {
                    // Find from DRAM
                    ramNode = FindNode(DRAM, newPage->getPageNumber());

                    // Update statistics
                    gNumRonDRAM++;

                    // Remove from system list and dram list, remove page when removing from system list
                    lSystem.removePage(*sysNode);
                    lSystem.removeNode(*sysNode);
                    lDRAM.removeNode(*ramNode);

                    // Put the new page to system list and dram list
                    lSystem.addBack(*newSysNode);
                    lDRAM.addBack(*newRamNode);

                    // Update maps
                    UpdateMap(SYSTEM, newSysNode->page->getPageNumber(), newSysNode);
                    UpdateMap(DRAM, newRamNode->page->getPageNumber(), newRamNode);
                } else {
                    // Find from NVRAM
                    ramNode = FindNode(NVRAM, newPage->getPageNumber());

                    // Update statistics
                    gNumRonNVRAM++;

                    // Remove and add back the new node, remove page when removing from system list
                    RemoveFromMap(SYSTEM, sysNode->page->getPageNumber());
                    // Remove from nvram
                    RemoveFromMap(NVRAM, ramNode->page->getPageNumber());
                    // Remove from system list
                    lSystem.removePage(*sysNode);
                    lSystem.removeNode(*sysNode);

                    // These are special code for baseline, we do not move pages from NVRAM when read hit
                    newSysNode->page->setMemType(NVRAM);
                    newRamNode->page->setMemType(NVRAM);

                    // Add back to system
                    lSystem.addBack(*newSysNode);

                    // Update system map
                    UpdateMap(SYSTEM, newSysNode->page->getPageNumber(), newSysNode);

                    // Remove from nvram list
                    aNVRAM[0].removeNode(*ramNode);

                    // Read hit on NVRAM does not flush it to DRAM
#if 0
                    // Update statistics
                    gNumFlush++;
                    gNumEvict++;

                    // Add back to dram
                    if (CheckSize(DRAM) == DRAM_SIZE) {
                        // DRAM full, evict one from dram
                        Node *tempRamNode = FindNode(SYSTEM, lDRAM.getTop()->page->getPageNumber());
                        RemoveFromMap(SYSTEM, tempRamNode->page->getPageNumber());
                        RemoveFromMap(DRAM, tempRamNode->page->getPageNumber());
                        lDRAM.removeTop();
                        lSystem.removePage(*tempRamNode);
                        lSystem.removeNode(*tempRamNode);

                        gNumEvict++;
                    }
#endif
                    // Add the new page to nvram, 0 is for no consecutive read dirty pages
                    aNVRAM[0].addBack(*newRamNode);
                    // Update nvram map
                    UpdateMap(NVRAM, newRamNode->page->getPageNumber(), newRamNode);
                }
            } else {
                // Cache miss
#if DEBUG
                cout << "Cache miss (R)" << endl;
#endif
                // Update read miss
                gRMiss++;

                if (CheckSize(DRAM) == DRAM_SIZE) {
                    // DRAM full, evict one from dram
                    Node *tempRamNode = FindNode(SYSTEM, lDRAM.getTop()->page->getPageNumber());
                    RemoveFromMap(SYSTEM, tempRamNode->page->getPageNumber());
                    RemoveFromMap(DRAM, tempRamNode->page->getPageNumber());
                    lDRAM.removeTop();
                    lSystem.removePage(*tempRamNode);
                    lSystem.removeNode(*tempRamNode);

                    gNumEvict++;
                }

                // Put the new page in DRAM
                lSystem.addBack(*newSysNode);
                lDRAM.addBack(*newRamNode);

                // Update maps
                UpdateMap(SYSTEM, newSysNode->page->getPageNumber(), newSysNode);
                UpdateMap(DRAM, newRamNode->page->getPageNumber(), newRamNode);
            }
        }
#if DEBUG
        CheckVector();
#endif
    }

    //fprintf(pfOutput, "Eviction Count: %" SCNuMAX " Flush Count: %" SCNuMAX "\n", gNumEvict, gNumFlush);
    //fprintf(pfOutput, "RR: %" SCNuMAX " RW: %" SCNuMAX " WW: %" SCNuMAX " WR: %" SCNuMAX "\n", gNumRR, gNumRW, gNumWW, gNumWR);
    //fprintf(pfOutput, "Read on DRAM: %" SCNuMAX "\nRead on NVRAM: %" SCNuMAX "\nWrite on DRAM: %" SCNuMAX "\nWrite on NVRAM: %" SCNuMAX, gNumRonDRAM, gNumRonNVRAM, gNumWonDRAM, gNumWonNVRAM);

    fprintf(pfOutput, "%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%"     SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX "\n%" SCNuMAX, gNumEvict, gNumFlush, gNumRR, gNumRW, gNumWW, gNumWR, gNumRonDRAM, gNumRonNVRAM, gNumWonDRAM, gNumWonNVRAM, gWMiss, gRMiss, gWMissEvictNVRAM,      gWMissEvictDRAM, gWHit, gRHit, gWTotal, gRTotal);

    // Close file
    fclose(pfInput);
    fclose(pfOutput);

    return true;
}
