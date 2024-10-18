#pragma once

#include "PeHeader.h"
#include "PeTypes.h"

namespace pe_iterator {

/**
 * @brief A wrapper class above the imports directory that provides a module iterator.
 * @tparam Arch Image architecture.
 */
template<Architecture Arch> class Import {
public:
    class ModuleIterator;

    /**
     * @brief Wrapper class over the module imported functions.
     */
    class FunctionIterator {
    public:
        /**
         * @brief Initialization constructor.
         * @param header Image header.
         * @param moduleIterator Module iterator.
         * @param index The base index of the relocations from which the iteration will begin. By default, 0.
         */
        FunctionIterator(const Header<Arch>& header, const ModuleIterator& moduleIterator, size_t index = 0)
            : header_(header)
            , moduleIterator_(moduleIterator)
            , index_(index)
        {
        }

        /**
         * @brief Returns current index.
         */
        size_t GetIndex() const noexcept { return index_; }

        /**
         * @brief Returns pointer to IAT of the function.
         */
        const ImportAddressTable<Arch>* GetImportAddressTable() const { return &moduleIterator_.GetImportAddressTable()[index_]; }

        /**
         * @brief Returns pointer to ILT of the function.
         */
        const ImportLookupTable<Arch>* GetImportLookupTable() const { return &moduleIterator_.GetImportLookupTable()[index_]; }

        /**
         * @brief Returns true if current function imported by ordinal.
         */
        bool IsImportedByOrdinal() const { return IMAGE_SNAP_BY_ORDINAL(GetImportLookupTable()->u1.Ordinal); }

        /**
         * @brief Returns a pointer to the IMAGE_IMPORT_BY_NAME structure if the function is exported by name, or nullptr.
         */
        const ImportByName* GetFunctionName() const
        {
            if (!IsImportedByOrdinal())
                return header_.RvaToVA<ImportByName>(GetImportLookupTable()->u1.AddressOfData);

            return nullptr;
        }

        /**
         * @brief Returns function ordinal if the function is exported by ordinal, or 0.
         */
        uint64_t GetFunctionOrdinal() const
        {
            if (!IsImportedByOrdinal())
                return GetImportLookupTable()->u1.AddressOfData;

            return 0;
        }

        /**
         * @brief Returns true if function are valid.
         */
        bool IsValid() const noexcept { return GetImportLookupTable()->u1.ForwarderString; }

        FunctionIterator& operator++()
        {
            ++index_;
            return *this;
        }

        FunctionIterator operator++(int)
        {
            auto prev = *this;
            ++index_;
            return prev;
        }

        bool operator==(const FunctionIterator& entry) const noexcept { return GetIndex() == entry.GetIndex(); }
        bool operator!=(const FunctionIterator& entry) const { return GetIndex() != entry.GetIndex(); }
        bool operator==(IteratorEnd) const { return IsValid(); }
        bool operator!=(IteratorEnd) const { return IsValid(); }

        const FunctionIterator& operator*() const { return *this; }
        FunctionIterator&       operator*() { return *this; }

    private:
        const Header<Arch>&   header_;
        const ModuleIterator& moduleIterator_;
        size_t                index_;
    };

    /**
     * @brief A wrapper class over the imported modules that provides a function iterator.
     */
    class ModuleIterator {
    public:
        /**
         * @bruef Initialization constructor.
         * @param header Image header.
         * @param directoryDescriptor A pointer to the beginning import directory descriptor.s
         */
        ModuleIterator(const Header<Arch>& header, const ImportDirectoryDescriptor* directoryDescriptor)
            : header_(header)
            , directoryDescriptor_(directoryDescriptor) {};

        /**
         * @brief Returns pointer to the current directory descriptor.
         */
        const ImportDirectoryDescriptor* GetDirectoryDescriptor() const noexcept { return directoryDescriptor_; }

        /**
         * @brief Returns pointer to module name string.
         */
        const char* GetModuleName() const { return header_.RvaToVA<char>(directoryDescriptor_->Name); }

        /**
         * @brief Returns pointer to the current IAT.
         */
        const ImportAddressTable<Arch>* GetImportAddressTable() const { return header_.RvaToVA<ImportAddressTable<Arch>>(directoryDescriptor_->FirstThunk); }

        /**
         * @brief Returns pointer to the current ILT.
         */
        const ImportLookupTable<Arch>* GetImportLookupTable() const
        {
            return header_.RvaToVA<ImportLookupTable<Arch>>(directoryDescriptor_->OriginalFirstThunk);
        }

        /**
         * @brief Returns true if current module directory descriptor is not nullptr.
         */
        bool IsValid() const noexcept { return directoryDescriptor_ && directoryDescriptor_->Characteristics; }

        /**
         * @brief  Returns an iterator pointing to the beginning of functions.
         */
        FunctionIterator begin() const noexcept { return { header_, *this }; }

        /**
         * @brief  Returns an iterator pointing to the end of functions.
         */
        IteratorEnd end() const noexcept { return {}; }

        ModuleIterator& operator++()
        {
            ++directoryDescriptor_;
            return *this;
        }

        ModuleIterator operator++(int)
        {
            const auto prev = *this;
            operator++();
            return prev;
        }

        bool operator==(const ModuleIterator& other) const noexcept { return directoryDescriptor_ == other.directoryDescriptor_; }
        bool operator==(const IteratorEnd) const { return IsValid(); }
        bool operator!=(const ModuleIterator& other) const noexcept { return directoryDescriptor_ != other.directoryDescriptor_; }
        bool operator!=(const IteratorEnd) const { return !IsValid(); }

        const ModuleIterator& operator*() const { return *this; }
        ModuleIterator&       operator*() { return *this; }

    private:
        const Header<Arch>&              header_;
        const ImportDirectoryDescriptor* directoryDescriptor_;
    };

    /**
     * @brief Initialization constructor.
     * @param header Image header.
     */
    explicit Import(const Header<Arch>& header)
        : header_(header)
        , directoryDescriptor_(header.GetDirectoryDescriptor<ImportDirectoryDescriptor>(ImportDirectoryIndex))
    {
    }

    /**
     * @brief Returns true if current import directory descriptor is not nullptr.
     */
    bool IsValid() const noexcept { return directoryDescriptor_ != nullptr; }

    /**
     * @brief Returns an iterator pointing to the first module.
     */
    ModuleIterator begin() const noexcept { return ModuleIterator(header_, directoryDescriptor_); }

    /**
     * @brief Returns an iterator pointing to the end of modules.
     */
    IteratorEnd end() const noexcept { return {}; }

private:
    const Header<Arch>&              header_;
    const ImportDirectoryDescriptor* directoryDescriptor_;
};

/**
 * @brief A wrapper class over a catalog of pending imports which provides a module iterator.
 * @tparam Arch Image architecture.
 */
template<Architecture Arch> class DelayedImport {
public:
    class ModuleIterator;

    /**
     * @brief Wrapper class over the module imported functions.
     */
    class FunctionIterator {
    public:
        /**
         * @brief Initialization constructor.
         * @param header Image header.
         * @param moduleIterator Module iterator.
         * @param index The base index of the relocations from which the iteration will begin. By default, 0.
         */
        FunctionIterator(const Header<Arch>& header, const ModuleIterator& moduleIterator, size_t index = 0)
            : header_(header)
            , moduleIterator_(moduleIterator)
            , index_(index)
        {
        }

        /**
         * @brief Returns current index.
         */
        size_t GetIndex() const noexcept { return index_; }

        /**
         * @brief Returns pointer to IAT of the function.
         */
        const ImportAddressTable<Arch>* GetImportAddressTable() const { return &moduleIterator_.GetImportAddressTable()[index_]; }

        /**
         * @brief Returns pointer to ILT of the function.
         */
        const ImportLookupTable<Arch>* GetImportLookupTable() const { return &moduleIterator_.GetImportLookupTable()[index_]; }

        /**
         * @brief Returns true if current function imported by ordinal.
         */
        bool IsImportedByOrdinal() const { return IMAGE_SNAP_BY_ORDINAL(GetImportLookupTable()->u1.Ordinal); }

        /**
         * @brief Returns a pointer to the IMAGE_IMPORT_BY_NAME structure if the function is exported by name, or nullptr.
         */
        const ImportByName* GetFunctionName() const
        {
            return IsImportedByOrdinal() ? nullptr : header_.RvaToVA<ImportByName>(GetImportLookupTable()->u1.AddressOfData);
        }

        /**
         * @brief Returns function ordinal if the function is exported by ordinal, or 0.
         */
        uint64_t GetFunctionOrdinal() const { return IsImportedByOrdinal() ? 0 : GetImportLookupTable()->u1.AddressOfData; }

        /**
         * @brief Returns true if function are valid.
         */
        bool IsValid() const noexcept { return GetImportLookupTable()->u1.ForwarderString; }

        FunctionIterator& operator++()
        {
            ++index_;
            return *this;
        }

        FunctionIterator operator++(int)
        {
            const auto prev = *this;
            operator++();
            return prev;
        }

        bool operator==(const FunctionIterator& entry) const noexcept { return GetIndex() == entry.GetIndex(); }
        bool operator!=(const FunctionIterator& entry) const { return GetIndex() != entry.GetIndex(); }
        bool operator==(IteratorEnd) const { return IsValid(); }
        bool operator!=(IteratorEnd) const { return IsValid(); }

        const FunctionIterator& operator*() const { return *this; }
        FunctionIterator&       operator*() { return *this; }

    private:
        const Header<Arch>&   header_;
        const ModuleIterator& moduleIterator_;
        size_t                index_;
    };

    /**
     * @brief A wrapper class over the imported modules that provides a function iterator.
     */
    class ModuleIterator {
    public:
        /**
         * @bruef Initialization constructor.
         * @param header Image header.
         * @param directoryDescriptor A pointer to the beginning import directory descriptor.s
         */
        ModuleIterator(const Header<Arch>& header, const DelayImportDirectoryDescriptor* directoryDescriptor)
            : header_(header)
            , directoryDescriptor_(directoryDescriptor) {};

        /**
         * @brief Returns pointer to the current directory descriptor.
         */
        const DelayImportDirectoryDescriptor* GetDirectoryDescriptor() const noexcept { return directoryDescriptor_; }

        /**
         * @brief Returns pointer to module name string.
         */
        const char* GetModuleName() const { return header_.RvaToVA<char>(directoryDescriptor_->DllNameRVA); }

        /**
         * @brief Returns pointer to IAT.
         */
        const ImportAddressTable<Arch>* GetImportAddressTable() const
        {
            return header_.RvaToVA<ImportAddressTable<Arch>>(directoryDescriptor_->ImportAddressTableRVA);
        }

        /**
         * @brief Returns pointer to ILT.
         */
        const ImportLookupTable<Arch>* GetImportLookupTable() const
        {
            return header_.RvaToVA<ImportLookupTable<Arch>>(directoryDescriptor_->ImportNameTableRVA);
        }

        /**
         * @brief Returns true if current module directory descriptor is not nullptr.
         */
        bool IsValid() const noexcept { return directoryDescriptor_ && directoryDescriptor_->DllNameRVA; }

        /**
         * @brief  Returns an iterator pointing to the beginning of functions.
         */
        FunctionIterator begin() const noexcept { return { header_, *this }; }

        /**
         * @brief  Returns an iterator pointing to the end of functions.
         */
        IteratorEnd end() const noexcept { return {}; }

        ModuleIterator& operator++()
        {
            ++directoryDescriptor_;
            return *this;
        }

        ModuleIterator operator++(int)
        {
            const auto prev = *this;
            operator++();
            return prev;
        }

        bool operator==(const ModuleIterator& other) const noexcept { return directoryDescriptor_ == other.directoryDescriptor_; }
        bool operator==(const IteratorEnd) const { return IsValid(); }
        bool operator!=(const ModuleIterator& other) const noexcept { return directoryDescriptor_ != other.directoryDescriptor_; }
        bool operator!=(const IteratorEnd) const { return !IsValid(); }

        const ModuleIterator& operator*() const { return *this; }
        ModuleIterator&       operator*() { return *this; }

    private:
        const Header<Arch>&                   header_;
        const DelayImportDirectoryDescriptor* directoryDescriptor_;
    };

    /**
     * @brief Initialization constructor.
     * @param header Image header.
     */
    explicit DelayedImport(const Header<Arch>& header)
        : header_(header)
        , directoryDescriptor_(header.GetDirectoryDescriptor<DelayImportDirectoryDescriptor>(DelayImportDirectoryIndex))
    {
    }

    /**
     * @brief Returns true if current import directory descriptor is not nullptr.
     */
    bool IsValid() const noexcept { return directoryDescriptor_ != nullptr; }

    /**
     * @brief Returns an iterator pointing to the first module.
     */
    ModuleIterator begin() const noexcept { return { header_, directoryDescriptor_ }; }

    /**
     * @brief Returns an iterator pointing to the end of modules.
     */
    IteratorEnd end() const noexcept { return {}; }

private:
    const Header<Arch>&                   header_;
    const DelayImportDirectoryDescriptor* directoryDescriptor_;
};

}