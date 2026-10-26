// doubly.go — узлы и операции двусвязного списка, а также команды L*.
package main

import (
	"fmt"
	"os"
)

// Узел двусвязного списка с указателями вперёд и назад.
type DNode struct {
	value      string
	prev, next *DNode
}

// Двусвязный список позволяет переходить к соседям в обоих направлениях.
type DoublyList struct {
	head, tail *DNode
	size       int
}

// Добавляет элемент в конец списка.
func (l *DoublyList) PushBack(value string) {
	node := &DNode{value: value, prev: l.tail}
	if l.tail == nil {
		l.head = node
	} else {
		l.tail.next = node
	}
	l.tail = node
	l.size++
}

// Добавляет элемент в начало списка.
func (l *DoublyList) PushFront(value string) {
	node := &DNode{value: value, next: l.head}
	if l.head != nil {
		l.head.prev = node
	} else {
		l.tail = node
	}
	l.head = node
	l.size++
}

// Ищет первый узел с указанным значением.
func (l *DoublyList) find(value string) *DNode {
	for node := l.head; node != nil; node = node.next {
		if node.value == value {
			return node
		}
	}
	return nil
}

// Вставляет новый элемент перед найденным значением.
func (l *DoublyList) InsertBefore(target, value string) bool {
	current := l.find(target)
	if current == nil {
		return false
	}
	if current == l.head {
		l.PushFront(value)
		return true
	}
	node := &DNode{value: value, prev: current.prev, next: current}
	current.prev.next = node
	current.prev = node
	l.size++
	return true
}

// Вставляет новый элемент после найденного значения.
func (l *DoublyList) InsertAfter(target, value string) bool {
	current := l.find(target)
	if current == nil {
		return false
	}
	node := &DNode{value: value, prev: current, next: current.next}
	if current.next != nil {
		current.next.prev = node
	} else {
		l.tail = node
	}
	current.next = node
	l.size++
	return true
}

func (l *DoublyList) eraseNode(node *DNode) bool {
	if node == nil {
		return false
	}
	if node.prev == nil {
		l.head = node.next
	} else {
		node.prev.next = node.next
	}
	if node.next == nil {
		l.tail = node.prev
	} else {
		node.next.prev = node.prev
	}
	l.size--
	return true
}

// Удаляет первый элемент списка.
func (l *DoublyList) PopFront() bool {
	if l.head == nil {
		return false
	}
	return l.eraseNode(l.head)
}

// Удаляет последний элемент списка.
func (l *DoublyList) PopBack() bool {
	if l.tail == nil {
		return false
	}
	return l.eraseNode(l.tail)
}

// Удаляет первое вхождение указанного значения.
func (l *DoublyList) EraseValue(value string) bool { return l.eraseNode(l.find(value)) }

// Удаляет элемент непосредственно перед заданным значением.
func (l *DoublyList) EraseBefore(target string) bool {
	current := l.find(target)
	if current == nil || current.prev == nil {
		return false
	}
	return l.eraseNode(current.prev)
}

// Удаляет элемент непосредственно после заданного значения.
func (l *DoublyList) EraseAfter(target string) bool {
	current := l.find(target)
	if current == nil || current.next == nil {
		return false
	}
	return l.eraseNode(current.next)
}

// Проверяет, находится ли значение в структуре.
func (l *DoublyList) Contains(value string) bool { return l.find(value) != nil }

// Возвращает элемент по индексу, если индекс допустим.
func (l *DoublyList) At(index int) (string, bool) {
	if index < 0 || index >= l.size {
		return "", false
	}
	var node *DNode
	if index < l.size/2 {
		node = l.head
		for i := 0; i < index; i++ {
			node = node.next
		}
	} else {
		node = l.tail
		for i := l.size - 1; i > index; i-- {
			node = node.prev
		}
	}
	return node.value, true
}

// Возвращает элементы в порядке их хранения.
func (l *DoublyList) Values() []string {
	values := make([]string, 0, l.size)
	for node := l.head; node != nil; node = node.next {
		values = append(values, node.value)
	}
	return values
}

// Обрабатывает команды двусвязного списка.
func executeDoublyCommand(db *Database, filename string, args []string) int {
	cmd := args[0]
	// Проверяет количество аргументов у команды.
	require := func(count int, syntax string) bool {
		if len(args) < count {
			fmt.Fprintln(os.Stderr, "ERROR: usage:", syntax)
			return false
		}
		return true
	}
	// Сохраняет изменения в файл данных.
	persist := func() error { return saveDatabase(filename, *db) }
	persistOrError := func() bool {
		if err := persist(); err != nil {
			fmt.Fprintln(os.Stderr, "ERROR:", err)
			return false
		}
		return true
	}
	switch cmd {
	case "LPUSH", "LHEAD":
		if !require(3, cmd+" <name> <value>") {
			return 2
		}
		list := ensureDoubly(db, args[1])
		if cmd == "LPUSH" {
			list.PushBack(args[2])
		} else {
			list.PushFront(args[2])
		}
		if !persistOrError() {
			return 1
		}
		fmt.Println("-> OK")
		return 0
	case "LBEFORE", "LAFTER":
		if !require(4, cmd+" <name> <target> <value>") {
			return 2
		}
		list := db.Doubly[args[1]]
		if list == nil {
			fmt.Println("-> FALSE")
			return 0
		}
		var ok bool
		if cmd == "LBEFORE" {
			ok = list.InsertBefore(args[2], args[3])
		} else {
			ok = list.InsertAfter(args[2], args[3])
		}
		if ok && !persistOrError() {
			return 1
		}
		fmt.Println("->", boolText(ok))
		return 0
	case "LDEL", "LDELHEAD", "LDELTAIL", "LDELBEFORE", "LDELAFTER":
		req := 3
		if cmd == "LDELHEAD" || cmd == "LDELTAIL" {
			req = 2
		}
		if !require(req, cmd+" <name> [value/target]") {
			return 2
		}
		list := db.Doubly[args[1]]
		if list == nil {
			fmt.Println("-> FALSE")
			return 0
		}
		var ok bool
		switch cmd {
		case "LDEL":
			ok = list.EraseValue(args[2])
		case "LDELHEAD":
			ok = list.PopFront()
		case "LDELTAIL":
			ok = list.PopBack()
		case "LDELBEFORE":
			ok = list.EraseBefore(args[2])
		case "LDELAFTER":
			ok = list.EraseAfter(args[2])
		}
		if ok && !persistOrError() {
			return 1
		}
		fmt.Println("->", boolText(ok))
		return 0
	case "LGET", "LSEARCH":
		if !require(2, cmd+" <name> [index/value]") {
			return 2
		}
		list := db.Doubly[args[1]]
		if list == nil {
			fmt.Fprintln(os.Stderr, "ERROR: list not found:", args[1])
			return 1
		}
		if cmd == "LSEARCH" {
			if !require(3, "LSEARCH <name> <value>") {
				return 2
			}
			fmt.Println("->", boolText(list.Contains(args[2])))
		} else if len(args) >= 3 {
			index, ok := parseIndex(args[2])
			if !ok {
				fmt.Println("-> FALSE")
			} else if value, found := list.At(index); found {
				fmt.Println("->", value)
			} else {
				fmt.Println("-> FALSE")
			}
		} else {
			printValues(list.Values())
		}
		return 0
	}
	return 2 // Команда не относится к этому модулю.
}
