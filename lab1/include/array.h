#pragma once
#include "common.h"

// Динамический массив строк (удвоение ёмкости).
class DynArray
{
public:
    DynArray();
    ~DynArray();
    DynArray(const DynArray&) = delete;
    DynArray& operator=(const DynArray&) = delete;

    size_t size() const { return size_; }
    void push(const std::string& v);                  // в конец, O(1) амортизированно
    void insert(size_t index, const std::string& v);  // по индексу, O(n)
    const std::string& get(size_t index) const;       // O(1)
    void set(size_t index, const std::string& v);     // O(1)
    void erase(size_t index);                         // O(n)
    void print() const;
    json toJson() const;

private:
    void grow();
    void checkIndex(size_t index, bool allowEnd = false) const;
    std::string* data_;
    size_t size_;
    size_t cap_;
};

bool handleArray(Cmd cmd, json& db, const std::string& name, Args& args);
