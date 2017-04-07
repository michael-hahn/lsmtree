
#ifndef bplustree_hpp
#define bplustree_hpp
#pragma once

#include <tuple>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Definitions.hpp"
#include "Printer.hpp"
#include "comp.h"

class InternalNode;
class LeafNode;
class Node;


/// Main class providing the API for the Interactive B+ Tree.
class BPlusTree
{
public:
    /// Sole constructor.  Accepts an optional order for the B+ Tree.
    /// The default order will provide a reasonable demonstration of the
    /// data structure and its operations.
    explicit BPlusTree(int fd, int aOrder = DEFAULT_ORDER);
    
    /// The type used in the API for inserting a new key-value pair
    /// into the tree.  The third item is the type of the Node into
    /// which the key will be inserted.
    using EntryType = std::tuple<KeyType, std::pair<int, int>, LeafNode*>;
    
    /// Returns true if this B+ tree has no keys or values.
    bool isEmpty() const;
    
    /// Insert a key-value pair into this B+ tree.
    void insert(KeyType aKey, ValueType aValue);
    
    /// Remove a key and its value from this B+ tree.
    void remove(KeyType aKey);
    
    /// Print this B+ tree to stdout using a simple command-line
    /// ASCII graphic scheme.
    /// @param[in] aVerbose Determins whether printing should include addresses.
    void print(bool aVerbose = false);
    
    /// Print the bottom rank of this B+ tree, consisting of its leaves.
    /// This shows all the keys in the B+ tree in sorted order.
    /// @param[in] aVerbose Determins whether printing should include addresses.
    void printLeaves(bool aVerbose = false);
    
    LeafNode* find_first_leaf_node(Node* aRoot);
    
    /// Same as printLeaves function
    /// Except it retruns a string instead of I/O
    std::pair<unsigned long, std::string> print_leaves_string (int fd, std::set<std::pair<int, bool>, set_compare>& found_once);
    
    /// Print the value associated with a given key, along with the address
    /// at which the tree stores that value.
    /// @param[in] aVerbose Determines whether printing should include addresses.
    void printValue(KeyType aKey, bool aVerbose = false);
    
    /// Get the value associated with a given key, if exists.
    /// Otherwise, return an empty string.
    std::string getValue(KeyType aKey);
    
    /// Print the path from the root to the leaf bearing key aKey.
    /// @param[in] aVerbose Determines whether printing should include addresses.
    void printPathTo(KeyType aKey, bool aVerbose = false);

    /// Print key, value, and address for each item in the range
    /// from aStart to aEnd, including both.
    void printRange(KeyType aStart, KeyType aEnd);
    
    /// Instead of printing out to the console, return a string of key:value pairs
    /// Functionality-wise, same as printRange
    std::string getRange(KeyType aStart, KeyType aEnd);

    /// Remove all elements from the B+ tree. You can then build
    /// it up again by inserting new elements into it.
    void destroyTree();
    
    /// Read elements to be inserted into the B+ tree from a text file.
    /// Each new element should consist of a single integer on a line by itself.
    /// This B+ tree treats each such input as both a new value and the key
    /// under which to store it.
    void readInputFromFile(std::string aFileName);
    
    Node* getRoot();
private:
    void startNewTree(KeyType aKey, ValueType aValue);
    void insertIntoLeaf(KeyType aKey, ValueType aValue);
    void insertIntoParent(Node* aOldNode, KeyType aKey, Node* aNewNode);
    template <typename T> T* split(T* aNode);
    void removeFromLeaf(KeyType aKey);
    template <typename N> void coalesceOrRedistribute(N* aNode);
    template <typename N> void coalesce(N* aNeighborNode, N* aNode, InternalNode* aParent, int aIndex);
    template <typename N> void redistribute(N* aNeighborNode, N* aNode, InternalNode* aParent, int aIndex);
    void adjustRoot();
    LeafNode* findLeafNode(KeyType aKey, bool aPrinting = false, bool aVerbose = false);
    void printValue(KeyType aKey, bool aPrintPath, bool aVerbose);
    std::vector<EntryType> range(KeyType aStart, KeyType aEnd);
    const int fOrder;
    Node* fRoot;
    Printer fPrinter;
    int fd;
    int cur_page_num;
    int cur_elmt_num;
};

#include "BPlusTree.cpp"
#endif
