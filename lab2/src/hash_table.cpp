#include "hash_table.h"

// ---------- метод цепочек ----------

HashTable::HashTable() {
    size_ = 11;
    count_ = 0;
    table_ = new HashNode*[size_]();
}

HashTable::~HashTable() {
    for (int i = 0; i < size_; i++) {
        HashNode* current = table_[i];
        while (current != nullptr) {
            HashNode* tmp = current;
            current = current->next;
            delete tmp;
        }
    }
    delete[] table_;
}

int HashTable::hash(const std::string& key, int size) {
    int h = 0;
    for (char c : key) {
        h = (h * 31 + c) % size;
    }
    if (h < 0) {
        h += size;
    }
    return h;
}

int* HashTable::get(const std::string& key) {
    HashNode* current = table_[hash(key, size_)];
    while (current != nullptr) {
        if (current->key == key) {
            return &current->value;
        }
        current = current->next;
    }
    return nullptr;
}

bool HashTable::search(const std::string& key) {
    return get(key) != nullptr;
}

void HashTable::add(const std::string& key, int value) {
    int* old = get(key);
    if (old != nullptr) {
        *old = value;
        return;
    }
    if (count_ > size_ * 2) {
        rehash();
    }
    int index = hash(key, size_);
    table_[index] = new HashNode{key, value, table_[index]};
    count_++;
}

bool HashTable::remove(const std::string& key) {
    int index = hash(key, size_);
    HashNode* current = table_[index];
    HashNode* prev = nullptr;
    while (current != nullptr) {
        if (current->key == key) {
            if (prev == nullptr) {
                table_[index] = current->next;
            } else {
                prev->next = current->next;
            }
            delete current;
            count_--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

void HashTable::rehash() {
    int newSize = size_ * 2 + 1;
    HashNode** newTable = new HashNode*[newSize]();
    for (int i = 0; i < size_; i++) {
        HashNode* current = table_[i];
        while (current != nullptr) {
            HashNode* next = current->next;
            int index = hash(current->key, newSize);
            current->next = newTable[index];
            newTable[index] = current;
            current = next;
        }
    }
    delete[] table_;
    table_ = newTable;
    size_ = newSize;
}

void HashTable::getKeys(DynArray& keys) {
    for (int i = 0; i < size_; i++) {
        HashNode* current = table_[i];
        while (current != nullptr) {
            keys.push(current->key);
            current = current->next;
        }
    }
}

// ---------- хеширование кукушки ----------

Cuckoo::Cuckoo() {
    create(11);
}

void Cuckoo::create(int size) {
    size_ = size;
    count_ = 0;
    keys1_ = new std::string[size_];
    keys2_ = new std::string[size_];
    values1_ = new int[size_];
    values2_ = new int[size_];
    used1_ = new bool[size_]();  // false - ячейка пустая
    used2_ = new bool[size_]();
}

Cuckoo::~Cuckoo() {
    delete[] keys1_;
    delete[] keys2_;
    delete[] values1_;
    delete[] values2_;
    delete[] used1_;
    delete[] used2_;
}

int Cuckoo::hash1(const std::string& key) {
    int h = 0;
    for (char c : key) {
        h = (h * 31 + c) % size_;
    }
    if (h < 0) {
        h += size_;
    }
    return h;
}

int Cuckoo::hash2(const std::string& key) {
    int h = 7;
    for (char c : key) {
        h = (h * 131 + c) % size_;
    }
    if (h < 0) {
        h += size_;
    }
    return h;
}

int* Cuckoo::get(const std::string& key) {
    int i1 = hash1(key);
    if (used1_[i1] && keys1_[i1] == key) {
        return &values1_[i1];
    }
    int i2 = hash2(key);
    if (used2_[i2] && keys2_[i2] == key) {
        return &values2_[i2];
    }
    return nullptr;
}

bool Cuckoo::search(const std::string& key) {
    return get(key) != nullptr;
}

void Cuckoo::add(const std::string& key, int value) {
    int* old = get(key);
    if (old != nullptr) {
        *old = value;
        return;
    }
    insertNew(key, value);
}

void Cuckoo::insertNew(std::string key, int value) {
    for (int tempt = 0; tempt < MAXTEMPT; tempt++) {
        int i1 = hash1(key);
        if (used1_[i1] == false) {
            keys1_[i1] = key;
            values1_[i1] = value;
            used1_[i1] = true;
            count_++;
            return;
        }
        std::string tmpKey = keys1_[i1];
        int tmpValue = values1_[i1];
        keys1_[i1] = key;
        values1_[i1] = value;
        key = tmpKey;
        value = tmpValue;

        int i2 = hash2(key);
        if (used2_[i2] == false) {
            keys2_[i2] = key;
            values2_[i2] = value;
            used2_[i2] = true;
            count_++;
            return;
        }
        tmpKey = keys2_[i2];
        tmpValue = values2_[i2];
        keys2_[i2] = key;
        values2_[i2] = value;
        key = tmpKey;
        value = tmpValue;
    }
    grow();
    insertNew(key, value);
}

void Cuckoo::grow() {
    int oldSize = size_;
    std::string* oldKeys1 = keys1_;
    std::string* oldKeys2 = keys2_;
    int* oldValues1 = values1_;
    int* oldValues2 = values2_;
    bool* oldUsed1 = used1_;
    bool* oldUsed2 = used2_;

    create(oldSize * 2 + 1);

    for (int i = 0; i < oldSize; i++) {
        if (oldUsed1[i]) insertNew(oldKeys1[i], oldValues1[i]);
        if (oldUsed2[i]) insertNew(oldKeys2[i], oldValues2[i]);
    }

    delete[] oldKeys1;
    delete[] oldKeys2;
    delete[] oldValues1;
    delete[] oldValues2;
    delete[] oldUsed1;
    delete[] oldUsed2;
}

bool Cuckoo::remove(const std::string& key) {
    int i1 = hash1(key);
    if (used1_[i1] && keys1_[i1] == key) {
        used1_[i1] = false;
        count_--;
        return true;
    }
    int i2 = hash2(key);
    if (used2_[i2] && keys2_[i2] == key) {
        used2_[i2] = false;
        count_--;
        return true;
    }
    return false;
}

void Cuckoo::getKeys(DynArray& keys) {
    for (int i = 0; i < size_; i++) {
        if (used1_[i]) keys.push(keys1_[i]);
        if (used2_[i]) keys.push(keys2_[i]);
    }
}
