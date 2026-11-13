package main

// Задание 6.2 (вариант 9). Родословная: GADD имя потомок родитель, GRUN имя, GPRINT имя.
// GRUN выводит имена по алфавиту с числом потомков и затем само дерево.
// Используется хеш-таблица из 6.1 (имя -> номер).

import (
	"fmt"
	"strings"
)

type People struct {
	ids      *ChainTable // имя -> номер человека
	names    []string    // имена в порядке появления
	children [][]string  // children[номер] - дети в порядке ввода
	parent   []int       // номер родителя или -1
	desc     []int       // число потомков
	visited  int
}

func NewPeople() *People {
	return &People{ids: NewChainTable()}
}

func (p *People) addName(name string) {
	if _, ok := p.ids.Get(name); !ok {
		p.ids.Add(name, len(p.names))
		p.names = append(p.names, name)
	}
}

// init выделяет массивы, когда все имена уже известны.
func (p *People) init() {
	n := len(p.names)
	p.children = make([][]string, n)
	p.parent = make([]int, n)
	p.desc = make([]int, n)
	for i := 0; i < n; i++ {
		p.parent[i] = -1
	}
}

func (p *People) id(name string) int {
	n, _ := p.ids.Get(name)
	return n
}

// countDesc: потомки человека = дети + потомки каждого из детей.
func (p *People) countDesc(name string) int {
	id := p.id(name)
	p.visited++
	total := 0
	for _, kid := range p.children[id] {
		total += p.countDesc(kid) + 1
	}
	p.desc[id] = total
	return total
}

func (p *People) printTree(name string, depth int) {
	fmt.Println(strings.Repeat("  ", depth) + name)
	for _, kid := range p.children[p.id(name)] {
		p.printTree(kid, depth+1)
	}
}

// sortNames - сортировка вставками.
func sortNames(names []string) {
	for i := 1; i < len(names); i++ {
		key := names[i]
		j := i - 1
		for j >= 0 && names[j] > key {
			names[j+1] = names[j]
			j--
		}
		names[j+1] = key
	}
}

func handleGenealogy(cmd string, db DB, name string, args *Args) (bool, error) {
	if cmd == "GADD" {
		child, err := args.word("потомок")
		if err != nil {
			return false, err
		}
		parent, err := args.word("родитель")
		if err != nil {
			return false, err
		}
		if child == parent {
			return false, fmt.Errorf("человек не может быть родителем самому себе")
		}
		entry, err := openData(db, name, "genealogy", true)
		if err != nil {
			return false, err
		}
		list, _ := entry["data"].([]any)
		entry["data"] = append(list, []any{child, parent})
		fmt.Printf("-> %s %s\n", child, parent)
		return true, nil
	}

	entry, err := openData(db, name, "genealogy", false)
	if err != nil {
		return false, err
	}
	pairs, _ := entry["data"].([]any)
	if cmd == "GPRINT" {
		for _, e := range pairs {
			pair, _ := e.([]any)
			fmt.Println(toString(pair[0]) + " " + toString(pair[1]))
		}
		return false, nil
	}
	if len(pairs) == 0 {
		return false, fmt.Errorf("в %s нет пар, добавьте их через GADD", name)
	}

	// GRUN. Сначала запоминаем всех людей, потом связи между ними.
	family := NewPeople()
	for _, e := range pairs {
		pair, _ := e.([]any)
		family.addName(toString(pair[0]))
		family.addName(toString(pair[1]))
	}
	family.init()
	for _, e := range pairs {
		pair, _ := e.([]any)
		child, parent := toString(pair[0]), toString(pair[1])
		childID := family.id(child)
		if family.parent[childID] != -1 {
			return false, fmt.Errorf("у %s указано два родителя", child)
		}
		family.parent[childID] = family.id(parent)
		family.children[family.id(parent)] = append(family.children[family.id(parent)], child)
	}

	// родоначальник - тот, у кого нет родителя, он должен быть один
	roots := 0
	root := 0
	for i := range family.names {
		if family.parent[i] == -1 {
			root = i
			roots++
		}
	}
	if roots != 1 {
		return false, fmt.Errorf("родоначальник должен быть один, а найдено %d", roots)
	}

	family.countDesc(family.names[root])
	if family.visited != len(family.names) {
		return false, fmt.Errorf("в данных есть цикл")
	}

	sorted := make([]string, len(family.names))
	copy(sorted, family.names)
	sortNames(sorted)
	for _, n := range sorted {
		fmt.Printf("%s %d\n", n, family.desc[family.id(n)])
	}

	fmt.Println("\nДерево:")
	family.printTree(family.names[root], 0)
	return false, nil
}
