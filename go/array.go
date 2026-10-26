// array.go — операции с динамическим массивом и команды M*.
// Массив хранится в []string: Go автоматически управляет его размером.
package main

import (
	"fmt"
	"os"
)

// Обрабатывает команды динамического массива.
func executeArrayCommand(db *Database, filename string, args []string) int {
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
	case "MPUSH", "MINSERT":
		if cmd == "MPUSH" {
			if !require(3, "MPUSH <name> <value>") {
				return 2
			}
			db.Arrays[args[1]] = append(db.Arrays[args[1]], args[2])
		} else {
			if !require(4, "MINSERT <name> <index> <value>") {
				return 2
			}
			index, ok := parseIndex(args[2])
			if !ok {
				fmt.Println("-> FALSE")
				return 0
			}
			values, exists := db.Arrays[args[1]]
			if !exists {
				values = []string{}
			}
			if index > len(values) {
				fmt.Println("-> FALSE")
				return 0
			}
			values = append(values, "")
			copy(values[index+1:], values[index:])
			values[index] = args[3]
			db.Arrays[args[1]] = values
		}
		if !persistOrError() {
			return 1
		}
		if cmd == "MPUSH" {
			fmt.Println("-> OK")
		} else {
			fmt.Println("-> TRUE")
		}
		return 0
	case "MGET", "MREAD", "MSET", "MDEL", "MLEN":
		if !require(2, cmd+" <name> [index] [value]") {
			return 2
		}
		values, exists := db.Arrays[args[1]]
		if !exists {
			fmt.Fprintln(os.Stderr, "ERROR: array not found:", args[1])
			return 1
		}
		switch cmd {
		case "MGET", "MREAD":
			if len(args) >= 3 {
				index, ok := parseIndex(args[2])
				if !ok || index >= len(values) {
					fmt.Println("-> FALSE")
				} else {
					fmt.Println("->", values[index])
				}
			} else {
				printValues(values)
			}
		case "MLEN":
			fmt.Println("->", len(values))
		case "MSET", "MDEL":
			req := 3
			syntax := "MDEL <name> <index>"
			if cmd == "MSET" {
				req = 4
				syntax = "MSET <name> <index> <value>"
			}
			if !require(req, syntax) {
				return 2
			}
			index, ok := parseIndex(args[2])
			if !ok || index >= len(values) {
				fmt.Println("-> FALSE")
				return 0
			}
			if cmd == "MSET" {
				values[index] = args[3]
			} else {
				values = append(values[:index], values[index+1:]...)
			}
			db.Arrays[args[1]] = values
			if !persistOrError() {
				return 1
			}
			fmt.Println("-> TRUE")
		}
		return 0
	}
	return 2 // Команда не относится к этому модулю.
}
