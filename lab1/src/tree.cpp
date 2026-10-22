#include "tree.h"
#include <algorithm>

AvlTree::~AvlTree() { destroy(root_); }

void AvlTree::destroy(Node* n)
{
    if (!n) return;
    destroy(n->left);
    destroy(n->right);
    delete n;
}

void AvlTree::update(Node* n) { n->height = 1 + std::max(height(n->left), height(n->right)); }
int AvlTree::factor(const Node* n) { return n ? height(n->left) - height(n->right) : 0; }

Balance AvlTree::balanceOf(const Node* n)
{
    int f = factor(n);
    if (f > 0) return Balance::LeftHeavy;
    if (f < 0) return Balance::RightHeavy;
    return Balance::Even;
}

const char* toString(Balance b)
{
    switch (b)
    {
        case Balance::LeftHeavy: return "левое выше (+1)";
        case Balance::RightHeavy: return "правое выше (-1)";
        default: return "сбалансирован (0)";
    }
}

// Левый поворот вокруг x (правый ребёнок y становится корнем):
//   x                y
//    y   ->        x   c
//   b c             b
AvlTree::Node* AvlTree::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    update(x);
    update(y);
    return y;
}

AvlTree::Node* AvlTree::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    update(y);
    update(x);
    return x;
}

AvlTree::Node* AvlTree::rebalance(Node* n)
{
    update(n);
    int f = factor(n);
    if (f > 1)                                   // перекос влево
    {
        if (factor(n->left) < 0) n->left = rotateLeft(n->left);   // случай «лево-право»
        return rotateRight(n);
    }
    if (f < -1)                                  // перекос вправо
    {
        if (factor(n->right) > 0) n->right = rotateRight(n->right); // случай «право-лево»
        return rotateLeft(n);
    }
    return n;
}

AvlTree::Node* AvlTree::insert(Node* n, int key, bool& added)
{
    if (!n)
    {
        added = true;
        return new Node{key, 1, nullptr, nullptr};
    }
    if (key < n->key) n->left = insert(n->left, key, added);
    else if (key > n->key) n->right = insert(n->right, key, added);
    else return n;   // дубликат
    return rebalance(n);
}

AvlTree::Node* AvlTree::erase(Node* n, int key, bool& removed)
{
    if (!n) return nullptr;
    if (key < n->key) n->left = erase(n->left, key, removed);
    else if (key > n->key) n->right = erase(n->right, key, removed);
    else
    {
        removed = true;
        if (!n->left || !n->right)               // 0 или 1 потомок
        {
            Node* child = n->left ? n->left : n->right;
            delete n;
            return child;
        }
        Node* succ = n->right;                   // 2 потомка: заменяем минимумом правого поддерева
        while (succ->left) succ = succ->left;
        n->key = succ->key;
        bool dummy = false;
        n->right = erase(n->right, succ->key, dummy);
    }
    return rebalance(n);
}

bool AvlTree::insert(int key)
{
    bool added = false;
    root_ = insert(root_, key, added);
    return added;
}

bool AvlTree::erase(int key)
{
    bool removed = false;
    root_ = erase(root_, key, removed);
    return removed;
}

const AvlTree::Node* AvlTree::find(int key) const
{
    const Node* n = root_;
    while (n && n->key != key) n = key < n->key ? n->left : n->right;
    return n;
}

static void walk(const AvlTree::Node* n, Order order, std::vector<int>& out)
{
    if (!n) return;
    if (order == Order::Pre) out.push_back(n->key);
    walk(n->left, order, out);
    if (order == Order::In) out.push_back(n->key);
    walk(n->right, order, out);
    if (order == Order::Post) out.push_back(n->key);
}

std::vector<int> AvlTree::traverse(Order order) const
{
    std::vector<int> out;
    if (order != Order::Level)
    {
        walk(root_, order, out);
        return out;
    }
    std::vector<const Node*> queue;              // обход в ширину
    if (root_) queue.push_back(root_);
    for (size_t i = 0; i < queue.size(); i++)
    {
        out.push_back(queue[i]->key);
        if (queue[i]->left) queue.push_back(queue[i]->left);
        if (queue[i]->right) queue.push_back(queue[i]->right);
    }
    return out;
}

void AvlTree::drawNode(const Node* n, const std::string& prefix, Side side, std::string& out)
{
    if (!n) return;
    drawNode(n->right, prefix + (side == Side::Right ? "    " : (side == Side::Left ? "│   " : "    ")), Side::Right, out);
    const char* mark = side == Side::Root ? "" : (side == Side::Right ? "┌── " : "└── ");
    out += prefix + mark + std::to_string(n->key) + "\n";
    drawNode(n->left, prefix + (side == Side::Right ? "│   " : "    "), Side::Left, out);
}

std::string AvlTree::draw() const
{
    std::string out;
    drawNode(root_, "", Side::Root, out);
    return out;
}

json AvlTree::toJson(const Node* n)
{
    if (!n) return nullptr;
    json j = {{"key", n->key}};
    if (n->left) j["left"] = toJson(n->left);
    if (n->right) j["right"] = toJson(n->right);
    return j;
}

json AvlTree::toJson() const { return toJson(root_); }

AvlTree::Node* AvlTree::fromJsonNode(const json& j)
{
    if (j.is_null()) return nullptr;
    Node* n = new Node{j.at("key").get<int>(), 1, nullptr, nullptr};
    if (j.contains("left")) n->left = fromJsonNode(j["left"]);
    if (j.contains("right")) n->right = fromJsonNode(j["right"]);
    update(n);
    return n;
}

void AvlTree::fromJson(const json& j)
{
    destroy(root_);
    root_ = fromJsonNode(j);
}

static void printOrder(const char* title, const std::vector<int>& v)
{
    std::cout << title;
    for (size_t i = 0; i < v.size(); i++) std::cout << (i ? " " : "") << v[i];
    if (v.empty()) std::cout << "(пусто)";
    std::cout << "\n";
}

bool handleTree(Cmd cmd, json& db, const std::string& name, Args& args)
{
    json* data = openData(db, name, "tree", cmd == Cmd::TINSERT);

    AvlTree tree;
    tree.fromJson(*data);

    bool changed = false;
    switch (cmd)
    {
        case Cmd::TINSERT:
        {
            long k = readInt(args, "ключ");
            if (!tree.insert(static_cast<int>(k))) throw DbError("ключ " + std::to_string(k) + " уже есть в дереве");
            std::cout << "-> " << k << "\n";
            changed = true;
            break;
        }
        case Cmd::TDEL:
        {
            long k = readInt(args, "ключ");
            if (!tree.erase(static_cast<int>(k))) throw DbError("ключа " + std::to_string(k) + " нет в дереве");
            std::cout << "-> " << k << "\n";
            changed = true;
            break;
        }
        case Cmd::TFIND:
        case Cmd::ISMEMBER:
            std::cout << "-> " << (tree.find(static_cast<int>(readInt(args, "ключ"))) ? "TRUE" : "FALSE") << "\n";
            break;
        case Cmd::TGET:
        {
            long k = readInt(args, "ключ");
            const AvlTree::Node* n = tree.find(static_cast<int>(k));
            if (!n) throw DbError("ключа " + std::to_string(k) + " нет в дереве");
            std::cout << "-> " << n->key << ": высота=" << n->height
                      << ", баланс=" << toString(AvlTree::balanceOf(n))
                      << ", левый=" << (n->left ? std::to_string(n->left->key) : "-")
                      << ", правый=" << (n->right ? std::to_string(n->right->key) : "-") << "\n";
            break;
        }
        case Cmd::TPRINT:
            printOrder("прямой (pre):      ", tree.traverse(Order::Pre));
            printOrder("симметричный (in): ", tree.traverse(Order::In));
            printOrder("обратный (post):   ", tree.traverse(Order::Post));
            printOrder("в ширину (bfs):    ", tree.traverse(Order::Level));
            std::cout << "высота дерева: " << tree.height() << "\n" << tree.draw();
            break;
        default:
            break;
    }

    if (changed) *data = tree.toJson();
    return changed;
}
