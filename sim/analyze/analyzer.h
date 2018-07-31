#include <unordered_map>
#include <vector>

#include <cinttypes>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

using namespace std;

enum OP_TYPE {
    ww = 0,
    wr = 1,
    rw = 2,
    rr = 3
};

class Trace {
    public:
        uintmax_t line;
        int operation; // 0 for write and 1 for read
};

