#pragma once

#include "PeHeader.h"
#include "PeTypes.h"

namespace pe_iterator {

/**
 * @brief A wrapper class over the exports directory which provides a function iterator.
 * @tparam Arch Image architecture.
 */
template<Architecture Arch> class Export {
    // Exports tables.
    struct Tables {
        const DWORD*   Names;     // Pointer to functions names table.
        const Ordinal* Ordinals;  // Pointer to functions ordinals table.
        const RVA*     Functions; // Pointer to functions RVA table.
    };

public:
    /**
     * @brief Wrapper class over the exported function.
     */
    class Function {
    public:
        /**
         * @brief Initialization constructor.
         * @param address Pointer to the function.
         * @param ordinal Function ordinal
         * @param isForwarded true if forwarded.
         */
        Function(const BYTE* address, Ordinal ordinal, bool isForwarded)
            : address_(address)
            , ordinal_(ordinal)
            , forwarded_(isForwarded)
        {
        }

        /**
         * @brief Returns function address or nullptr if function forwarded.
         */
        const BYTE* GetAddress() const noexcept { return forwarded_ ? nullptr : address_; }

        /**
         * @brief Returns function forwarded string or nullptr if function is not forwarded.
         */
        const char* GetForwardedName() const noexcept { return forwarded_ ? reinterpret_cast<const char*>(address_) : nullptr; }

        /**
         * @brief Returns function ordinal.
         */
        Ordinal GetOrdinal() const noexcept { return ordinal_; }

        /**
         * @brief Returns true if function forwarded.
         */
        bool IsForwarded() const noexcept { return forwarded_; }

    private:
        const BYTE*   address_;
        const Ordinal ordinal_;
        const bool    forwarded_;
    };

    class Iterator {
    public:
        /**
         * @brief Initialization constructor.
         * @param header Image header
         * @param moduleExport Export class.
         * @param tables Export tables.
         * @param index The base index of the relocations from which the iteration will begin. By default, 0.
         */
        Iterator(const Header<Arch>& header, const Export& moduleExport, const Tables& tables, size_t index = 0)
            : header_(header)
            , export_(moduleExport)
            , tables_(tables)
            , index_(index)
        {
        }

        /**
         * @brief Returns current index.
         */
        size_t GetIndex() const noexcept { return index_; }

        /**
         * @brief Returns function name.
         */
        const char* GetName() const { return header_.RvaToVA<char>(tables_.Names[index_]); }

        /**
         * @brief Returns true if function forwarded.
         */
        bool IsForwarded() const { return export_.IsForwarded(tables_.Functions[index_]); }

        /**
         * @brief Returns function ordinal or 0 if function forwarded.
         */
        Ordinal GetOrdinal() const { return IsForwarded() ? 0 : export_.GetDirectoryDescriptor()->Base + index_; }

        /**
         * @brief Returns function address or nullptr if function forwarded.
         */
        const BYTE* GetAddress() const { return IsForwarded() ? nullptr : header_.RvaToVA<uint8_t>(tables_.Functions[index_]); }

        /**
         * @brief Returns function address or nullptr if function forwarded.
         */
        const char* GetForwardedName() const { return IsForwarded() ? header_.RvaToVA<char>(tables_.Functions[index_]) : nullptr; }

        /**
         * @brief Returns true if function are valid.
         */
        bool Validate() const { return index_ < export_.GetCountFunctions(); }

        Iterator& operator++()
        {
            ++index_;
            return *this;
        }

        Iterator operator++(int)
        {
            auto prev = *this;
            ++index_;
            return prev;
        }

        bool operator==(const Iterator& other) const noexcept { return index_ == other.index_; }
        bool operator!=(const Iterator& other) const { return index_ != other.index_; }
        bool operator==(IteratorEnd) const { return Validate(); }
        bool operator!=(IteratorEnd) const { return Validate(); }

        const Iterator& operator*() const { return *this; }
        Iterator&       operator*() { return *this; }

    private:
        const Header<Arch>& header_;
        const Export&       export_;
        const Tables&       tables_;
        size_t              index_;
    };

    /**
     * @brief Initialization constructor.
     * @param header Image header.
     */
    explicit Export(const Header<Arch>& header)
        : header_(header)
        , directoryDescriptor_(header_.GetDirectoryDescriptor<ExportDirectoryDescriptor>(ExportDirectoryIndex))
        , tables_ { header_.RvaToVA<DWORD>(directoryDescriptor_->AddressOfNames), header_.RvaToVA<Ordinal>(directoryDescriptor_->AddressOfNameOrdinals),
                    header_.RvaToVA<RVA>(directoryDescriptor_->AddressOfFunctions) }
    {
    }

    /**
     * @brief Returns pointer to exports directory descriptor.
     */
    const ExportDirectoryDescriptor* GetDirectoryDescriptor() const noexcept { return directoryDescriptor_; }

    /**
     * @brief Returns count of exported functions.
     */
    DWORD GetCountFunctions() const noexcept { return directoryDescriptor_->NumberOfFunctions; }

    /**
     * @brief Returns count of exported named functions.
     */
    DWORD GetCountOfFunctionsNames() const noexcept { return directoryDescriptor_->NumberOfNames; }

    /**
     * @brief Returns name of the module.
     */
    const char* GetModuleName() const { return header_.RvaToVA<char>(directoryDescriptor_->Name); }

    /**
     * @brief Returns true if exports directory descriptor in not nullptr.
     */
    bool IsValid() const noexcept { return directoryDescriptor_; }

    /**
     * @brief Returns true if exports are empty.
     */
    bool Empty() const noexcept { return !IsValid() || !directoryDescriptor_->NumberOfFunctions; }

    /**
     * @brief Returns true if RVA function is forwarded.
     * @param rva RVA offset
     */
    bool IsForwarded(RVA rva) const
    {
        auto dataDirectory = header_.GetDataDirectory(ExportDirectoryIndex);
        return rva > dataDirectory->VirtualAddress && rva < dataDirectory->VirtualAddress + dataDirectory->Size;
    }

    /**
     * @brief Searching function by name.
     * @param function Function name.
     * @return Exported function.
     */
    Function FindFunction(const char* function) const
    {
        if (!function || !IsValid())
            return {};

        // using binary search.
        uint32_t left = 0, right = GetCountOfFunctionsNames();
        while (left <= right) {
            auto curPos = left + (right - left) / 2;
            auto name   = header_.RvaToVA<char>(tables_.Names[curPos]);
            auto cmpRes = strcmp(name, function);

            if (cmpRes > 0)
                right = curPos;
            else if (cmpRes < 0)
                left = curPos + 1;
            else {
                left = curPos;
                break;
            }
        }

        if (left == right)
            return {};

        Ordinal functionHint = tables_.Ordinals[left];
        RVA     functionRVA  = tables_.Functions[functionHint];

        return Function(header_.RvaToVA<uint8_t>(functionRVA), GetDirectoryDescriptor()->Base + functionHint, IsForwarded(functionRVA));
    }

    /**
     * @brief Searching function by ordinal.
     * @param ordinal Function ordinal.
     * @return Exported function.
     */
    Function FindFunction(WORD ordinal) const
    {
        if (!IsValid() || !ordinal)
            return {};

        auto functionHint = ordinal - GetDirectoryDescriptor()->Base;
        if (functionHint >= GetCountFunctions())
            return {};

        RVA functionRVA = tables_.Functions[functionHint];
        return Function(header_.RvaToVA<uint8_t>(functionRVA), ordinal, IsForwarded(functionRVA));
    }

    /**
     * @brief Returns an iterator pointing to the beginning of functions.
     */
    Iterator begin() const { return Iterator(header_, *this, tables_); }

    /**
     *@brief Returns an iterator pointing to the end of functions.
     */
    IteratorEnd end() const { return {}; }

private:
    const Header<Arch>&              header_;
    const ExportDirectoryDescriptor* directoryDescriptor_;
    const Tables                     tables_;
};

}