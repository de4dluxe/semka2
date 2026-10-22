#include "array.h"

DynArray::DynArray() : data_(new std::string[4]), size_(0), cap_(4) {}
DynArray::~DynArray() { delete[] data_; }

void DynArray::grow()
{
    std::string* bigger = new std::string[cap_ * 2];
    for (size_t i = 0; i < size_; i++) bigger[i] = std::move(data_[i]);
    delete[] data_;
    data_ = bigger;
    cap_ *= 2;
}

void DynArray::checkIndex(size_t index, bool allowEnd) const
{
    if (index > size_ || (index == size_ && !allowEnd))
        throw DbError("индекс " + std::to_string(index) + " вне массива (длина " + std::to_string(size_) + ")");
}

void DynArray::push(const std::string& v)
{
    if (size_ == cap_) grow();
    data_[size_++] = v;
}

void DynArray::insert(size_t index, const std::string& v)
{
    checkIndex(index, true);
    if (size_ == cap_) grow();
    // сдвигаем хвост вправо, начиная с последнего элемента
    for (size_t i = size_; i > index; i--) data_[i] = std::move(data_[i - 1]);
    data_[index] = v;
    size_++;
}

const std::string& DynArray::get(size_t index) const
{
    checkIndex(index);
    return data_[index];
}

void DynArray::set(size_t index, const std::string& v)
{
    checkIndex(index);
    data_[index] = v;
}

void DynArray::erase(size_t index)
{
    checkIndex(index);
    for (size_t i = index; i + 1 < size_; i++) data_[i] = std::move(data_[i + 1]);
    size_--;
}

void DynArray::print() const
{
    std::cout << "[";
    for (size_t i = 0; i < size_; i++) std::cout << (i ? ", " : "") << data_[i];
    std::cout << "] (длина=" << size_ << ", ёмкость=" << cap_ << ")\n";
}

json DynArray::toJson() const
{
    json a = json::array();
    for (size_t i = 0; i < size_; i++) a.push_back(data_[i]);
    return a;
}

// Индекс из запроса: отрицательный сразу считаем ошибкой.
static size_t readIndex(Args& args)
{
    long i = readInt(args, "индекс");
    if (i < 0) throw DbError("индекс не может быть отрицательным");
    return static_cast<size_t>(i);
}

bool handleArray(Cmd cmd, json& db, const std::string& name, Args& args)
{
    bool create = (cmd == Cmd::MPUSH || cmd == Cmd::MINSERT);
    json* data = openData(db, name, "array", create);

    DynArray arr;
    for (auto& e : *data) arr.push(e.get<std::string>());

    bool changed = false;
    switch (cmd)
    {
        case Cmd::MPUSH:
        {
            std::string v = readWord(args, "значение");
            arr.push(v);
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        }
        case Cmd::MINSERT:
        {
            size_t i = readIndex(args);
            std::string v = readWord(args, "значение");
            arr.insert(i, v);
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        }
        case Cmd::MGET:
        {
            std::string v = arr.get(readIndex(args));
            std::cout << "-> " << v << "\n";
            break;
        }
        case Cmd::MSET:
        {
            size_t i = readIndex(args);
            std::string v = readWord(args, "значение");
            arr.set(i, v);
            std::cout << "-> " << v << "\n";
            changed = true;
            break;
        }
        case Cmd::MDEL:
        {
            size_t i = readIndex(args);
            std::string old = arr.get(i);
            arr.erase(i);
            std::cout << "-> " << old << "\n";
            changed = true;
            break;
        }
        case Cmd::MLEN:
            std::cout << "-> " << arr.size() << "\n";
            break;
        case Cmd::MPRINT:
            arr.print();
            break;
        default:
            break;
    }

    if (changed) *data = arr.toJson();
    return changed;
}
