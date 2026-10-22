#pragma once
#include <vector>
#include "common.h"

// Стек на односвязном списке: вершина = голова списка.
class Stack
{
public:
    Stack() = default;
    ~Stack();
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    void push(const std::string& v);          // O(1)
    bool pop(std::string& out);               // O(1)
    bool peek(std::string& out) const;        // O(1)
    std::vector<std::string> fromTop() const; // от вершины к дну
    json toJson() const;                      // от дна к вершине

private:
    struct Node
    {
        std::string val;
        Node* below;
    };
    Node* top_ = nullptr;
};

bool handleStack(Cmd cmd, json& db, const std::string& name, Args& args);
