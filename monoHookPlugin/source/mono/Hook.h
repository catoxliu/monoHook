#pragma once


enum class MemoryFlags
{
    ExecuteRead,
    ReadWrite,
    ExecuteReadWrite,
};

void SetMemoryProtection(void *addr, size_t size, MemoryFlags flags);

void ForceWrite(void *dst, const void *src, size_t s);
template<class T> inline void ForceWrite(T &dst, const T &src);
template<class Body> inline void ForceWrite(void *dst, size_t size, const Body& body);



// emit jmp from "from" to "to" instructions.
// instructions will be 5 byte if relative address is < 32 bit, oterwise 14 byte.
// return next address of last written byte (from+5 or from+14).
void* EmitJumpInstruction(void* from, const void* to);

void* OverrideDLLImport(void *target_module, const char *modname, const char *funcname, void *replacement);
void* OverrideDLLExport(void *target_module, const char *funcname, void *replacement);


void* FindSymbolByName(const char *name);


// impl

template<class T>
inline void ForceWrite(T &dst, const T &src)
{
    ForceWrite(&dst, &src, sizeof(T));
}

template<class Body>
inline void ForceWrite(void *dst, size_t size, const Body& body)
{
    SetMemoryProtection(dst, size, MemoryFlags::ExecuteReadWrite);
    body();
    SetMemoryProtection(dst, size, MemoryFlags::ExecuteRead);
}
