#include "utils_a.h"
#include <string>

utils_a::utils_a() {}

#ifdef _WIN32  // 仅在 Windows 平台上使用 WideCharToMultiByte
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Utils {
std::string wstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
}
#else  // 非 Windows 平台提供一个空实现或错误提示
namespace Utils {
std::string wstringToUtf8(const std::wstring& wstr) {
    static_assert(false, "wstringToUtf8 is not implemented for non-Windows platforms.");
    return "";  // 这个 return 只是为了避免编译错误
}
}
#endif
