#include "dlist.h"

DList::~DList()
{
    while (head_)
    {
        Node* n = head_->next;
        delete head_;
        head_ = n;
    }
}

DList::Node* DList::find(const std::string& v) const
{
    for (Node* n = head_; n; n = n->next)
        if (n->val == v) return n;
    return nullptr;
}

// Вырезает узел из списка и удаляет его.
void DList::unlink(Node* n)
{
    if (n->prev) n->prev->next = n->next;
    else head_ = n->next;
    if (n->next) n->next->prev = n->prev;
    else tail_ = n->prev;
    delete n;
    size_--;
}

void DList::pushHead(const std::string& v)
{
    Node* n = new Node{v, nullptr, head_};
    if (head_) head_->prev = n;
    else tail_ = n;
    head_ = n;
    size_++;
}

void DList::pushTail(const std::string& v)
{
    Node* n = new Node{v, tail_, nullptr};
    if (tail_) tail_->next = n;
    else head_ = n;
    tail_ = n;
    size_++;
}

bool DList::pushAfter(const std::string& key, const std::string& v)
{
    Node* at = find(key);
    if (!at) return false;
    Node* n = new Node{v, at, at->next};
    if (at->next) at->next->prev = n;
    else tail_ = n;
    at->next = n;
    size_++;
    return true;
}

bool DList::pushBefore(const std::string& key, const std::string& v)
{
    Node* at = find(key);
    if (!at) return false;
    Node* n = new Node{v, at->prev, at};
    if (at->prev) at->prev->next = n;
    else head_ = n;
    at->prev = n;
    size_++;
    return true;
}

bool DList::popHead(std::string& out)
{
    if (!head_) return false;
    out = head_->val;
    unlink(head_);
    return true;
}

bool DList::popTail(std::string& out)
{
    if (!tail_) return false;
    out = tail_->val;
    unlink(tail_);
    return true;
}

bool DList::delAfter(const std::string& key, std::string& out)
{
    Node* at = find(key);
    if (!at || !at->next) return false;
    out = at->next->val;
    unlink(at->next);
    return true;
}

bool DList::delBefore(const std::string& key, std::string& out)
{
    Node* at = find(key);
    if (!at || !at->prev) return false;
    out = at->prev->val;
    unlink(at->prev);
    return true;
}

bool DList::delValue(const std::string& v)
{
    Node* n = find(v);
    if (!n) return false;
    unlink(n);
    return true;
}

bool DList::contains(const std::string& v) const { return find(v) != nullptr; }

const std::string& DList::get(size_t index) const
{
    if (index >= size_) throw DbError("индекс " + std::to_string(index) + " вне списка (длина " + std::to_string(size_) + ")");
    if (index < size_ / 2)
    {
        Node* n = head_;
        while (index--) n = n->next;
        return n->val;
    }
    Node* n = tail_;
    for (size_t i = size_ - 1; i > index; i--) n = n->prev;
    return n->val;
}

std::vector<std::string> DList::items() const
{
    std::vector<std::string> r;
    for (Node* n = head_; n; n = n->next) r.push_back(n->val);
    return r;
}

std::vector<std::string> DList::itemsReversed() const
{
    std::vector<std::string> r;
    for (Node* n = tail_; n; n = n->prev) r.push_back(n->val);
    return r;
}

json DList::toJson() const { return json(items()); }

bool handleDList(Cmd cmd, json& db, const std::string& name, Args& args)
{
    bool create = (cmd == Cmd::LPUSHH || cmd == Cmd::LPUSHT || cmd == Cmd::LPUSHA || cmd == Cmd::LPUSHB);
    json* data = openData(db, name, "dlist", create);

    DList list;
    for (auto& e : *data) list.pushTail(e.get<std::string>());

    bool changed = false;
    std::string key, v, out;
    switch (cmd)
    {
        case Cmd::LPUSHH:
            v = readWord(args, "значение");
            list.pushHead(v);
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::LPUSHT:
            v = readWord(args, "значение");
            list.pushTail(v);
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::LPUSHA:
            key = readWord(args, "элемент, после которого вставить");
            v = readWord(args, "значение");
            if (!list.pushAfter(key, v)) throw DbError("элемент " + key + " не найден");
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::LPUSHB:
            key = readWord(args, "элемент, перед которым вставить");
            v = readWord(args, "значение");
            if (!list.pushBefore(key, v)) throw DbError("элемент " + key + " не найден");
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::LDELH:
            if (!list.popHead(out)) throw DbError("список пуст");
            std::cout << "-> " << out << "\n";
            changed = true;
            break;
        case Cmd::LDELT:
            if (!list.popTail(out)) throw DbError("список пуст");
            std::cout << "-> " << out << "\n";
            changed = true;
            break;
        case Cmd::LDELA:
            key = readWord(args, "элемент");
            if (!list.delAfter(key, out)) throw DbError("после " + key + " удалять нечего (или " + key + " нет в списке)");
            std::cout << "-> " << out << "\n";
            changed = true;
            break;
        case Cmd::LDELB:
            key = readWord(args, "элемент");
            if (!list.delBefore(key, out)) throw DbError("перед " + key + " удалять нечего (или " + key + " нет в списке)");
            std::cout << "-> " << out << "\n";
            changed = true;
            break;
        case Cmd::LDEL:
            v = readWord(args, "значение");
            if (!list.delValue(v)) throw DbError("элемент " + v + " не найден");
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::LFIND:
            std::cout << "-> " << (list.contains(readWord(args, "значение")) ? "TRUE" : "FALSE") << "\n";
            break;
        case Cmd::LGET:
        {
            long i = readInt(args, "индекс");
            if (i < 0) throw DbError("индекс не может быть отрицательным");
            std::string v = list.get(static_cast<size_t>(i));
            std::cout << "-> " << v << "\n";
            break;
        }
        case Cmd::LPRINT:
            printItems(list.items());
            break;
        case Cmd::LPRINTR:
            printItems(list.itemsReversed());   // идём по prev от хвоста
            break;
        default:
            break;
    }

    if (changed) *data = list.toJson();
    return changed;
}
