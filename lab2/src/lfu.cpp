#include "lfu.h"

LFUCache::LFUCache(int capacity) {
    capacity_ = capacity;
    count_ = 0;
    clock_ = 0;
    keys_ = new int[capacity_ + 1];
    values_ = new int[capacity_ + 1];
    freqs_ = new int[capacity_ + 1];
    times_ = new int[capacity_ + 1];
}

LFUCache::~LFUCache() {
    delete[] keys_;
    delete[] values_;
    delete[] freqs_;
    delete[] times_;
}

int LFUCache::find(int key) {
    for (int i = 0; i < count_; i++) {
        if (keys_[i] == key) {
            return i;
        }
    }
    return -1;
}

void LFUCache::touch(int i) {
    freqs_[i]++;
    clock_++;
    times_[i] = clock_;
}

int LFUCache::get(int key) {
    int i = find(key);
    if (i == -1) {
        return -1;
    }
    touch(i);
    return values_[i];
}

void LFUCache::set(int key, int value) {
    if (capacity_ == 0) return;

    int i = find(key);
    if (i != -1) {
        values_[i] = value;
        touch(i);
        return;
    }

    if (count_ == capacity_) {
        // место закончилось: ищем ключ с наименьшим числом обращений (при равенстве - самый давний)
        int worst = 0;
        for (int j = 1; j < count_; j++) {
            if (freqs_[j] < freqs_[worst] || (freqs_[j] == freqs_[worst] && times_[j] < times_[worst])) {
                worst = j;
            }
        }
        keys_[worst] = keys_[count_ - 1];
        values_[worst] = values_[count_ - 1];
        freqs_[worst] = freqs_[count_ - 1];
        times_[worst] = times_[count_ - 1];
        count_--;
    }

    keys_[count_] = key;
    values_[count_] = value;
    freqs_[count_] = 0;
    count_++;
    touch(count_ - 1);
}

void LFUCache::getItem(int i, int& key, int& value, int& freq, int& time) const {
    key = keys_[i];
    value = values_[i];
    freq = freqs_[i];
    time = times_[i];
}

void LFUCache::restore(int key, int value, int freq, int time) {
    if (count_ == capacity_) return;
    keys_[count_] = key;
    values_[count_] = value;
    freqs_[count_] = freq;
    times_[count_] = time;
    count_++;
    if (time > clock_) {
        clock_ = time;
    }
}
