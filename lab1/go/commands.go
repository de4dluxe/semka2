package main

import (
	"encoding/json"
	"fmt"
	"strconv"
)

// Family - к какой структуре относится команда.
type Family int

const (
	FamArray Family = iota
	FamSList
	FamDList
	FamStack
	FamQueue
	FamTree
	FamPrint
)

// Таблица команд: имя -> семейство. Имена те же, что в C++ версии.
var commands = map[string]Family{
	"MPUSH": FamArray, "MINSERT": FamArray, "MGET": FamArray, "MSET": FamArray,
	"MDEL": FamArray, "MLEN": FamArray, "MPRINT": FamArray,

	"FPUSHH": FamSList, "FPUSHT": FamSList, "FPUSHA": FamSList, "FPUSHB": FamSList,
	"FDELH": FamSList, "FDELT": FamSList, "FDELA": FamSList, "FDELB": FamSList, "FDEL": FamSList,
	"FFIND": FamSList, "FGET": FamSList, "FPRINT": FamSList, "FPRINTR": FamSList,

	"LPUSHH": FamDList, "LPUSHT": FamDList, "LPUSHA": FamDList, "LPUSHB": FamDList,
	"LDELH": FamDList, "LDELT": FamDList, "LDELA": FamDList, "LDELB": FamDList, "LDEL": FamDList,
	"LFIND": FamDList, "LGET": FamDList, "LPRINT": FamDList, "LPRINTR": FamDList,

	"SPUSH": FamStack, "SPOP": FamStack, "SPEEK": FamStack, "SPRINT": FamStack,
	"QPUSH": FamQueue, "QPOP": FamQueue, "QPEEK": FamQueue, "QPRINT": FamQueue,

	"TINSERT": FamTree, "TDEL": FamTree, "TFIND": FamTree, "ISMEMBER": FamTree,
	"TGET": FamTree, "TPRINT": FamTree,

	"PRINT": FamPrint,
}

// Entry - одна структура в файле: {"data": ..., "type": "..."}.
// Data хранится "как есть", поэтому деревья (их пишет C++ версия) Go не портит.
type Entry struct {
	Data json.RawMessage `json:"data"`
	Type string          `json:"type"`
}

type DB map[string]*Entry

// userError - ошибка пользователя (неверный индекс, пустой стек и т.п.).
// Выбрасывается через fail и ловится в main, поэтому в обработчиках
// не нужно после каждого вызова проверять err.
type userError struct{ msg string }

func fail(format string, a ...any) {
	panic(userError{fmt.Sprintf(format, a...)})
}

// Args - слова запроса после имени структуры.
type Args struct {
	words []string
	pos   int
}

func (a *Args) Word(what string) string {
	if a.pos >= len(a.words) {
		fail("не хватает аргумента: %s", what)
	}
	w := a.words[a.pos]
	a.pos++
	return w
}

func (a *Args) Int(what string) int {
	w := a.Word(what)
	v, err := strconv.Atoi(w)
	if err != nil {
		fail("%s должен быть целым числом, а не '%s'", what, w)
	}
	return v
}

// Index читает индекс и сразу отвергает отрицательный.
func (a *Args) Index() int {
	i := a.Int("индекс")
	if i < 0 {
		fail("индекс не может быть отрицательным")
	}
	return i
}

// openData возвращает содержимое структуры name типа typ как []string.
// Если структуры нет: create == true даёт пустой список, иначе ошибка.
func openData(db DB, name, typ string, create bool) []string {
	e, ok := db[name]
	if !ok {
		if !create {
			fail("нет структуры %s", name)
		}
		return []string{}
	}
	if e.Type != typ {
		fail("%s это не %s", name, typ)
	}
	items := []string{}
	if err := json.Unmarshal(e.Data, &items); err != nil {
		fail("в данных %s ошибка: %v", name, err)
	}
	return items
}

// store записывает элементы обратно в db.
func store(db DB, name, typ string, items []string) {
	if items == nil {
		items = []string{}
	}
	raw, _ := json.Marshal(items)
	db[name] = &Entry{Data: raw, Type: typ}
}

func printItems(items []string) {
	if len(items) == 0 {
		fmt.Println("(пусто)")
		return
	}
	for i, it := range items {
		if i > 0 {
			fmt.Print(" -> ")
		}
		fmt.Print(it)
	}
	fmt.Println()
}

func boolText(b bool) string {
	if b {
		return "TRUE"
	}
	return "FALSE"
}
