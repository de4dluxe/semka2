package main

// Задание 2. АТД Множество: SETADD, SETDEL, SET_AT, SETPRINT.
// Множество устроено на хеш-таблице из задания 6.1 (значение всегда 1),
// поэтому все операции выполняются за O(1) в среднем.

import (
	"fmt"
	"strings"
)

type Set struct {
	table *ChainTable
}

func NewSet() *Set {
	return &Set{table: NewChainTable()}
}

func (s *Set) SetAdd(x string) {
	s.table.Add(x, 1)
}

func (s *Set) SetDel(x string) bool {
	return s.table.Remove(x)
}

func (s *Set) SetAt(x string) bool {
	_, ok := s.table.Get(x)
	return ok
}

func (s *Set) Count() int {
	return s.table.Count()
}

func (s *Set) All() []string {
	return s.table.Keys()
}

func handleSet(cmd string, db DB, name string, args *Args) (bool, error) {
	entry, err := openData(db, name, "set", cmd == "SETADD")
	if err != nil {
		return false, err
	}

	// множество собирается из массива в файле
	set := NewSet()
	list, _ := entry["data"].([]any)
	for _, e := range list {
		set.SetAdd(toString(e))
	}

	if cmd == "SETPRINT" {
		fmt.Printf("{%s} (размер=%d)\n", strings.Join(set.All(), ", "), set.Count())
		return false, nil
	}

	x, err := args.word("элемент")
	if err != nil {
		return false, err
	}
	switch cmd {
	case "SET_AT":
		fmt.Printf("-> %t\n", set.SetAt(x))
		return false, nil
	case "SETADD":
		if set.SetAt(x) {
			fmt.Printf("-> %s (уже есть)\n", x)
			return false, nil
		}
		set.SetAdd(x)
	default: // SETDEL
		if !set.SetDel(x) {
			return false, fmt.Errorf("элемента %s нет в множестве %s", x, name)
		}
	}
	fmt.Printf("-> %s\n", x)

	// записываем множество обратно в файл
	arr := []any{}
	for _, e := range set.All() {
		arr = append(arr, e)
	}
	entry["data"] = arr
	return true, nil
}
