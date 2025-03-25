//#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <QImage>
#include <bitset>
#include <cstring>
#include <fstream>

struct Color {
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

struct Point {
    int X;
    int Y;
};

class Linear_Image {
public:
    static bool isPNG(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            return false;
        }

        std::vector<unsigned char> buffer(8);
        file.read(reinterpret_cast<char*>(buffer.data()), 8);
        file.close();

        const unsigned char pngHeader[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        return buffer.size() == 8 && std::equal(std::begin(buffer), std::end(buffer), std::begin(pngHeader));
    }

    static int DecodePixel(Color pixel) {
        int red = pixel.R & 3;
        int green = pixel.G & 7;
        int blue = pixel.B & 7;
        int value = blue | (green << 3) | (red << 6);
        return value;
    }

    static Point LinearIndexToPoint(int index, int width, int height) {
        (void) height;
        if (index < 0) {
            index *= -1;
        }
        return {index % width, static_cast<int>(std::floor(static_cast<double>(index) / width))};
    }

    static Color EncodePixel(Color pixel, int value) {
        int blueValue = value & 7;
        int greenValue = (value >> 3) & 7;
        int redValue = (value >> 6) & 3;
        int red = (pixel.R & 0xFC) | redValue;
        int green = (pixel.G & 0xF8) | greenValue;
        int blue = (pixel.B & 0xF8) | blueValue;
        return {static_cast<uint8_t>(red), static_cast<uint8_t>(green), static_cast<uint8_t>(blue)};
    }

    static std::vector<uint8_t> Decode(QImage img) {
        // 确保图像是以正确的格式
        if (img.format() != QImage::Format_ARGB32) {
            img = img.convertToFormat(QImage::Format_ARGB32);
        }

        int bytes = img.sizeInBytes();
        std::vector<uint8_t> rgbValues(img.bits(), img.bits() + bytes);

        int maxLinear = img.width() * img.height();
        int c = 0;
        std::vector<char> binaryCharArray(32);
        int count = 0;
        while (c < maxLinear && count < 32) {
            Point point = LinearIndexToPoint(c, img.width(), img.height());
            int decodedValue = DecodePixelFromArray(rgbValues, point, img.width(), img.bytesPerLine());
            binaryCharArray[count] = static_cast<char>(decodedValue);
            count++;
            c++;
        }
        std::string binaryString(binaryCharArray.begin(), binaryCharArray.end());
        int fileLength = std::stoi(binaryString, nullptr, 2);

        std::vector<uint8_t> fileData;
        fileData.reserve(fileLength);
        for (int i = 0; i < fileLength; i++) {
            Point point = LinearIndexToPoint(c, img.width(), img.height());
            fileData.push_back(static_cast<uint8_t>(DecodePixelFromArray(rgbValues, point, img.width(), img.bytesPerLine())));
            c++;
        }

        return fileData;
    }

    static int DecodePixelFromArray(const std::vector<uint8_t>& rgbValues, Point point, int width, int stride) {
        (void) width;
        int index = point.Y * stride + point.X * 3; // 每个像素4字节
        int blue = rgbValues[index];
        int green = rgbValues[index + 1];
        int red = rgbValues[index + 2];
        int redValue = red & 3;
        int greenValue = green & 7;
        int blueValue = blue & 7;
        int value = blueValue | (greenValue << 3) | (redValue << 6);
        return value;
    }

    static void Encode(QImage* img, const std::vector<uint8_t>& data) {
        // 确保图像是以正确的格式
        if (img->format() != QImage::Format_ARGB32) {
            *img = img->convertToFormat(QImage::Format_ARGB32);
        }

        int bytes = img->sizeInBytes();
        std::vector<uint8_t> rgbValues;
        rgbValues.reserve(bytes);

        // 获取图像数据的首地址
        uint8_t* ptr = img->bits();
        int stride = img->bytesPerLine();

        // 将像素数据复制到数组
        std::memcpy(rgbValues.data(), ptr, bytes);
        int c = 0;
        //int maxLinear = img->width() * img->height();
        size_t maxLinear = static_cast<size_t>(img->width()) * static_cast<size_t>(img->height());

        if (data.size() < maxLinear) {
            std::string binaryString = std::bitset<32>(data.size()).to_string(); // 转换为二制字符串，左侧填充零以达到 32 位
            std::vector<char> binaryCharArray(binaryString.begin(), binaryString.end()); // 将二进制字符串转换为字符组
            for (int i = 0; i < static_cast<int>(binaryCharArray.size()); i++) {
                Point point = LinearIndexToPoint(c, img->width(), img->height());
                char letter = binaryCharArray[i];
                int value = static_cast<int>(letter);
                EncodePixelToArray(rgbValues, point, value, img->width(), stride);
                c++;
            }
            // Write data
            for (int i = 0; i < static_cast<int>(data.size()); i++) {
                Point point = LinearIndexToPoint(c, img->width(), img->height());
                EncodePixelToArray(rgbValues, point, data[i], img->width(), stride);
                c++;
            }
        }
        std::memcpy(ptr, rgbValues.data(), bytes);
    }

    static void EncodePixelToArray(std::vector<uint8_t>& rgbValues, Point point, int value, int width, int stride) {
        (void) width;
        int index = point.Y * stride + point.X * 3; // 每个像素4字节
        int blueValue = value & 7;
        int greenValue = (value >> 3) & 7;
        int redValue = (value >> 6) & 3;
        rgbValues[index] = (rgbValues[index] & 0xF8) | blueValue;
        rgbValues[index + 1] = (rgbValues[index + 1] & 0xF8) | greenValue;
        rgbValues[index + 2] = (rgbValues[index + 2] & 0xFC) | redValue;
    }

    static double CheckSize(const QImage& img) {
        return std::round(img.width() * img.height() / 1024 * 0.97);
    }
};


