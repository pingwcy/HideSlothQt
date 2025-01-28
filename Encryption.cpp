#include <openssl/evp.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <QDebug>
#include <openssl/rand.h> // 引入用于随机数生成的头文件
#include "GlobalSettings.h"
#include <limits>

class Encryption{
public:
struct EncryptedData {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> iv;
    std::vector<unsigned char> tag;
    std::vector<unsigned char> ciphertext;
};

static const EVP_MD* getEvpFunction(const std::string& hashType) {
    if (hashType == "SHA256") {
        return EVP_sha256();
    } else if (hashType == "SHA384") {
        return EVP_sha384();
    } else if (hashType == "SHA512") {
        return EVP_sha512();
    } else {
        return nullptr; // 未知的散列类型
    }
}

static bool aes_gcm_encrypt(const unsigned char *plaintext, int plaintext_len,
                            const unsigned char *aad, int aad_len,
                            const unsigned char *key, const unsigned char *iv,
                            unsigned char *ciphertext, unsigned char *tag) {

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    int len;
    //int ciphertext_len;
    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        return false;
    if (!EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv))
        return false;
    if (!EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
        return false;
    if (!EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        return false;
    //ciphertext_len = len;
    if (!EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        return false;
    //ciphertext_len += len;
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))
        return false;
    EVP_CIPHER_CTX_free(ctx);
    return true;
}

static bool chacha20_poly1305_encrypt(const unsigned char *plaintext, int plaintext_len,
                                      const unsigned char *aad, int aad_len,
                                      const unsigned char *key, const unsigned char *iv,
                                      unsigned char *ciphertext, unsigned char *tag) {

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    int len;
    //int ciphertext_len;

    // 初始化加密操作
    if (!EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, NULL, NULL))
        return false;
    if (!EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv))
        return false;

    // 提供 AAD 数据
    if (aad && aad_len > 0) {
        if (!EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
            return false;
    }

    // 提供明文数据并加密
    if (!EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        return false;
    //ciphertext_len = len;

    // 完成加密操作
    if (!EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        return false;
    //ciphertext_len += len;

    // 获取TAG值
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, 16, tag))
        return false;

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

static bool chacha20_poly1305_decrypt(const unsigned char *ciphertext, int ciphertext_len,
                                      const unsigned char *aad, int aad_len,
                                      const unsigned char *tag, const unsigned char *key, const unsigned char *iv,
                                      unsigned char *plaintext) {

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    int len;
    //int plaintext_len;

    // 初始化解密操作
    if (!EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, NULL, NULL))
        return false;
    if (!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv))
        return false;

    // 提供 AAD 数据
    if (aad && aad_len > 0) {
        if (!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
            return false;
    }

    // 提供密文数据并解密
    if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        return false;
    //plaintext_len = len;

    // 设置期望的TAG值
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, 16, (void *)tag))
        return false;

    // 完成解密操作
    if (!EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    //plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

static bool aes_gcm_decrypt(const unsigned char *ciphertext, int ciphertext_len,
                            const unsigned char *aad, int aad_len,
                            const unsigned char *tag, const unsigned char *key, const unsigned char *iv,
                            unsigned char *plaintext) {

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    int len;
    //int plaintext_len;
    // 初始化解密操作
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        return false;
    if (!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv))
        return false;
    // 提供 AAD 数据
    if (!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
        return false;
    // 提供密文数据
    if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        return false;
    //plaintext_len = len;
    // 设置期望的 TAG 值
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void *)tag))
        return false;
    // 完成解密
    if (!EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    //plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return true;
}

static std::vector<unsigned char> dec(const unsigned char *ciphertext, const char *password, int ciphertexyLength){
    if (ciphertexyLength < 44) {
        // 处理错误情况
    }

    EncryptedData result;
    result.salt = std::vector<unsigned char>(ciphertext, ciphertext + 16);
    // 提取 iv (接下来的 12 字节)
    result.iv = std::vector<unsigned char>(ciphertext + 16, ciphertext + 28);
    // 提取 tag (接下来的 16 字节)
    result.tag = std::vector<unsigned char>(ciphertext + 28, ciphertext + 44);
    // 提取剩余的密文
    result.ciphertext = std::vector<unsigned char>(ciphertext + 44, ciphertext + ciphertexyLength);
    const int keyLength = 32;
    std::vector<unsigned char> key(keyLength);
    const EVP_MD* md;
    int iterc;
    if (GlobalSettings::instance().getCstHash()){
        md = getEvpFunction(GlobalSettings::instance().getHash());
    }
    else{
        md = getEvpFunction(GlobalSettings::instance().getDefhash());
    }
    if (GlobalSettings::instance().getCstIter()){
        iterc = GlobalSettings::instance().getIter();
    }
    else{
        iterc = GlobalSettings::instance().getDefIter();
    }
    size_t saltSize = result.salt.size();
    if (saltSize > static_cast<size_t>(std::numeric_limits<int>::max())) {
        qDebug() << "Salt size is too large to fit into an int.";
    }
    if (PKCS5_PBKDF2_HMAC(password, -1, result.salt.data(), static_cast<int>(saltSize), iterc, md, keyLength, key.data()) != 1) {
        qDebug() << "Failure in PBKDF2";
    }
    if (GlobalSettings::instance().getEncalg() == "AES256-GCM"){
        if (aes_gcm_decrypt(result.ciphertext.data(), ciphertexyLength-44, nullptr, 0,result.tag.data(), key.data(), result.iv.data(), result.ciphertext.data())) {
            std::cout << "Decryption succeeded." << std::endl;
        } else {
            std::cout << "Decryption failed." << std::endl;
        }
    }
    if (GlobalSettings::instance().getEncalg() == "ChaCha20-Poly1305"){
        if (chacha20_poly1305_decrypt(result.ciphertext.data(), ciphertexyLength-44, nullptr, 0,result.tag.data(), key.data(), result.iv.data(), result.ciphertext.data())) {
            std::cout << "Decryption succeeded." << std::endl;
        } else {
            std::cout << "Decryption failed." << std::endl;
        }
    }
    std::vector<uint8_t> vectorData(result.ciphertext.data(), result.ciphertext.data() + result.ciphertext.size());
    return vectorData;
}

static EncryptedData enc(const unsigned char *plaintext, const char *password, int plaintextLength) {
    EncryptedData result;
    // 初始化salt, iv, tag和ciphertext
    result.salt.resize(16);
    result.iv.resize(12);  // 使用12字节长度的IV
    result.tag.resize(16);
    result.ciphertext.resize(plaintextLength); // 加密后的数据大小等于明文长度加上16字节的标签
    // 生成salt和iv
    if (!RAND_bytes(result.salt.data(), static_cast<int>(result.salt.size())) ||
        !RAND_bytes(result.iv.data(), static_cast<int>(result.iv.size()))) {
        qDebug() << "Failure in getting RAND_bytes";
    }
    // 密钥派生
    const int keyLength = 32;
    std::vector<unsigned char> key(keyLength);
    const EVP_MD* md;
    int iterc;
    if (GlobalSettings::instance().getCstHash()){
        md = getEvpFunction(GlobalSettings::instance().getHash());
        }
    else{
        md = getEvpFunction(GlobalSettings::instance().getDefhash());
        }
    if (GlobalSettings::instance().getCstIter()){
        iterc = GlobalSettings::instance().getIter();
        }
    else{
        iterc = GlobalSettings::instance().getDefIter();
        }
        size_t saltSize = result.salt.size();
        if (saltSize > static_cast<size_t>(std::numeric_limits<int>::max())) {
            qDebug() << "Salt size is too large to fit into an int.";
        }

        if (PKCS5_PBKDF2_HMAC(password, -1, result.salt.data(), static_cast<int>(saltSize), iterc, md, keyLength, key.data()) != 1) {
            qDebug() << "Failure in PBKDF2";
        }
    // 加密操作
    if (GlobalSettings::instance().getEncalg() == "AES256-GCM"){
        if (aes_gcm_encrypt(plaintext, plaintextLength, nullptr, 0, key.data(), result.iv.data(), result.ciphertext.data(), result.tag.data())) {
            std::cout << "Encryption succeeded." << std::endl;
        } else {
            std::cout << "Encryption failed." << std::endl;
            result.ciphertext.resize(0); // 如果加密失败，清空密文
        }
    }
    if (GlobalSettings::instance().getEncalg() == "ChaCha20-Poly1305"){
        if (chacha20_poly1305_encrypt(plaintext, plaintextLength, nullptr, 0, key.data(), result.iv.data(), result.ciphertext.data(), result.tag.data())) {
            std::cout << "Encryption succeeded." << std::endl;
        } else {
            std::cout << "Encryption failed." << std::endl;
            result.ciphertext.resize(0); // 如果加密失败，清空密文
        }
    }
    return result;
}
};
