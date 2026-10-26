//Этот файл(класс) нужен, чтобы просматривать последовательность строк без использования std::vector и без копирования всех элементов.
#pragma once

#include <cstddef>
#include <string>

// Лёгкое представление последовательности строк без копирования и std::vector.
// Поддерживает и обычный массив, и кольцевой буфер очереди.
class StringSequenceView {
public:
    class const_iterator {
    public:
        const_iterator(const StringSequenceView* view, std::size_t index)
            : view_(view), index_(index) {}

        const std::string& operator*() const { return (*view_)[index_]; }
        const_iterator& operator++() { ++index_; return *this; }
        bool operator!=(const const_iterator& other) const {
            return view_ != other.view_ || index_ != other.index_;
        }
        bool operator==(const const_iterator& other) const { return !(*this != other); }

    private:
        const StringSequenceView* view_;
        std::size_t index_;
    };

    StringSequenceView(const std::string* data, std::size_t size,
                       std::size_t capacity, std::size_t offset = 0,
                       bool circular = false)
        : data_(data), size_(size), capacity_(capacity), offset_(offset),
          circular_(circular) {}

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    const std::string& operator[](std::size_t index) const {
        const std::size_t physicalIndex = circular_
            ? (offset_ + index) % capacity_
            : index;
        return data_[physicalIndex];
    }

    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, size_); }

private:
    const std::string* data_;
    std::size_t size_;
    std::size_t capacity_;
    std::size_t offset_;
    bool circular_;
};
