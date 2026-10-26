#pragma once

#include "array.hpp"
#include "string_sequence_view.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

// Стек реализован поверх собственного динамического массива (без std::vector).
// LIFO: последний добавленный элемент удаляется первым.
class Stack {
public:
    void push(const std::string& value) { data_.pushBack(value); }

    bool pop(std::string& value) { return data_.popBack(value); }

    bool peek(std::string& value) const {
        if (data_.empty()) return false;
        return data_.get(data_.size() - 1, value);
    }

    bool empty() const { return data_.empty(); }
    std::size_t size() const { return data_.size(); }
    StringSequenceView values() const { return data_.values(); }

private:
    DynamicArray data_;
};

// Очередь реализована вручную как кольцевой динамический буфер без std::deque/std::vector.
// FIFO: первым удаляется элемент, который находился в очереди дольше остальных.
class Queue {
public:
    Queue() = default;

    Queue(const Queue& other)
        : data_(other.capacity_ ? std::make_unique<std::string[]>(other.capacity_) : nullptr),
          head_(0), size_(other.size_), capacity_(other.capacity_) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.logicalAt(i);
        head_ = 0;
    }

    Queue& operator=(const Queue& other) {
        if (this == &other) return *this;
        Queue copy(other);
        swap(copy);
        return *this;
    }

    Queue(Queue&&) noexcept = default;
    Queue& operator=(Queue&&) noexcept = default;

    void push(const std::string& value) {
        if (size_ == capacity_) reserve(capacity_ == 0 ? 4 : capacity_ * 2);
        const std::size_t tail = (head_ + size_) % capacity_;
        data_[tail] = value;
        ++size_;
    }

    bool pop(std::string& value) {
        if (size_ == 0) return false;
        value = std::move(data_[head_]);
        data_[head_].clear();
        head_ = (head_ + 1) % capacity_;
        --size_;
        if (size_ == 0) head_ = 0;
        return true;
    }

    bool peek(std::string& value) const {
        if (size_ == 0) return false;
        value = data_[head_];
        return true;
    }

    bool empty() const { return size_ == 0; }
    std::size_t size() const { return size_; }
    StringSequenceView values() const {
        return StringSequenceView(data_.get(), size_, capacity_, head_, true);
    }

private:
    std::unique_ptr<std::string[]> data_;
    std::size_t head_ = 0;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;

    const std::string& logicalAt(std::size_t index) const {
        return data_[(head_ + index) % capacity_];
    }

    void reserve(std::size_t newCapacity) {
        auto newData = std::make_unique<std::string[]>(newCapacity);
        for (std::size_t i = 0; i < size_; ++i) {
            newData[i] = std::move(data_[(head_ + i) % capacity_]);
        }
        data_ = std::move(newData);
        capacity_ = newCapacity;
        head_ = 0;
    }

    void swap(Queue& other) noexcept {
        data_.swap(other.data_);
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
};
