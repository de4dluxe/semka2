package main

// Задание 6.1 (вариант 2). Хеш-таблицы: метод цепочек и хеширование кукушки.
//   CHSET имя ключ число, CHGET имя ключ, CHDEL имя ключ, CHPRINT имя     (цепочки)
//   CKSET имя ключ число, CKGET имя ключ, CKDEL имя ключ, CKPRINT имя     (кукушка)
// В файле таблица хранится объектом {"ключ": число}.

import (
	"fmt"
	"sort"
	"strings"
)

// runTable работает для обеих таблиц: у них одинаковые методы.
func runTable(op, typ string, db DB, name string, args *Args, table Table) (bool, error) {
	entry, err := openObject(db, name, typ, op == "SET")
	if err != nil {
		return false, err
	}
	data, _ := entry["data"].(map[string]any)
	// ключи из файла берём по алфавиту, чтобы вывод не зависел от порядка в map
	fileKeys := make([]string, 0, len(data))
	for key := range data {
		fileKeys = append(fileKeys, key)
	}
	sort.Strings(fileKeys)
	for _, key := range fileKeys {
		table.Add(key, toInt(data[key]))
	}

	if op == "PRINT" {
		var parts []string
		for _, key := range table.Keys() {
			value, _ := table.Get(key)
			parts = append(parts, fmt.Sprintf("%s: %d", key, value))
		}
		fmt.Printf("{%s} (размер=%d)\n", strings.Join(parts, ", "), table.Count())
		return false, nil
	}

	key, err := args.word("ключ")
	if err != nil {
		return false, err
	}
	switch op {
	case "GET":
		value, ok := table.Get(key)
		if !ok {
			return false, fmt.Errorf("нет ключа %s", key)
		}
		fmt.Printf("-> %d\n", value)
		return false, nil
	case "DEL":
		if !table.Remove(key) {
			return false, fmt.Errorf("нет ключа %s", key)
		}
		fmt.Printf("-> %s\n", key)
	default: // SET
		value, err := args.number("число")
		if err != nil {
			return false, err
		}
		table.Add(key, value)
		fmt.Printf("-> %s %d\n", key, value)
	}

	// записываем таблицу обратно в файл
	obj := map[string]any{}
	for _, k := range table.Keys() {
		value, _ := table.Get(k)
		obj[k] = value
	}
	entry["data"] = obj
	return true, nil
}

func handleChain(cmd string, db DB, name string, args *Args) (bool, error) {
	return runTable(cmd[2:], "hashchain", db, name, args, NewChainTable())
}

func handleCuckoo(cmd string, db DB, name string, args *Args) (bool, error) {
	return runTable(cmd[2:], "hashcuckoo", db, name, args, NewCuckoo())
}
