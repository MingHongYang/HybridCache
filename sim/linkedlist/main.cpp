#include "sim.h"
#include "LinkedList.h"

uintmax_t gCounter = 0;

int main() {
    LinkedList myList;

    PageInfo *tmp = new PageInfo(WRITE, 123222, DRAM, gCounter);
    Node *tmpNode = new Node();

    tmpNode->page = tmp;

    myList.addFront(tmpNode);

    cout << myList.getSize();

    tmp = new PageInfo(WRITE, 123, DRAM, gCounter);
    tmpNode = new Node();
    tmpNode->page = tmp;

    myList.addFront(tmpNode);

    tmpNode = myList.getTop();
    cout << myList.getSize() << endl;
    cout << tmpNode->page->getPageNumber() << endl;

    myList.removeTop();

    tmpNode = myList.getTop();
    cout << myList.getSize() << endl;
    cout << tmpNode->page->getPageNumber() << endl;

    tmp = new PageInfo(WRITE, 5555, DRAM, gCounter);
    Node *rmNode = new Node();
    rmNode->page = tmp;
    myList.addFront(rmNode);
    tmp = new PageInfo(WRITE, 4444, DRAM, gCounter);
    tmpNode = new Node();
    tmpNode->page = tmp;
    myList.addFront(tmpNode);

    cout << myList.getSize();
    myList.printList();
    myList.removeNode(rmNode);
    myList.printList();

    tmp = new PageInfo(WRITE, 666, DRAM, gCounter);
    tmpNode = new Node();
    tmpNode->page = tmp;
    myList.addBack(tmpNode);

    myList.printList();

    cout << myList.find(666) << endl;
    myList.remove(666);

    myList.removeBottom();
    myList.printList();

    return 0;
}
