#pragma once
#include "slist.h"

// Двусвязный список строк.
class DList
{
public:
    DList() = default;
    ~DList();
    DList(const DList&) = delete;
    DList& operator=(const DList&) = delete;

    void pushHead(const std::string& v);                           // O(1)
    void pushTail(const std::string& v);                           // O(1)
    bool pushAfter(const std::string& key, const std::string& v);  // O(n) поиск, вставка O(1)
    bool pushBefore(const std::string& key, const std::string& v); // O(n) поиск, вставка O(1)
    bool popHead(std::string& out);                                // O(1)
    bool popTail(std::string& out);                                // O(1)
    bool delAfter(const std::string& key, std::string& out);       // O(n) поиск
    bool delBefore(const std::string& key, std::string& out);      // O(n) поиск
    bool delValue(const std::string& v);                           // O(n)
    bool contains(const std::string& v) const;                     // O(n)
    const std::string& get(size_t index) const;                    // O(n), с ближнего конца
    size_t size() const { return size_; }
    std::vector<std::string> items() const;                        // с начала
    std::vector<std::string> itemsReversed() const;                // с конца
    json toJson() const;

private:
    struct Node
    {
        std::string val;
        Node* prev;
        Node* next;
    };
    Node* find(const std::string& v) const;
    void unlink(Node* n);
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    size_t size_ = 0;
};

bool handleDList(Cmd cmd, json& db, const std::string& name, Args& args);
