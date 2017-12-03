#include "pch.h"
#include "Hook.h"

#ifdef _WIN32
    #include <windows.h>
    #include <dbghelp.h>
    #pragma comment(lib, "dbghelp.lib")
#else
    #include <unistd.h>
    #include <sys/mman.h>
    using BYTE = uint8_t;
    using WORD = uint16_t;
    using DWORD = uint32_t;
    using DWORD_PTR = size_t;
#endif

void SetMemoryProtection(void *addr, size_t size, MemoryFlags flags)
{
#ifdef _WIN32
    DWORD flag = 0;
    switch (flags) {
    case MemoryFlags::ReadWrite: flag = PAGE_READWRITE; break;
    case MemoryFlags::ExecuteRead: flag = PAGE_EXECUTE_READ; break;
    case MemoryFlags::ExecuteReadWrite: flag = PAGE_EXECUTE_READWRITE; break;
    }
    DWORD old_flag;
    VirtualProtect(addr, size, flag, &old_flag);
#else
    int flag = 0;
    switch (flags) {
    case MemoryFlags::ReadWrite: flag = PROT_READ | PROT_WRITE; break;
    case MemoryFlags::ExecuteRead: flag = PROT_EXEC | PROT_READ; break;
    case MemoryFlags::ExecuteReadWrite: flag = PROT_EXEC | PROT_READ | PROT_WRITE; break;
    }
    void *page = (void*)((size_t)addr - ((size_t)addr % getpagesize()));
    mprotect(page, size, flag);
#endif
}

void ForceWrite(void *dst, const void *src, size_t size)
{
    SetMemoryProtection(dst, size, MemoryFlags::ExecuteReadWrite);
    memcpy(dst, src, size);
    SetMemoryProtection(dst, size, MemoryFlags::ExecuteRead);
}

void* EmitJumpInstruction(void* from_, const void* to_)
{
    BYTE *base, *from, *to;
    base = from = (BYTE*)from_;
    to = (BYTE*)to_;

    BYTE* jump_from = from + 5;
    size_t distance = jump_from > to ? jump_from - to : to - jump_from;

    // emit 5 byte jmp (0xe9 RVA) if relative address is less than 32 bit.
    // otherwise emit 14 byte long jmp (0xff 0x25 [memory] + target)
    if (distance <= 0x7fff0000) {
        from[0] = 0xe9;
        from += 1;
        *((DWORD*)from) = (DWORD)((size_t)to - (size_t)jump_from);
        from += 4;
    }
    else {
        from[0] = 0xff;
        from[1] = 0x25;
        from += 2;
#if defined(__Arch_x86_64__)
        *((DWORD*)from) = (DWORD)((size_t)from + 4);
#elif defined(__Arch_x86__)
        *((DWORD*)from) = (DWORD)0;
#endif
        from += 4;
        *((DWORD_PTR*)from) = (DWORD_PTR)(to);
        from += 8;
    }

#ifdef _WIN32
    ::FlushInstructionCache(nullptr, base, size_t(from - base));
#else
    __builtin___clear_cache((char*)from, (char*)base);
#endif
    return from;
}

void* OverrideDLLImport(void *module, const char *modname, const char *funcname, void *replacement)
{
#ifdef _WIN32
    if (!module) { return nullptr; }

    size_t ImageBase = (size_t)module;
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
    PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(ImageBase + pDosHeader->e_lfanew);

    size_t RVAImports = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    IMAGE_IMPORT_DESCRIPTOR *pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*)(ImageBase + RVAImports);
    while (pImportDesc->Name != 0) {
        if (_stricmp((const char*)(ImageBase + pImportDesc->Name), modname) == 0) {
            //const char *dllname = (const char*)(ImageBase + pImportDesc->Name);
            IMAGE_IMPORT_BY_NAME **func_names = (IMAGE_IMPORT_BY_NAME**)(ImageBase + pImportDesc->Characteristics);
            void **import_table = (void**)(ImageBase + pImportDesc->FirstThunk);
            for (size_t i = 0; ; ++i) {
                if ((size_t)func_names[i] == 0) { break; }
                const char *n = (const char*)(ImageBase + (size_t)func_names[i]->Name);
                if (strcmp(n, funcname) == 0) {
                    void *before = import_table[i];
                    ForceWrite<void*>(import_table[i], replacement);
                    return before;
                }
            }
        }
        ++pImportDesc;
    }
    return nullptr;

#else

    // not implemented yet
    return nullptr;
#endif
}

void* OverrideDLLExport(void *module, const char *funcname, void *replacement)
{
#ifdef _WIN32
    if (!module) { return nullptr; }

    size_t ImageBase = (size_t)module;
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) { return nullptr; }

    PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(ImageBase + pDosHeader->e_lfanew);
    DWORD RVAExports = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (RVAExports == 0) { return nullptr; }

    IMAGE_EXPORT_DIRECTORY *pExportDirectory = (IMAGE_EXPORT_DIRECTORY *)(ImageBase + RVAExports);
    DWORD *RVANames = (DWORD*)(ImageBase + pExportDirectory->AddressOfNames);
    WORD *RVANameOrdinals = (WORD*)(ImageBase + pExportDirectory->AddressOfNameOrdinals);
    DWORD *RVAFunctions = (DWORD*)(ImageBase + pExportDirectory->AddressOfFunctions);
    for (size_t i = 0; i < pExportDirectory->NumberOfFunctions; ++i) {
        char *pName = (char*)(ImageBase + RVANames[i]);
        if (strcmp(pName, funcname) == 0) {
            size_t ret = (size_t)module + RVAFunctions[RVANameOrdinals[i]];
            ForceWrite<DWORD>(RVAFunctions[RVANameOrdinals[i]], (DWORD)((size_t)replacement - ImageBase));
            return (void*)ret;
        }
    }
    return nullptr;

#else

    // not implemented yet
    return nullptr;
#endif
}

void* FindSymbolByName(const char *name)
{
#ifdef _WIN32
    static bool s_first = true;

    if (s_first) {
        s_first = false;

        // set path to main module to symbol search path
        char sympath[MAX_PATH] = "";
        {
            auto ret = ::GetModuleFileNameA(::GetModuleHandleA(nullptr), (LPSTR)sympath, sizeof(sympath));
            for (int i = ret - 1; i > 0; --i) {
                if (sympath[i] == '\\') {
                    sympath[i] = '\0';
                    break;
                }
            }
        }

        DWORD opt = ::SymGetOptions();
        opt |= SYMOPT_DEFERRED_LOADS;
        opt &= ~SYMOPT_UNDNAME;
        //opt |= SYMOPT_DEBUG;
        ::SymSetOptions(opt);
        ::SymInitialize(::GetCurrentProcess(), sympath, TRUE);
    }

    char buf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
    PSYMBOL_INFO sinfo = (PSYMBOL_INFO)buf;
    sinfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    sinfo->MaxNameLen = MAX_SYM_NAME;
    if (::SymFromName(::GetCurrentProcess(), name, sinfo) == FALSE) {
        return nullptr;
    }
    return (void*)sinfo->Address;

#else

    // not implemented yet
    return nullptr;
#endif
}
