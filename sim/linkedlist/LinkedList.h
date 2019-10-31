#include <map>

class PageInfo;

struct Node {
    Node() {
        page = NULL;
        prev = NULL;
        next = NULL;
    };

    Node(PageInfo *newPage) : page(newPage) {
        prev = NULL;
        next = NULL;
    };

    ~Node() {
        free(page);
    }

    PageInfo *page;
    Node *prev;
    Node *next;
};

class LinkedList {
    public:
        // Constructor
        LinkedList();
        // Destructor
        virtual ~LinkedList();

        void addFront(Node *newNode);
        void addBack(Node *newNode);
        bool removeTop();
        bool removeBottom();
        Node* getTop();
        Node* getBottom();
        bool remove(uintmax_t pageNumber);
        bool removeNode(Node *victim);
        void removePage(Node *victim);
        int getSize();
        void printList();
        bool find(uintmax_t);
    private:
        Node *top;
        Node *bottom;
        int size;
        std::unordered_map<uintmax_t, Node *> mapping;
};

LinkedList::LinkedList() {
    top = NULL;
    bottom = NULL;
    size = 0;
}

LinkedList::~LinkedList() {
}

void LinkedList::addFront(Node *newNode) {
    if (size == 0) {
        // Empty list, add the new one
        top = newNode;
        bottom = newNode;
    } else {
        newNode->next = top;
        top->prev = newNode;
        
        top = newNode;
    }

    // Increase size
    size++;

    // Update map
    mapping.insert(std::make_pair(newNode->page->getPageNumber(), newNode));
}

void LinkedList::addBack(Node *newNode) {
    if (size == 0) {
        // Empty list, add the new one
        top = newNode;
        bottom = newNode;
    } else {
        newNode->prev = bottom;
        bottom->next = newNode;

        bottom = newNode;
    }

    // Increase size
    size++;

    // Update map
    mapping.insert(std::make_pair(newNode->page->getPageNumber(), newNode));
}

bool LinkedList::removeTop() {
    if (size == 0) {
        // Empty list, return false;
        return false;
    } else if (size == 1) {
        free(top);

        // Remove from map
        mapping.clear();

        top = NULL;
        bottom = NULL;
    } else {
        Node *temp = top;
        top = top->next;
        top->prev = NULL;

        // Remove from map
        mapping.erase(temp->page->getPageNumber());

        free(temp);
    }

    // Decrease size;
    size--;

    return true;
}

bool LinkedList::removeBottom() {
    if (size == 0) {
        // Empty list, return false;
        return false;
    } else if (size == 1) {
        free(bottom);

        // Remove from map
        mapping.clear();

        top = NULL;
        bottom = NULL;
    } else {
        Node *temp = bottom;
        bottom = bottom->prev;
        bottom->next = NULL;

        // Remove from map
        mapping.erase(temp->page->getPageNumber());

        free(temp);
    }

    // Decrease size;
    size--;

    return true;
}

Node* LinkedList::getTop() {
    return top;
}

Node* LinkedList::getBottom() {
    return bottom;
}

bool LinkedList::remove(uintmax_t pageNumber) {
    if (mapping.count(pageNumber)) {
        removeNode(mapping[pageNumber]);

        return true;
    }

    return false;
}

bool LinkedList::removeNode(Node *victim) {
    if (victim->prev != NULL) {
        victim->prev->next = victim->next;
    } else {
        // This is top
        top = victim->next;
    }

    if (victim->next != NULL) {
        victim->next->prev = victim->prev;
    } else {
        // This is bottom
        bottom = victim->prev;
    }

    // Remove from map
    mapping.erase(victim->page->getPageNumber());

    free(victim);

    // Decrease size
    size--;
}

void LinkedList::removePage(Node *victim) {
    free(victim->page);
}

int LinkedList::getSize() {
    return size;
}

void LinkedList::printList() {
    Node *tmp = top;

    while (tmp != NULL) {
        cout << tmp->page->getPageNumber() << " ";
        tmp = tmp->next;
    }

    cout << endl;
}

bool LinkedList::find(uintmax_t pageNumber) {
    if (mapping.count(pageNumber)) {
        return true;
    }

    return false;
}

