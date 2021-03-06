//
//  LeafNode.hpp
//  BPlusTree.2a
//
//  Created by Amittai Aviram on 6/12/16.
//  Copyright © 2016 Amittai Aviram. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef leafnode_hpp
#define leafnode_hpp
#pragma once

#include <tuple>
#include <utility>
#include <vector>
#include "Node.hpp"
#include "Record.hpp"
#include "comp.h"

class LeafNode : public Node
{
public:
    explicit LeafNode(int aOrder);
    explicit LeafNode(int aOrder, Node* aParent);
    ~LeafNode() override;
    using MappingType = std::pair<KeyType, Record*>;
    using EntryType = std::tuple<KeyType, std::pair<int, int>, LeafNode*>;
    bool isLeaf() const override;
    LeafNode* next() const;
    void setNext(LeafNode* aNext);
    int size() const override;
    int minSize() const override;
    int maxSize() const override;
    int createAndInsertRecord(KeyType aKey, ValueType aValue, int fd, int page_num, int elmt_num);
    void insert(KeyType aKey, Record* aRecord);
    Record* lookup(KeyType aKey) const;
    int removeAndDeleteRecord(KeyType aKey);
    KeyType firstKey() const;
    void moveHalfTo(LeafNode* aRecipient);
    void moveAllTo(LeafNode* aRecipient, int /* Unused */);
    void moveFirstToEndOf(LeafNode* aRecipient);
    void moveLastToFrontOf(LeafNode* aRecipient, int aParentIndex);
    void copyRangeStartingFrom(KeyType aKey, std::vector<EntryType>& aVector);
    KeyType smallest();
    KeyType largest();
    void copyLargest(std::vector<EntryType>& aVector);
    void copyWithinSameNode(KeyType aStart, KeyType aEnd, std::vector<EntryType>& aVector);
    void copyRangeUntil(KeyType aKey, std::vector<EntryType>& aVector);
    void copyRange(std::vector<EntryType>& aVector);
    std::string toString(bool aVerbose = false) const override;
    std::pair<unsigned long, std::string> key_value_pairs(int fd, std::set<std::pair<int, bool>, set_compare>& found_once) const;
    std::vector<std::pair<KeyType, Record*>> get_mappings();
private:
    void copyHalfFrom(std::vector<MappingType>& aMappings);
    void copyAllFrom(std::vector<MappingType>& aMappings);
    void copyLastFrom(MappingType aPair);
    void copyFirstFrom(MappingType aPair, int aParentIndex);
    std::vector<MappingType> fMappings;
    LeafNode* fNext;
};

#include "LeafNode.cpp"
#endif