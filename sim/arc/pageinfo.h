class LinkedList;

enum OPType {WRITE, READ};
enum MemType {DRAM, NVRAM};
enum PageType {DIRTY, CLEAN};
enum CacheType {RRR, RRG, RFR, RFG, WRR, WRG, WFR, WFG};

class PageInfo
{
    uintmax_t pageNumber;
    uintmax_t timeStamp;
    OPType eOPType;
    MemType eMemType;
    PageType ePageType;
    CacheType eCacheType;
    LinkedList *pCurList;

    public:

    bool operator = (const PageInfo *page) {
        pageNumber = page->pageNumber;
        timeStamp = page->timeStamp;
        eOPType = page->eOPType;
        eMemType = page->eMemType;
        ePageType = page->ePageType;
        eCacheType = page->eCacheType;
        pCurList = page->pCurList;
    }

    PageInfo(PageInfo *page) {
        pageNumber = page->pageNumber;
        timeStamp = page->timeStamp;
        eOPType = page->eOPType;
        eMemType = page->eMemType;
        ePageType = page->ePageType;
        eCacheType = page->eCacheType;
        pCurList = page->pCurList;
    }

    PageInfo(OPType opType, uintmax_t pageID, MemType type, CacheType cacheType, uintmax_t& curTimeStamp) : pageNumber(pageID), eMemType(type), eOPType(opType), eCacheType(cacheType) {
        curTimeStamp++;
        timeStamp = curTimeStamp;

        if (eOPType == WRITE) {
            ePageType = DIRTY;
        } else {
            ePageType = CLEAN;
        }
    }

    PageInfo(OPType op, uintmax_t pageID, uintmax_t &curTS) :pageNumber(pageID), eOPType(op) {
        timeStamp = ++curTS;
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
};
