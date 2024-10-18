#pragma once

#include "PeException.h"
#include "PeExport.h"
#include "PeHeader.h"
#include "PeImport.h"
#include "PeRelocation.h"
#include "PeSection.h"
#include "PeTls.h"
#include "PeTypes.h"

namespace pe_iterator {

template<Architecture Arch> class Image {
public:
    /**
     * @brief Initialization header.
     * @param imageBase Pointer to the image base.
     * @param imageType Image type.
     */
    explicit Image(const BYTE* imageBase, ImageType imageType = ImageType::kModule)
        : imageBase_(imageBase)
        , header_(imageBase, imageType)
    {
    }

    /**
     * @brief Returns image header.
     */
    Header<Arch> GetHeader() const noexcept { return header_; }

    /**
     * @brief Returns image sections.
     */
    Section GetSection() const noexcept { return Section(IMAGE_FIRST_SECTION(header_.GetNtHeaders()), header_.GetNtHeaders()->FileHeader.NumberOfSections); }

    /**
     * @bried Returns image relocations.
     */
    Relocation<Arch> GetRelocation() const noexcept { return Relocation<Arch>(header_); }

    /**
     * @brief Returns image imports.
     */
    Import<Arch> GetImport() const noexcept { return Import<Arch>(header_); }

    /**
     * @brief Returns image delayed imports.
     */
    DelayedImport<Arch> GetDelayedImport() const noexcept { return DelayedImport<Arch>(header_); }

    /**
     * @brief Returns image exports.
     */
    Export<Arch> GetExport() const noexcept { return Export<Arch>(header_); }

    /**
     * @brief Returns image exceptions.
     */
    Exception<Arch> GetException() const noexcept { return Exception<Arch>(header_); }

    /**
     * @brief Returns image TLS.
     */
    Tls<Arch> GetTls() const noexcept { return Tls<Arch>(header_); }

private:
    const BYTE*        imageBase_;
    const Header<Arch> header_;
};

}
