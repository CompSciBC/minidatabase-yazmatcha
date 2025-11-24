#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <vector>
#include "BST.h"
#include "Record.h"
// add header files as needed

using namespace std;

// Converts a string to lowercase (used for case-insensitive searches)
static inline string toLower(string s)
{
    for (char &c : s)
        c = (char)tolower((unsigned char)c);
    return s;
}

// ================== Index Engine ==================
// Acts like a small "database engine" that manages records and two BST indexes:
// 1) idIndex: maps student_id → record index (unique key)
// 2) lastIndex: maps lowercase(last_name) → list of record indices (non-unique key)
struct Engine
{
    vector<Record> heap;                // the main data store (simulates a heap file)
    BST<int, int> idIndex;              // index by student ID
    BST<string, vector<int>> lastIndex; // index by last name (can have duplicates)

    // Inserts a new record and updates both indexes.
    // Returns the record ID (RID) in the heap.
    int insertRecord(const Record &recIn)
    {
        // Add the student's record to the heap
        heap.push_back(recIn);
        int recordIndex = heap.size() - 1;

        // Insert record index into the ID index
        idIndex.insert(recIn.id, recordIndex);

        // Find if there are any other students with the same last name
        string lastLower = toLower(recIn.last);
        vector<int> *lastNameList = lastIndex.find(lastLower);

        if (lastNameList != nullptr)
        {
            // If found, append the new record index to the existing list
            lastNameList->push_back(recordIndex);
        }
        else
        {
            // If not found, create a new list with the current record index
            vector<int> newList = {recordIndex};
            lastIndex.insert(lastLower, newList);
        }

        return recordIndex;
    }

    // Deletes a record logically (marks as deleted and updates indexes)
    // Returns true if deletion succeeded.
    bool deleteById(int id)
    {
        // Delete the record from heap
        int recordIndex = -1;
        string lastName = "";
        for (int i = 0; i < heap.size(); i++)
        {
            if (heap[i].id == id)
            {
                recordIndex = i;
                lastName = heap[i].last;
                heap[i].deleted = true;
                break;
            }
        }

        // Record with this ID didn't exist
        if (recordIndex == -1)
        {
            return false;
        }

        // Delete from binary tree by ID
        idIndex.erase(id);

        // Delete from the last name list by last name
        string lastLower = toLower(lastName);
        vector<int> *lastNameList = lastIndex.find(lastLower);
        if (lastNameList != nullptr) {
            for (int i = 0; i < lastNameList->size(); i++)
            {
                if (lastNameList->at(i) == recordIndex)
                {
                    lastNameList->erase(lastNameList->begin() + i);
                    break;
                }
            }
        }

        return true;
    }

    // Finds a record by student ID.
    // Returns a pointer to the record, or nullptr if not found.
    // Outputs the number of comparisons made in the search.
    const Record *findById(int id, int &cmpOut)
    {
        int comparisons = idIndex.comparisons;
        int *foundIndex = idIndex.find(id);
        cmpOut = idIndex.comparisons - comparisons;

        if (foundIndex == nullptr)
        {
            return nullptr;
        }
        else
        {
            Record *record = &heap[*foundIndex];

            if (record->deleted)
            {
                return nullptr;
            }

            return record;
        }
    }

    // Returns all records with ID in the range [lo, hi].
    // Also reports the number of key comparisons performed.
    vector<const Record *> rangeById(int lo, int hi, int &cmpOut)
    {
        vector<const Record*> results;
        auto findRecords = [&](int id, int recordIndex)
        {
            Record* record = &heap[recordIndex];
            if (!record->deleted) {
                results.push_back(record);
            }
        };

        int comparisons = idIndex.comparisons;
        idIndex.rangeApply(lo, hi, findRecords);
        cmpOut = idIndex.comparisons - comparisons;
        return results;
    }

    // Returns all records whose last name begins with a given prefix.
    // Case-insensitive using lowercase comparison.
    vector<const Record *> prefixByLast(const string &prefix, int &cmpOut)
    {
        vector<const Record*> results;
        string lowerPrefix = toLower(prefix);
        auto findRecords = [&](const string& lastName, const vector<int>& recordIndexes)
        {
            string lowerLastName = toLower(lastName);
            for (int recordIndex : recordIndexes) {
                Record* record = &heap[recordIndex];
                if (!record -> deleted && lowerLastName.size() >= lowerPrefix.size()) {
                    bool prefixMatches = true;
                    for (int i = 0; i < lowerPrefix.size(); i++) {
                        if (lowerLastName.at(i) != lowerPrefix.at(i)) {
                            prefixMatches = false;
                            break;
                        }
                    }

                    if (prefixMatches) {
                        results.push_back(record);
                    }
                }
            }
        };

        int comparisons = lastIndex.comparisons;
        lastIndex.rangeApply(lowerPrefix, lowerPrefix + "z", findRecords);
        cmpOut = lastIndex.comparisons - comparisons;
        return results;
    }
};

#endif
