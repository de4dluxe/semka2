package main

import (
	"bufio"
	"bytes"
	"encoding/json"
	"fmt"
	"os"
	"strings"
)

// PRINT <имя>: тип берём из файла и вызываем команду печати.
func handlePrint(db DB, name string, args *Args) bool {
	e, ok := db[name]
	if !ok {
		fail("нет структуры %s", name)
	}
	switch e.Type {
	case "array":
		return handleArray("MPRINT", db, name, args)
	case "flist":
		return handleSList("FPRINT", db, name, args)
	case "dlist":
		return handleDList("LPRINT", db, name, args)
	case "stack":
		return handleStack("SPRINT", db, name, args)
	case "queue":
		return handleQueue("QPRINT", db, name, args)
	case "tree":
		fail("деревья реализованы только в C++ версии")
	}
	fail("неизвестный тип структуры: %s", e.Type)
	return false
}

func run(cmd string, fam Family, db DB, name string, args *Args) bool {
	switch fam {
	case FamArray:
		return handleArray(cmd, db, name, args)
	case FamSList:
		return handleSList(cmd, db, name, args)
	case FamDList:
		return handleDList(cmd, db, name, args)
	case FamStack:
		return handleStack(cmd, db, name, args)
	case FamQueue:
		return handleQueue(cmd, db, name, args)
	case FamTree:
		fail("деревья реализованы только в C++ версии")
	case FamPrint:
		return handlePrint(db, name, args)
	}
	return false
}

// save записывает db в файл.
func save(db DB, fileName string) {
	var buf bytes.Buffer
	enc := json.NewEncoder(&buf)
	enc.SetEscapeHTML(false)
	enc.SetIndent("", "    ")
	if err := enc.Encode(db); err != nil {
		fail("не удалось сформировать json: %v", err)
	}
	if err := os.WriteFile(fileName, buf.Bytes(), 0o644); err != nil {
		fail("не удалось записать файл %s", fileName)
	}
}

// execute выполняет одну команду; если данные изменились - сразу пишет файл.
// Ошибки пользователя (fail) превращаются в возвращаемую строку.
func execute(db DB, fileName, query string) (errText string) {
	defer func() {
		if r := recover(); r != nil {
			ue, ok := r.(userError)
			if !ok {
				panic(r)
			}
			errText = ue.msg
		}
	}()
	words := strings.Fields(query)
	if len(words) == 0 {
		fail("пустой запрос")
	}
	fam, ok := commands[words[0]]
	if !ok {
		fail("неизвестная команда %s", words[0])
	}
	args := &Args{words: words[1:]}
	name := args.Word("имя структуры")
	if run(words[0], fam, db, name, args) {
		save(db, fileName)
	}
	return ""
}

const help = `Формат: КОМАНДА имя [аргументы], например SPUSH mystack 5
  M: MPUSH MINSERT MGET MSET MDEL MLEN MPRINT
  F/L (списки): PUSHH PUSHT PUSHA PUSHB DELH DELT DELA DELB DEL FIND GET PRINT PRINTR (с буквой F или L)
  S: SPUSH SPOP SPEEK SPRINT    Q: QPUSH QPOP QPEEK QPRINT
  PRINT имя    (деревья - только в C++)
  help - эта справка, exit - выход`

func main() {
	var fileName, query string
	for i := 1; i+1 < len(os.Args); i += 2 {
		switch os.Args[i] {
		case "--file":
			fileName = os.Args[i+1]
		case "--query":
			query = os.Args[i+1]
		}
	}
	if fileName == "" {
		fmt.Println("Использование: ./dbms_go --file <файл> [--query '<команда>']")
		fmt.Println("Без --query запускается ввод команд с клавиатуры.")
		os.Exit(1)
	}

	db := DB{}
	if raw, err := os.ReadFile(fileName); err == nil && len(bytes.TrimSpace(raw)) > 0 {
		if err := json.Unmarshal(raw, &db); err != nil {
			fmt.Printf("ошибка: файл %s имеет неверный формат: %v\n", fileName, err)
			os.Exit(1)
		}
	}

	// Один запрос из командной строки
	if query != "" {
		if e := execute(db, fileName, query); e != "" {
			fmt.Println("ошибка:", e)
			os.Exit(1)
		}
		return
	}

	// Ввод с клавиатуры: команды по одной в строке, пока не exit / Ctrl+D
	fmt.Println("Команды вводятся с клавиатуры (help - справка, exit - выход)")
	sc := bufio.NewScanner(os.Stdin)
	for {
		fmt.Print("> ")
		if !sc.Scan() {
			break
		}
		line := strings.TrimSpace(sc.Text())
		switch line {
		case "":
			continue
		case "exit", "quit":
			return
		case "help":
			fmt.Println(help)
			continue
		}
		if e := execute(db, fileName, line); e != "" {
			fmt.Println("ошибка:", e) // ошибка не прерывает сеанс
		}
	}
	fmt.Println()
}
