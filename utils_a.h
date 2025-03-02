#ifndef UTILS_A_H
#define UTILS_A_H

#include <string>  // 头文件中使用 std::string 需要包含此头文件
//#include "Encryption.cpp"
#include <vector>
class utils_a {
public:
    utils_a();
};

namespace Utils {
std::string wstringToUtf8(const std::wstring& wstr);
struct EncryptedData {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> iv;
    std::vector<unsigned char> tag;
    std::vector<unsigned char> ciphertext;
};

std::vector<uint8_t> encryptedDataToVector(const EncryptedData&);
}

#endif // UTILS_A_H
