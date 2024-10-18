#pragma once

#include "PeTypes.h"

namespace pe_iterator {

/**
 * @brief Wrapper class over sections providing a section iterator.
 */
class Section {
public:
    class Iterator {
    public:
        /**
         * @brief Initialization constructor.
         * @param section Pointer to the header of the first section.
         * @param count Total count of sections.
         * @param index The base index of the section from which the iteration will begin. By default, 0.
         */
        Iterator(const SectionHeader* section, size_t count, size_t index = 0)
            : section_(section)
            , count_(count)
            , index_(index)
        {
        }

        /**
         * @brief Returns the index of the current iteration.
         */
        size_t GetIndex() const noexcept { return index_; }

        /**
         * @brief  Returns true if the pointer is valid and the iteration has not gone beyond.
         */
        bool IsValid() const noexcept { return section_ && index_ < count_; }

        Iterator& operator++() noexcept
        {
            ++index_;
            return *this;
        }

        Iterator operator++(int) noexcept
        {
            const auto prev = *this;
            ++index_;
            return prev;
        }

        // Compare operators
        bool operator==(const Iterator& other) const noexcept { return index_ == other.index_; }
        bool operator!=(const Iterator& other) const noexcept { return index_ != other.index_; }
        bool operator==(IteratorEnd) const { return !IsValid(); }
        bool operator!=(IteratorEnd) const { return IsValid(); }

        // Access operators.
        const SectionHeader& operator*() const { return section_[index_]; }
        const SectionHeader* operator->() const { return &section_[index_]; }

    private:
        const SectionHeader* section_;
        size_t               count_;
        size_t               index_;
    };

    /**
     * @brief Initialization constructor.
     * @param section Pointer to the header of the first section.
     * @param count Total count of sections.
     */
    Section(const SectionHeader* section, size_t count)
        : section_(section)
        , count_(count)
    {
        Iterator it(section, count);
    }

    /**
     * @brief Returns the total number of the sections.
     */
    DWORD GetCount() const noexcept { return count_; }

    /**
     * @brief Returns true if section pointer is not nullptr.
     */
    bool IsValid() const noexcept { return section_ != nullptr; }

    /**
     * @brief Returns true if sections are empty.
     */
    bool Empty() const noexcept { return GetCount() == 0 || !IsValid(); }

    /**
     * @brief Returns an iterator pointing to the first section.
     */
    Iterator begin() const noexcept { return { section_, count_ }; }

    /**
     * @brief Returns an iterator pointing to the final section.
     */
    IteratorEnd end() const noexcept { return {}; }

private:
    const SectionHeader* section_;
    const size_t         count_;
};

}