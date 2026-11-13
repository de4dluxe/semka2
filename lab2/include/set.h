#pragma once
#include <string>
#include "hash_table.h"

class Set {
public:
    void setAdd(const std::string& x) { table_.add(x, 1); }
    bool setDel(const std::string& x) { return table_.remove(x); }
    bool setAt(const std::string& x) { return table_.search(x); }
    int count() const { return table_.count(); }
    void getAll(DynArray& all) { table_.getKeys(all); }

private:
    HashTable table_;
};
