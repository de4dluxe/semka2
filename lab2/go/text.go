package main

// Задание 4 (гистограмма, вариант 6) и задание 5 (Хаффман, вариант *).
//   HTEXT имя текст     запомнить текст          HIST имя   нарисовать гистограмму
//   HUFFENC имя текст   закодировать, сохранить таблицу кодов и код
//   HUFFDEC имя         расшифровать сохранённый код     HUFFPRINT имя   показать таблицу и код

import (
	"fmt"
	"strconv"
	"strings"
)

func isSpace(c byte) bool {
	return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'
}

// printHistogram хранит различные символы в массиве по возрастанию кода,
// пробелы и переводы строк не считаются.
func printHistogram(text string) error {
	var count [256]int
	var symbols []byte
	for i := 0; i < len(text); i++ {
		c := text[i]
		if isSpace(c) {
			continue
		}
		if count[c] == 0 {
			// ищем место, чтобы массив остался отсортированным
			pos := 0
			for pos < len(symbols) && symbols[pos] < c {
				pos++
			}
			symbols = append(symbols, 0)
			copy(symbols[pos+1:], symbols[pos:])
			symbols[pos] = c
		}
		count[c]++
	}
	if len(symbols) == 0 {
		return fmt.Errorf("в тексте нет символов кроме пробелов")
	}

	maxCount := 0
	for _, s := range symbols {
		if count[s] > maxCount {
			maxCount = count[s]
		}
	}

	// строки сверху вниз: столбик на этой высоте есть у символов, которых не меньше row
	for row := maxCount; row >= 1; row-- {
		line := make([]byte, len(symbols))
		for i, s := range symbols {
			if count[s] >= row {
				line[i] = '#'
			} else {
				line[i] = ' '
			}
		}
		fmt.Println(strings.TrimRight(string(line), " ")) // пробелы в конце строки не нужны
	}
	fmt.Println(string(symbols))
	return nil
}

func printTable(huff map[string]any) {
	fmt.Println("Таблица:")
	table, _ := huff["table"].(map[string]any)
	for i := 0; i < 256; i++ {
		if code, ok := table[strconv.Itoa(i)]; ok {
			fmt.Printf("'%s' %s\n", string([]byte{byte(i)}), toString(code))
		}
	}
	fmt.Println("Код: " + toString(huff["bits"]))
}

func handleText(cmd string, db DB, name string, args *Args) (bool, error) {
	if cmd == "HTEXT" {
		text, err := args.text("текст")
		if err != nil {
			return false, err
		}
		entry, err := openData(db, name, "text", true)
		if err != nil {
			return false, err
		}
		entry["data"] = text
		fmt.Printf("-> символов: %d\n", len(text))
		return true, nil
	}
	if cmd == "HIST" {
		entry, err := openData(db, name, "text", false)
		if err != nil {
			return false, err
		}
		text, _ := entry["data"].(string)
		return false, printHistogram(text)
	}

	if cmd == "HUFFENC" {
		text, err := args.text("текст")
		if err != nil {
			return false, err
		}
		var tree HuffTree
		tree.Build(text)

		// в файл: таблица кодов (ключ - номер символа, чтобы JSON остался корректным) и сам код
		table := map[string]any{}
		for i := 0; i < 256; i++ {
			if code := tree.GetCode(byte(i)); code != "" {
				table[strconv.Itoa(i)] = code
			}
		}
		huff := map[string]any{"table": table, "bits": tree.Encode(text)}
		entry, err := openObject(db, name, "huffman", true)
		if err != nil {
			return false, err
		}
		entry["data"] = huff
		printTable(huff)
		return true, nil
	}

	// HUFFDEC и HUFFPRINT
	entry, err := openObject(db, name, "huffman", false)
	if err != nil {
		return false, err
	}
	data, _ := entry["data"].(map[string]any)
	table, hasTable := data["table"].(map[string]any)
	bits, hasBits := data["bits"].(string)
	if !hasTable || !hasBits {
		return false, fmt.Errorf("в %s ничего нет, сначала HUFFENC", name)
	}
	if cmd == "HUFFPRINT" {
		printTable(data)
		return false, nil
	}

	// дерево собирается по сохранённой таблице, потом по нему расшифровывается код
	var tree HuffTree
	for key, code := range table { // порядок не важен: у каждого символа свой путь в дереве
		n, err := strconv.Atoi(key)
		if err != nil || n < 0 || n > 255 {
			return false, fmt.Errorf("в таблице неверный символ %s", key)
		}
		tree.AddCode(byte(n), toString(code))
	}
	text, ok := tree.Decode(bits)
	if !ok {
		return false, fmt.Errorf("код не подходит к таблице")
	}
	fmt.Println("-> " + text)
	return false, nil
}
