#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "BPlusTree.hpp"
#include "Exceptions.hpp"
#include "InternalNode.hpp"
#include "LeafNode.hpp"
#include "Node.hpp"

BPlusTree::BPlusTree(int fd, int aOrder) : fOrder{aOrder}, fRoot{nullptr} {
    this->fd = fd;
    this->cur_page_num = 0;
    this->cur_elmt_num = 0;
}

bool BPlusTree::isEmpty() const
{
    return !fRoot;
}

// INSERTION

void BPlusTree::insert(KeyType aKey, ValueType aValue)
{
    if (isEmpty()) {
        startNewTree(aKey, aValue);
    } else {
        insertIntoLeaf(aKey, aValue);
    }
}

void BPlusTree::startNewTree(KeyType aKey, ValueType aValue) {
    this->cur_page_num = 1;
    this->cur_elmt_num = 1;
    LeafNode* newLeafNode = new LeafNode(fOrder);
    newLeafNode->createAndInsertRecord(aKey, aValue, this->fd, this->cur_page_num, this->cur_elmt_num);
    fRoot = newLeafNode;
}

void BPlusTree::insertIntoLeaf(KeyType aKey, ValueType aValue)
{
    LeafNode* leafNode = findLeafNode(aKey);
    if (!leafNode) {
        throw LeafNotFoundException(aKey);
    }
    this->cur_elmt_num += 1;
    if (this->cur_elmt_num > MAX_ELEM_NUM) {
        this->cur_page_num += 1;
        if (this->cur_page_num > INIT_PAGE_NUM) {
            if (ftruncate(this->fd, sysconf(_SC_PAGE_SIZE) * this->cur_page_num) == -1) {
                perror("Error expanding the file");
                close(this->fd);
                exit(EXIT_FAILURE);
            }
        }
        this->cur_elmt_num = 1;
    }
    int newSize = leafNode->createAndInsertRecord(aKey, aValue, this->fd, this->cur_page_num, this->cur_elmt_num);
    if (newSize > leafNode->maxSize()) {
        LeafNode* newLeaf = split(leafNode);
        newLeaf->setNext(leafNode->next());
        leafNode->setNext(newLeaf);
        KeyType newKey = newLeaf->firstKey();
        insertIntoParent(leafNode, newKey, newLeaf);
    }
}

void BPlusTree::insertIntoParent(Node *aOldNode, KeyType aKey, Node *aNewNode)
{
    InternalNode* parent = static_cast<InternalNode*>(aOldNode->parent());
    if (parent == nullptr) {
        fRoot = new InternalNode(fOrder);
        parent = static_cast<InternalNode*>(fRoot);
        aOldNode->setParent(parent);
        aNewNode->setParent(parent);
        parent->populateNewRoot(aOldNode, aKey, aNewNode);
    } else {
        int newSize = parent->insertNodeAfter(aOldNode, aKey, aNewNode);
        if (newSize > parent->maxSize()) {
            InternalNode* newNode = split(parent);
            KeyType newKey = newNode->replaceAndReturnFirstKey();
            insertIntoParent(parent, newKey, newNode);
        }
    }
}

template <typename T>
T* BPlusTree::split(T* aNode)
{
    T* newNode = new T(fOrder, aNode->parent());
    aNode->moveHalfTo(newNode);
    return newNode;
}


// REMOVAL


void BPlusTree::remove(KeyType aKey)
{
    if (isEmpty()) {
        return;
    } else {
        removeFromLeaf(aKey);
    }
}

void BPlusTree::removeFromLeaf(KeyType aKey)
{
    LeafNode* leafNode = findLeafNode(aKey);
    if (!leafNode) {
        return;
    }
    if (!leafNode->lookup(aKey)) {
        return;
    }
    int newSize = leafNode->removeAndDeleteRecord(aKey);
    if (newSize < leafNode->minSize()) {
        coalesceOrRedistribute(leafNode);
    }
}

template <typename N>
void BPlusTree::coalesceOrRedistribute(N* aNode)
{
    if (aNode->isRoot()) {
        adjustRoot();
        return;
    }
    auto parent = static_cast<InternalNode*>(aNode->parent());
    int indexOfNodeInParent = parent->nodeIndex(aNode);
    int neighborIndex = (indexOfNodeInParent == 0) ? 1 : indexOfNodeInParent - 1;
    N* neighborNode = static_cast<N*>(parent->neighbor(neighborIndex));
    if (aNode->size() + neighborNode->size() <= neighborNode->maxSize()) {
        coalesce(neighborNode, aNode, parent, indexOfNodeInParent);
    } else {
        redistribute(neighborNode, aNode, parent, indexOfNodeInParent);
    }
}

template <typename N>
void BPlusTree::coalesce(N* aNeighborNode, N* aNode, InternalNode* aParent, int aIndex)
{
    if (aIndex == 0) {
        std::swap(aNode, aNeighborNode);
        aIndex = 1;
    }
    aNode->moveAllTo(aNeighborNode, aIndex);
    aParent->remove(aIndex);
    if (aParent->size() < aParent->minSize()) {
        coalesceOrRedistribute(aParent);
    }
    delete aNode;
}

template <typename N>
void BPlusTree::redistribute(N* aNeighborNode, N* aNode, InternalNode* aParent, int aIndex)
{
    if (aIndex == 0) {
        aNeighborNode->moveFirstToEndOf(aNode);
    } else {
        aNeighborNode->moveLastToFrontOf(aNode, aIndex);
    }
}

void BPlusTree::adjustRoot()
{
    if (!fRoot->isLeaf() && fRoot->size() == 1) {
        auto discardedNode = static_cast<InternalNode*>(fRoot);
        fRoot = static_cast<InternalNode*>(fRoot)->removeAndReturnOnlyChild();
        fRoot->setParent(nullptr);
        delete discardedNode;
    } else if (!fRoot->size()){
        delete fRoot;
        fRoot = nullptr;
    }
}


// UTILITIES AND PRINTING

LeafNode* BPlusTree::findLeafNode(KeyType aKey, bool aPrinting, bool aVerbose)
{
    if (isEmpty()) {
        if (aPrinting) {
            std::cout << "Not found: empty tree." << std::endl;
        }
        return nullptr;
    }
    auto node = fRoot;
    if (aPrinting) {
        std::cout << "Root: ";
        if (fRoot->isLeaf()) {
            std::cout << "\t" << static_cast<LeafNode*>(fRoot)->toString(aVerbose);
        } else {
            std::cout << "\t" << static_cast<InternalNode*>(fRoot)->toString(aVerbose);
        }
        std::cout << std::endl;
    }
    while (!node->isLeaf()) {
        auto internalNode = static_cast<InternalNode*>(node);
        if (aPrinting && node != fRoot) {
            std::cout << "\tNode: " << internalNode->toString(aVerbose) << std::endl;
        }
        node = internalNode->lookup(aKey);
    }
    return static_cast<LeafNode*>(node);
}

void BPlusTree::readInputFromFile(std::string aFileName)
{
    int key;
    std::ifstream input(aFileName);
    while (input) {
        input >> key;
        insert(key, key);
    }
}

void BPlusTree::print(bool aVerbose)
{
    fPrinter.setVerbose(aVerbose);
    fPrinter.printTree(fRoot);
}

void BPlusTree::printLeaves(bool aVerbose)
{
    fPrinter.setVerbose(aVerbose);
    fPrinter.printLeaves(fRoot);
}

LeafNode* BPlusTree::find_first_leaf_node(Node* aRoot) {
    if (!aRoot) {
        return nullptr;
    }
    auto node = aRoot;
    while (!node->isLeaf()) {
        node = static_cast<InternalNode*>(node)->firstChild();
    }
    auto leafNode = static_cast<LeafNode*>(node);
    return leafNode;
}

std::pair<unsigned long, std::string> BPlusTree::print_leaves_string (int fd, std::set<std::pair<int, bool>, set_compare>& found_once) {
    return fPrinter.key_value_pairs(fRoot, fd, found_once);
}

void BPlusTree::destroyTree()
{
    if (fRoot->isLeaf()) {
        delete static_cast<LeafNode*>(fRoot);
    } else {
        delete static_cast<InternalNode*>(fRoot);
    }
    fRoot = nullptr;
}

void BPlusTree::printValue(KeyType aKey, bool aVerbose)
{
    printValue(aKey, false, aVerbose);
}

void BPlusTree::printValue(KeyType aKey, bool aPrintPath, bool aVerbose)
{
    LeafNode* leaf = findLeafNode(aKey, aPrintPath, aVerbose);
    if (!leaf) {
        //std::cout << "Leaf not found with key " << aKey << "." << std::endl;
        return;
    }
    if (aPrintPath) {
        std::cout << "\t";
    }
    std::cout << "Leaf: " << leaf->toString(aVerbose) << std::endl;
    Record* record = leaf->lookup(aKey);
    if (!record) {
        //std::cout << "Record not found with key " << aKey << "." << std::endl;
        return;
    }
    if (aPrintPath) {
        std::cout << "\t";
    }
    //std::cout << "Record found at location " << std::hex << record << std::dec << ":" << std::endl;
    //std::cout << "\tKey: " << aKey << "   Value: " << record->value() << std::endl;
}

std::string BPlusTree::getValue(KeyType aKey) {
    std::string rtn = "";
    LeafNode* leaf = findLeafNode(aKey, false, false);
    if (!leaf) {
        //std::cout << "LOGINFO:\t\t" << "Leaf not found with key " << aKey << "." << std::endl;
        return rtn;
    }
    Record* record = leaf->lookup(aKey);
    if (!record) {
        //std::cout << "LOGINFO:\t\t" << "Record not found with key " << aKey << "." << std::endl;
        return rtn;
    }
    //ValueType value = record->value();
    std::pair<int, int> addr = record->value();
#ifdef SYNC
    if (get_stop){
        pthread_exit(NULL);
    }
    get_stop = true;
#endif
    long* map = (long*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, sysconf(_SC_PAGE_SIZE) * (addr.first - 1));
    if (map == MAP_FAILED) {
        close(this->fd);
        perror("Error mmapping the file for insertion");
        exit(EXIT_FAILURE);
    }
    long value = map[addr.second - 1];
    if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
        perror("Error unmapping the file");
        close(fd);
        exit(EXIT_FAILURE);
    }
    std::stringstream out;
    out << value;
    rtn = out.str();
    return rtn;
}

void BPlusTree::printPathTo(KeyType aKey, bool aVerbose)
{
    printValue(aKey, true, aVerbose);
}

void BPlusTree::printRange(KeyType aStart, KeyType aEnd)
{
    std::cout << "THIS FUNCTION SHOULD NEVER BE CALLED!" << std::endl;
//    auto rangeVector = range(aStart, aEnd);
//    for (auto entry : rangeVector) {
//        std::cout << "Key: " << std::get<0>(entry);
//        std::cout << "    Value: " << std::get<1>(entry);
//        std::cout << "    Leaf: " << std::hex << std::get<2>(entry) << std::dec << std::endl;
//    }
}

std::string BPlusTree::getRange(KeyType aStart, KeyType aEnd) {
    std::string rtn = "";
    auto rangeVector = range(aStart, aEnd);
    for (auto entry : rangeVector) {
        std::stringstream key_ss;
        key_ss << std::get<0>(entry);
        std::stringstream value_ss;
        std::pair<int, int> addr = std::get<1>(entry);
        long* map = (long*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, sysconf(_SC_PAGE_SIZE) * (addr.first - 1));
        if (map == MAP_FAILED) {
            close(this->fd);
            perror("Error mmapping the file for insertion");
            exit(EXIT_FAILURE);
        }
        long value = map[addr.second - 1];
        if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
            perror("Error unmapping the file");
            close(this->fd);
            exit(EXIT_FAILURE);
        }
        value_ss << value;
        rtn += key_ss.str() +  ":" + value_ss.str() + " ";
    }
    return rtn;
}

std::vector<BPlusTree::EntryType> BPlusTree::range(KeyType aStart, KeyType aEnd)
{
    auto startLeaf = findLeafNode(aStart);
    auto endLeaf = findLeafNode(aEnd);
    std::vector<std::tuple<KeyType, std::pair<int, int>, LeafNode*>> entries;
    if (startLeaf == endLeaf) {
        if (aEnd <= startLeaf->smallest())
            return entries;
        else if (aStart > startLeaf->largest())
            return entries;
        else if (aStart == startLeaf->largest()) {
            startLeaf->copyLargest(entries);
            return entries;
        } else {
            startLeaf->copyWithinSameNode(aStart, aEnd, entries);
            return entries;
        }
    }
    if (!startLeaf || !endLeaf) {
        return entries;
    }
    startLeaf->copyRangeStartingFrom(aStart, entries);
    startLeaf = startLeaf->next();
    while (startLeaf != endLeaf) {
        startLeaf->copyRange(entries);
        startLeaf = startLeaf->next();
    }
    startLeaf->copyRangeUntil(aEnd, entries);
    return entries;
}

Node* BPlusTree::getRoot() {
    return this->fRoot;
}