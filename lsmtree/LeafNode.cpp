//
//  LeafNode.cpp
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

#include <sstream>
#include "Exceptions.hpp"
#include "InternalNode.hpp"
#include "LeafNode.hpp"

LeafNode::LeafNode(int aOrder) : fNext{nullptr}, Node(aOrder) {}

LeafNode::LeafNode(int aOrder, Node* aParent) : fNext{nullptr}, Node(aOrder, aParent) {}

LeafNode::~LeafNode()
{
    for (auto mapping : fMappings) {
        delete mapping.second;
    }
}

bool LeafNode::isLeaf() const
{
    return true;
}

LeafNode* LeafNode::next() const
{
    return fNext;
}

void LeafNode::setNext(LeafNode* aNext)
{
    fNext = aNext;
}

int LeafNode::size() const
{
    return static_cast<int>(fMappings.size());
}

int LeafNode::minSize() const
{
    return order()/2;
}

int LeafNode::maxSize() const
{
    return order() - 1;
}

std::string LeafNode::toString(bool aVerbose) const
{
    std::ostringstream keyToTextConverter;
    if (aVerbose) {
        keyToTextConverter << "[" << std::hex << this << std::dec << "]<" << fMappings.size() << "> ";
    }
    bool first = true;
    for (auto mapping : fMappings) {
        if (first) {
            first = false;
        } else {
            keyToTextConverter << " ";
        }
        keyToTextConverter << mapping.first;
    }
    if (aVerbose) {
        keyToTextConverter << "[" << std::hex << fNext << ">";
    }
    return keyToTextConverter.str();
}

std::pair<unsigned long, std::string> LeafNode::key_value_pairs(int fd, std::set<std::pair<int, bool>, set_compare>& found_once) const {
    std::string rtn = "";
    unsigned long count = 0;
    std::pair<std::set<std::pair<int, bool>, set_compare>::iterator, bool> set_rtn;
    
    for (auto mapping : fMappings) {
        set_rtn = found_once.insert(std::pair<int, bool>(mapping.first, true));
        if (set_rtn.second) {
            std::pair<int, int> addr = mapping.second->value();
            long* map = (long*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, sysconf(_SC_PAGE_SIZE) * (addr.first - 1));
            if (map == MAP_FAILED) {
                close(fd);
                perror("Error mmapping the file for insertion");
                exit(EXIT_FAILURE);
            }
            long value = map[addr.second - 1];
            if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
                perror("Error unmapping the file");
                close(fd);
                exit(EXIT_FAILURE);
            }
            std::stringstream keyToTextConverter;
            std::stringstream valueToTextConverter;
            keyToTextConverter << mapping.first;
            valueToTextConverter << value;
            rtn += keyToTextConverter.str() + ":" + valueToTextConverter.str() + " ";
            count++;
        }
    }
    std::pair<unsigned long, std::string> pair(count, rtn);
    return pair;
}

std::vector<std::pair<KeyType, Record*>> LeafNode::get_mappings() {
    return fMappings;
}

int LeafNode::createAndInsertRecord(KeyType aKey, ValueType aValue, int fd, int page_num, int elmt_num)
{
    Record* existingRecord = lookup(aKey);
    if (!existingRecord) {
        Record* newRecord = new Record(std::pair<int, int>(page_num, elmt_num));
        long* map = (long*) mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, sysconf(_SC_PAGE_SIZE) * (page_num - 1));
        if (map == MAP_FAILED) {
            close(fd);
            perror("Error mmapping the file for insertion");
            exit(EXIT_FAILURE);
        }
        map[elmt_num - 1] = aValue;
        if (munmap(map, sysconf(_SC_PAGE_SIZE)) == -1) {
            perror("Error unmapping the file");
            close(fd);
            exit(EXIT_FAILURE);
        }
        insert(aKey, newRecord);
    }
    return static_cast<int>(fMappings.size());
}

void LeafNode::insert(KeyType aKey, Record* aRecord)
{
    auto insertionPoint = fMappings.begin();
    auto end = fMappings.end();
    while (insertionPoint != end && insertionPoint->first < aKey) {
        ++insertionPoint;
    }
    fMappings.insert(insertionPoint, MappingType(aKey, aRecord));
}

Record* LeafNode::lookup(KeyType aKey) const
{
    for (auto mapping : fMappings) {
        if (mapping.first == aKey) {
            return mapping.second;
        }
    }
    return nullptr;
}

void LeafNode::copyRangeStartingFrom(KeyType aKey, std::vector<EntryType>& aVector)
{
    bool found = false;
    for (auto mapping : fMappings) {
        if (mapping.first >= aKey) {
            found = true;
        }
        if (found) {
            aVector.push_back(std::make_tuple(mapping.first, mapping.second->value(), this));
        }
    }
}

KeyType LeafNode::smallest() {
    return fMappings[0].first;
}

KeyType LeafNode::largest() {
    return fMappings[fMappings.size() - 1].first;
}

void LeafNode::copyLargest(std::vector<EntryType>& aVector) {
    auto mapping = fMappings[fMappings.size() - 1];
    aVector.push_back(std::make_tuple(mapping.first, mapping.second->value(), this));
}

void LeafNode::copyWithinSameNode(KeyType aStart, KeyType aEnd, std::vector<EntryType>& aVector) {
    for (auto mapping: fMappings) {
        if (mapping.first >= aStart && mapping.first < aEnd) {
            aVector.push_back(std::make_tuple(mapping.first, mapping.second->value(), this));
        }
    }
}
void LeafNode::copyRangeUntil(KeyType aKey, std::vector<EntryType>& aVector)
{
    bool found = false;
    for (auto mapping : fMappings) {
        if (mapping.first >= aKey) {
            found = true;
        }
        if (!found) {
            aVector.push_back(std::make_tuple(mapping.first, mapping.second->value(), this));
        }
    }
}

void LeafNode::copyRange(std::vector<EntryType>& aVector)
{
    for (auto mapping : fMappings) {
        aVector.push_back(std::make_tuple(mapping.first, mapping.second->value(), this));
    }
}


int LeafNode::removeAndDeleteRecord(KeyType aKey)
{
    auto removalPoint = fMappings.begin();
    auto end = fMappings.end();
    while (removalPoint != end && removalPoint->first != aKey) {
        ++removalPoint;
    }
    if (removalPoint == end) {
        //throw RecordNotFoundException(aKey);
        return static_cast<int>(fMappings.size());
    }
    auto record = *removalPoint;
    fMappings.erase(removalPoint);
    delete record.second;
    return static_cast<int>(fMappings.size());
}

KeyType LeafNode::firstKey() const
{
    return fMappings[0].first;
}

void LeafNode::moveHalfTo(LeafNode *aRecipient)
{
    aRecipient->copyHalfFrom(fMappings);
    size_t size = fMappings.size();
    for (size_t i = minSize(); i < size; ++i) {
        fMappings.pop_back();
    }
}

void LeafNode::copyHalfFrom(std::vector<std::pair<KeyType, Record*> > &aMappings)
{
    for (size_t i = minSize(); i < aMappings.size(); ++i) {
        fMappings.push_back(aMappings[i]);
    }
}

void LeafNode::moveAllTo(LeafNode *aRecipient, int)
{
    aRecipient->copyAllFrom(fMappings);
    fMappings.clear();
    aRecipient->setNext(next());
}

void LeafNode::copyAllFrom(std::vector<std::pair<KeyType, Record*> > &aMappings)
{
    for (auto mapping : aMappings) {
        fMappings.push_back(mapping);
    }
}

void LeafNode::moveFirstToEndOf(LeafNode* aRecipient)
{
    aRecipient->copyLastFrom(fMappings.front());
    fMappings.erase(fMappings.begin());
    static_cast<InternalNode*>(parent())->setKeyAt(1, fMappings.front().first);
}

void LeafNode::copyLastFrom(MappingType aPair)
{
    fMappings.push_back(aPair);
}

void LeafNode::moveLastToFrontOf(LeafNode *aRecipient, int aParentIndex)
{
    aRecipient->copyFirstFrom(fMappings.back(), aParentIndex);
    fMappings.pop_back();
}

void LeafNode::copyFirstFrom(MappingType aPair, int aParentIndex)
{
    fMappings.insert(fMappings.begin(), aPair);
    static_cast<InternalNode*>(parent())->setKeyAt(aParentIndex, fMappings.front().first);
}