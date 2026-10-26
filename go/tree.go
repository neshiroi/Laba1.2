// tree.go — реализация Full Binary Tree и команды T*.
// Вместо удаления узла по условию лабораторной работы выполняется проверка свойства Full Binary Tree.
package main

import (
	"fmt"
	"os"
	"strings"
)

// Узел бинарного дерева: значение и ссылки на двух потомков.
type TNode struct {
	value       string
	left, right *TNode
}

// Бинарное дерево; новые элементы добавляются по уровням.
type FullBinaryTree struct {
	root       *TNode
	levelOrder []*TNode
}

// Добавляет новый элемент в следующую свободную позицию дерева.
func (t *FullBinaryTree) Insert(value string) {
	node := &TNode{value: value}
	index := len(t.levelOrder)
	if index == 0 {
		t.root = node
	} else {
		parent := t.levelOrder[(index-1)/2]
		if index%2 == 1 {
			parent.left = node
		} else {
			parent.right = node
		}
	}
	t.levelOrder = append(t.levelOrder, node)
}

// Проверяет, находится ли значение в структуре.
func (t *FullBinaryTree) Contains(value string) bool {
	for _, node := range t.levelOrder {
		if node.value == value {
			return true
		}
	}
	return false
}

// Проверяет свойство Full Binary Tree: каждый узел имеет 0 или 2 потомка.
func (t *FullBinaryTree) IsFull() bool {
	for _, node := range t.levelOrder {
		hasLeft := node.left != nil
		hasRight := node.right != nil
		if hasLeft != hasRight {
			return false
		}
	}
	return true
}

// Возвращает элементы в порядке их хранения.
func (t *FullBinaryTree) Values() []string {
	values := make([]string, 0, len(t.levelOrder))
	for _, node := range t.levelOrder {
		values = append(values, node.value)
	}
	return values
}

// Группирует элементы дерева по уровням для вывода.
func (t *FullBinaryTree) Levels() [][]string {
	levels := make([][]string, 0)
	if t.root == nil {
		return levels
	}
	queue := []*TNode{t.root}
	for head := 0; head < len(queue); {
		end := len(queue)
		current := make([]string, 0, end-head)
		for head < end {
			node := queue[head]
			head++
			current = append(current, node.value)
			if node.left != nil {
				queue = append(queue, node.left)
			}
			if node.right != nil {
				queue = append(queue, node.right)
			}
		}
		levels = append(levels, current)
	}
	return levels
}

// Показывает дерево по уровням и результат проверки свойства Full Binary Tree.
func printTree(name string, tree *FullBinaryTree) {
	fmt.Printf("T %s (Full Binary Tree)\n", name)
	levels := tree.Levels()
	if len(levels) == 0 {
		fmt.Println("(empty)")
	}
	for depth, row := range levels {
		fmt.Printf("Level %d: %s\n", depth, strings.Join(row, " "))
	}
	fmt.Printf("Full: %s\n", boolText(tree.IsFull()))
}

// Обрабатывает команды полного бинарного дерева.
func executeTreeCommand(db *Database, filename string, args []string) int {
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
	case "TINSERT":
		if !require(3, "TINSERT <name> <value>") {
			return 2
		}
		ensureTree(db, args[1]).Insert(args[2])
		if !persistOrError() {
			return 1
		}
		fmt.Println("-> OK")
		return 0
	case "TDEL":
		fmt.Fprintln(os.Stderr, "ERROR: TDEL is replaced by TFULL for the Full Binary Tree variant.")
		return 2
	case "ISMEMBER", "TGET", "TFULL":
		if !require(2, cmd+" <name> [value]") {
			return 2
		}
		tree := db.Trees[args[1]]
		if tree == nil {
			fmt.Fprintln(os.Stderr, "ERROR: tree not found:", args[1])
			return 1
		}
		switch cmd {
		case "ISMEMBER":
			if !require(3, "ISMEMBER <name> <value>") {
				return 2
			}
			fmt.Println("->", boolText(tree.Contains(args[2])))
		case "TGET":
			printValues(tree.Values())
		case "TFULL":
			fmt.Println("->", boolText(tree.IsFull()))
		}
		return 0
	}
	return 2 // Команда не относится к этому модулю.
}
