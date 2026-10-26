#include "array.hpp"
#include "doubly.hpp"
#include "singly.hpp"
#include "stack_queue.hpp"
#include "tree.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// В базе данных хранятся именованные экземпляры всех шести структур.
struct Database {
    std::map<std::string, DynamicArray> arrays;
    std::map<std::string, SinglyList> singlyLists;
    std::map<std::string, DoublyList> doublyLists;
    std::map<std::string, Stack> stacks;
    std::map<std::string, Queue> queues;
    std::map<std::string, FullBinaryTree> trees;
};

// Выводит последовательность элементов в едином формате.
template <typename Range>
void printValues(const Range& values) {
    std::cout << "->";
    if (values.empty()) {
        std::cout << " (empty)";
    } else {
        for (const std::string& value : values) std::cout << ' ' << value;
    }
    std::cout << '\n';
}

// Удобное текстовое представление результата логической проверки.
std::string boolText(bool value) {
    return value ? "TRUE" : "FALSE";
}

// Записывает одну структуру в файл: тип, имя и значения через пробел.
template <typename Range>
void writeRow(std::ofstream& output, char type, const std::string& name,
              const Range& values) {
    output << type << ' ' << name;
    for (const std::string& value : values) output << ' ' << value;
    output << '\n';
}

// Читает базу из файла. Если файла пока нет, возвращается пустая база.
Database loadDatabase(const std::string& filename) {
    Database db;
    std::ifstream input(filename);
    if (!input) return db;

    std::string line;
    while (std::getline(input, line)) {
        // Пустые строки и строки-комментарии пропускаем.
        if (line.empty() || line[0] == '#') continue;

        std::istringstream row(line);
        char type;
        std::string name;
        if (!(row >> type >> name)) continue;

        std::vector<std::string> values;
        std::string value;
        while (row >> value) values.push_back(value);

        // По первому символу определяем тип структуры и восстанавливаем её элементы.
        switch (type) {
            case 'M':
                for (const auto& item : values) db.arrays[name].pushBack(item);
                db.arrays.try_emplace(name);
                break;
            case 'F':
                for (const auto& item : values) db.singlyLists[name].pushBack(item);
                db.singlyLists.try_emplace(name);
                break;
            case 'L':
                for (const auto& item : values) db.doublyLists[name].pushBack(item);
                db.doublyLists.try_emplace(name);
                break;
            case 'S':
                for (const auto& item : values) db.stacks[name].push(item);
                db.stacks.try_emplace(name);
                break;
            case 'Q':
                for (const auto& item : values) db.queues[name].push(item);
                db.queues.try_emplace(name);
                break;
            case 'T':
                for (const auto& item : values) db.trees[name].insert(item);
                db.trees.try_emplace(name);
                break;
            default:
                // Неизвестный тип строки пропускается.
                break;
        }
    }
    return db;
}

// Сохраняет все структуры. При каждом изменении команды этот метод вызывается повторно.
void saveDatabase(const std::string& filename, const Database& db) {
    std::ofstream output(filename, std::ios::trunc);
    if (!output) throw std::runtime_error("cannot write data file: " + filename);

    output << "# ADSDB v1: type name values...\n";
    for (const auto& item : db.arrays) writeRow(output, 'M', item.first, item.second.values());
    for (const auto& item : db.singlyLists) writeRow(output, 'F', item.first, item.second.values());
    for (const auto& item : db.doublyLists) writeRow(output, 'L', item.first, item.second.values());
    for (const auto& item : db.stacks) writeRow(output, 'S', item.first, item.second.values());
    for (const auto& item : db.queues) writeRow(output, 'Q', item.first, item.second.values());
    for (const auto& item : db.trees) writeRow(output, 'T', item.first, item.second.values());

    if (!output) throw std::runtime_error("failed while writing data file: " + filename);
}

// Проверяет, что текст можно использовать как неотрицательный индекс.
bool parseIndex(const std::string& text, std::size_t& index) {
    try {
        std::size_t used = 0;
        long long number = std::stoll(text, &used);
        if (used != text.size() || number < 0) return false;
        index = static_cast<std::size_t>(number);
        return true;
    } catch (...) {
        return false;
    }
}

// Печать дерева по уровням и результата проверки свойства Full Binary Tree.
void printTree(const std::string& name, const FullBinaryTree& tree) {
    std::cout << "T " << name << " (Full Binary Tree)\n";
    auto levels = tree.levels();
    if (levels.empty()) std::cout << "(empty)\n";

    for (std::size_t depth = 0; depth < levels.size(); ++depth) {
        std::cout << "Level " << depth << ": ";
        for (std::size_t i = 0; i < levels[depth].size(); ++i) {
            if (i > 0) std::cout << ' ';
            std::cout << levels[depth][i];
        }
        std::cout << '\n';
    }
    std::cout << "Full: " << boolText(tree.isFull()) << '\n';
}

// Выводит структуру по типу и имени.
bool printContainer(const Database& db, char type, const std::string& name) {
    switch (type) {
        case 'M': {
            auto it = db.arrays.find(name);
            if (it == db.arrays.end()) return false;
            std::cout << "M " << name << ": ";
            printValues(it->second.values());
            return true;
        }
        case 'F': {
            auto it = db.singlyLists.find(name);
            if (it == db.singlyLists.end()) return false;
            std::cout << "F " << name << ": ";
            printValues(it->second.values());
            return true;
        }
        case 'L': {
            auto it = db.doublyLists.find(name);
            if (it == db.doublyLists.end()) return false;
            std::cout << "L " << name << ": ";
            printValues(it->second.values());
            return true;
        }
        case 'S': {
            auto it = db.stacks.find(name);
            if (it == db.stacks.end()) return false;
            std::cout << "S " << name << " (bottom -> top): ";
            printValues(it->second.values());
            return true;
        }
        case 'Q': {
            auto it = db.queues.find(name);
            if (it == db.queues.end()) return false;
            std::cout << "Q " << name << " (front -> back): ";
            printValues(it->second.values());
            return true;
        }
        case 'T': {
            auto it = db.trees.find(name);
            if (it == db.trees.end()) return false;
            printTree(name, it->second);
            return true;
        }
        default:
            return false;
    }
}

// Печать всех контейнеров, сгруппированных по типу.
void printAll(const Database& db) {
    bool found = false;
    for (const auto& item : db.arrays) found = printContainer(db, 'M', item.first) || found;
    for (const auto& item : db.singlyLists) found = printContainer(db, 'F', item.first) || found;
    for (const auto& item : db.doublyLists) found = printContainer(db, 'L', item.first) || found;
    for (const auto& item : db.stacks) found = printContainer(db, 'S', item.first) || found;
    for (const auto& item : db.queues) found = printContainer(db, 'Q', item.first) || found;
    for (const auto& item : db.trees) found = printContainer(db, 'T', item.first) || found;
    if (!found) std::cout << "(no containers)\n";
}

// Краткая подсказка по синтаксису команд.
void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " --file file.data --query 'COMMAND arguments'\n"
              << "Array: MPUSH, MINSERT, MGET, MSET, MDEL, MLEN\n"
              << "Singly list: FPUSH, FHEAD, FBEFORE, FAFTER, FDEL, FDELHEAD, FDELTAIL, FDELBEFORE, FDELAFTER, FSEARCH, FGET\n"
              << "Doubly list: LPUSH, LHEAD, LBEFORE, LAFTER, LDEL, LDELHEAD, LDELTAIL, LDELBEFORE, LDELAFTER, LSEARCH, LGET\n"
              << "Stack: SPUSH, SPOP, SPEEK, SGET; Queue: QPUSH, QPOP, QPEEK, QGET\n"
              << "Tree: TINSERT, ISMEMBER, TGET, TFULL; PRINT\n";
}

// Выполняет одну команду из --query.
int execute(Database& db, const std::string& filename, const std::vector<std::string>& a) {
    if (a.empty()) return 2;
    const std::string& command = a[0];

    // Общие вспомогательные действия для всех команд.
    auto need = [&](std::size_t count, const std::string& format) {
        if (a.size() >= count) return true;
        std::cerr << "Usage: " << format << '\n';
        return false;
    };
    auto persist = [&]() { saveDatabase(filename, db); };

    // PRINT выводит либо все структуры, либо выбранный контейнер.
    if (command == "PRINT") {
        if (a.size() == 1) {
            printAll(db);
            return 0;
        }
        if (a.size() == 3 && a[1].size() == 1) {
            if (printContainer(db, a[1][0], a[2])) return 0;
            std::cerr << "ERROR: container not found\n";
            return 1;
        }
        if (a.size() == 2) {
            bool found = false;
            for (char type : std::string("MFL SQT")) {
                if (type == ' ') continue;
                found = printContainer(db, type, a[1]) || found;
            }
            if (found) return 0;
        }
        std::cerr << "Usage: PRINT [type name | name]\n";
        return 2;
    }

    // ------------------------- Массив -------------------------
    if (command == "MPUSH") {
        if (!need(3, "MPUSH name value")) return 2;
        db.arrays[a[1]].pushBack(a[2]);
        persist();
        std::cout << "-> OK\n";
        return 0;
    }

    if (command == "MINSERT") {
        if (!need(4, "MINSERT name index value")) return 2;
        std::size_t index;
        if (!parseIndex(a[2], index)) { std::cout << "-> FALSE\n"; return 0; }
        bool ok = db.arrays[a[1]].insertAt(index, a[3]);
        if (ok) persist();
        std::cout << "-> " << boolText(ok) << '\n';
        return 0;
    }

    if (command == "MGET" || command == "MREAD") {
        if (!need(2, "MGET name [index]")) return 2;
        auto it = db.arrays.find(a[1]);
        if (it == db.arrays.end()) { std::cerr << "ERROR: array not found\n"; return 1; }
        if (a.size() == 2) printValues(it->second.values());
        else {
            std::size_t index;
            std::string value;
            if (!parseIndex(a[2], index) || !it->second.get(index, value)) std::cout << "-> FALSE\n";
            else std::cout << "-> " << value << '\n';
        }
        return 0;
    }

    if (command == "MSET" || command == "MDEL" || command == "MLEN") {
        if (!need(2, command + " name [index] [value]")) return 2;
        auto it = db.arrays.find(a[1]);
        if (it == db.arrays.end()) { std::cerr << "ERROR: array not found\n"; return 1; }
        if (command == "MLEN") { std::cout << "-> " << it->second.size() << '\n'; return 0; }
        if (command == "MSET" && !need(4, "MSET name index value")) return 2;
        if (command == "MDEL" && !need(3, "MDEL name index")) return 2;
        std::size_t index;
        if (!parseIndex(a[2], index)) { std::cout << "-> FALSE\n"; return 0; }
        bool ok = command == "MSET" ? it->second.set(index, a[3]) : it->second.eraseAt(index);
        if (ok) persist();
        std::cout << "-> " << boolText(ok) << '\n';
        return 0;
    }

    // ---------------------- Односвязный список ----------------------
    if (command == "FPUSH" || command == "FHEAD") {
        if (!need(3, command + " name value")) return 2;
        if (command == "FPUSH") db.singlyLists[a[1]].pushBack(a[2]);
        else db.singlyLists[a[1]].pushFront(a[2]);
        persist();
        std::cout << "-> OK\n";
        return 0;
    }

    if (command == "FBEFORE" || command == "FAFTER") {
        if (!need(4, command + " name target value")) return 2;
        auto it = db.singlyLists.find(a[1]);
        if (it == db.singlyLists.end()) { std::cout << "-> FALSE\n"; return 0; }
        bool ok = command == "FBEFORE" ? it->second.insertBefore(a[2], a[3]) : it->second.insertAfter(a[2], a[3]);
        if (ok) persist();
        std::cout << "-> " << boolText(ok) << '\n';
        return 0;
    }

    if (command == "FDEL" || command == "FDELHEAD" || command == "FDELTAIL" ||
        command == "FDELBEFORE" || command == "FDELAFTER") {
        std::size_t required = (command == "FDELHEAD" || command == "FDELTAIL") ? 2 : 3;
        if (!need(required, command + " name [value/target]")) return 2;
        auto it = db.singlyLists.find(a[1]);
        if (it == db.singlyLists.end()) { std::cout << "-> FALSE\n"; return 0; }

        bool ok = false;
        if (command == "FDEL") ok = it->second.eraseValue(a[2]);
        else if (command == "FDELHEAD") ok = it->second.popFront();
        else if (command == "FDELTAIL") ok = it->second.popBack();
        else if (command == "FDELBEFORE") ok = it->second.eraseBefore(a[2]);
        else ok = it->second.eraseAfter(a[2]);
        if (ok) persist();
        std::cout << "-> " << boolText(ok) << '\n';
        return 0;
    }

    if (command == "FGET" || command == "FSEARCH") {
        if (!need(2, command + " name [index/value]")) return 2;
        auto it = db.singlyLists.find(a[1]);
        if (it == db.singlyLists.end()) { std::cerr << "ERROR: list not found\n"; return 1; }
        if (command == "FSEARCH") {
            if (!need(3, "FSEARCH name value")) return 2;
            std::cout << "-> " << boolText(it->second.contains(a[2])) << '\n';
        } else if (a.size() == 2) {
            printValues(it->second.values());
        } else {
            std::size_t index;
            std::string value;
            if (!parseIndex(a[2], index) || !it->second.at(index, value)) std::cout << "-> FALSE\n";
            else std::cout << "-> " << value << '\n';
        }
        return 0;
    }

    // ----------------------- Двусвязный список -----------------------
    if (command == "LPUSH" || command == "LHEAD") {
        if (!need(3, command + " name value")) return 2;
        if (command == "LPUSH") db.doublyLists[a[1]].pushBack(a[2]);
        else db.doublyLists[a[1]].pushFront(a[2]);
        persist();
        std::cout << "-> OK\n";
        return 0;
    }

    if (command == "LBEFORE" || command == "LAFTER") {
        if (!need(4, command + " name target value")) return 2;
        auto it = db.doublyLists.find(a[1]);
        if (it == db.doublyLists.end()) { std::cout << "-> FALSE\n"; return 0; }
        bool ok = command == "LBEFORE" ? it->second.insertBefore(a[2], a[3]) : it->second.insertAfter(a[2], a[3]);
        if (ok) persist();
        std::cout << "-> " << boolText(ok) << '\n';
        return 0;
    }

    if (command == "LDEL" || command == "LDELHEAD" || command == "LDELTAIL" ||
        command == "LDELBEFORE" || command == "LDELAFTER") {
        std::size_t required = (command == "LDELHEAD" || command == "LDELTAIL") ? 2 : 3;
        if (!need(required, command + " name [value/target]")) return 2;
        auto it = db.doublyLists.find(a[1]);
        if (it == db.doublyLists.end()) { std::cout << "-> FALSE\n"; return 0; }

        bool ok = false;
        if (command == "LDEL") ok = it->second.eraseValue(a[2]);
        else if (command == "LDELHEAD") ok = it->second.popFront();
        else if (command == "LDELTAIL") ok = it->second.popBack();
        else if (command == "LDELBEFORE") ok = it->second.eraseBefore(a[2]);
        else ok = it->second.eraseAfter(a[2]);
        if (ok) persist();
        std::cout << "-> " << boolText(ok) << '\n';
        return 0;
    }

    if (command == "LGET" || command == "LSEARCH") {
        if (!need(2, command + " name [index/value]")) return 2;
        auto it = db.doublyLists.find(a[1]);
        if (it == db.doublyLists.end()) { std::cerr << "ERROR: list not found\n"; return 1; }
        if (command == "LSEARCH") {
            if (!need(3, "LSEARCH name value")) return 2;
            std::cout << "-> " << boolText(it->second.contains(a[2])) << '\n';
        } else if (a.size() == 2) {
            printValues(it->second.values());
        } else {
            std::size_t index;
            std::string value;
            if (!parseIndex(a[2], index) || !it->second.at(index, value)) std::cout << "-> FALSE\n";
            else std::cout << "-> " << value << '\n';
        }
        return 0;
    }

    // --------------------------- Стек ---------------------------
    if (command == "SPUSH") {
        if (!need(3, "SPUSH name value")) return 2;
        db.stacks[a[1]].push(a[2]);
        persist();
        std::cout << "-> OK\n";
        return 0;
    }

    if (command == "SPOP" || command == "SPEEK" || command == "SGET") {
        if (!need(2, command + " name")) return 2;
        auto it = db.stacks.find(a[1]);
        if (it == db.stacks.end()) { std::cout << "-> (empty)\n"; return 0; }
        if (command == "SGET") { printValues(it->second.values()); return 0; }

        std::string value;
        bool ok = command == "SPEEK" ? it->second.peek(value) : it->second.pop(value);
        if (!ok) std::cout << "-> (empty)\n";
        else std::cout << "-> " << value << '\n';
        if (ok && command == "SPOP") persist();
        return 0;
    }

    // -------------------------- Очередь --------------------------
    if (command == "QPUSH") {
        if (!need(3, "QPUSH name value")) return 2;
        db.queues[a[1]].push(a[2]);
        persist();
        std::cout << "-> OK\n";
        return 0;
    }

    if (command == "QPOP" || command == "QPEEK" || command == "QGET") {
        if (!need(2, command + " name")) return 2;
        auto it = db.queues.find(a[1]);
        if (it == db.queues.end()) { std::cout << "-> (empty)\n"; return 0; }
        if (command == "QGET") { printValues(it->second.values()); return 0; }

        std::string value;
        bool ok = command == "QPEEK" ? it->second.peek(value) : it->second.pop(value);
        if (!ok) std::cout << "-> (empty)\n";
        else std::cout << "-> " << value << '\n';
        if (ok && command == "QPOP") persist();
        return 0;
    }

    // -------------------- Full Binary Tree --------------------
    if (command == "TINSERT") {
        if (!need(3, "TINSERT name value")) return 2;
        db.trees[a[1]].insert(a[2]);
        persist();
        std::cout << "-> OK\n";
        return 0;
    }

    if (command == "TDEL") {
        std::cerr << "ERROR: TDEL is replaced by TFULL for the Full Binary Tree variant\n";
        return 2;
    }

    if (command == "ISMEMBER" || command == "TGET" || command == "TFULL") {
        if (!need(2, command + " name [value]")) return 2;
        auto it = db.trees.find(a[1]);
        if (it == db.trees.end()) { std::cerr << "ERROR: tree not found\n"; return 1; }
        if (command == "ISMEMBER") {
            if (!need(3, "ISMEMBER name value")) return 2;
            std::cout << "-> " << boolText(it->second.contains(a[2])) << '\n';
        } else if (command == "TGET") {
            printValues(it->second.values());
        } else {
            std::cout << "-> " << boolText(it->second.isFull()) << '\n';
        }
        return 0;
    }

    // Если команда не совпала с известной, показываем ошибку.
    std::cerr << "ERROR: unknown command: " << command << '\n';
    return 2;
}

// Точка входа: обрабатываем аргументы запуска, загружаем файл и выполняем запрос.
int main(int argc, char* argv[]) {
    std::string filename = "file.data";
    std::string query;

    // Аргументы --file и --query задают путь к данным и выполняемую команду.
    for (int i = 1; i < argc; ++i) {
        std::string argument = argv[i];
        if (argument == "--file" && i + 1 < argc) filename = argv[++i];
        else if (argument == "--query" && i + 1 < argc) query = argv[++i];
        else {
            printUsage(argv[0]);
            return 2;
        }
    }

    if (query.empty()) {
        printUsage(argv[0]);
        return 2;
    }

    try {
        Database db = loadDatabase(filename);
        std::istringstream queryStream(query);
        std::vector<std::string> arguments;
        std::string token;
        while (queryStream >> token) arguments.push_back(token);
        return execute(db, filename, arguments);
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
