#pragma once

#include "../Unity/IUnityInterface.h"

enum class Platform
{
    Unknown,
    // any 32bit architectures are treated as "Unknown" :)
    Windows_x86_64,
    Linux_x86_64,
    Mac_x86_64,
    Android_ARM64,
    iOS_ARM64,
    PS4,
};

using module_t = void*;

extern "C" {

UNITY_INTERFACE_EXPORT Platform    GetPlatform();
UNITY_INTERFACE_EXPORT const char* GetModulePath();
UNITY_INTERFACE_EXPORT void        AddDLLSearchPath(const char *v);
UNITY_INTERFACE_EXPORT void        SetEnv(const char *name, const char *value);
UNITY_INTERFACE_EXPORT const char* GetEnv(const char *name);

UNITY_INTERFACE_EXPORT module_t DLLLoad(const char *path);
UNITY_INTERFACE_EXPORT void     DLLUnload(module_t mod);
UNITY_INTERFACE_EXPORT void*    DLLGetSymbol(module_t mod, const char *name);
UNITY_INTERFACE_EXPORT module_t DLLGetHandle(const char *modname);

UNITY_INTERFACE_EXPORT void     usdiSetPluginPath(const char *path);

} // extern "C"
