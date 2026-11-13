#pragma once

class LFUCache {
public:
    LFUCache(int capacity);
    ~LFUCache();
    LFUCache(const LFUCache&) = delete;
    LFUCache& operator=(const LFUCache&) = delete;

    int get(int key);
    void set(int key, int value);

    // для сохранения в файл и загрузки из него
    int size() const { return count_; }
    void getItem(int i, int& key, int& value, int& freq, int& time) const;
    void restore(int key, int value, int freq, int time);

private:
    int capacity_;
    int count_;
    int clock_;      // номер последнего обращения
    int* keys_;
    int* values_;
    int* freqs_;     // сколько раз обращались
    int* times_;     // когда обращались в последний раз
    int find(int key);
    void touch(int i);
};
