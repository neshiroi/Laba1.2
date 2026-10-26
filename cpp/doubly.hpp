#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// Узел двусвязного списка хранит ссылки на предыдущий и следующий элементы.
struct DoublyNode {
    std::string value;
    DoublyNode* previous = nullptr;
    std::unique_ptr<DoublyNode> next;

    explicit DoublyNode(const std::string& text) : value(text) {}
};

// В двусвязном списке можно двигаться как вперёд, так и назад.
class DoublyList {
public:
    // Добавление в конец и начало списка.
    void pushBack(const std::string& value) {
        auto node = std::make_unique<DoublyNode>(value);
        DoublyNode* rawNode = node.get();
        node->previous = tail_;
        if (!head_) head_ = std::move(node);
        else tail_->next = std::move(node);
        tail_ = rawNode;
        ++size_;
    }

    void pushFront(const std::string& value) {
        auto node = std::make_unique<DoublyNode>(value);
        DoublyNode* rawNode = node.get();
        node->next = std::move(head_);
        if (node->next) node->next->previous = rawNode;
        head_ = std::move(node);
        if (!tail_) tail_ = head_.get();
        ++size_;
    }

    // Вставка перед/после найденного узла.
    bool insertBefore(const std::string& target, const std::string& value) {
        DoublyNode* current = findNode(target);
        if (!current) return false;
        if (current == head_.get()) {
            pushFront(value);
            return true;
        }

        DoublyNode* previous = current->previous;
        auto node = std::make_unique<DoublyNode>(value);
        DoublyNode* rawNode = node.get();
        node->previous = previous;
        node->next = std::move(previous->next);
        node->next->previous = rawNode;
        previous->next = std::move(node);
        ++size_;
        return true;
    }

    bool insertAfter(const std::string& target, const std::string& value) {
        DoublyNode* current = findNode(target);
        if (!current) return false;

        auto node = std::make_unique<DoublyNode>(value);
        DoublyNode* rawNode = node.get();
        node->previous = current;
        node->next = std::move(current->next);
        if (node->next) node->next->previous = rawNode;
        else tail_ = rawNode;
        current->next = std::move(node);
        ++size_;
        return true;
    }

    // Удаление первого и последнего узлов.
    bool popFront() {
        if (!head_) return false;
        head_ = std::move(head_->next);
        --size_;
        if (head_) head_->previous = nullptr;
        else tail_ = nullptr;
        return true;
    }

    bool popBack() {
        if (!tail_) return false;
        if (tail_ == head_.get()) return popFront();
        DoublyNode* previous = tail_->previous;
        previous->next.reset();
        tail_ = previous;
        --size_;
        return true;
    }

    // Удаление по значению или относительно заданного элемента.
    bool eraseValue(const std::string& value) {
        DoublyNode* node = findNode(value);
        return node ? eraseNode(node) : false;
    }

    bool eraseBefore(const std::string& target) {
        DoublyNode* node = findNode(target);
        return node && node->previous ? eraseNode(node->previous) : false;
    }

    bool eraseAfter(const std::string& target) {
        DoublyNode* node = findNode(target);
        return node && node->next ? eraseNode(node->next.get()) : false;
    }

    // Поиск и чтение по индексу. Для дальнего элемента начинаем с ближайшего конца.
    bool contains(const std::string& value) const { return findNode(value) != nullptr; }

    bool at(std::size_t index, std::string& value) const {
        if (index >= size_) return false;
        const DoublyNode* node;
        if (index < size_ / 2) {
            node = head_.get();
            while (index > 0) { node = node->next.get(); --index; }
        } else {
            node = tail_;
            std::size_t stepsBack = size_ - 1 - index;
            while (stepsBack > 0) { node = node->previous; --stepsBack; }
        }
        value = node->value;
        return true;
    }

    std::vector<std::string> values() const {
        std::vector<std::string> result;
        for (const DoublyNode* node = head_.get(); node; node = node->next.get()) {
            result.push_back(node->value);
        }
        return result;
    }

    std::size_t size() const { return size_; }
    bool empty() const { return !head_; }

private:
    std::unique_ptr<DoublyNode> head_;
    DoublyNode* tail_ = nullptr;
    std::size_t size_ = 0;

    // Поиск возвращает узел или nullptr, если значение отсутствует.
    DoublyNode* findNode(const std::string& value) const {
        for (DoublyNode* node = head_.get(); node; node = node->next.get()) {
            if (node->value == value) return node;
        }
        return nullptr;
    }

    // Удаление узла из середины с восстановлением обеих связей.
    bool eraseNode(DoublyNode* node) {
        if (node == head_.get()) return popFront();
        if (node == tail_) return popBack();

        DoublyNode* previous = node->previous;
        auto removed = std::move(previous->next);
        previous->next = std::move(removed->next);
        previous->next->previous = previous;
        --size_;
        return true;
    }
};
