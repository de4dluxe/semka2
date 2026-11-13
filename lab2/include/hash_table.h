#pragma once
#include <string>
#include "array.h"

// Хеш-таблица, метод цепочек. Ключ - строка, значение - число.
struct HashNode {
    std::string key;
    int value;
    HashNode* next;
};

class HashTable {
public:
    HashTable();
    ~HashTable();
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void add(const std::string& key, int value);
    bool search(const std::string& key);
    int* get(const std::string& key);
    bool remove(const std::string& key);
    int count() const { return count_; }
    void getKeys(DynArray& keys);

private:
    HashNode** table_;
    int size_;
    int count_;
    int hash(const std::string& key, int size);
    void rehash();
};

// Хеш-таблица, хеширование кукушки: две таблицы и две хеш-функции.
class Cuckoo {
public:
    Cuckoo();
    ~Cuckoo();
    Cuckoo(const Cuckoo&) = delete;
    Cuckoo& operator=(const Cuckoo&) = delete;

    void add(const std::string& key, int value);
    bool search(const std::string& key);
    int* get(const std::string& key);
    bool remove(const std::string& key);
    int count() const { return count_; }
    void getKeys(DynArray& keys);

private:
    static const int MAXTEMPT = 50;
    int size_;
    int count_;
    std::string* keys1_;
    std::string* keys2_;
    int* values1_;
    int* values2_;
    bool* used1_;
    bool* used2_;

    int hash1(const std::string& key);
    int hash2(const std::string& key);
    void create(int size);
    void insertNew(std::string key, int value);
    void grow();
};
