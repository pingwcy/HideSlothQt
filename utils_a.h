#ifndef UTILS_A_H
#define UTILS_A_H

#include <string>  // 头文件中使用 std::string 需要包含此头文件

class utils_a {
public:
    utils_a();
};

namespace Utils {
std::string wstringToUtf8(const std::wstring& wstr);
}

#endif // UTILS_A_H
