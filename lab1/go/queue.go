package main

import "fmt"

type qNode struct {
	val  string
	next *qNode
}

// Queue - очередь на односвязном списке: вход в хвост, выход из головы.
type Queue struct{ head, tail *qNode }

func (q *Queue) Push(v string) {
	n := &qNode{val: v}
	if q.tail != nil {
		q.tail.next = n
	} else {
		q.head = n
	}
	q.tail = n
}

func (q *Queue) Pop() (string, bool) {
	if q.head == nil {
		return "", false
	}
	v := q.head.val
	q.head = q.head.next
	if q.head == nil {
		q.tail = nil
	}
	return v, true
}

func (q *Queue) Peek() (string, bool) {
	if q.head == nil {
		return "", false
	}
	return q.head.val, true
}

func (q *Queue) Items() []string {
	r := []string{}
	for n := q.head; n != nil; n = n.next {
		r = append(r, n.val)
	}
	return r
}

func handleQueue(cmd string, db DB, name string, args *Args) bool {
	q := &Queue{}
	for _, it := range openData(db, name, "queue", cmd == "QPUSH") {
		q.Push(it)
	}

	changed := false
	switch cmd {
	case "QPUSH":
		v := args.Word("значение")
		q.Push(v)
		fmt.Println("->", v)
		changed = true
	case "QPOP":
		v, ok := q.Pop()
		if !ok {
			fail("очередь пуста")
		}
		fmt.Println("->", v)
		changed = true
	case "QPEEK":
		v, ok := q.Peek()
		if !ok {
			fail("очередь пуста")
		}
		fmt.Println("->", v)
	case "QPRINT":
		items := q.Items()
		fmt.Print("голова: ")
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
		store(db, name, "queue", q.Items())
	}
	return changed
}
