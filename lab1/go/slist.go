package main

import "fmt"

type sNode struct {
	val  string
	next *sNode
}

// SList - односвязный список (голова + хвост).
type SList struct {
	head, tail *sNode
	size       int
}

func (l *SList) find(v string) *sNode {
	for n := l.head; n != nil; n = n.next {
		if n.val == v {
			return n
		}
	}
	return nil
}

func (l *SList) PushHead(v string) {
	l.head = &sNode{v, l.head}
	if l.tail == nil {
		l.tail = l.head
	}
	l.size++
}

func (l *SList) PushTail(v string) {
	n := &sNode{val: v}
	if l.tail != nil {
		l.tail.next = n
	} else {
		l.head = n
	}
	l.tail = n
	l.size++
}

func (l *SList) PushAfter(key, v string) bool {
	at := l.find(key)
	if at == nil {
		return false
	}
	at.next = &sNode{v, at.next}
	if at == l.tail {
		l.tail = at.next
	}
	l.size++
	return true
}

func (l *SList) PushBefore(key, v string) bool {
	if l.head == nil {
		return false
	}
	if l.head.val == key {
		l.PushHead(v)
		return true
	}
	for p := l.head; p.next != nil; p = p.next {
		if p.next.val == key {
			p.next = &sNode{v, p.next}
			l.size++
			return true
		}
	}
	return false
}

func (l *SList) PopHead() (string, bool) {
	if l.head == nil {
		return "", false
	}
	n := l.head
	l.head = n.next
	if l.head == nil {
		l.tail = nil
	}
	l.size--
	return n.val, true
}

func (l *SList) PopTail() (string, bool) {
	if l.head == nil {
		return "", false
	}
	if l.head == l.tail {
		return l.PopHead()
	}
	p := l.head
	for p.next != l.tail { // предпоследний
		p = p.next
	}
	v := l.tail.val
	p.next = nil
	l.tail = p
	l.size--
	return v, true
}

func (l *SList) DelAfter(key string) (string, bool) {
	at := l.find(key)
	if at == nil || at.next == nil {
		return "", false
	}
	victim := at.next
	at.next = victim.next
	if victim == l.tail {
		l.tail = at
	}
	l.size--
	return victim.val, true
}

func (l *SList) DelBefore(key string) (string, bool) {
	if l.head == nil || l.head.val == key {
		return "", false // перед головой ничего нет
	}
	var prev *sNode // узел перед удаляемым
	cur := l.head   // удаляемый
	for cur.next != nil && cur.next.val != key {
		prev = cur
		cur = cur.next
	}
	if cur.next == nil { // key не найден
		return "", false
	}
	if prev != nil {
		prev.next = cur.next
	} else {
		l.head = cur.next
	}
	l.size--
	return cur.val, true
}

func (l *SList) DelValue(v string) bool {
	var prev *sNode
	for n := l.head; n != nil; prev, n = n, n.next {
		if n.val != v {
			continue
		}
		if prev != nil {
			prev.next = n.next
		} else {
			l.head = n.next
		}
		if n == l.tail {
			l.tail = prev
		}
		l.size--
		return true
	}
	return false
}

func (l *SList) Contains(v string) bool { return l.find(v) != nil }

func (l *SList) Get(i int) string {
	if i >= l.size {
		fail("индекс %d вне списка (длина %d)", i, l.size)
	}
	n := l.head
	for ; i > 0; i-- {
		n = n.next
	}
	return n.val
}

func (l *SList) Items() []string {
	r := []string{}
	for n := l.head; n != nil; n = n.next {
		r = append(r, n.val)
	}
	return r
}

func reversed(s []string) []string {
	r := make([]string, len(s))
	for i, v := range s {
		r[len(s)-1-i] = v
	}
	return r
}

func handleSList(cmd string, db DB, name string, args *Args) bool {
	create := cmd == "FPUSHH" || cmd == "FPUSHT" || cmd == "FPUSHA" || cmd == "FPUSHB"
	list := &SList{}
	for _, it := range openData(db, name, "flist", create) {
		list.PushTail(it)
	}

	changed := false
	switch cmd {
	case "FPUSHH":
		v := args.Word("значение")
		list.PushHead(v)
		fmt.Println("->", v)
		changed = true
	case "FPUSHT":
		v := args.Word("значение")
		list.PushTail(v)
		fmt.Println("->", v)
		changed = true
	case "FPUSHA":
		key := args.Word("элемент, после которого вставить")
		v := args.Word("значение")
		if !list.PushAfter(key, v) {
			fail("элемент %s не найден", key)
		}
		fmt.Println("->", v)
		changed = true
	case "FPUSHB":
		key := args.Word("элемент, перед которым вставить")
		v := args.Word("значение")
		if !list.PushBefore(key, v) {
			fail("элемент %s не найден", key)
		}
		fmt.Println("->", v)
		changed = true
	case "FDELH":
		v, ok := list.PopHead()
		if !ok {
			fail("список пуст")
		}
		fmt.Println("->", v)
		changed = true
	case "FDELT":
		v, ok := list.PopTail()
		if !ok {
			fail("список пуст")
		}
		fmt.Println("->", v)
		changed = true
	case "FDELA":
		key := args.Word("элемент")
		v, ok := list.DelAfter(key)
		if !ok {
			fail("после %s удалять нечего (или %s нет в списке)", key, key)
		}
		fmt.Println("->", v)
		changed = true
	case "FDELB":
		key := args.Word("элемент")
		v, ok := list.DelBefore(key)
		if !ok {
			fail("перед %s удалять нечего (или %s нет в списке)", key, key)
		}
		fmt.Println("->", v)
		changed = true
	case "FDEL":
		v := args.Word("значение")
		if !list.DelValue(v) {
			fail("элемент %s не найден", v)
		}
		fmt.Println("->", v)
		changed = true
	case "FFIND":
		fmt.Println("->", boolText(list.Contains(args.Word("значение"))))
	case "FGET":
		fmt.Println("->", list.Get(args.Index()))
	case "FPRINT":
		printItems(list.Items())
	case "FPRINTR":
		printItems(reversed(list.Items())) // назад по односвязному списку не пройти - разворачиваем копию
	}

	if changed {
		store(db, name, "flist", list.Items())
	}
	return changed
}
