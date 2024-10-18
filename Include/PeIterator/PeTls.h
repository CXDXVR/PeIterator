#pragma once

#include "PeHeader.h"
#include "PeTypes.h"

namespace pe_iterator {

/**
 * @brief A wrapper class over the TLS callback directory providing a callback function iterator.
 * @tparam Arch Image architecture.
 */
template<Architecture Arch> class Tls {
public:
    /**
     * @brief Callback wrapper class with functions.
     */
    class Iterator {
    public:
        /**
         * @brief Initialization constructor.
         * @param header Image header.
         * @param callback Pointer to first callback function.
         */
        Iterator(const Header<Arch>& header, const TlsCallback* callback)
            : header_(header)
            , callback_(callback)
        {
        }

        /**
         * @brief Returns current tls callback function.
         */
        const TlsCallback* GetCallback() const noexcept
        {
            const auto rva = static_cast<RVA>(reinterpret_cast<uint64_t>(*callback_) - header_.GetOptionalHeader()->ImageBase);
            return header_.RvaToVA<TlsCallback>(rva);
        }

        /**
         * @brief Returns true if callback are valid.
         */
        bool Validate() const noexcept { return callback_ && *callback_; }

        Iterator& operator++()
        {
            ++callback_;
            return *this;
        }

        Iterator operator++(int)
        {
            auto prev = *this;
            ++callback_;
            return prev;
        }

        bool operator==(const Iterator& other) const noexcept { return callback_ == other.callback_; }
        bool operator!=(const Iterator& other) const { return callback_ != other.callback_; }
        bool operator==(IteratorEnd) const { return Validate(); }
        bool operator!=(IteratorEnd) const { return Validate(); }

        const Iterator& operator*() const { return *this; }
        Iterator&       operator*() { return *this; }

    private:
        const Header<Arch>& header_;
        const TlsCallback*  callback_;
    };

    /**
     * @brief Initialization constructor.
     * @param header Image header.
     */
    explicit Tls(const Header<Arch>& header)
        : header_(header)
        , directoryDescriptor_(header_.GetDirectoryDescriptor<TlsDirectoryDescriptor<Arch>>(TlsDirectoryIndex)) {};

    /**
     * @brief Returns pointer to directory descriptor.
     */
    const TlsDirectoryDescriptor<Arch>* GetDirectoryDescriptor() const noexcept { return directoryDescriptor_; }

    /**
     * @brief Returns pointer to first callback from callbacks table.
     */
    const TlsCallback* GetCallbacks() const noexcept
    {
        return header_.GetImageType() == ImageType::kModule
            ? reinterpret_cast<TlsCallback*>(GetDirectoryDescriptor()->AddressOfCallBacks)
            : header_.RvaToVA<TlsCallback>(GetDirectoryDescriptor()->AddressOfCallBacks - header_.GetOptionalHeader()->ImageBase);
    }

    /**
     * @brief Returns true if tls directory are valid.
     */
    bool IsValid() const noexcept { return directoryDescriptor_; }

    // Returns an iterator pointing to the beginning of the callback functions.
    Iterator begin() const noexcept { return { header_, GetCallbacks() }; }

    // Returns an iterator pointing to the end of the function callback.
    IteratorEnd end() const noexcept { return {}; }

private:
    const Header<Arch>&                 header_;
    const TlsDirectoryDescriptor<Arch>* directoryDescriptor_;
};

};