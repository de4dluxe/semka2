#pragma once
#include <vector>
#include "common.h"

// Очередь на односвязном списке: вход в хвост, выход из головы.
class Queue
{
public:
    Queue() = default;
    ~Queue();
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void push(const std::string& v);          // O(1)
    bool pop(std::string& out);               // O(1)
    bool peek(std::string& out) const;        // O(1)
    std::vector<std::string> items() const;   // от головы к хвосту
    json toJson() const;

private:
    struct Node
    {
        std::string val;
        Node* next;
    };
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
};

bool handleQueue(Cmd cmd, json& db, const std::string& name, Args& args);
