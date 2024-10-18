#pragma once

#include "PeHeader.h"
#include "PeTypes.h"

namespace pe_iterator {

/**
 * @brief A wrapper class over the relocation directory which provides a relocation block iterator.
 * @tparam Arch Image architecture.
 */
template<Architecture Arch> class Relocation {
public:
    /**
     * @brief Relocations iterator.
     */
    class RelocationIterator {
    public:
        /**
         * @brief Initialization constructor.
         * @param directoryDescriptor Pointer to the relocation block.
         * @param count Count of the relocations in the relocation block.
         * @param index The base index of the relocations from which the iteration will begin. By default, 0.
         */
        RelocationIterator(const BaseRelocationDirectoryDescriptor* directoryDescriptor, size_t count, size_t index = 0)
            : directoryDescriptor_(directoryDescriptor)
            , count_(count)
            , index_(index) {};

        /**
         * @brief Returns current reloc index.
         */
        size_t GetIndex() const noexcept { return index_; }

        /**
         * @brief Returns current relocation.
         */
        const IMAGE_RELOC* GetRelocation() const
        {
            const auto relocsList = reinterpret_cast<const IMAGE_RELOC*>(directoryDescriptor_ + 1);
            return &relocsList[index_];
        }

        /**
         * @brief Returns address to fix of the current relocation.
         */
        const BYTE* GetAddress() const { return reinterpret_cast<const BYTE*>(directoryDescriptor_) + GetRelocation()->offset; }

        /**
         * @brief Returns true if the pointer is valid and the iteration has not gone beyond.
         */
        bool IsValid() const noexcept { return index_ < count_; }

        RelocationIterator& operator++()
        {
            ++index_;
            return *this;
        }

        RelocationIterator operator++(int)
        {
            auto prev = *this;
            ++index_;
            return prev;
        }

        // Compare operators.
        bool operator==(const RelocationIterator& entry) const noexcept { return GetIndex() == entry.GetIndex(); }
        bool operator!=(const RelocationIterator& entry) const { return GetIndex() != entry.GetIndex(); }
        bool operator==(IteratorEnd) const { return IsValid(); }
        bool operator!=(IteratorEnd) const { return IsValid(); }

        const RelocationIterator& operator*() const { return *this; }
        RelocationIterator&       operator*() { return *this; }

    private:
        const BaseRelocationDirectoryDescriptor* directoryDescriptor_;
        size_t                                   count_;
        size_t                                   index_;
    };

    /**
     * @brief Relocation block iterator.
     */
    class BlockIterator {
    public:
        /**
         * @brief Initialization constructor.
         * @param directoryDescriptor Pointer to the relocation directory descriptor.
         */
        explicit BlockIterator(const BaseRelocationDirectoryDescriptor* directoryDescriptor)
            : directoryDescriptor_(directoryDescriptor) {};

        /**
         * @brief Returns pointer to the current relocation block.
         */
        const BaseRelocationDirectoryDescriptor* GetBlock() const noexcept { return directoryDescriptor_; }

        /**
         * @brief Returns count of relocations in the current relocation block.
         */
        size_t GetRelocationsCount() const noexcept { return (directoryDescriptor_->SizeOfBlock - sizeof(*directoryDescriptor_)) / sizeof(IMAGE_RELOC); }

        /**
         * @brief Returns true if the current relaxation block is valid.
         */
        bool IsValid() const noexcept { return directoryDescriptor_ && directoryDescriptor_->SizeOfBlock && directoryDescriptor_->VirtualAddress; }

        /**
         * @brief Returns an iterator pointing to the beginning of relocations in the current reloc block.
         */
        RelocationIterator begin() const noexcept { return RelocationIterator(directoryDescriptor_, GetRelocationsCount()); }

        /**
         * @brief Returns an iterator pointing to the end of the relocations in the current reloc block.
         */
        IteratorEnd end() const noexcept { return {}; }

        BlockIterator& operator++()
        {
            directoryDescriptor_ = reinterpret_cast<const BaseRelocationDirectoryDescriptor*>(reinterpret_cast<const BYTE*>(directoryDescriptor_)
                                                                                              + directoryDescriptor_->SizeOfBlock);
            return *this;
        }

        BlockIterator operator++(int)
        {
            const auto prev = *this;
            operator++();
            return prev;
        }

        bool operator==(const BlockIterator& other) const noexcept { return directoryDescriptor_ == other.directoryDescriptor_; }
        bool operator!=(const BlockIterator& other) const noexcept { return directoryDescriptor_ != other.directoryDescriptor_; }
        bool operator==(const IteratorEnd) const { return IsValid(); }
        bool operator!=(const IteratorEnd) const { return !IsValid(); }

        const BlockIterator& operator*() const { return *this; }
        BlockIterator&       operator*() { return *this; }

    private:
        const BaseRelocationDirectoryDescriptor* directoryDescriptor_;
    };

    /**
     * @brief Initialization constructor.
     * @param header Image header.
     */
    explicit Relocation(const Header<Arch>& header)
        : header_(header)
        , directoryDescriptor_(header_.GetDirectoryDescriptor<BaseRelocationDirectoryDescriptor>(BaseRelocationDirectoryIndex))
    {
    }

    /**
     * @brief Returns pointer to directory descriptor.
     */
    const BaseRelocationDirectoryDescriptor* GetDirectoryDescriptor() const noexcept { return directoryDescriptor_; }

    /**
     * @brief Returns true if relocs directory descriptor is not nullptr.
     */
    bool IsValid() const noexcept { return GetDirectoryDescriptor(); }

    /**
     * @brief Returns an iterator pointing to the beginning of the blocks.
     */
    BlockIterator begin() const noexcept { return BlockIterator(directoryDescriptor_); }

    /**
     * @brief Returns an iterator pointing to the end of blocks.
     */
    IteratorEnd end() const noexcept { return {}; }

private:
    const Header<Arch>&                      header_;
    const BaseRelocationDirectoryDescriptor* directoryDescriptor_;
};

}