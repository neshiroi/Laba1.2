// stack_queue.go — операции со стеком и очередью, а также команды S* и Q*.
package main

import (
	"fmt"
	"os"
)

// Очередь хранит элементы в порядке FIFO: первый вошёл — первый вышел.
type Queue struct {
	items []string
	front int
}

// Добавляет элемент в структуру.
func (q *Queue) Push(value string) {
	q.items = append(q.items, value)
}

// Возвращает количество элементов в очереди.
func (q *Queue) Len() int { return len(q.items) - q.front }

// Читает следующий элемент без удаления.
func (q *Queue) Peek() (string, bool) {
	if q.Len() == 0 {
		return "", false
	}
	return q.items[q.front], true
}

// Извлекает следующий элемент и удаляет его из структуры.
func (q *Queue) Pop() (string, bool) {
	if q.Len() == 0 {
		return "", false
	}
	value := q.items[q.front]
	q.front++
	if q.front == len(q.items) {
		q.items = nil
		q.front = 0
	} else if q.front >= 64 && q.front*2 >= len(q.items) {
		q.items = append([]string{}, q.items[q.front:]...)
		q.front = 0
	}
	return value, true
}

// Возвращает элементы в порядке их хранения.
func (q *Queue) Values() []string {
	return append([]string{}, q.items[q.front:]...)
}

// Обрабатывает команды стека и очереди.
func executeStackQueueCommand(db *Database, filename string, args []string) int {
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
	case "SPUSH":
		if !require(3, "SPUSH <name> <value>") {
			return 2
		}
		db.Stacks[args[1]] = append(db.Stacks[args[1]], args[2])
		if !persistOrError() {
			return 1
		}
		fmt.Println("-> OK")
		return 0
	case "SPOP", "SPEEK", "SGET":
		if !require(2, cmd+" <name>") {
			return 2
		}
		values, exists := db.Stacks[args[1]]
		if !exists || len(values) == 0 {
			fmt.Println("-> (empty)")
			return 0
		}
		switch cmd {
		case "SGET":
			printValues(values)
		case "SPEEK":
			fmt.Println("->", values[len(values)-1])
		case "SPOP":
			value := values[len(values)-1]
			db.Stacks[args[1]] = values[:len(values)-1]
			if !persistOrError() {
				return 1
			}
			fmt.Println("->", value)
		}
		return 0
	case "QPUSH":
		if !require(3, "QPUSH <name> <value>") {
			return 2
		}
		ensureQueue(db, args[1]).Push(args[2])
		if !persistOrError() {
			return 1
		}
		fmt.Println("-> OK")
		return 0
	case "QPOP", "QPEEK", "QGET":
		if !require(2, cmd+" <name>") {
			return 2
		}
		queue := db.Queues[args[1]]
		if queue == nil || queue.Len() == 0 {
			fmt.Println("-> (empty)")
			return 0
		}
		switch cmd {
		case "QGET":
			printValues(queue.Values())
		case "QPEEK":
			value, _ := queue.Peek()
			fmt.Println("->", value)
		case "QPOP":
			value, _ := queue.Pop()
			if !persistOrError() {
				return 1
			}
			fmt.Println("->", value)
		}
		return 0
	}
	return 2 // Команда не относится к этому модулю.
}
