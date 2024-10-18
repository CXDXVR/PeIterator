#pragma once

#include "PeHeader.h"
#include "PeTypes.h"

namespace pe_iterator {

/**
 * @brief Wrapper class over exception directory providing iterator of handler functions.
 * @tparam Arch Image architecture.
 */
template<Architecture Arch> class Exception {
public:
    /**
     * @brief Class Wrapper over an exception handler function.
     */
    class Iterator {
    public:
        /**
         * @brief Initialization constructor.
         * @param directoryDescriptor Pointer to the exception directory descriptor.
         */
        explicit Iterator(const ExceptionDirectoryDescriptor* directoryDescriptor)
            : directoryDescriptor_(directoryDescriptor) {};

        /**
         * @brief Returns current runtime function.
         */
        const ExceptionDirectoryDescriptor* GetRuntimeFunction() const noexcept { return directoryDescriptor_; }

        /**
         * @brief Returns true if current function are valid
         */
        bool IsValid() const noexcept { return directoryDescriptor_ && directoryDescriptor_->BeginAddress; }

        Iterator& operator++()
        {
            ++directoryDescriptor_;
            return *this;
        }

        Iterator operator++(int)
        {
            auto prev = *this;
            ++directoryDescriptor_;
            return prev;
        }

        bool operator==(const Iterator& other) const noexcept { return directoryDescriptor_ == other.directoryDescriptor_; }
        bool operator!=(const Iterator& other) const { return directoryDescriptor_ != other.directoryDescriptor_; }
        bool operator==(IteratorEnd) const { return IsValid(); }
        bool operator!=(IteratorEnd) const { return IsValid(); }

        const Iterator& operator*() const { return *this; }
        Iterator&       operator*() { return *this; }

    private:
        const ExceptionDirectoryDescriptor* directoryDescriptor_;
    };

    /**
     * @brief Initialization constructor.
     * @param header Image header.
     */
    explicit Exception(const Header<Arch>& header)
        : header_(header)
        , directoryDescriptor_(header_.GetDirectoryDescriptor<ExceptionDirectoryDescriptor>(ExceptionsDirectoryIndex))
    {
    }

    /**
     * @brief Returns pointer to directory descriptor.
     */
    const ExceptionDirectoryDescriptor* GetDirectoryDescriptor() const noexcept { return directoryDescriptor_; }

    /**
     * @brief Returns true if exceptions directory is not nullptr.
     */
    bool IsValid() const noexcept { return directoryDescriptor_; }

    /**
     * @brief Returns an iterator pointing to the beginning of functions.
     */
    Iterator begin() const { return Iterator(directoryDescriptor_); }

    /**
     * @brief Returns an iterator pointing to the end of functions.
     */
    IteratorEnd end() const { return {}; }

private:
    const Header<Arch>&                 header_;
    const ExceptionDirectoryDescriptor* directoryDescriptor_;
};

}