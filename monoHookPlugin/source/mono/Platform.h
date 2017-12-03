#pragma once

#if defined(_M_X64) || defined(__x86_64__)
    #define __Arch_x86_64__
#elif defined(_M_IX86) || defined(__x86__)
    #define __Arch_x86__
#endif


#if defined(_WIN32) && (_M_X64)
    #define __Windows_x86_64__
#elif defined(__linux__ ) && defined(__x86_64__)
    #define __Linux_x86_64__
#elif defined(__APPLE__) && defined(__x86_64__)
    #define __Mac_x86_64__
#elif defined(__APPLE__) && defined(__arm64__)
    #define __iOS_ARM64__
#elif defined(__ANDROID__) && defined(__arm64__)
    #define __Android_ARM64__
// todo: PS4
#endif

#if defined(_WIN32)
    #define LIBRARY_PATH "PATH"
#elif defined(__APPLE__)
    #define LIBRARY_PATH "DYLD_LIBRARY_PATH"
#else
    #define LIBRARY_PATH "LD_LIBRARY_PATH"
#endif
