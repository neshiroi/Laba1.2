#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// Узел односвязного списка: значение и ссылка на следующий узел.
struct SinglyNode {
    std::string value;
    std::unique_ptr<SinglyNode> next;

    explicit SinglyNode(const std::string& text) : value(text) {}
};

// Односвязный список. head_ указывает на начало, tail_ — на последний узел.
class SinglyList {
public:
    // Добавление в конец списка.
    void pushBack(const std::string& value) {
        auto node = std::make_unique<SinglyNode>(value);
        SinglyNode* rawNode = node.get();

        if (!head_) head_ = std::move(node);
        else tail_->next = std::move(node);

        tail_ = rawNode;
        ++size_;
    }

    // Добавление в начало списка.
    void pushFront(const std::string& value) {
        auto node = std::make_unique<SinglyNode>(value);
        node->next = std::move(head_);
        head_ = std::move(node);
        if (!tail_) tail_ = head_.get();
        ++size_;
    }

    // Вставка перед первым узлом с заданным значением.
    bool insertBefore(const std::string& target, const std::string& value) {
        if (!head_) return false;
        if (head_->value == target) {
            pushFront(value);
            return true;
        }

        SinglyNode* previous = head_.get();
        while (previous->next && previous->next->value != target) {
            previous = previous->next.get();
        }
        if (!previous->next) return false;

        auto node = std::make_unique<SinglyNode>(value);
        node->next = std::move(previous->next);
        previous->next = std::move(node);
        ++size_;
        return true;
    }

    // Вставка после первого узла с заданным значением.
    bool insertAfter(const std::string& target, const std::string& value) {
        SinglyNode* current = findNode(target);
        if (!current) return false;

        auto node = std::make_unique<SinglyNode>(value);
        SinglyNode* rawNode = node.get();
        node->next = std::move(current->next);
        current->next = std::move(node);
        if (tail_ == current) tail_ = rawNode;
        ++size_;
        return true;
    }

    // Удаление первого узла.
    bool popFront() {
        if (!head_) return false;
        head_ = std::move(head_->next);
        --size_;
        if (!head_) tail_ = nullptr;
        return true;
    }

    // Удаление последнего узла: для односвязного списка нужно найти предыдущий.
    bool popBack() {
        if (!head_) return false;
        if (head_.get() == tail_) return popFront();

        SinglyNode* previous = head_.get();
        while (previous->next.get() != tail_) previous = previous->next.get();
        previous->next.reset();
        tail_ = previous;
        --size_;
        return true;
    }

    // Удаление первого элемента с указанным значением.
    bool eraseValue(const std::string& value) {
        if (!head_) return false;
        if (head_->value == value) return popFront();

        SinglyNode* previous = head_.get();
        while (previous->next && previous->next->value != value) {
            previous = previous->next.get();
        }
        if (!previous->next) return false;

        bool removesTail = previous->next.get() == tail_;
        previous->next = std::move(previous->next->next);
        if (removesTail) tail_ = previous;
        --size_;
        return true;
    }

    // Удаление узла, стоящего непосредственно перед target.
    bool eraseBefore(const std::string& target) {
        if (!head_ || head_->value == target || !head_->next) return false;
        if (head_->next->value == target) return popFront();

        SinglyNode* beforePrevious = head_.get();
        SinglyNode* previous = head_->next.get();
        while (previous->next && previous->next->value != target) {
            beforePrevious = previous;
            previous = previous->next.get();
        }
        if (!previous->next) return false;

        beforePrevious->next = std::move(previous->next);
        --size_;
        return true;
    }

    // Удаление узла, стоящего непосредственно после target.
    bool eraseAfter(const std::string& target) {
        SinglyNode* current = findNode(target);
        if (!current || !current->next) return false;

        bool removesTail = current->next.get() == tail_;
        current->next = std::move(current->next->next);
        if (removesTail) tail_ = current;
        --size_;
        return true;
    }

    // Поиск значения и получение элемента по индексу.
    bool contains(const std::string& value) const { return findNode(value) != nullptr; }

    bool at(std::size_t index, std::string& value) const {
        if (index >= size_) return false;
        const SinglyNode* node = head_.get();
        while (index > 0) {
            node = node->next.get();
            --index;
        }
        value = node->value;
        return true;
    }

    // Копирование значений в вектор используется при выводе и сохранении файла.
    std::vector<std::string> values() const {
        std::vector<std::string> result;
        for (const SinglyNode* node = head_.get(); node; node = node->next.get()) {
            result.push_back(node->value);
        }
        return result;
    }

    std::size_t size() const { return size_; }
    bool empty() const { return !head_; }

private:
    std::unique_ptr<SinglyNode> head_;
    SinglyNode* tail_ = nullptr; // Не владеет памятью — только указывает на последний узел.
    std::size_t size_ = 0;

    // Вспомогательный проход по списку для поиска узла по значению.
    SinglyNode* findNode(const std::string& value) const {
        for (SinglyNode* node = head_.get(); node; node = node->next.get()) {
            if (node->value == value) return node;
        }
        return nullptr;
    }
};
