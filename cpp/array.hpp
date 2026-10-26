#pragma once

#include "string_sequence_view.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

// Самостоятельно реализованный динамический массив строк без std::vector.
class DynamicArray {
public:
    DynamicArray() = default;

    DynamicArray(const DynamicArray& other)
        : data_(other.capacity_ ? std::make_unique<std::string[]>(other.capacity_) : nullptr),
          size_(other.size_), capacity_(other.capacity_) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this == &other) return *this;
        DynamicArray copy(other);
        swap(copy);
        return *this;
    }

    DynamicArray(DynamicArray&&) noexcept = default;
    DynamicArray& operator=(DynamicArray&&) noexcept = default;

    // Добавляет элемент в конец. Увеличение буфера выполняется вручную.
    void pushBack(const std::string& value) {
        reserve(size_ + 1);
        data_[size_] = value;
        ++size_;
    }

    // Вставляет элемент по индексу; элементы справа сдвигаются на одну позицию.
    bool insertAt(std::size_t index, const std::string& value) {
        if (index > size_) return false;
        reserve(size_ + 1);
        for (std::size_t i = size_; i > index; --i) {
            data_[i] = std::move(data_[i - 1]);
        }
        data_[index] = value;
        ++size_;
        return true;
    }

    bool get(std::size_t index, std::string& value) const {
        if (index >= size_) return false;
        value = data_[index];
        return true;
    }

    bool set(std::size_t index, const std::string& value) {
        if (index >= size_) return false;
        data_[index] = value;
        return true;
    }

    bool eraseAt(std::size_t index) {
        if (index >= size_) return false;
        for (std::size_t i = index; i + 1 < size_; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }
        data_[size_ - 1].clear();
        --size_;
        return true;
    }

    bool popBack(std::string& value) {
        if (size_ == 0) return false;
        value = std::move(data_[size_ - 1]);
        data_[size_ - 1].clear();
        --size_;
        return true;
    }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    StringSequenceView values() const {
        return StringSequenceView(data_.get(), size_, capacity_);
    }

private:
    std::unique_ptr<std::string[]> data_;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;

    void reserve(std::size_t required) {
        if (required <= capacity_) return;
        std::size_t newCapacity = capacity_ == 0 ? 4 : capacity_;
        while (newCapacity < required) newCapacity *= 2;
        auto newData = std::make_unique<std::string[]>(newCapacity);
        for (std::size_t i = 0; i < size_; ++i) {
            newData[i] = std::move(data_[i]);
        }
        data_ = std::move(newData);
        capacity_ = newCapacity;
    }

    void swap(DynamicArray& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
};
