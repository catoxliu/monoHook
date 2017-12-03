#pragma once

#ifdef _MSC_VER
    #define and     &&
    #define and_eq  &=
    #define bitand  &
    #define bitor   |
    #define compl   ~
    #define not     !
    #define not_eq  !=
    #define or      ||
    #define or_eq   |=
    #define xor     ^
    #define xor_eq  ^=
#endif
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#define BOOST_PYTHON_STATIC_LIB
	#define NOMINMAX

	#define BUILD_COMPONENT_SRC_PREFIX "pxr/"
	#define BUILD_OPTLEVEL_OPT
	#define TF_NO_GNU_EXT

	#define USD_ENABLE_CACHED_NEW

	#include <windows.h>
#endif

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <atomic>
#include <cstdarg>
#include <cmath>
#include <cctype>
#include <type_traits>

#pragma warning(pop)

#include "mono/Platform.h"

#pragma warning(disable:4201)
