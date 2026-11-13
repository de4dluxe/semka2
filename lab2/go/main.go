// Лабораторная 2 на Go. Одна программа для всех заданий.
//
//	go run . --file data.json --query 'SETADD s 5'    один запрос
//	go run . --file data.json                         команды с клавиатуры
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"strings"
)

type handler func(cmd string, db DB, name string, args *Args) (bool, error)

// Какая команда каким обработчиком выполняется.
var handlers = map[string]handler{
	"SETADD": handleSet, "SETDEL": handleSet, "SET_AT": handleSet, "SETPRINT": handleSet,
	"AADD": handleAsteroids, "ARUN": handleAsteroids, "APRINT": handleAsteroids,
	"TURADD": handleTurtles, "TURRUN": handleTurtles, "TURPRINT": handleTurtles,
	"HTEXT": handleText, "HIST": handleText, "HUFFENC": handleText, "HUFFDEC": handleText, "HUFFPRINT": handleText,
	"CHSET": handleChain, "CHGET": handleChain, "CHDEL": handleChain, "CHPRINT": handleChain,
	"CKSET": handleCuckoo, "CKGET": handleCuckoo, "CKDEL": handleCuckoo, "CKPRINT": handleCuckoo,
	"GADD": handleGenealogy, "GRUN": handleGenealogy, "GPRINT": handleGenealogy,
	"LFUINIT": handleLFU, "LFUSET": handleLFU, "LFUGET": handleLFU, "LFUPRINT": handleLFU,
}

// Какой командой печатается структура данного типа (для PRINT <имя>).
var printCommands = map[string]string{
	"set": "SETPRINT", "asteroids": "APRINT", "turtles": "TURPRINT", "huffman": "HUFFPRINT",
	"hashchain": "CHPRINT", "hashcuckoo": "CKPRINT", "genealogy": "GPRINT", "lfu": "LFUPRINT",
}

func saveDB(db DB, fileName string) error {
	file, err := os.Create(fileName)
	if err != nil {
		return fmt.Errorf("не удалось записать файл %s", fileName)
	}
	defer file.Close()
	encoder := json.NewEncoder(file)
	encoder.SetIndent("", "    ")
	encoder.SetEscapeHTML(false)
	return encoder.Encode(db)
}

// execute выполняет одну команду. Если данные изменились, сразу записывает файл.
func execute(db DB, fileName, query string) error {
	args := &Args{rest: query}
	word, _ := args.word("команда")

	var run handler
	cmd := word
	if word == "PRINT" {
		name, err := args.word("имя структуры")
		if err != nil {
			return err
		}
		entry, ok := db[name].(map[string]any)
		if !ok {
			return fmt.Errorf("нет структуры %s", name)
		}
		typ, _ := entry["type"].(string)
		if typ == "text" {
			fmt.Println(toString(entry["data"]))
			return nil
		}
		cmd = printCommands[typ]
		if cmd == "" {
			return fmt.Errorf("неизвестный тип структуры: %s", typ)
		}
		run = handlers[cmd]
		args = &Args{rest: name}
	} else {
		run = handlers[word]
	}
	if run == nil {
		return fmt.Errorf("неизвестная команда %s", word)
	}

	name, err := args.word("имя структуры")
	if err != nil {
		return err
	}
	changed, err := run(cmd, db, name, args)
	if err != nil {
		return err
	}
	if changed {
		return saveDB(db, fileName)
	}
	return nil
}

func printHelp() {
	fmt.Print("Формат: КОМАНДА имя [аргументы], например SETADD s 5\n" +
		"  множество (2):     SETADD SETDEL SET_AT SETPRINT   имя x\n" +
		"  астероиды (1):     AADD имя размер R|L, ARUN имя, APRINT имя\n" +
		"  черепахи (3):      TURADD имя a b, TURRUN имя, TURPRINT имя\n" +
		"  гистограмма (4):   HTEXT имя текст, HIST имя\n" +
		"  Хаффман (5):       HUFFENC имя текст, HUFFDEC имя, HUFFPRINT имя\n" +
		"  хеш (6.1):         CHSET/CKSET имя ключ число, CHGET/CKGET имя ключ,\n" +
		"                     CHDEL/CKDEL имя ключ, CHPRINT/CKPRINT имя (CH - цепочки, CK - кукушка)\n" +
		"  родословная (6.2): GADD имя потомок родитель, GRUN имя, GPRINT имя\n" +
		"  LFU-кэш (7):       LFUINIT имя ёмкость, LFUSET имя ключ значение, LFUGET имя ключ, LFUPRINT имя\n" +
		"  PRINT имя - вывести любую структуру\n" +
		"help - эта справка, exit - выход\n")
}

func main() {
	fileName := ""
	query := ""
	for i := 1; i+1 < len(os.Args); i += 2 {
		switch os.Args[i] {
		case "--file":
			fileName = os.Args[i+1]
		case "--query":
			query = os.Args[i+1]
		}
	}
	if fileName == "" {
		fmt.Println("Использование: go run . --file <файл> [--query '<команда>']\n" +
			"Без --query команды вводятся с клавиатуры.")
		os.Exit(1)
	}

	// загружаем базу из файла (если файла нет, база пустая)
	db := DB{}
	content, err := os.ReadFile(fileName)
	if err == nil && strings.TrimSpace(string(content)) != "" {
		if err := json.Unmarshal(content, &db); err != nil {
			fmt.Printf("ошибка: не удалось прочитать %s: %v\n", fileName, err)
			os.Exit(1)
		}
	}

	// один запрос из командной строки
	if query != "" {
		if err := execute(db, fileName, query); err != nil {
			fmt.Println("ошибка: " + err.Error())
			os.Exit(1)
		}
		return
	}

	// ввод с клавиатуры: по одной команде в строке, пока не exit
	fmt.Println("Введите команду (help - справка, exit - выход)")
	scanner := bufio.NewScanner(os.Stdin)
	for {
		fmt.Print("> ")
		if !scanner.Scan() {
			break
		}
		line := strings.TrimRight(scanner.Text(), "\r")
		if strings.TrimSpace(line) == "" {
			continue
		}
		if line == "exit" {
			break
		}
		if line == "help" {
			printHelp()
			continue
		}
		if err := execute(db, fileName, line); err != nil {
			fmt.Println("ошибка: " + err.Error()) // ошибка не завершает программу
		}
	}
}
