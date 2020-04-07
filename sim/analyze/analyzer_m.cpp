#include "analyzer.h"
#include <iostream>

unordered_map<uintmax_t, vector<uintmax_t>*> gTrace;

// Usage: a.out input_file_name output_file_name window_size

int main(int argc, char *argv[]) {
    FILE *pInput = fopen(argv[1], "r");
    FILE *pOutput = fopen(argv[2], "w");
    int size = atoi(argv[3]);
    
    int operation;
    uintmax_t line = 0;
    uintmax_t page = 0;
    uintmax_t intervals = 0;
    uintmax_t distances = 0;
    unordered_map<uintmax_t, vector<uintmax_t>*>::iterator it;

    while (!feof(pInput)) {
        // Read from file
        fscanf(pInput, "%" SCNuMAX " %d", &page, &operation);

        if (operation == READ) {
            it = gTrace.find(page);
            if (it == gTrace.end()) {
                gTrace.insert(pair<int, vector<uintmax_t>*>(page, new vector<uintmax_t>));
            }

            gTrace[page]->push_back(line);

            // Increase the line number
            line++;
        }

        // Output
        if (line % size == 0) {
        }
    }

    cout << "Parsing file done..." << endl;


    // Write to file
    for (it = gTrace.begin(); it != gTrace.end(); it++) {
        uintmax_t lastWrite = 0;
        uintmax_t interval = 0;

        if (it->second->size() > 1) {
            for (vector<uintmax_t>::iterator itVec = it->second->begin(); itVec != it->second->end(); itVec++) {
                uintmax_t tmpInterval = *itVec - lastWrite;

                if (lastWrite == 0) {
                    lastWrite = *itVec;
                    continue;
                }

                interval = interval + tmpInterval;
            }

            fprintf(pOutput, "%" SCNuMAX " %f\n", it->first, (double) interval / (it->second->size() - 1));
        }
    }

    fclose(pInput);
    fclose(pOutput);

    return 0;
}

