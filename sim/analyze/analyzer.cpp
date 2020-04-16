#include "analyzer.h"

unordered_map<uintmax_t, vector<Trace>*> gRecord;
unordered_map<uintmax_t, vector<Trace>*> gWOnlyRecord;

//#define FENGGANG
// Usage: a.out input_file_name

int main(int argc, char *argv[]) {
    char filename[50];
    FILE *pInput = fopen(argv[1], "r");

    sprintf(filename, "%s.out", argv[2]);
    FILE *pOutput = fopen(filename, "w");
#ifdef FENGGANG
    FILE *pWOut = fopen(argv[3], "w");
#endif
    // New format for showing trace statistics
    sprintf(filename, "%s.rr", argv[2]);
    FILE *pRR = fopen(filename, "w");
    sprintf(filename, "%s.rw", argv[2]);
    FILE *pRW = fopen(filename, "w");
    sprintf(filename, "%s.wr", argv[2]);
    FILE *pWR = fopen(filename, "w");
    sprintf(filename, "%s.ww", argv[2]);
    FILE *pWW = fopen(filename, "w");

    uintmax_t line = 0;
    uintmax_t page = 0;
    int op = 0;
    uintmax_t reads = 0;
    uintmax_t writes = 0;
    uintmax_t rr = 0;
    uintmax_t rw = 0;
    uintmax_t wr = 0;
    uintmax_t ww = 0;
    uintmax_t intervals = 0;
    uintmax_t distances = 0;
    uintmax_t lastAccess = 0;
    int lastOperation = -1;
    Trace tempTrace;
    unordered_map<uintmax_t, vector<Trace>*>::iterator it;
    uintmax_t distance[9] = {0};
    uintmax_t wwDist[9] = {0};
    uintmax_t wrDist[9] = {0};
    uintmax_t rrDist[9] = {0};
    uintmax_t rwDist[9] = {0};

    while (!feof(pInput)) {

        // Read from file
        fscanf(pInput, "%" SCNuMAX " %d", &page, &op);

        tempTrace.operation = (op == 1) ? READ : WRITE;
        tempTrace.line = line;

        if (tempTrace.operation == WRITE) {
            Trace tmpWrite;
            writes++;
#ifdef FENGGANG
            tmpWrite.line = writes;

            // Add to Write only map
            it = gWOnlyRecord.find(page);
            if (it == gWOnlyRecord.end()) {
                gWOnlyRecord.insert(pair<int, vector<Trace>*>(page, new vector<Trace>));
            }
            gWOnlyRecord[page]->push_back(tmpWrite);
#endif
        } else {
            reads++;
        }
#ifndef FENGGANG
        it = gRecord.find(page);
        if (it == gRecord.end()) {
            gRecord.insert(pair<int, vector<Trace>*>(page, new vector<Trace>));
        }
        gRecord[page]->push_back(tempTrace);

        // Increase the line number
        line++;
#endif
    }
#ifndef FENGGANG
    // Write to file
    for (it = gRecord.begin(); it != gRecord.end(); it++) {
        //fprintf(pOutput, "%" SCNuMAX ": ", it->first);
        lastOperation = -1; // Clear the previous record
        lastAccess = 0;

        if (it->second->size() > 1) {
            // Calculate the stack distance and RR, RW, WW, WR
            for (vector<Trace>::iterator itVec = it->second->begin(); itVec != it->second->end(); itVec++) {
                //fprintf(pOutput, "%" SCNuMAX "/%d ", itVec->line, itVec->operation);

                if (lastOperation == -1) {
                    // Do nothing, will be assigned later
                } else if (lastOperation == 0) {
                    if (itVec->operation == 0) {
                        ww++;
                        fprintf(pWW, "%" SCNuMAX " %" SCNuMAX "\n", itVec->line, it->first);
                    } else {
                        wr++;
                        fprintf(pWR, "%" SCNuMAX " %" SCNuMAX "\n", itVec->line, it->first);
                    }
                } else if (lastOperation == 1) {
                    if (itVec->operation == 0) {
                        rw++;
                        fprintf(pRW, "%" SCNuMAX " %" SCNuMAX "\n", itVec->line, it->first);
                    } else {
                        rr++;
                        fprintf(pRR, "%" SCNuMAX " %" SCNuMAX "\n", itVec->line, it->first);
                    }
                } else {
                    // WTF?
                    exit(1);
                }

                if (lastAccess == 0) {
                    // Do nothing, will be assigned later
                } else {
                    intervals++;

                    // Record distribution
                    if (itVec->line - lastAccess < 1000) {
                        distance[0]++;

                        if (lastOperation == 0 && itVec->operation == 0) {
                            wwDist[0]++;
                        } else if (lastOperation == 0 && itVec->operation == 1) {
                            wrDist[0]++;
                        } else if (lastOperation == 1 && itVec->operation == 0) {
                            rwDist[0]++;
                        } else if (lastOperation == 1 && itVec->operation == 1) {
                            rrDist[0]++;
                        }
                    } else if (itVec->line - lastAccess < 2000) {
                        distance[1]++;

                        if (lastOperation == 0 && itVec->operation == 0) {
                            wwDist[1]++;
                        } else if (lastOperation == 0 && itVec->operation == 1) {
                            wrDist[1]++;
                        } else if (lastOperation == 1 && itVec->operation == 0) {
                            rwDist[1]++;
                        } else if (lastOperation == 1 && itVec->operation == 1) {
                            rrDist[1]++;
                        }
                    } else if (itVec->line - lastAccess < 4000) {
                        distance[2]++;

                        if (lastOperation == 0 && itVec->operation == 0) {
                            wwDist[2]++;
                        } else if (lastOperation == 0 && itVec->operation == 1) {
                            wrDist[2]++;
                        } else if (lastOperation == 1 && itVec->operation == 0) {
                            rwDist[2]++;
                        } else if (lastOperation == 1 && itVec->operation == 1) {
                            rrDist[2]++;
                        }
                    } else if (itVec->line - lastAccess < 8000) {
                        distance[3]++;

                        if (lastOperation == 0 && itVec->operation == 0) {
                            wwDist[3]++;
                        } else if (lastOperation == 0 && itVec->operation == 1) {
                            wrDist[3]++;
                        } else if (lastOperation == 1 && itVec->operation == 0) {
                            rwDist[3]++;
                        } else if (lastOperation == 1 && itVec->operation == 1) {
                            rrDist[3]++;
                        }
                    } else if (itVec->line - lastAccess < 16000) {
                        distance[4]++;

                        if (lastOperation == 0 && itVec->operation == 0) {
                            wwDist[4]++;
                        } else if (lastOperation == 0 && itVec->operation == 1) {
                            wrDist[4]++;
                        } else if (lastOperation == 1 && itVec->operation == 0) {
                            rwDist[4]++;
                        } else if (lastOperation == 1 && itVec->operation == 1) {
                            rrDist[4]++;
                        }
                    } else if (itVec->line - lastAccess < 32000) {
                        distance[5]++;

                        if (lastOperation == 0 && itVec->operation == 0) {
                            wwDist[5]++;
                        } else if (lastOperation == 0 && itVec->operation == 1) {
                            wrDist[5]++;
                        } else if (lastOperation == 1 && itVec->operation == 0) {
                            rwDist[5]++;
                        } else if (lastOperation == 1 && itVec->operation == 1) {
                            rrDist[5]++;
                        }
                    } else if (itVec->line - lastAccess < 64000) {
                        distance[6]++;

                        if (lastOperation == 0 && itVec->operation == 0) {
                            wwDist[6]++;
                        } else if (lastOperation == 0 && itVec->operation == 1) {
                            wrDist[6]++;
                        } else if (lastOperation == 1 && itVec->operation == 0) {
                            rwDist[6]++;
                        } else if (lastOperation == 1 && itVec->operation == 1) {
                            rrDist[6]++;
                        }
                    } else if (itVec->line - lastAccess < 128000) {
                        distance[7]++;

                        if (lastOperation == 0 && itVec->operation == 0) {
                            wwDist[7]++;
                        } else if (lastOperation == 0 && itVec->operation == 1) {
                            wrDist[7]++;
                        } else if (lastOperation == 1 && itVec->operation == 0) {
                            rwDist[7]++;
                        } else if (lastOperation == 1 && itVec->operation == 1) {
                            rrDist[7]++;
                        }
                    } else {
                        distance[8]++;

                        if (lastOperation == 0 && itVec->operation == 0) {
                            wwDist[8]++;
                        } else if (lastOperation == 0 && itVec->operation == 1) {
                            wrDist[8]++;
                        } else if (lastOperation == 1 && itVec->operation == 0) {
                            rwDist[8]++;
                        } else if (lastOperation == 1 && itVec->operation == 1) {
                            rrDist[8]++;
                        }
                    }

                    distances = distances + itVec->line - lastAccess;
                }

                // Set operation
                lastOperation = itVec->operation;
                lastAccess = itVec->line;
            }
        }

        //fprintf(pOutput, "\n");
    }
#endif
#ifdef FENGGANG
    // Write the gWOnluRecord to file
    for (it = gWOnlyRecord.begin(); it != gWOnlyRecord.end(); it++) {
        uintmax_t lastWrite = 0;
        uintmax_t intervalSquare = 0;
        uintmax_t interval = 0;
        uintmax_t medium = 0;

        if (it->second->size() > 1) {
            for (vector<Trace>::iterator itVec = it->second->begin(); itVec != it->second->end(); itVec++) {
                uintmax_t tmpInterval = itVec->line - lastWrite;

                if (lastWrite == 0) {
                    lastWrite = itVec->line;
                    continue;
                }

                interval = interval + tmpInterval;
                intervalSquare = intervalSquare + (tmpInterval * tmpInterval);
            }

            medium = it->second->at(it->second->size() / 2).line - it->second->at((it->second->size() / 2) - 1).line;

            fprintf(pWOut, "%" SCNuMAX " %d %" SCNuMAX " %" SCNuMAX " %" SCNuMAX "\n", it->first, it->second->size() - 1, interval, intervalSquare, medium);
        }
    }
#endif
#ifndef FENGGANG
    fprintf(pOutput, "Number of pages: %" SCNuMAX "\n", gRecord.size());
    fprintf(pOutput, "Read pages: %" SCNuMAX "\n", reads);
    fprintf(pOutput, "Write pages: %" SCNuMAX "\n", writes);
    fprintf(pOutput, "Average stack distance: %" SCNuMAX "\n", distances/intervals);
    fprintf(pOutput, "Intervals: %" SCNuMAX "\n", intervals);
    fprintf(pOutput, "RR: %f\n", (float) rr / (rr + rw + ww + wr));
    fprintf(pOutput, "RW: %f\n", (float) rw / (rr + rw + ww + wr));
    fprintf(pOutput, "WR: %f\n", (float) wr / (rr + rw + ww + wr));
    fprintf(pOutput, "WW: %f\n", (float) ww / (rr + rw + ww + wr));

    fprintf(pOutput, "Distance Distribution:\n");
    for (int i = 0; i < 9; i++) {
        fprintf(pOutput, "%d: %f\n", i, (float) distance[i] / intervals);
    }

    fprintf(pOutput, "Detailed Distribution:\n");
    for (int i = 0; i < 9; i++) {
        fprintf(pOutput, "%d: %f %f %f %f\n", i, (float) wwDist[i] / ww, (float) wrDist[i] / wr, (float) rrDist[i] / rr, (float) rwDist[i] / rw);
    }
#endif
    fclose(pInput);
    fclose(pOutput);

    fclose(pWW);
    fclose(pWR);
    fclose(pRW);
    fclose(pRR);
#ifdef FENGGANG
    fclose(pWOut);
#endif

    return 0;
}

