#pragma once

#include "PeTypes.h"

namespace pe_iterator {

template<Architecture Arch> class Header {
public:
    /**
     * @brief Initialization header.
     * @param imageBase Pointer to the image base.
     * @param imageType Image type.
     */
    explicit Header(const BYTE* imageBase, const ImageType imageType = ImageType::kModule)
        : imageBase_(imageBase)
        , imageType_(imageType)
    {
    }

    /**
     * @brief Returns pointer to the DOS header.
     */
    const DosHeader* GetDosHeader() const { return reinterpret_cast<const DosHeader*>(imageBase_); }

    /**
     * @brief Returns pointer to the NT headers.
     */
    const NtHeaders<Arch>* GetNtHeaders() const { return reinterpret_cast<const NtHeaders<Arch>*>(imageBase_ + GetDosHeader()->e_lfanew); }

    /**
     * @brief Returns pointer to the optional header.
     */
    const OptionalHeader<Arch>* GetOptionalHeader() const { return reinterpret_cast<const OptionalHeader<Arch>*>(&GetNtHeaders()->OptionalHeader); }

    /**
     * @brief Returns pointer to the file header.
     */
    const FileHeader* GetFileHeader() const { return reinterpret_cast<const FileHeader*>(&GetNtHeaders()->FileHeader); }

    /**
     * @brief Returns pointer to the specified data directory.
     * @param directory Directory index.
     */
    const DataDirectory* GetDataDirectory(size_t directory) const
    {
        return reinterpret_cast<const DataDirectory*>(&GetOptionalHeader()->DataDirectory[directory]);
    }

    /**
     * @brief Returns pointer to the specified data directory descriptor, or nullptr if directory is empty.
     * @tparam DirectoryDescriptor Directory descriptor type.
     * @param directory Directory index.
     */
    template<typename DirectoryDescriptor> const DirectoryDescriptor* GetDirectoryDescriptor(size_t directory) const
    {
        const auto dataDirectory = GetDataDirectory(directory);
        if (dataDirectory->VirtualAddress && dataDirectory->Size)
            return RvaToVA<DirectoryDescriptor>(dataDirectory->VirtualAddress);

        return nullptr;
    }

    /**
     * @brief Calculating VA from base address + RVA.
     * @tparam Return Returns type value.
     * @param rva RVA offset.
     * @return Pointer to base address + RVA, or nullptr.
     */
    template<typename Return> Return* RvaToVA(RVA rva) const
    {
        if (imageType_ == ImageType::kModule)
            return (Return*)(imageBase_ + rva);

        auto       section       = IMAGE_FIRST_SECTION(GetNtHeaders());
        const auto sectionsCount = GetFileHeader()->NumberOfSections;
        const auto fileAlignment = GetOptionalHeader()->FileAlignment;

        for (WORD cx = 0; cx < sectionsCount; ++cx, ++section) {
            auto realSize       = section->SizeOfRawData + (fileAlignment - 1) & ~(fileAlignment - 1);
            auto virtualAddress = section->VirtualAddress;

            if (rva >= virtualAddress && rva < (virtualAddress + realSize))
                return (Return*)(imageBase_ + (rva - virtualAddress + section->PointerToRawData));
        }

        return nullptr;
    }

    /**
     * @brief Return true if headers are valid.
     */
    bool IsValid() const { return GetDosHeader()->e_magic == IMAGE_DOS_SIGNATURE && GetNtHeaders()->Signature == IMAGE_NT_SIGNATURE; }

    /**
     * @brief Returns a pointer to the current image.
     * @tparam T Return type.
     */
    template<typename T> T GetImageBase() const noexcept { return reinterpret_cast<T>(imageBase_); }

    /**
     * @brief Returns pointer to the image entry point.
     * @tparam T Entry point type.
     */
    template<typename T> T GetImageEntryPoint() const noexcept { return RvaToVA<T>(GetOptionalHeader()->AddressOfEntryPoint); }

    /**
     * @brief Returns current image type.
     */
    ImageType GetImageType() const noexcept { return imageType_; }

private:
    const BYTE*     imageBase_;
    const ImageType imageType_;
};

}