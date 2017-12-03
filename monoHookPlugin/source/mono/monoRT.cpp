#include "pch.h"
#include "monoRT.h"
#include "Platform.h"

extern "C" {

UNITY_INTERFACE_EXPORT Platform GetPlatform()
{
#if defined(__Windows_x86_64__)
    return Platform::Windows_x86_64;
#elif defined(__Linux_x86_64__)
    return Platform::Linux_x86_64;
#elif defined(__Mac_x86_64__)
    return Platform::Mac_x86_64;
#elif defined(__Android_ARM64__)
    return Platform::Android_ARM64;
#elif defined(__iOS_ARM64__)
    return Platform::iOS_ARM64;
#elif defined(__PS4__)
    return Platform::PS4;
#endif
}

UNITY_INTERFACE_EXPORT const char* GetModulePath()
{
#ifdef _WIN32
    static char s_path[MAX_PATH + 1];
    if (s_path[0] == 0) {
        HMODULE mod = 0;
        ::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)&GetModulePath, &mod);
        DWORD size = ::GetModuleFileNameA(mod, s_path, sizeof(s_path));
        for (int i = size - 1; i >= 0; --i) {
            if (s_path[i] == '\\') {
                s_path[i] = '\0';
                break;
            }
        }
    }
    return s_path;
#else
    static std::string s_path;
    if(s_path.empty()) {
        Dl_info info;
        dladdr((const void*)&GetModulePath, &info);
        s_path = info.dli_fname;
        
        auto i = s_path.find(".bundle");
        if(i == std::string::npos) {
            i = s_path.empty() ? 0 : s_path.size() - 1;
        }
        for(; i > 0; --i) {
            if(s_path[i] == '/') {
                s_path[i] = '\0';
                break;
            }
        }
    }
    return s_path.c_str();
#endif
}

UNITY_INTERFACE_EXPORT void AddDLLSearchPath(const char *v)
{
#ifdef _WIN32
    std::string path;
    {
        DWORD size = ::GetEnvironmentVariableA(LIBRARY_PATH, nullptr, 0);
        if (size > 0) {
            path.resize(size);
            ::GetEnvironmentVariableA(LIBRARY_PATH, &path[0], (DWORD)path.size());
            path.pop_back(); // delete last '\0'
        }
    }
    if (path.find(v) == std::string::npos) {
        if(!path.empty()) { path += ";"; }
        auto pos = path.size();
        path += v;
        for (size_t i = pos; i < path.size(); ++i) {
            char& c = path[i];
            if (c == '/') { c = '\\'; }
        }
        ::SetEnvironmentVariableA(LIBRARY_PATH, path.c_str());
    }
#else
    std::string path;
    if (auto path_ = ::getenv(LIBRARY_PATH)) {
        path = path_;
    }
    if (path.find(v) == std::string::npos) {
        if(!path.empty()) { path += ":"; }
        auto pos = path.size();
        path += v;
        for (size_t i = pos; i < path.size(); ++i) {
            char& c = path[i];
            if (c == '\\') { c = '/'; }
        }
        ::setenv(LIBRARY_PATH, path.c_str(), 1);
    }
#endif
}

UNITY_INTERFACE_EXPORT void SetEnv(const char *name, const char *value)
{
#ifdef _WIN32
    // get/setenv() and Set/GetEnvironmentVariable() is *not* compatible.
    // set both to make sure.
    ::_putenv_s(name, value);
    ::SetEnvironmentVariableA(name, value);
#else
    ::setenv(name, value, 1);
#endif
}

UNITY_INTERFACE_EXPORT const char* GetEnv(const char *name)
{
#ifdef _WIN32
    static std::string s_value;
    {
        auto n = ::GetEnvironmentVariableA(name, nullptr, 0);
        if (n > 0) {
            s_value.resize(n);
            ::GetEnvironmentVariableA(name, &s_value[0], n);
            s_value.pop_back(); // remove last '\0'
            return s_value.c_str();
        }
        else {
            return ::getenv(name);
        }
    }
#else
    ::getenv(name);
#endif
}

#ifdef _WIN32

UNITY_INTERFACE_EXPORT module_t  DLLLoad(const char *path) { return ::LoadLibraryA(path); }
UNITY_INTERFACE_EXPORT void      DLLUnload(module_t mod) { ::FreeLibrary((HMODULE)mod); }
UNITY_INTERFACE_EXPORT void*     DLLGetSymbol(module_t mod, const char *name) { return ::GetProcAddress((HMODULE)mod, name); }
UNITY_INTERFACE_EXPORT module_t  DLLGetHandle(const char *modname) { return ::GetModuleHandleA(modname); }

#else

UNITY_INTERFACE_EXPORT module_t  DLLLoad(const char *path) { return ::dlopen(path, RTLD_LAZY); }
UNITY_INTERFACE_EXPORT void      DLLUnload(module_t mod) { ::dlclose(mod); }
UNITY_INTERFACE_EXPORT void*     DLLGetSymbol(module_t mod, const char *name)
{
    return ::dlsym(mod, name);
}

UNITY_INTERFACE_EXPORT module_t DLLGetHandle(const char *modname)
{
#ifdef __APPLE__

    int n = (int)_dyld_image_count();
    for (int i = 0; i < n; ++i) {
        auto *path = _dyld_get_image_name(i);
        if (strstr(path, modname)) {
            return dlopen(path, RTLD_LAZY);
        }
    }

#else

    auto *mod = dlopen(nullptr, RTLD_LAZY);
    link_map *it = nullptr;
    dlinfo(mod, RTLD_DI_LINKMAP, &it);
    while (it) {
        if (strstr(it->l_name, modname)) {
            return dlopen(it->l_name, RTLD_LAZY);
        }
        it = it->l_next;
    }

#endif
    return nullptr;
}

#endif

UNITY_INTERFACE_EXPORT void usdiSetPluginPath(const char *path_)
{
    std::string path = path_;
    for (char& c : path) {
#if _WIN32
        if (c == '/') { c = '\\'; }
#else
        if (c == '\\') { c = '/'; }
#endif
    }
    SetEnv("PXR_PLUGINPATH_NAME", path.c_str());
}

} // extern "C"


int main(int argc, char *argv[])
{
    return 0;
}
