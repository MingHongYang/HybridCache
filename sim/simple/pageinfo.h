class LinkedList;

enum OPType {WRITE, READ};
enum MemType {DRAM, NVRAM};
enum PageType {R, F};
enum CacheType {REAL, GHOST};
enum PageStatus {CLEAN, DIRTY};

class PageInfo
{
    uintmax_t pageNumber;
    uintmax_t timeStamp;
    int freq;
    OPType eOPType;
    MemType eMemType;
    PageType ePageType;
    CacheType eCacheType;
    LinkedList *pCurList;
    PageStatus ePageStatus;

    public:

    bool operator = (const PageInfo *page) {
        pageNumber = page->pageNumber;
        timeStamp = page->timeStamp;
        eOPType = page->eOPType;
        eMemType = page->eMemType;
        ePageType = page->ePageType;
        eCacheType = page->eCacheType;
        pCurList = page->pCurList;
        ePageStatus = page->ePageStatus;
        freq = page->freq;
    }

    PageInfo(PageInfo *page) {
        pageNumber = page->pageNumber;
        timeStamp = page->timeStamp;
        eOPType = page->eOPType;
        eMemType = page->eMemType;
        ePageType = page->ePageType;
        eCacheType = page->eCacheType;
        pCurList = page->pCurList;
        ePageStatus = page->ePageStatus;
        freq = page->freq;
    }

    PageInfo(OPType opType, uintmax_t pageID, MemType type, CacheType cacheType, PageStatus pageStatus, uintmax_t& curTimeStamp) : pageNumber(pageID), eMemType(type), eOPType(opType), eCacheType(cacheType), ePageStatus(pageStatus) {
        curTimeStamp++;
        timeStamp = curTimeStamp;

        freq = 1;
    }

    PageInfo(OPType op, uintmax_t pageID, uintmax_t &curTS) :pageNumber(pageID), eOPType(op) {
        timeStamp = ++curTS;
        eMemType = NVRAM;
        freq = 1;
    }

    uintmax_t getPageNumber() {
        return pageNumber;
    }

    OPType getOP() {
        return eOPType;
    }

    void setOP(OPType type) {
        eOPType = type;
    }

    uintmax_t getTimeStamp() {
        return timeStamp;
    }

    void setTimeStamp(uintmax_t& ts) {
        ts++;
        timeStamp = ts;
    }

    MemType getMemType() {
        return eMemType;
    }

    void setMemType(MemType type) {
        eMemType = type;
    }

    PageType getPageType() {
        return ePageType;
    }

    void setPageType(PageType type) {
        ePageType = type;
    }

    CacheType getCacheType() {
        return eCacheType;
    }

    void setCacheType(CacheType type) {
        eCacheType = type;
    }

    LinkedList* getList() {
        return pCurList;
    }

    void setList(LinkedList *list) {
        pCurList = list;
    }

    void pageHit() {
        freq++;
    }

    int getFreq() {
        return freq;
    }

    PageStatus getPageStatus() {
        return ePageStatus;
    }

    void setPageStatus(PageStatus status) {
        ePageStatus = status;
    }
};
