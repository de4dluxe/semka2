package main

// Задание 3 (вариант 6). Черепахи на множестве из задания 2.
// TURADD имя a b (впереди a черепах, позади b), TURRUN имя, TURPRINT имя.
//
// Черепаха на позиции p (с нуля) видит впереди p черепах, а позади N-1-p. Значит, говорить
// правду она может только если a + b = N - 1, и тогда её позиция равна a. Две черепахи
// не могут стоять на одном месте, поэтому ответ - сколько разных значений a среди подходящих.

import (
	"fmt"
	"strconv"
	"strings"
)

func handleTurtles(cmd string, db DB, name string, args *Args) (bool, error) {
	if cmd == "TURADD" {
		a, err := args.number("a (сколько впереди)")
		if err != nil {
			return false, err
		}
		b, err := args.number("b (сколько позади)")
		if err != nil {
			return false, err
		}
		if a < 0 || b < 0 {
			return false, fmt.Errorf("a и b не могут быть отрицательными")
		}
		entry, err := openData(db, name, "turtles", true)
		if err != nil {
			return false, err
		}
		list, _ := entry["data"].([]any)
		entry["data"] = append(list, []any{a, b})
		fmt.Printf("-> (%d, %d)\n", a, b)
		return true, nil
	}

	entry, err := openData(db, name, "turtles", false)
	if err != nil {
		return false, err
	}
	list, _ := entry["data"].([]any)
	if cmd == "TURPRINT" {
		var parts []string
		for _, e := range list {
			pair, _ := e.([]any)
			parts = append(parts, fmt.Sprintf("(%d, %d) ", toInt(pair[0]), toInt(pair[1])))
		}
		fmt.Printf("%s(черепах: %d)\n", strings.Join(parts, ""), len(list))
		return false, nil
	}

	n := len(list)
	positions := NewSet()
	for _, e := range list {
		pair, _ := e.([]any)
		a, b := toInt(pair[0]), toInt(pair[1])
		if a+b == n-1 {
			positions.SetAdd(strconv.Itoa(a))
		}
	}
	fmt.Printf("-> %d\n", positions.Count())
	return false, nil
}
