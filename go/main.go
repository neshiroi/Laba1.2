// Дополнительная реализация лабораторной работы на Go. Основной модульный проект на C++ лежит в каталоге cpp.
// Команды и формат file.data совместимы с C++-версией.
package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// Набор именованных контейнеров, загружаемых из файла данных.
type Database struct {
	Arrays map[string][]string
	Singly map[string]*SinglyList
	Doubly map[string]*DoublyList
	Stacks map[string][]string
	Queues map[string]*Queue
	Trees  map[string]*FullBinaryTree
}

// Создаёт пустую базу данных всех структур.
func newDatabase() Database {
	return Database{
		Arrays: make(map[string][]string), Singly: make(map[string]*SinglyList),
		Doubly: make(map[string]*DoublyList), Stacks: make(map[string][]string),
		Queues: make(map[string]*Queue), Trees: make(map[string]*FullBinaryTree),
	}
}

// Возвращает список по имени и создаёт его, если его ещё нет.
func ensureSingly(db *Database, name string) *SinglyList {
	if db.Singly[name] == nil {
		db.Singly[name] = &SinglyList{}
	}
	return db.Singly[name]
}

// Возвращает двусвязный список по имени и при необходимости создаёт его.
func ensureDoubly(db *Database, name string) *DoublyList {
	if db.Doubly[name] == nil {
		db.Doubly[name] = &DoublyList{}
	}
	return db.Doubly[name]
}

// Возвращает дерево по имени и создаёт его при первом обращении.
func ensureTree(db *Database, name string) *FullBinaryTree {
	if db.Trees[name] == nil {
		db.Trees[name] = &FullBinaryTree{}
	}
	return db.Trees[name]
}

// Возвращает очередь по имени или создаёт новую.
func ensureQueue(db *Database, name string) *Queue {
	if db.Queues[name] == nil {
		db.Queues[name] = &Queue{}
	}
	return db.Queues[name]
}

// Читает файл и восстанавливает из его строк контейнеры и значения.
func loadDatabase(filename string) (Database, error) {
	db := newDatabase()
	file, err := os.Open(filename)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			return db, nil
		}
		return db, fmt.Errorf("cannot read data file: %w", err)
	}
	defer file.Close()
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}
		fields := strings.Fields(line)
		if len(fields) < 2 {
			continue
		}
		typ, name := fields[0], fields[1]
		for _, value := range fields[2:] {
			switch typ {
			case "M":
				db.Arrays[name] = append(db.Arrays[name], value)
			case "F":
				ensureSingly(&db, name).PushBack(value)
			case "L":
				ensureDoubly(&db, name).PushBack(value)
			case "S":
				db.Stacks[name] = append(db.Stacks[name], value)
			case "Q":
				ensureQueue(&db, name).Push(value)
			case "T":
				ensureTree(&db, name).Insert(value)
			}
		}
		// Preserve explicitly stored empty containers too.
		switch typ {
		case "M":
			if _, ok := db.Arrays[name]; !ok {
				db.Arrays[name] = []string{}
			}
		case "F":
			ensureSingly(&db, name)
		case "L":
			ensureDoubly(&db, name)
		case "S":
			if _, ok := db.Stacks[name]; !ok {
				db.Stacks[name] = []string{}
			}
		case "Q":
			ensureQueue(&db, name)
		case "T":
			ensureTree(&db, name)
		}
	}
	if err := scanner.Err(); err != nil {
		return db, fmt.Errorf("cannot read data file: %w", err)
	}
	return db, nil
}

// Записывает одну структуру данных в строку файла.
func writeRow(writer *bufio.Writer, typ, name string, values []string) error {
	if _, err := fmt.Fprintf(writer, "%s %s", typ, name); err != nil {
		return err
	}
	for _, value := range values {
		if _, err := fmt.Fprint(writer, " ", value); err != nil {
			return err
		}
	}
	_, err := fmt.Fprintln(writer)
	return err
}

// Сохраняет все контейнеры в общий файл данных.
func saveDatabase(filename string, db Database) error {
	file, err := os.Create(filename)
	if err != nil {
		return fmt.Errorf("cannot write data file: %w", err)
	}
	writer := bufio.NewWriter(file)
	if _, err = fmt.Fprintln(writer, "# ADSDB v1: type name values..."); err != nil {
		file.Close()
		return err
	}
	for name, values := range db.Arrays {
		if err = writeRow(writer, "M", name, values); err != nil {
			file.Close()
			return err
		}
	}
	for name, list := range db.Singly {
		if err = writeRow(writer, "F", name, list.Values()); err != nil {
			file.Close()
			return err
		}
	}
	for name, list := range db.Doubly {
		if err = writeRow(writer, "L", name, list.Values()); err != nil {
			file.Close()
			return err
		}
	}
	for name, values := range db.Stacks {
		if err = writeRow(writer, "S", name, values); err != nil {
			file.Close()
			return err
		}
	}
	for name, queue := range db.Queues {
		if err = writeRow(writer, "Q", name, queue.Values()); err != nil {
			file.Close()
			return err
		}
	}
	for name, tree := range db.Trees {
		if err = writeRow(writer, "T", name, tree.Values()); err != nil {
			file.Close()
			return err
		}
	}
	if err = writer.Flush(); err != nil {
		file.Close()
		return fmt.Errorf("cannot flush data file: %w", err)
	}
	if err = file.Close(); err != nil {
		return fmt.Errorf("cannot close data file: %w", err)
	}
	return nil
}

// Преобразует текст в неотрицательный индекс.
func parseIndex(text string) (int, bool) {
	value, err := strconv.Atoi(text)
	return value, err == nil && value >= 0
}

// Преобразует bool в строки TRUE или FALSE.
func boolText(value bool) string {
	if value {
		return "TRUE"
	}
	return "FALSE"
}

// Выводит последовательность значений в общем формате.
func printValues(values []string) {
	if len(values) == 0 {
		fmt.Println("-> (empty)")
		return
	}
	fmt.Println("-> " + strings.Join(values, " "))
}

// Проверяет, существует ли контейнер указанного типа и имени.
func hasName(db Database, typ, name string) bool {
	switch typ {
	case "M":
		_, ok := db.Arrays[name]
		return ok
	case "F":
		_, ok := db.Singly[name]
		return ok
	case "L":
		_, ok := db.Doubly[name]
		return ok
	case "S":
		_, ok := db.Stacks[name]
		return ok
	case "Q":
		_, ok := db.Queues[name]
		return ok
	case "T":
		_, ok := db.Trees[name]
		return ok
	}
	return false
}

// Выводит один контейнер выбранного типа.
func printContainer(db Database, typ, name string) {
	switch typ {
	case "M":
		fmt.Printf("M %s: ", name)
		printValues(db.Arrays[name])
	case "F":
		fmt.Printf("F %s: ", name)
		printValues(db.Singly[name].Values())
	case "L":
		fmt.Printf("L %s: ", name)
		printValues(db.Doubly[name].Values())
	case "S":
		fmt.Printf("S %s (bottom -> top): ", name)
		printValues(db.Stacks[name])
	case "Q":
		fmt.Printf("Q %s (front -> back): ", name)
		printValues(db.Queues[name].Values())
	case "T":
		printTree(name, db.Trees[name])
	}
}

// Выводит все контейнеры базы данных.
func printAll(db Database) {
	count := 0
	for name := range db.Arrays {
		printContainer(db, "M", name)
		count++
	}
	for name := range db.Singly {
		printContainer(db, "F", name)
		count++
	}
	for name := range db.Doubly {
		printContainer(db, "L", name)
		count++
	}
	for name := range db.Stacks {
		printContainer(db, "S", name)
		count++
	}
	for name := range db.Queues {
		printContainer(db, "Q", name)
		count++
	}
	for name := range db.Trees {
		printContainer(db, "T", name)
		count++
	}
	if count == 0 {
		fmt.Println("(no containers)")
	}
}

// Показывает синтаксис запуска и список основных команд.
func usage(program string) {
	fmt.Fprintf(os.Stderr, "Usage: %s --file file.data --query 'COMMAND arguments'\n", program)
	fmt.Fprintln(os.Stderr, "Array: MPUSH name value | MINSERT name index value | MGET name [index] | MSET name index value | MDEL name index | MLEN name")
	fmt.Fprintln(os.Stderr, "Singly list: FPUSH/FHEAD name value | FBEFORE/FAFTER name target value | FDEL name value | FDELHEAD/FDELTAIL name | FDELBEFORE/FDELAFTER name target | FSEARCH/FGET name [value or index]")
	fmt.Fprintln(os.Stderr, "Doubly list: LPUSH/LHEAD name value | LBEFORE/LAFTER name target value | LDEL name value | LDELHEAD/LDELTAIL name | LDELBEFORE/LDELAFTER name target | LSEARCH/LGET name [value or index]")
	fmt.Fprintln(os.Stderr, "Stack: SPUSH name value | SPOP/SPEEK/SGET name; Queue: QPUSH name value | QPOP/QPEEK/QGET name")
	fmt.Fprintln(os.Stderr, "Tree: TINSERT name value | ISMEMBER name value | TGET/TFULL name; Display: PRINT [type] [name]")
}

// Разбирает команду и выполняет соответствующую операцию.
// execute выполняет PRINT или передаёт команду подходящему модулю.
func execute(db *Database, filename string, args []string) int {
	if len(args) == 0 {
		return 2
	}
	cmd := args[0]

	// PRINT доступен всем структурам данных.
	if cmd == "PRINT" {
		if len(args) == 1 {
			printAll(*db)
			return 0
		}
		if len(args) >= 3 && len(args[1]) == 1 && strings.Contains("MFL SQT", args[1]) && hasName(*db, args[1], args[2]) {
			printContainer(*db, args[1], args[2])
			return 0
		}
		name, found := args[1], false
		for _, typ := range []string{"M", "F", "L", "S", "Q", "T"} {
			if hasName(*db, typ, name) {
				printContainer(*db, typ, name)
				found = true
			}
		}
		if !found {
			fmt.Fprintln(os.Stderr, "ERROR: container not found:", name)
			return 1
		}
		return 0
	}

	// В зависимости от префикса команда передаётся нужному модулю.
	switch cmd {
	case "MPUSH", "MINSERT", "MGET", "MREAD", "MSET", "MDEL", "MLEN":
		return executeArrayCommand(db, filename, args)
	case "FPUSH", "FHEAD", "FBEFORE", "FAFTER", "FDEL", "FDELHEAD", "FDELTAIL", "FDELBEFORE", "FDELAFTER", "FGET", "FSEARCH":
		return executeSinglyCommand(db, filename, args)
	case "LPUSH", "LHEAD", "LBEFORE", "LAFTER", "LDEL", "LDELHEAD", "LDELTAIL", "LDELBEFORE", "LDELAFTER", "LGET", "LSEARCH":
		return executeDoublyCommand(db, filename, args)
	case "SPUSH", "SPOP", "SPEEK", "SGET", "QPUSH", "QPOP", "QPEEK", "QGET":
		return executeStackQueueCommand(db, filename, args)
	case "TINSERT", "TDEL", "ISMEMBER", "TGET", "TFULL":
		return executeTreeCommand(db, filename, args)
	default:
		fmt.Fprintln(os.Stderr, "ERROR: unknown command:", cmd)
		return 2
	}
}

// Обрабатывает параметры командной строки, загружает файл и запускает запрос.
func run(args []string) int {
	filename, query := "file.data", ""
	for i := 0; i < len(args); i++ {
		switch args[i] {
		case "--file":
			if i+1 >= len(args) {
				usage(os.Args[0])
				return 2
			}
			i++
			filename = args[i]
		case "--query":
			if i+1 >= len(args) {
				usage(os.Args[0])
				return 2
			}
			i++
			query = args[i]
		default:
			usage(os.Args[0])
			return 2
		}
	}
	if strings.TrimSpace(query) == "" {
		usage(os.Args[0])
		return 2
	}
	db, err := loadDatabase(filename)
	if err != nil {
		fmt.Fprintln(os.Stderr, "ERROR:", err)
		return 1
	}
	return execute(&db, filename, strings.Fields(query))
}

// Точка входа программы Go.
func main() { os.Exit(run(os.Args[1:])) }
