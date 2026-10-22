#include "stack.h"
#include <algorithm>

Stack::~Stack()
{
    std::string tmp;
    while (pop(tmp)) {}
}

void Stack::push(const std::string& v) { top_ = new Node{v, top_}; }

bool Stack::pop(std::string& out)
{
    if (!top_) return false;
    Node* n = top_;
    out = n->val;
    top_ = n->below;
    delete n;
    return true;
}

bool Stack::peek(std::string& out) const
{
    if (!top_) return false;
    out = top_->val;
    return true;
}

std::vector<std::string> Stack::fromTop() const
{
    std::vector<std::string> r;
    for (Node* n = top_; n; n = n->below) r.push_back(n->val);
    return r;
}

json Stack::toJson() const
{
    auto r = fromTop();
    std::reverse(r.begin(), r.end());
    return json(r);
}

bool handleStack(Cmd cmd, json& db, const std::string& name, Args& args)
{
    json* data = openData(db, name, "stack", cmd == Cmd::SPUSH);

    Stack st;
    for (auto& e : *data) st.push(e.get<std::string>());   // в файле дно -> вершина

    bool changed = false;
    std::string v;
    switch (cmd)
    {
        case Cmd::SPUSH:
            v = readWord(args, "значение");
            st.push(v);
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::SPOP:
            if (!st.pop(v)) throw DbError("стек пуст");
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::SPEEK:
            if (!st.peek(v)) throw DbError("стек пуст");
            std::cout << "-> " << v << "\n";
            break;
        case Cmd::SPRINT:
        {
            auto items = st.fromTop();
            std::cout << "вершина: ";
            if (items.empty()) std::cout << "(пусто)";
            for (size_t i = 0; i < items.size(); i++) std::cout << (i ? " | " : "") << items[i];
            std::cout << "\n";
            break;
        }
        default:
            break;
    }

    if (changed) *data = st.toJson();
    return changed;
}
