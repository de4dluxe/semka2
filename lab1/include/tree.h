#pragma once
#include <vector>
#include "common.h"

// Порядок обхода дерева.
enum class Order { Pre, In, Post, Level };

// Баланс узла АВЛ-дерева: сравнение высот левого и правого поддеревьев.
enum class Balance { LeftHeavy, Even, RightHeavy };

// Сторона, с которой узел присоединён к родителю (нужно при рисовании).
enum class Side { Root, Left, Right };

// АВЛ-дерево с целыми ключами. Повторяющиеся ключи не хранятся.
class AvlTree
{
public:
    struct Node
    {
        int key;
        int height;
        Node* left;
        Node* right;
    };

    AvlTree() = default;
    ~AvlTree();
    AvlTree(const AvlTree&) = delete;
    AvlTree& operator=(const AvlTree&) = delete;

    bool insert(int key);                 // false, если ключ уже есть; O(log n)
    bool erase(int key);                  // false, если ключа нет;     O(log n)
    const Node* find(int key) const;      // O(log n)
    std::vector<int> traverse(Order order) const;  // O(n)
    std::string draw() const;             // дерево «на боку»: правое поддерево сверху
    int height() const { return height(root_); }
    static Balance balanceOf(const Node* n);

    json toJson() const;                  // вложенный объект {"key":..,"left":..,"right":..}
    void fromJson(const json& j);

private:
    static int height(const Node* n) { return n ? n->height : 0; }
    static void update(Node* n);
    static int factor(const Node* n);     // высота(left) - высота(right)
    static Node* rotateLeft(Node* x);
    static Node* rotateRight(Node* y);
    static Node* rebalance(Node* n);
    static Node* insert(Node* n, int key, bool& added);
    static Node* erase(Node* n, int key, bool& removed);
    static void destroy(Node* n);
    static json toJson(const Node* n);
    static Node* fromJsonNode(const json& j);
    static void drawNode(const Node* n, const std::string& prefix, Side side, std::string& out);

    Node* root_ = nullptr;
};

const char* toString(Balance b);

bool handleTree(Cmd cmd, json& db, const std::string& name, Args& args);
