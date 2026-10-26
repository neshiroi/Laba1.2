// singly.go — узлы и операции односвязного списка, а также команды F*.
package main

import (
	"fmt"
	"os"
)

// Узел односвязного списка: значение и ссылка на следующий узел.
type SNode struct {
	value string
	next  *SNode
}

// Односвязный список хранит начало, конец и количество элементов.
type SinglyList struct {
	head, tail *SNode
	size       int
}

// Добавляет элемент в конец списка.
func (l *SinglyList) PushBack(value string) {
	node := &SNode{value: value}
	if l.head == nil {
		l.head = node
	} else {
		l.tail.next = node
	}
	l.tail = node
	l.size++
}

// Добавляет элемент в начало списка.
func (l *SinglyList) PushFront(value string) {
	node := &SNode{value: value, next: l.head}
	l.head = node
	if l.tail == nil {
		l.tail = node
	}
	l.size++
}

// Ищет первый узел с указанным значением.
func (l *SinglyList) find(value string) *SNode {
	for node := l.head; node != nil; node = node.next {
		if node.value == value {
			return node
		}
	}
	return nil
}

// Вставляет новый элемент перед найденным значением.
func (l *SinglyList) InsertBefore(target, value string) bool {
	if l.head == nil {
		return false
	}
	if l.head.value == target {
		l.PushFront(value)
		return true
	}
	prev := l.head
	for prev.next != nil && prev.next.value != target {
		prev = prev.next
	}
	if prev.next == nil {
		return false
	}
	prev.next = &SNode{value: value, next: prev.next}
	l.size++
	return true
}

// Вставляет новый элемент после найденного значения.
func (l *SinglyList) InsertAfter(target, value string) bool {
	current := l.find(target)
	if current == nil {
		return false
	}
	node := &SNode{value: value, next: current.next}
	current.next = node
	if l.tail == current {
		l.tail = node
	}
	l.size++
	return true
}

// Удаляет первый элемент списка.
func (l *SinglyList) PopFront() bool {
	if l.head == nil {
		return false
	}
	l.head = l.head.next
	l.size--
	if l.head == nil {
		l.tail = nil
	}
	return true
}

// Удаляет последний элемент списка.
func (l *SinglyList) PopBack() bool {
	if l.head == nil {
		return false
	}
	if l.head == l.tail {
		return l.PopFront()
	}
	prev := l.head
	for prev.next != l.tail {
		prev = prev.next
	}
	prev.next = nil
	l.tail = prev
	l.size--
	return true
}

// Удаляет первое вхождение указанного значения.
func (l *SinglyList) EraseValue(value string) bool {
	if l.head == nil {
		return false
	}
	if l.head.value == value {
		return l.PopFront()
	}
	prev := l.head
	for prev.next != nil && prev.next.value != value {
		prev = prev.next
	}
	if prev.next == nil {
		return false
	}
	victim := prev.next
	prev.next = victim.next
	if l.tail == victim {
		l.tail = prev
	}
	l.size--
	return true
}

// Удаляет элемент непосредственно перед заданным значением.
func (l *SinglyList) EraseBefore(target string) bool {
	if l.head == nil || l.head.value == target || l.head.next == nil {
		return false
	}
	if l.head.next.value == target {
		return l.PopFront()
	}
	beforePrev, prev := l.head, l.head.next
	current := prev.next
	for current != nil && current.value != target {
		beforePrev, prev = prev, current
		current = current.next
	}
	if current == nil {
		return false
	}
	beforePrev.next = current
	l.size--
	return true
}

// Удаляет элемент непосредственно после заданного значения.
func (l *SinglyList) EraseAfter(target string) bool {
	current := l.find(target)
	if current == nil || current.next == nil {
		return false
	}
	victim := current.next
	current.next = victim.next
	if l.tail == victim {
		l.tail = current
	}
	l.size--
	return true
}

// Проверяет, находится ли значение в структуре.
func (l *SinglyList) Contains(value string) bool { return l.find(value) != nil }

// Возвращает элемент по индексу, если индекс допустим.
func (l *SinglyList) At(index int) (string, bool) {
	if index < 0 || index >= l.size {
		return "", false
	}
	node := l.head
	for i := 0; i < index; i++ {
		node = node.next
	}
	return node.value, true
}

// Возвращает элементы в порядке их хранения.
func (l *SinglyList) Values() []string {
	values := make([]string, 0, l.size)
	for node := l.head; node != nil; node = node.next {
		values = append(values, node.value)
	}
	return values
}

// Обрабатывает команды односвязного списка.
func executeSinglyCommand(db *Database, filename string, args []string) int {
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
	case "FPUSH", "FHEAD":
		if !require(3, cmd+" <name> <value>") {
			return 2
		}
		list := ensureSingly(db, args[1])
		if cmd == "FPUSH" {
			list.PushBack(args[2])
		} else {
			list.PushFront(args[2])
		}
		if !persistOrError() {
			return 1
		}
		fmt.Println("-> OK")
		return 0
	case "FBEFORE", "FAFTER":
		if !require(4, cmd+" <name> <target> <value>") {
			return 2
		}
		list := db.Singly[args[1]]
		if list == nil {
			fmt.Println("-> FALSE")
			return 0
		}
		var ok bool
		if cmd == "FBEFORE" {
			ok = list.InsertBefore(args[2], args[3])
		} else {
			ok = list.InsertAfter(args[2], args[3])
		}
		if ok && !persistOrError() {
			return 1
		}
		fmt.Println("->", boolText(ok))
		return 0
	case "FDEL", "FDELHEAD", "FDELTAIL", "FDELBEFORE", "FDELAFTER":
		req := 3
		if cmd == "FDELHEAD" || cmd == "FDELTAIL" {
			req = 2
		}
		if !require(req, cmd+" <name> [value/target]") {
			return 2
		}
		list := db.Singly[args[1]]
		if list == nil {
			fmt.Println("-> FALSE")
			return 0
		}
		var ok bool
		switch cmd {
		case "FDEL":
			ok = list.EraseValue(args[2])
		case "FDELHEAD":
			ok = list.PopFront()
		case "FDELTAIL":
			ok = list.PopBack()
		case "FDELBEFORE":
			ok = list.EraseBefore(args[2])
		case "FDELAFTER":
			ok = list.EraseAfter(args[2])
		}
		if ok && !persistOrError() {
			return 1
		}
		fmt.Println("->", boolText(ok))
		return 0
	case "FGET", "FSEARCH":
		if !require(2, cmd+" <name> [index/value]") {
			return 2
		}
		list := db.Singly[args[1]]
		if list == nil {
			fmt.Fprintln(os.Stderr, "ERROR: list not found:", args[1])
			return 1
		}
		if cmd == "FSEARCH" {
			if !require(3, "FSEARCH <name> <value>") {
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
