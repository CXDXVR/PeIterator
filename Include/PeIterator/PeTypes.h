#pragma once

#if !defined(_WIN32)
#error Unsupported platform
#endif

#include <cstdint>
#include <Windows.h>

namespace pe_iterator {

// Image architecture.
enum class Architecture : BYTE { kX32, kX64, kNative = (sizeof(size_t) == sizeof(unsigned long) ? kX32 : kX64) };

// Image type.
enum class ImageType : BYTE {
    kFile,  // Raw file.
    kModule // Loaded module.
};

constexpr size_t ImportDirectoryIndex         = IMAGE_DIRECTORY_ENTRY_IMPORT;
constexpr size_t DelayImportDirectoryIndex    = IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT;
constexpr size_t ExportDirectoryIndex         = IMAGE_DIRECTORY_ENTRY_EXPORT;
constexpr size_t BaseRelocationDirectoryIndex = IMAGE_DIRECTORY_ENTRY_BASERELOC;
constexpr size_t TlsDirectoryIndex            = IMAGE_DIRECTORY_ENTRY_TLS;
constexpr size_t ExceptionsDirectoryIndex     = IMAGE_DIRECTORY_ENTRY_EXCEPTION;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Arch-Dependent
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<Architecture> struct NtHeaders;
template<Architecture> struct OptionalHeader;
template<Architecture> struct ImportAddressTable;
template<Architecture> struct ImportLookupTable;
template<Architecture> struct ImportNameTable;
template<Architecture> struct TlsDirectoryDescriptor;

template<> struct NtHeaders<Architecture::kX32> : IMAGE_NT_HEADERS32 { };
template<> struct NtHeaders<Architecture::kX64> : IMAGE_NT_HEADERS64 { };
template<> struct OptionalHeader<Architecture::kX32> : IMAGE_OPTIONAL_HEADER32 { };
template<> struct OptionalHeader<Architecture::kX64> : IMAGE_OPTIONAL_HEADER64 { };
template<> struct ImportAddressTable<Architecture::kX32> : IMAGE_THUNK_DATA32 { };
template<> struct ImportAddressTable<Architecture::kX64> : IMAGE_THUNK_DATA64 { };
template<> struct ImportLookupTable<Architecture::kX32> : ImportAddressTable<Architecture::kX32> { };
template<> struct ImportLookupTable<Architecture::kX64> : ImportAddressTable<Architecture::kX64> { };
template<> struct ImportNameTable<Architecture::kX32> : ImportAddressTable<Architecture::kX32> { };
template<> struct ImportNameTable<Architecture::kX64> : ImportAddressTable<Architecture::kX64> { };
template<> struct TlsDirectoryDescriptor<Architecture::kX32> : IMAGE_TLS_DIRECTORY32 { };
template<> struct TlsDirectoryDescriptor<Architecture::kX64> : IMAGE_TLS_DIRECTORY64 { };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Arch-Independent
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct IteratorEnd { }; // Iterator sentinel.

#pragma warning(push)
#pragma warning(disable : 4214) // nonstandard extension
typedef struct {
    WORD offset : 12;
    WORD type : 4;
} IMAGE_RELOC, *PIMAGE_RELOC;
#pragma warning(pop)

using DosHeader     = IMAGE_DOS_HEADER;
using FileHeader    = IMAGE_FILE_HEADER;
using DataDirectory = IMAGE_DATA_DIRECTORY;
using SectionHeader = IMAGE_SECTION_HEADER;
using ImportByName  = IMAGE_IMPORT_BY_NAME;

using Ordinal     = WORD;
using TlsCallback = PIMAGE_TLS_CALLBACK;
using RVA         = DWORD;

using ImportDirectoryDescriptor         = IMAGE_IMPORT_DESCRIPTOR;
using DelayImportDirectoryDescriptor    = IMAGE_DELAYLOAD_DESCRIPTOR;
using ExportDirectoryDescriptor         = IMAGE_EXPORT_DIRECTORY;
using BaseRelocationDirectoryDescriptor = IMAGE_BASE_RELOCATION;
using ExceptionDirectoryDescriptor      = RUNTIME_FUNCTION;

}
