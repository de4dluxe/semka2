#pragma once
#include <vector>
#include "common.h"

// Односвязный список строк (голова + хвост).
class SList
{
public:
    SList() = default;
    ~SList();
    SList(const SList&) = delete;
    SList& operator=(const SList&) = delete;

    void pushHead(const std::string& v);                           // O(1)
    void pushTail(const std::string& v);                           // O(1)
    bool pushAfter(const std::string& key, const std::string& v);  // O(n)
    bool pushBefore(const std::string& key, const std::string& v); // O(n)
    bool popHead(std::string& out);                                // O(1)
    bool popTail(std::string& out);                                // O(n)
    bool delAfter(const std::string& key, std::string& out);       // O(n)
    bool delBefore(const std::string& key, std::string& out);      // O(n)
    bool delValue(const std::string& v);                           // O(n)
    bool contains(const std::string& v) const;                     // O(n)
    const std::string& get(size_t index) const;                    // O(n)
    size_t size() const { return size_; }
    std::vector<std::string> items() const;                        // с начала
    json toJson() const;

private:
    struct Node
    {
        std::string val;
        Node* next;
    };
    Node* find(const std::string& v) const;
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    size_t size_ = 0;
};

bool handleSList(Cmd cmd, json& db, const std::string& name, Args& args);

// Общий вывод списка для односвязного и двусвязного.
void printItems(const std::vector<std::string>& items);
