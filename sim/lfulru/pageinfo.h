class LinkedList;

enum OPType {WRITE, READ};
enum MemType {DRAM, NVRAM};
enum PageType {R, F};
enum CacheType {REAL, GHOST};

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
    LinkedList *pListR;
    LinkedList *pListF;

    public:

    bool operator = (const PageInfo *page) {
        pageNumber = page->pageNumber;
        timeStamp = page->timeStamp;
        eOPType = page->eOPType;
        eMemType = page->eMemType;
        ePageType = page->ePageType;
        eCacheType = page->eCacheType;
        pCurList = page->pCurList;
        freq = page->freq;
        pListR = page->pListR;
        pListF = page->pListF;
    }

    PageInfo(PageInfo *page) {
        pageNumber = page->pageNumber;
        timeStamp = page->timeStamp;
        eOPType = page->eOPType;
        eMemType = page->eMemType;
        ePageType = page->ePageType;
        eCacheType = page->eCacheType;
        pCurList = page->pCurList;
        freq = page->freq;
        pListR = page->pListR;
        pListF = page->pListF;
    }

    PageInfo(OPType opType, uintmax_t pageID, MemType type, CacheType cacheType, uintmax_t& curTimeStamp) : pageNumber(pageID), eMemType(type), eOPType(opType), eCacheType(cacheType) {
        curTimeStamp++;
        timeStamp = curTimeStamp;

        freq = 1;
    }

    PageInfo(OPType op, uintmax_t pageID, uintmax_t &curTS) :pageNumber(pageID), eOPType(op) {
        timeStamp = ++curTS;
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

    LinkedList* getListR() {
        return pListR;
    }

    void setListR(LinkedList *list) {
        pListR = list;
    }

    LinkedList* getListF() {
        return pListF;
    }

    void setListF(LinkedList *list) {
        pListF = list;
    }

    void pageHit(int maxFreq) {
        if (freq < maxFreq)
            freq++;
    }

    int getFreq() {
        return freq;
    }
};
