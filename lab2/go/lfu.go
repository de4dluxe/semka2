package main

// Задание 7 (вариант 2). LFU-кэш.
//   LFUINIT имя ёмкость, LFUSET имя ключ значение, LFUGET имя ключ, LFUPRINT имя
// В файле: {"cap": 2, "items": [[ключ, значение, сколько раз обращались, когда в последний раз], ...]}

import (
	"fmt"
)

type LFUCache struct {
	capacity int
	keys     []int
	values   []int
	freqs    []int // сколько раз обращались к ключу
	times    []int // когда обращались в последний раз
	clock    int
}

func NewLFUCache(capacity int) *LFUCache {
	return &LFUCache{capacity: capacity}
}

func (c *LFUCache) find(key int) int {
	for i := range c.keys {
		if c.keys[i] == key {
			return i
		}
	}
	return -1
}

func (c *LFUCache) touch(i int) {
	c.freqs[i]++
	c.clock++
	c.times[i] = c.clock
}

// Get возвращает значение или -1, если ключа нет.
func (c *LFUCache) Get(key int) int {
	i := c.find(key)
	if i == -1 {
		return -1
	}
	c.touch(i)
	return c.values[i]
}

func (c *LFUCache) Set(key, value int) {
	if c.capacity == 0 {
		return
	}
	i := c.find(key)
	if i != -1 {
		c.values[i] = value
		c.touch(i)
		return
	}

	if len(c.keys) == c.capacity {
		// место закончилось: ищем ключ с наименьшим числом обращений (при равенстве - самый давний)
		worst := 0
		for j := 1; j < len(c.keys); j++ {
			if c.freqs[j] < c.freqs[worst] || (c.freqs[j] == c.freqs[worst] && c.times[j] < c.times[worst]) {
				worst = j
			}
		}
		last := len(c.keys) - 1
		c.keys[worst], c.values[worst], c.freqs[worst], c.times[worst] = c.keys[last], c.values[last], c.freqs[last], c.times[last]
		c.keys, c.values, c.freqs, c.times = c.keys[:last], c.values[:last], c.freqs[:last], c.times[:last]
	}

	c.keys = append(c.keys, key)
	c.values = append(c.values, value)
	c.freqs = append(c.freqs, 0)
	c.times = append(c.times, 0)
	c.touch(len(c.keys) - 1)
}

func (c *LFUCache) Size() int {
	return len(c.keys)
}

// Restore добавляет элемент, прочитанный из файла.
func (c *LFUCache) Restore(key, value, freq, time int) {
	if len(c.keys) == c.capacity {
		return
	}
	c.keys = append(c.keys, key)
	c.values = append(c.values, value)
	c.freqs = append(c.freqs, freq)
	c.times = append(c.times, time)
	if time > c.clock {
		c.clock = time
	}
}

func handleLFU(cmd string, db DB, name string, args *Args) (bool, error) {
	if cmd == "LFUINIT" {
		capacity, err := args.number("ёмкость")
		if err != nil {
			return false, err
		}
		if capacity < 0 {
			return false, fmt.Errorf("ёмкость не может быть отрицательной")
		}
		entry, err := openObject(db, name, "lfu", true)
		if err != nil {
			return false, err
		}
		entry["data"] = map[string]any{"cap": capacity, "items": []any{}}
		fmt.Printf("-> ёмкость %d\n", capacity)
		return true, nil
	}

	entry, err := openObject(db, name, "lfu", false)
	if err != nil {
		return false, err
	}
	data, _ := entry["data"].(map[string]any)
	capValue, hasCap := data["cap"]
	if !hasCap {
		return false, fmt.Errorf("кэш %s не создан, сначала LFUINIT", name)
	}

	// кэш собирается из файла
	capacity := toInt(capValue)
	cache := NewLFUCache(capacity)
	items, _ := data["items"].([]any)
	for _, e := range items {
		item, _ := e.([]any)
		cache.Restore(toInt(item[0]), toInt(item[1]), toInt(item[2]), toInt(item[3]))
	}

	if cmd == "LFUPRINT" {
		for i := range cache.keys {
			fmt.Printf("%d:%d (обращений: %d)  ", cache.keys[i], cache.values[i], cache.freqs[i])
		}
		fmt.Printf("[%d/%d]\n", cache.Size(), capacity)
		return false, nil
	}

	key, err := args.number("ключ")
	if err != nil {
		return false, err
	}
	if cmd == "LFUGET" {
		fmt.Printf("-> %d\n", cache.Get(key))
	} else { // LFUSET
		value, err := args.number("значение")
		if err != nil {
			return false, err
		}
		cache.Set(key, value)
		fmt.Println("-> ok")
	}

	// записываем кэш обратно в файл (GET тоже меняет число обращений)
	newItems := []any{}
	for i := range cache.keys {
		newItems = append(newItems, []any{cache.keys[i], cache.values[i], cache.freqs[i], cache.times[i]})
	}
	data["items"] = newItems
	return true, nil
}
