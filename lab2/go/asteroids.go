package main

// Задание 1 (вариант 8). Астероиды: AADD имя размер R|L, ARUN имя, APRINT имя.
// ARUN показывает, что останется после столкновений (сами данные не меняет).

import (
	"fmt"
	"strings"
)

type Asteroid struct {
	Size int
	Dir  byte // 'R' - летит вправо, 'L' - влево
}

// Stack - стек астероидов.
type Stack struct {
	items []Asteroid
}

func (s *Stack) Push(a Asteroid) {
	s.items = append(s.items, a)
}

func (s *Stack) Pop() Asteroid {
	a := s.items[len(s.items)-1]
	s.items = s.items[:len(s.items)-1]
	return a
}

func (s *Stack) Peek() (Asteroid, bool) {
	if len(s.items) == 0 {
		return Asteroid{}, false
	}
	return s.items[len(s.items)-1], true
}

// В файле астероид записан строкой "размер направление".
func (a Asteroid) toText() string {
	return fmt.Sprintf("%d %c", a.Size, a.Dir)
}

func asteroidFromText(s string) Asteroid {
	var a Asteroid
	var dir string
	fmt.Sscanf(s, "%d %s", &a.Size, &dir)
	if dir != "" {
		a.Dir = dir[0]
	}
	return a
}

func printAsteroids(list []Asteroid) {
	var parts []string
	for _, a := range list {
		parts = append(parts, fmt.Sprintf("('%d', '%c')", a.Size, a.Dir))
	}
	fmt.Printf("[%s]\n", strings.Join(parts, ", "))
}

// collide идёт слева направо. Астероид, летящий влево, сталкивается с теми, что лежат
// на вершине стека и летят вправо. Меньший взрывается, больший становится равным
// разнице размеров, равные взрываются оба.
func collide(list []Asteroid) []Asteroid {
	var stack Stack
	for _, a := range list {
		alive := true
		for alive && a.Dir == 'L' {
			top, ok := stack.Peek()
			if !ok || top.Dir != 'R' {
				break
			}
			stack.Pop()
			if top.Size < a.Size {
				a.Size -= top.Size // взорвался тот, что в стеке, летим дальше
			} else if top.Size > a.Size {
				top.Size -= a.Size // взорвался текущий
				stack.Push(top)
				alive = false
			} else {
				alive = false // взорвались оба
			}
		}
		if alive {
			stack.Push(a)
		}
	}
	return stack.items // от дна к вершине = слева направо
}

func handleAsteroids(cmd string, db DB, name string, args *Args) (bool, error) {
	if cmd == "AADD" {
		size, err := args.number("размер")
		if err != nil {
			return false, err
		}
		dir, err := args.word("направление (R или L)")
		if err != nil {
			return false, err
		}
		if size <= 0 {
			return false, fmt.Errorf("размер должен быть положительным")
		}
		if dir != "R" && dir != "L" {
			return false, fmt.Errorf("направление должно быть R или L, а не %s", dir)
		}
		entry, err := openData(db, name, "asteroids", true)
		if err != nil {
			return false, err
		}
		list, _ := entry["data"].([]any)
		entry["data"] = append(list, Asteroid{size, dir[0]}.toText())
		fmt.Printf("-> (%d, %s)\n", size, dir)
		return true, nil
	}

	entry, err := openData(db, name, "asteroids", false)
	if err != nil {
		return false, err
	}
	var list []Asteroid
	items, _ := entry["data"].([]any)
	for _, e := range items {
		list = append(list, asteroidFromText(toString(e)))
	}
	if cmd == "ARUN" {
		printAsteroids(collide(list))
	} else {
		printAsteroids(list)
	}
	return false, nil
}
