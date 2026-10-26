#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Узел дерева хранит значение и индексы левого и правого потомков.
// Число -1 означает, что соответствующего потомка нет.
struct TreeNode {
    std::string value;
    std::ptrdiff_t left = -1;
    std::ptrdiff_t right = -1;
};

// Бинарное дерево хранится в векторе в порядке обхода по уровням.
// У элемента с индексом i родитель имеет индекс (i-1)/2.
// Использование индексов вместо указателей упрощает управление памятью.
class FullBinaryTree {
public:
    // Добавляет узел в следующую свободную позицию по уровням.
    void insert(const std::string& value) {
        std::size_t index = nodes_.size();

        // Связываем новую позицию с родителем. Чётность индекса определяет сторону.
        if (index > 0) {
            std::size_t parent = (index - 1) / 2;
            if (index % 2 == 1) nodes_[parent].left = static_cast<std::ptrdiff_t>(index);
            else nodes_[parent].right = static_cast<std::ptrdiff_t>(index);
        }

        // Добавляем сам узел; у него пока нет потомков.
        nodes_.push_back({value, -1, -1});
    }

    // Линейный поиск значения по всем узлам.
    bool contains(const std::string& value) const {
        for (const TreeNode& node : nodes_) {
            if (node.value == value) return true;
        }
        return false;
    }

    // Full Binary Tree: каждый узел имеет либо 0, либо 2 потомка.
    // Один-единственный потомок хотя бы у одного узла означает FALSE.
    bool isFull() const {
        for (const TreeNode& node : nodes_) {
            const bool hasLeft = node.left != -1;
            const bool hasRight = node.right != -1;
            if (hasLeft != hasRight) return false;
        }
        return true;
    }

    // Возвращаем значения по уровням слева направо — такой порядок подходит для файла.
    std::vector<std::string> values() const {
        std::vector<std::string> result;
        for (const TreeNode& node : nodes_) result.push_back(node.value);
        return result;
    }

    bool empty() const { return nodes_.empty(); }

    // Формируем строки уровней, чтобы дерево было удобно читать в консоли.
    std::vector<std::vector<std::string>> levels() const {
        std::vector<std::vector<std::string>> result;
        std::size_t index = 0;
        std::size_t width = 1;

        while (index < nodes_.size()) {
            std::vector<std::string> level;
            for (std::size_t count = 0; count < width && index < nodes_.size(); ++count) {
                level.push_back(nodes_[index].value);
                ++index;
            }
            result.push_back(level);
            width *= 2;
        }
        return result;
    }

private:
    std::vector<TreeNode> nodes_; // Доступ к родителю и потомкам выполняется через индексы.
};
