package main

import (
	"fmt"
	"strconv"
	"strings"
)

// DB - вся база из JSON-файла: имя структуры -> {"type": ..., "data": ...}.
type DB = map[string]any

// Args - остаток запроса, из которого команды берут аргументы по одному.
type Args struct {
	rest string
}

// word возвращает следующее слово запроса.
func (a *Args) word(what string) (string, error) {
	s := strings.TrimLeft(a.rest, " \t\r\n")
	if s == "" {
		return "", fmt.Errorf("не хватает аргумента: %s", what)
	}
	end := strings.IndexAny(s, " \t\r\n")
	if end < 0 {
		end = len(s)
	}
	a.rest = s[end:]
	return s[:end], nil
}

// number возвращает следующее слово как целое число.
func (a *Args) number(what string) (int, error) {
	w, err := a.word(what)
	if err != nil {
		return 0, err
	}
	n, err := strconv.Atoi(w)
	if err != nil {
		return 0, fmt.Errorf("%s должен быть целым числом, а не '%s'", what, w)
	}
	return n, nil
}

// text возвращает всё, что осталось в запросе (текст с пробелами).
func (a *Args) text(what string) (string, error) {
	s := strings.TrimLeft(a.rest, " \t\r\n")
	if s == "" {
		return "", fmt.Errorf("не хватает аргумента: %s", what)
	}
	a.rest = ""
	return s, nil
}

// openData находит структуру name типа typ и возвращает её запись {"type", "data"}.
// Если структуры нет: create == true создаёт пустую, иначе ошибка.
func openData(db DB, name, typ string, create bool) (map[string]any, error) {
	v, exists := db[name]
	if !exists {
		if !create {
			return nil, fmt.Errorf("нет структуры %s", name)
		}
		entry := map[string]any{"type": typ, "data": []any{}}
		db[name] = entry
		return entry, nil
	}
	entry, ok := v.(map[string]any)
	if !ok || entry["type"] != typ {
		return nil, fmt.Errorf("%s это не %s", name, typ)
	}
	return entry, nil
}

// openObject то же самое, но данные хранятся объектом (таблицы, Хаффман, кэш).
func openObject(db DB, name, typ string, create bool) (map[string]any, error) {
	entry, err := openData(db, name, typ, create)
	if err != nil {
		return nil, err
	}
	if list, ok := entry["data"].([]any); ok && len(list) == 0 {
		entry["data"] = map[string]any{}
	}
	return entry, nil
}

// toInt превращает число в int: из JSON оно приходит как float64,
// а то, что программа записала сама в этом запуске, лежит как int.
func toInt(v any) int {
	switch n := v.(type) {
	case float64:
		return int(n)
	case int:
		return n
	}
	return 0
}

// toString превращает значение из JSON в строку.
func toString(v any) string {
	if s, ok := v.(string); ok {
		return s
	}
	return fmt.Sprint(v)
}
