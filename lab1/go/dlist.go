package main

import "fmt"

type dNode struct {
	val        string
	prev, next *dNode
}

// DList - двусвязный список.
type DList struct {
	head, tail *dNode
	size       int
}

func (l *DList) find(v string) *dNode {
	for n := l.head; n != nil; n = n.next {
		if n.val == v {
			return n
		}
	}
	return nil
}

// unlink вырезает узел из списка.
func (l *DList) unlink(n *dNode) {
	if n.prev != nil {
		n.prev.next = n.next
	} else {
		l.head = n.next
	}
	if n.next != nil {
		n.next.prev = n.prev
	} else {
		l.tail = n.prev
	}
	l.size--
}

func (l *DList) PushHead(v string) {
	n := &dNode{val: v, next: l.head}
	if l.head != nil {
		l.head.prev = n
	} else {
		l.tail = n
	}
	l.head = n
	l.size++
}

func (l *DList) PushTail(v string) {
	n := &dNode{val: v, prev: l.tail}
	if l.tail != nil {
		l.tail.next = n
	} else {
		l.head = n
	}
	l.tail = n
	l.size++
}

func (l *DList) PushAfter(key, v string) bool {
	at := l.find(key)
	if at == nil {
		return false
	}
	n := &dNode{val: v, prev: at, next: at.next}
	if at.next != nil {
		at.next.prev = n
	} else {
		l.tail = n
	}
	at.next = n
	l.size++
	return true
}

func (l *DList) PushBefore(key, v string) bool {
	at := l.find(key)
	if at == nil {
		return false
	}
	n := &dNode{val: v, prev: at.prev, next: at}
	if at.prev != nil {
		at.prev.next = n
	} else {
		l.head = n
	}
	at.prev = n
	l.size++
	return true
}

func (l *DList) PopHead() (string, bool) {
	if l.head == nil {
		return "", false
	}
	v := l.head.val
	l.unlink(l.head)
	return v, true
}

func (l *DList) PopTail() (string, bool) {
	if l.tail == nil {
		return "", false
	}
	v := l.tail.val
	l.unlink(l.tail)
	return v, true
}

func (l *DList) DelAfter(key string) (string, bool) {
	at := l.find(key)
	if at == nil || at.next == nil {
		return "", false
	}
	v := at.next.val
	l.unlink(at.next)
	return v, true
}

func (l *DList) DelBefore(key string) (string, bool) {
	at := l.find(key)
	if at == nil || at.prev == nil {
		return "", false
	}
	v := at.prev.val
	l.unlink(at.prev)
	return v, true
}

func (l *DList) DelValue(v string) bool {
	n := l.find(v)
	if n == nil {
		return false
	}
	l.unlink(n)
	return true
}

func (l *DList) Contains(v string) bool { return l.find(v) != nil }

// Get идёт с ближнего к индексу конца.
func (l *DList) Get(i int) string {
	if i >= l.size {
		fail("индекс %d вне списка (длина %d)", i, l.size)
	}
	if i < l.size/2 {
		n := l.head
		for ; i > 0; i-- {
			n = n.next
		}
		return n.val
	}
	n := l.tail
	for j := l.size - 1; j > i; j-- {
		n = n.prev
	}
	return n.val
}

func (l *DList) Items() []string {
	r := []string{}
	for n := l.head; n != nil; n = n.next {
		r = append(r, n.val)
	}
	return r
}

func (l *DList) ItemsReversed() []string {
	r := []string{}
	for n := l.tail; n != nil; n = n.prev {
		r = append(r, n.val)
	}
	return r
}

func handleDList(cmd string, db DB, name string, args *Args) bool {
	create := cmd == "LPUSHH" || cmd == "LPUSHT" || cmd == "LPUSHA" || cmd == "LPUSHB"
	list := &DList{}
	for _, it := range openData(db, name, "dlist", create) {
		list.PushTail(it)
	}

	changed := false
	switch cmd {
	case "LPUSHH":
		v := args.Word("значение")
		list.PushHead(v)
		fmt.Println("->", v)
		changed = true
	case "LPUSHT":
		v := args.Word("значение")
		list.PushTail(v)
		fmt.Println("->", v)
		changed = true
	case "LPUSHA":
		key := args.Word("элемент, после которого вставить")
		v := args.Word("значение")
		if !list.PushAfter(key, v) {
			fail("элемент %s не найден", key)
		}
		fmt.Println("->", v)
		changed = true
	case "LPUSHB":
		key := args.Word("элемент, перед которым вставить")
		v := args.Word("значение")
		if !list.PushBefore(key, v) {
			fail("элемент %s не найден", key)
		}
		fmt.Println("->", v)
		changed = true
	case "LDELH":
		v, ok := list.PopHead()
		if !ok {
			fail("список пуст")
		}
		fmt.Println("->", v)
		changed = true
	case "LDELT":
		v, ok := list.PopTail()
		if !ok {
			fail("список пуст")
		}
		fmt.Println("->", v)
		changed = true
	case "LDELA":
		key := args.Word("элемент")
		v, ok := list.DelAfter(key)
		if !ok {
			fail("после %s удалять нечего (или %s нет в списке)", key, key)
		}
		fmt.Println("->", v)
		changed = true
	case "LDELB":
		key := args.Word("элемент")
		v, ok := list.DelBefore(key)
		if !ok {
			fail("перед %s удалять нечего (или %s нет в списке)", key, key)
		}
		fmt.Println("->", v)
		changed = true
	case "LDEL":
		v := args.Word("значение")
		if !list.DelValue(v) {
			fail("элемент %s не найден", v)
		}
		fmt.Println("->", v)
		changed = true
	case "LFIND":
		fmt.Println("->", boolText(list.Contains(args.Word("значение"))))
	case "LGET":
		fmt.Println("->", list.Get(args.Index()))
	case "LPRINT":
		printItems(list.Items())
	case "LPRINTR":
		printItems(list.ItemsReversed()) // идём по prev от хвоста
	}

	if changed {
		store(db, name, "dlist", list.Items())
	}
	return changed
}
