package main

// Дерево Хаффмана для задания 5.

type HuffNode struct {
	symbol      byte
	weight      int
	left, right *HuffNode
}

type HuffTree struct {
	root  *HuffNode
	codes [256]string
}

// takeMin достаёт из списка свободных узлов узел с наименьшим весом.
func takeMin(nodes []*HuffNode) (*HuffNode, []*HuffNode) {
	mini := 0
	for i := 1; i < len(nodes); i++ {
		if nodes[i].weight < nodes[mini].weight {
			mini = i
		}
	}
	result := nodes[mini]
	nodes[mini] = nodes[len(nodes)-1]
	return result, nodes[:len(nodes)-1]
}

func (t *HuffTree) Build(text string) {
	t.root = nil
	for i := range t.codes {
		t.codes[i] = ""
	}

	var freq [256]int
	for i := 0; i < len(text); i++ {
		freq[text[i]]++
	}

	// сначала у каждого символа свой лист
	var nodes []*HuffNode
	for i := 0; i < 256; i++ {
		if freq[i] > 0 {
			nodes = append(nodes, &HuffNode{symbol: byte(i), weight: freq[i]})
		}
	}
	if len(nodes) == 0 {
		return
	}

	if len(nodes) == 1 {
		// в тексте один символ: ему достаётся код "0"
		t.root = &HuffNode{weight: nodes[0].weight, left: nodes[0]}
	} else {
		// два самых лёгких узла склеиваем в один, пока не останется один узел
		for len(nodes) > 1 {
			var first, second *HuffNode
			first, nodes = takeMin(nodes)
			second, nodes = takeMin(nodes)
			nodes = append(nodes, &HuffNode{weight: first.weight + second.weight, left: first, right: second})
		}
		t.root = nodes[0]
	}
	t.buildCodes(t.root, "")
}

// buildCodes: влево идём по 0, вправо по 1. Дошли до листа - запомнили код символа.
func (t *HuffTree) buildCodes(node *HuffNode, code string) {
	if node == nil {
		return
	}
	if node.left == nil && node.right == nil {
		t.codes[node.symbol] = code
		return
	}
	t.buildCodes(node.left, code+"0")
	t.buildCodes(node.right, code+"1")
}

// AddCode добавляет в дерево путь для символа (так дерево собирается из таблицы кодов).
func (t *HuffTree) AddCode(symbol byte, code string) {
	if t.root == nil {
		t.root = &HuffNode{}
	}
	current := t.root
	for i := 0; i < len(code); i++ {
		next := &current.left
		if code[i] == '1' {
			next = &current.right
		}
		if *next == nil {
			*next = &HuffNode{}
		}
		current = *next
	}
	current.symbol = symbol
	t.codes[symbol] = code
}

func (t *HuffTree) GetCode(symbol byte) string {
	return t.codes[symbol]
}

func (t *HuffTree) Encode(text string) string {
	result := ""
	for i := 0; i < len(text); i++ {
		result += t.codes[text[i]]
	}
	return result
}

// Decode: стоим в корне, читаем бит: 0 - влево, 1 - вправо. Попали в лист - записали
// символ и вернулись в корень. Второе значение false, если код не подходит к дереву.
func (t *HuffTree) Decode(bits string) (string, bool) {
	if t.root == nil {
		return "", false
	}
	var result []byte
	current := t.root
	for i := 0; i < len(bits); i++ {
		if bits[i] == '0' {
			current = current.left
		} else if bits[i] == '1' {
			current = current.right
		} else {
			return "", false
		}
		if current == nil {
			return "", false
		}
		if current.left == nil && current.right == nil {
			result = append(result, current.symbol)
			current = t.root
		}
	}
	return string(result), current == t.root // код не должен обрываться на середине символа
}
