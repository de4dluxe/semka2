package main

import "fmt"

// DynArray - динамический массив строк с удвоением ёмкости.
// Сделан вручную (без append), чтобы показать устройство массива.
type DynArray struct {
	data []string // len(data) == ёмкость
	size int
}

func NewDynArray() *DynArray { return &DynArray{data: make([]string, 4)} }

func (a *DynArray) grow() {
	bigger := make([]string, len(a.data)*2)
	copy(bigger, a.data[:a.size])
	a.data = bigger
}

func (a *DynArray) check(i int, allowEnd bool) {
	if i > a.size || (i == a.size && !allowEnd) {
		fail("индекс %d вне массива (длина %d)", i, a.size)
	}
}

func (a *DynArray) Push(v string) {
	if a.size == len(a.data) {
		a.grow()
	}
	a.data[a.size] = v
	a.size++
}

func (a *DynArray) Insert(i int, v string) {
	a.check(i, true)
	if a.size == len(a.data) {
		a.grow()
	}
	for j := a.size; j > i; j-- {
		a.data[j] = a.data[j-1]
	}
	a.data[i] = v
	a.size++
}

func (a *DynArray) Get(i int) string {
	a.check(i, false)
	return a.data[i]
}

func (a *DynArray) Set(i int, v string) {
	a.check(i, false)
	a.data[i] = v
}

func (a *DynArray) Erase(i int) {
	a.check(i, false)
	for j := i; j+1 < a.size; j++ {
		a.data[j] = a.data[j+1]
	}
	a.size--
}

func (a *DynArray) Items() []string {
	r := make([]string, a.size)
	copy(r, a.data[:a.size])
	return r
}

func (a *DynArray) Print() {
	fmt.Print("[")
	for i := 0; i < a.size; i++ {
		if i > 0 {
			fmt.Print(", ")
		}
		fmt.Print(a.data[i])
	}
	fmt.Printf("] (длина=%d, ёмкость=%d)\n", a.size, len(a.data))
}

func handleArray(cmd string, db DB, name string, args *Args) bool {
	items := openData(db, name, "array", cmd == "MPUSH" || cmd == "MINSERT")
	arr := NewDynArray()
	for _, it := range items {
		arr.Push(it)
	}

	changed := false
	switch cmd {
	case "MPUSH":
		v := args.Word("значение")
		arr.Push(v)
		fmt.Println("->", v)
		changed = true
	case "MINSERT":
		i := args.Index()
		v := args.Word("значение")
		arr.Insert(i, v)
		fmt.Println("->", v)
		changed = true
	case "MGET":
		fmt.Println("->", arr.Get(args.Index()))
	case "MSET":
		i := args.Index()
		v := args.Word("значение")
		arr.Set(i, v)
		fmt.Println("->", v)
		changed = true
	case "MDEL":
		i := args.Index()
		old := arr.Get(i)
		arr.Erase(i)
		fmt.Println("->", old)
		changed = true
	case "MLEN":
		fmt.Println("->", arr.size)
	case "MPRINT":
		arr.Print()
	}

	if changed {
		store(db, name, "array", arr.Items())
	}
	return changed
}
