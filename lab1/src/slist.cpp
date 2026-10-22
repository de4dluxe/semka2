#include "slist.h"
#include <algorithm>

SList::~SList()
{
    while (head_)
    {
        Node* n = head_->next;
        delete head_;
        head_ = n;
    }
}

SList::Node* SList::find(const std::string& v) const
{
    for (Node* n = head_; n; n = n->next)
        if (n->val == v) return n;
    return nullptr;
}

void SList::pushHead(const std::string& v)
{
    head_ = new Node{v, head_};
    if (!tail_) tail_ = head_;
    size_++;
}

void SList::pushTail(const std::string& v)
{
    Node* n = new Node{v, nullptr};
    if (tail_) tail_->next = n;
    else head_ = n;
    tail_ = n;
    size_++;
}

bool SList::pushAfter(const std::string& key, const std::string& v)
{
    Node* at = find(key);
    if (!at) return false;
    at->next = new Node{v, at->next};
    if (at == tail_) tail_ = at->next;
    size_++;
    return true;
}

bool SList::pushBefore(const std::string& key, const std::string& v)
{
    if (!head_) return false;
    if (head_->val == key)
    {
        pushHead(v);
        return true;
    }
    for (Node* p = head_; p->next; p = p->next)
    {
        if (p->next->val == key)
        {
            p->next = new Node{v, p->next};
            size_++;
            return true;
        }
    }
    return false;
}

bool SList::popHead(std::string& out)
{
    if (!head_) return false;
    Node* n = head_;
    out = n->val;
    head_ = n->next;
    if (!head_) tail_ = nullptr;
    delete n;
    size_--;
    return true;
}

bool SList::popTail(std::string& out)
{
    if (!head_) return false;
    if (head_ == tail_) return popHead(out);
    Node* p = head_;
    while (p->next != tail_) p = p->next;   // предпоследний
    out = tail_->val;
    delete tail_;
    p->next = nullptr;
    tail_ = p;
    size_--;
    return true;
}

bool SList::delAfter(const std::string& key, std::string& out)
{
    Node* at = find(key);
    if (!at || !at->next) return false;
    Node* victim = at->next;
    out = victim->val;
    at->next = victim->next;
    if (victim == tail_) tail_ = at;
    delete victim;
    size_--;
    return true;
}

bool SList::delBefore(const std::string& key, std::string& out)
{
    if (!head_ || head_->val == key) return false;   // перед головой ничего нет
    Node* prev = nullptr;                            // узел перед удаляемым
    Node* cur = head_;                               // удаляемый
    while (cur->next && cur->next->val != key)
    {
        prev = cur;
        cur = cur->next;
    }
    if (!cur->next) return false;                    // key не найден
    out = cur->val;
    if (prev) prev->next = cur->next;
    else head_ = cur->next;
    delete cur;
    size_--;
    return true;
}

bool SList::delValue(const std::string& v)
{
    Node* prev = nullptr;
    for (Node* n = head_; n; prev = n, n = n->next)
    {
        if (n->val != v) continue;
        if (prev) prev->next = n->next;
        else head_ = n->next;
        if (n == tail_) tail_ = prev;
        delete n;
        size_--;
        return true;
    }
    return false;
}

bool SList::contains(const std::string& v) const { return find(v) != nullptr; }

const std::string& SList::get(size_t index) const
{
    if (index >= size_) throw DbError("индекс " + std::to_string(index) + " вне списка (длина " + std::to_string(size_) + ")");
    Node* n = head_;
    while (index--) n = n->next;
    return n->val;
}

std::vector<std::string> SList::items() const
{
    std::vector<std::string> r;
    for (Node* n = head_; n; n = n->next) r.push_back(n->val);
    return r;
}

json SList::toJson() const { return json(items()); }

void printItems(const std::vector<std::string>& items)
{
    if (items.empty())
    {
        std::cout << "(пусто)\n";
        return;
    }
    for (size_t i = 0; i < items.size(); i++) std::cout << (i ? " -> " : "") << items[i];
    std::cout << "\n";
}

bool handleSList(Cmd cmd, json& db, const std::string& name, Args& args)
{
    bool create = (cmd == Cmd::FPUSHH || cmd == Cmd::FPUSHT || cmd == Cmd::FPUSHA || cmd == Cmd::FPUSHB);
    json* data = openData(db, name, "flist", create);

    SList list;
    for (auto& e : *data) list.pushTail(e.get<std::string>());

    bool changed = false;
    std::string key, v, out;
    switch (cmd)
    {
        case Cmd::FPUSHH:
            v = readWord(args, "значение");
            list.pushHead(v);
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::FPUSHT:
            v = readWord(args, "значение");
            list.pushTail(v);
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::FPUSHA:
            key = readWord(args, "элемент, после которого вставить");
            v = readWord(args, "значение");
            if (!list.pushAfter(key, v)) throw DbError("элемент " + key + " не найден");
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::FPUSHB:
            key = readWord(args, "элемент, перед которым вставить");
            v = readWord(args, "значение");
            if (!list.pushBefore(key, v)) throw DbError("элемент " + key + " не найден");
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::FDELH:
            if (!list.popHead(out)) throw DbError("список пуст");
            std::cout << "-> " << out << "\n";
            changed = true;
            break;
        case Cmd::FDELT:
            if (!list.popTail(out)) throw DbError("список пуст");
            std::cout << "-> " << out << "\n";
            changed = true;
            break;
        case Cmd::FDELA:
            key = readWord(args, "элемент");
            if (!list.delAfter(key, out)) throw DbError("после " + key + " удалять нечего (или " + key + " нет в списке)");
            std::cout << "-> " << out << "\n";
            changed = true;
            break;
        case Cmd::FDELB:
            key = readWord(args, "элемент");
            if (!list.delBefore(key, out)) throw DbError("перед " + key + " удалять нечего (или " + key + " нет в списке)");
            std::cout << "-> " << out << "\n";
            changed = true;
            break;
        case Cmd::FDEL:
            v = readWord(args, "значение");
            if (!list.delValue(v)) throw DbError("элемент " + v + " не найден");
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        case Cmd::FFIND:
            std::cout << "-> " << (list.contains(readWord(args, "значение")) ? "TRUE" : "FALSE") << "\n";
            break;
        case Cmd::FGET:
        {
            long i = readInt(args, "индекс");
            if (i < 0) throw DbError("индекс не может быть отрицательным");
            std::string v = list.get(static_cast<size_t>(i));
            std::cout << "-> " << v << "\n";
            break;
        }
        case Cmd::FPRINT:
            printItems(list.items());
            break;
        case Cmd::FPRINTR:
        {
            auto items = list.items();   // у односвязного списка назад ходить нельзя — разворачиваем копию
            std::reverse(items.begin(), items.end());
            printItems(items);
            break;
        }
        default:
            break;
    }

    if (changed) *data = list.toJson();
    return changed;
}
