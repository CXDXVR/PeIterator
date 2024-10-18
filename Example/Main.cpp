#include <cstdio>
#include <PeImage.h>
#include <string>
#include <vector>
#include <Windows.h>

static constexpr wchar_t kOptionAll[]         = L"/ALL";
static constexpr wchar_t kOptionSections[]    = L"/SECTIONS";
static constexpr wchar_t kOptionImports[]     = L"/IMPORTS";
static constexpr wchar_t kOptionExports[]     = L"/EXPORTS";
static constexpr wchar_t kOptionRelocations[] = L"/RELOCATIONS";
static constexpr wchar_t kOptionExceptions[]  = L"/EXCEPTIONS";
static constexpr wchar_t kOptionTls[]         = L"/TLS";

void ShowUsage()
{
    printf("Usage: PeLibExample.exe [option] [module]\nNOTE: The specified module must be loaded to the current process.\n\n");
    printf("  Options:\n\n");
    printf("    %ws\n", kOptionAll);
    printf("    %ws\n", kOptionSections);
    printf("    %ws\n", kOptionImports);
    printf("    %ws\n", kOptionExports);
    printf("    %ws\n", kOptionRelocations);
    printf("    %ws\n", kOptionExceptions);
    printf("    %ws\n\n", kOptionTls);
}

void ShowSections(const pe_iterator::Image<pe_iterator::Architecture::kNative>& image)
{
    const auto sections           = image.GetSection();
    const auto getCharacteristics = [](const pe_iterator::SectionHeader& section) -> std::string {
        std::string access;
        if (section.Characteristics & IMAGE_SCN_MEM_READ)
            access += "R";
        if (section.Characteristics & IMAGE_SCN_MEM_WRITE)
            access += "W";
        if (section.Characteristics & IMAGE_SCN_MEM_EXECUTE)
            access += "X";
        return access;
    };
    printf("******* SECTIONS HEADERS *******\n");
    if (sections.IsValid()) {
        for (const auto& section : sections) {
            printf("  %.8s\n", reinterpret_cast<const char*>(section.Name));
            printf("    Size of raw data: 0x%.8x\n", section.SizeOfRawData);
            printf("    Characteristics: %s\n", getCharacteristics(section).c_str());
        }
        printf("\n");
    } else {
        printf("  NO SECTIONS.\n\n");
    }
}

template<typename ImportsType> void ShowModuleNames(const ImportsType& imports, const char* header)
{
    printf("******* %s *******\n", header);
    if (imports.IsValid()) {
        for (const auto& import : imports) {
            printf("  Module: %s\n", import.GetModuleName());
            if (import.IsValid()) {
                for (const auto& function : import)
                    printf("    %s\n", function.GetFunctionName()->Name);
                printf("\n");
            } else {
                printf("  NO FUNCTIONS.\n\n");
            }
        }
    } else {
        printf("  NO %s.\n\n", header);
    }
}

void ShowImports(const pe_iterator::Image<pe_iterator::Architecture::kNative>& image)
{
    ShowModuleNames(image.GetImport(), "IMPORTS");
    ShowModuleNames(image.GetDelayedImport(), "DELAYED IMPORTS");
}

void ShowExports(const pe_iterator::Image<pe_iterator::Architecture::kNative>& image)
{
    const auto exports = image.GetExport();
    printf("******* EXPORTS *******\n");
    if (exports.IsValid()) {
        for (const auto& exp : exports) {
            printf("  Name: %s\n", exp.GetName());
            if (exp.IsForwarded())
                printf("  Forwarded name: %s\n", exp.GetForwardedName());
            else
                printf("  Ordinal: %d\n", exp.GetOrdinal());
            printf("\n");
        }
        printf("\n");
    } else {
        printf("  NO EXPORTS.\n\n");
    }
}

void ShowRelocations(const pe_iterator::Image<pe_iterator::Architecture::kNative>& image)
{
    const auto relocations       = image.GetRelocation();
    const auto getRelocationType = [](const pe_iterator::IMAGE_RELOC* reloc) -> std::string {
        switch (reloc->type) {
        case IMAGE_REL_BASED_ABSOLUTE: return "IMAGE_REL_BASED_ABSOLUTE";
        case IMAGE_REL_BASED_DIR64: return "IMAGE_REL_BASED_DIR64";
        case IMAGE_REL_BASED_HIGHLOW: return "IMAGE_REL_BASED_HIGHLOW";
        case IMAGE_REL_BASED_HIGHADJ: return "IMAGE_REL_BASED_HIGHADJ";
        case IMAGE_REL_BASED_HIGH: return "IMAGE_REL_BASED_HIGH";
        case IMAGE_REL_BASED_LOW: return "IMAGE_REL_BASED_LOW";
        }
        return {};
    };
    printf("******* RELOCATIONS *******\n");
    if (relocations.IsValid()) {
        for (const auto& block : relocations) {
            printf("  Block: 0x%lx", block.GetBlock()->VirtualAddress);
            for (const auto& reloc : block)
                printf("    %.25s  0x%lx (Offset in block: 0x%x)\n", getRelocationType(reloc.GetRelocation()).c_str(), reloc.GetAddress(),
                       reloc.GetRelocation()->offset);
            printf("\n");
        }
    } else {
        printf("  NO RELOCATIONS.\n\n");
    }
}

void ShowExceptions(const pe_iterator::Image<pe_iterator::Architecture::kNative>& image)
{
    const auto exceptions = image.GetException();
    printf("******* EXCEPTIONS *******\n");
    if (exceptions.IsValid()) {
        for (const auto& exception : exceptions)
            printf("  0x%x-0x%x\n", exception.GetRuntimeFunction()->BeginAddress, exception.GetRuntimeFunction()->EndAddress);
        printf("\n");
    } else
        printf("  NO EXCEPTIONS.\n\n");
}

void ShowTls(const pe_iterator::Image<pe_iterator::Architecture::kNative>& image)
{
    const auto tls = image.GetTls();
    printf("******* TLS *******\n");
    if (tls.IsValid()) {
        for (const auto& callback : tls)
            printf("  Callback: 0x%lx", callback.GetCallback());
        printf("\n");
    } else {
        printf("  NO TLS.\n\n");
    }
}

bool IsOptionSelected(const wchar_t* option, const wchar_t* argvOption)
{
    return lstrcmpW(option, argvOption) == 0 || lstrcmpW(argvOption, kOptionAll) == 0;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc < 3) {
        ShowUsage();
        return -1;
    }
    const auto moduleBase = GetModuleHandleW(argv[2]);
    if (!moduleBase) {
        printf("Module \"%ws\" not found.", argv[2]);
        return -1;
    }
    const pe_iterator::Image<pe_iterator::Architecture::kNative> image(reinterpret_cast<const BYTE*>(moduleBase));
    if (!image.GetHeader().IsValid()) {
        printf("The '%ws' module has an incorrect header.", argv[2]);
        return -1;
    }

    const wchar_t* option = argv[1];
    if (IsOptionSelected(kOptionSections, option))
        ShowSections(image);
    if (IsOptionSelected(kOptionImports, option))
        ShowImports(image);
    if (IsOptionSelected(kOptionExports, option))
        ShowExports(image);
    if (IsOptionSelected(kOptionRelocations, option))
        ShowRelocations(image);
    if (IsOptionSelected(kOptionExceptions, option))
        ShowExceptions(image);
    if (IsOptionSelected(kOptionTls, option))
        ShowTls(image);
    return 0;
}