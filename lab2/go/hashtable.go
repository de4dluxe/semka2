package main

// Хеш-таблицы для задания 6.1: метод цепочек и хеширование кукушки.
// Ключ - строка, значение - число.

// Table - то, что умеют обе таблицы.
type Table interface {
	Add(key string, value int)
	Get(key string) (int, bool)
	Remove(key string) bool
	Count() int
	Keys() []string
}

// hashKey считает хеш строки: h = (h*base + символ) % size, начиная с start.
func hashKey(key string, base, start, size int) int {
	h := start
	for i := 0; i < len(key); i++ {
		h = (h*base + int(int8(key[i]))) % size
	}
	if h < 0 {
		h += size
	}
	return h
}

// ---------- метод цепочек ----------

type chainNode struct {
	key   string
	value int
	next  *chainNode
}

type ChainTable struct {
	table []*chainNode
	size  int
	count int
}

func NewChainTable() *ChainTable {
	return &ChainTable{table: make([]*chainNode, 11), size: 11}
}

func (t *ChainTable) find(key string) *chainNode {
	for cur := t.table[hashKey(key, 31, 0, t.size)]; cur != nil; cur = cur.next {
		if cur.key == key {
			return cur
		}
	}
	return nil
}

func (t *ChainTable) Get(key string) (int, bool) {
	node := t.find(key)
	if node == nil {
		return 0, false
	}
	return node.value, true
}

func (t *ChainTable) Add(key string, value int) {
	if node := t.find(key); node != nil {
		node.value = value
		return
	}
	if t.count > t.size*2 { // в среднем больше двух элементов в корзине
		t.rehash()
	}
	index := hashKey(key, 31, 0, t.size)
	t.table[index] = &chainNode{key, value, t.table[index]}
	t.count++
}

func (t *ChainTable) Remove(key string) bool {
	index := hashKey(key, 31, 0, t.size)
	var prev *chainNode
	for cur := t.table[index]; cur != nil; cur = cur.next {
		if cur.key == key {
			if prev == nil {
				t.table[index] = cur.next
			} else {
				prev.next = cur.next
			}
			t.count--
			return true
		}
		prev = cur
	}
	return false
}

// rehash: таблица становится больше, все узлы переносятся на новые места.
func (t *ChainTable) rehash() {
	newSize := t.size*2 + 1
	newTable := make([]*chainNode, newSize)
	for i := 0; i < t.size; i++ {
		cur := t.table[i]
		for cur != nil {
			next := cur.next
			index := hashKey(cur.key, 31, 0, newSize)
			cur.next = newTable[index]
			newTable[index] = cur
			cur = next
		}
	}
	t.table = newTable
	t.size = newSize
}

func (t *ChainTable) Count() int {
	return t.count
}

func (t *ChainTable) Keys() []string {
	var keys []string
	for i := 0; i < t.size; i++ {
		for cur := t.table[i]; cur != nil; cur = cur.next {
			keys = append(keys, cur.key)
		}
	}
	return keys
}

// ---------- хеширование кукушки ----------
// Две таблицы и две хеш-функции. Ключ лежит либо в первой таблице, либо во второй,
// поэтому поиск - две проверки.

const maxTempt = 50 // сколько раз можно выселять ключи, потом таблица увеличивается

type Cuckoo struct {
	size, count  int
	keys1, keys2 []string
	vals1, vals2 []int
	used1, used2 []bool
}

func NewCuckoo() *Cuckoo {
	c := &Cuckoo{}
	c.create(11)
	return c
}

func (c *Cuckoo) create(size int) {
	c.size = size
	c.count = 0
	c.keys1 = make([]string, size)
	c.keys2 = make([]string, size)
	c.vals1 = make([]int, size)
	c.vals2 = make([]int, size)
	c.used1 = make([]bool, size) // false - ячейка пустая
	c.used2 = make([]bool, size)
}

func (c *Cuckoo) hash1(key string) int {
	return hashKey(key, 31, 0, c.size)
}

func (c *Cuckoo) hash2(key string) int {
	return hashKey(key, 131, 7, c.size)
}

func (c *Cuckoo) Get(key string) (int, bool) {
	i1 := c.hash1(key)
	if c.used1[i1] && c.keys1[i1] == key {
		return c.vals1[i1], true
	}
	i2 := c.hash2(key)
	if c.used2[i2] && c.keys2[i2] == key {
		return c.vals2[i2], true
	}
	return 0, false
}

func (c *Cuckoo) Add(key string, value int) {
	i1 := c.hash1(key)
	if c.used1[i1] && c.keys1[i1] == key {
		c.vals1[i1] = value
		return
	}
	i2 := c.hash2(key)
	if c.used2[i2] && c.keys2[i2] == key {
		c.vals2[i2] = value
		return
	}
	c.insertNew(key, value)
}

// insertNew кладёт ключ в первую таблицу. Если место занято, старый ключ выселяется
// во вторую, оттуда (если занято) обратно в первую и так далее.
func (c *Cuckoo) insertNew(key string, value int) {
	for tempt := 0; tempt < maxTempt; tempt++ {
		i1 := c.hash1(key)
		if !c.used1[i1] {
			c.keys1[i1], c.vals1[i1], c.used1[i1] = key, value, true
			c.count++
			return
		}
		key, c.keys1[i1] = c.keys1[i1], key
		value, c.vals1[i1] = c.vals1[i1], value

		i2 := c.hash2(key)
		if !c.used2[i2] {
			c.keys2[i2], c.vals2[i2], c.used2[i2] = key, value, true
			c.count++
			return
		}
		key, c.keys2[i2] = c.keys2[i2], key
		value, c.vals2[i2] = c.vals2[i2], value
	}
	// Ключи ходят по кругу: увеличиваем таблицу и кладём последний выселенный ключ заново.
	c.grow()
	c.insertNew(key, value)
}

func (c *Cuckoo) grow() {
	oldSize := c.size
	keys1, keys2 := c.keys1, c.keys2
	vals1, vals2 := c.vals1, c.vals2
	used1, used2 := c.used1, c.used2

	c.create(oldSize*2 + 1)

	for i := 0; i < oldSize; i++ {
		if used1[i] {
			c.insertNew(keys1[i], vals1[i])
		}
		if used2[i] {
			c.insertNew(keys2[i], vals2[i])
		}
	}
}

func (c *Cuckoo) Remove(key string) bool {
	i1 := c.hash1(key)
	if c.used1[i1] && c.keys1[i1] == key {
		c.used1[i1] = false
		c.count--
		return true
	}
	i2 := c.hash2(key)
	if c.used2[i2] && c.keys2[i2] == key {
		c.used2[i2] = false
		c.count--
		return true
	}
	return false
}

func (c *Cuckoo) Count() int {
	return c.count
}

func (c *Cuckoo) Keys() []string {
	var keys []string
	for i := 0; i < c.size; i++ {
		if c.used1[i] {
			keys = append(keys, c.keys1[i])
		}
		if c.used2[i] {
			keys = append(keys, c.keys2[i])
		}
	}
	return keys
}

var _ Table = (*ChainTable)(nil)
var _ Table = (*Cuckoo)(nil)
