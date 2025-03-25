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
namespace Utils{
//加密模块中，前端保存和传递数据的结构体
std::vector<uint8_t> encryptedDataToVector(const Utils::EncryptedData& encryptedData) {
    std::vector<uint8_t> result;
    // 添加 salt
    result.insert(result.end(), encryptedData.salt.begin(), encryptedData.salt.end());
    // 添加 iv
    result.insert(result.end(), encryptedData.iv.begin(), encryptedData.iv.end());
    // 添加 tag
    result.insert(result.end(), encryptedData.tag.begin(), encryptedData.tag.end());
    // 添加 ciphertext
    result.insert(result.end(), encryptedData.ciphertext.begin(), encryptedData.ciphertext.end());
    return result;
}

}
