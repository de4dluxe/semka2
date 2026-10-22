#include "queue.h"

Queue::~Queue()
{
    std::string tmp;
    while (pop(tmp)) {}
}

void Queue::push(const std::string& v)
{
    Node* n = new Node{v, nullptr};
    if (tail_) tail_->next = n;
    else head_ = n;
    tail_ = n;
}

bool Queue::pop(std::string& out)
{
    if (!head_) return false;
    Node* n = head_;
    out = n->val;
    head_ = n->next;
    if (!head_) tail_ = nullptr;
    delete n;
    return true;
}

bool Queue::peek(std::string& out) const
{
    if (!head_) return false;
    out = head_->val;
    return true;
}

std::vector<std::string> Queue::items() const
{
    std::vector<std::string> r;
    for (Node* n = head_; n; n = n->next) r.push_back(n->val);
    return r;
}

json Queue::toJson() const { return json(items()); }

bool handleQueue(Cmd cmd, json& db, const std::string& name, Args& args)
{
    json* data = openData(db, name, "queue", cmd == Cmd::QPUSH);

    Queue q;
    for (auto& e : *data) q.push(e.get<std::string>());

    bool changed = false;
    std::string v;
    switch (cmd)
    {
        case Cmd::QPUSH:
            v = readWord(args, "значение");
            q.push(v);
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::QPOP:
            if (!q.pop(v)) throw DbError("очередь пуста");
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::QPEEK:
            if (!q.peek(v)) throw DbError("очередь пуста");
            std::cout << "-> " << v << "\n";
            break;
        case Cmd::QPRINT:
        {
            auto items = q.items();
            std::cout << "голова: ";
            if (items.empty()) std::cout << "(пусто)";
            for (size_t i = 0; i < items.size(); i++) std::cout << (i ? " | " : "") << items[i];
            std::cout << "\n";
            break;
        }
        default:
            break;
    }

    if (changed) *data = q.toJson();
    return changed;
}
