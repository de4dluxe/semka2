package main

import "fmt"

type stNode struct {
	val   string
	below *stNode
}

// Stack - стек на односвязном списке: вершина = голова.
type Stack struct{ top *stNode }

func (s *Stack) Push(v string) { s.top = &stNode{v, s.top} }

func (s *Stack) Pop() (string, bool) {
	if s.top == nil {
		return "", false
	}
	v := s.top.val
	s.top = s.top.below
	return v, true
}

func (s *Stack) Peek() (string, bool) {
	if s.top == nil {
		return "", false
	}
	return s.top.val, true
}

func (s *Stack) FromTop() []string {
	r := []string{}
	for n := s.top; n != nil; n = n.below {
		r = append(r, n.val)
	}
	return r
}

func handleStack(cmd string, db DB, name string, args *Args) bool {
	st := &Stack{}
	for _, it := range openData(db, name, "stack", cmd == "SPUSH") { // в файле дно -> вершина
		st.Push(it)
	}

	changed := false
	switch cmd {
	case "SPUSH":
		v := args.Word("значение")
		st.Push(v)
		fmt.Println("->", v)
		changed = true
	case "SPOP":
		v, ok := st.Pop()
		if !ok {
			fail("стек пуст")
		}
		fmt.Println("->", v)
		changed = true
	case "SPEEK":
		v, ok := st.Peek()
		if !ok {
			fail("стек пуст")
		}
		fmt.Println("->", v)
	case "SPRINT":
		items := st.FromTop()
		fmt.Print("вершина: ")
		if len(items) == 0 {
			fmt.Print("(пусто)")
		}
		for i, it := range items {
			if i > 0 {
				fmt.Print(" | ")
			}
			fmt.Print(it)
		}
		fmt.Println()
	}

	if changed {
		store(db, name, "stack", reversed(st.FromTop())) // пишем от дна к вершине
	}
	return changed
}
