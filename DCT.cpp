#include <stdio.h>
//#include <jpeglibsrc/jpeglib.h>
#include <qtlibjpeg/jpeglib.h>
#include <qfile.h>
#include <QTextStream>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QStringBuilder>
#include <iostream>
#include <vector>
#include <array>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <bitset>

class DCT{
public:
    void forceLink() {
        jpeg_create_decompress(nullptr);  // 强制链接 jpeg_create_compress 函数
    }

    static struct jpeg_decompress_struct read_dct_coefficients(const char* filename,
                                                                    std::vector<std::vector<std::vector<std::array<short, 64>>>>& dct_coefficients) {
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;

        FILE* infile = fopen(filename, "rb");
        if (!infile) {
            std::cerr << "Error opening file: " << std::strerror(errno) << std::endl;
            // 可根据需求做更详细的错误处理
        }

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, infile);

        jpeg_read_header(&cinfo, TRUE);

        jvirt_barray_ptr* coef_arrays = jpeg_read_coefficients(&cinfo);

        dct_coefficients.resize(cinfo.num_components);

        for (int comp = 0; comp < cinfo.num_components; comp++) {
            jpeg_component_info* comp_info = &cinfo.comp_info[comp];
            dct_coefficients[comp].resize(comp_info->height_in_blocks);

            for (int row = 0; row < comp_info->height_in_blocks; row++) {
                JBLOCKARRAY coef_blocks = (cinfo.mem->access_virt_barray)(
                    (j_common_ptr)&cinfo, coef_arrays[comp], row, 1, FALSE);

                std::vector<std::array<short, 64>> row_blocks;
                row_blocks.reserve(comp_info->width_in_blocks);

                for (int col = 0; col < comp_info->width_in_blocks; col++) {
                    std::array<short, 64> new_block;
                    std::memcpy(new_block.data(), coef_blocks[0][col], sizeof(new_block));
                    row_blocks.push_back(new_block);
                }

                dct_coefficients[comp][row] = row_blocks;
            }
        }

        fclose(infile);
        return cinfo; // 返回解压缩结构
    }

    static void process_dct_coefficients(std::vector<std::vector<std::vector<std::array<short, 64>>>>& dct_coefficients, const std::vector<uint8_t>& data) {
        // 此方法仅为接口，用户可在此实现具体逻辑
        // 例如修改特定的DCT系数
        // Step 1: Convert `data` into a binary stream of 0s and 1s
        std::vector<uint8_t> binary_stream;
        uint16_t size = static_cast<uint16_t>(data.size());
        for (int i = 15; i >= 0; --i) {
            // 使用位掩码提取当前位的值
            uint8_t bit = (size >> i) & 1;
            // 将提取的位值添加到二进制流中
            binary_stream.push_back(bit);
        }

        for (uint8_t byte : data) {
            for (int i = 7; i >= 0; --i) {
                binary_stream.push_back((byte >> i) & 1); // Extract each bit (0 or 1)
            }
        }

        // Step 2: Write the binary stream into `dct_coefficients`
        size_t binary_index = 0;
        size_t total_coefficients = 0; // Track total elements written

        for (auto& plane : dct_coefficients) {
            for (auto& row : plane) {
                for (auto& block : row) {
                    if (binary_index == binary_stream.size()) {
                        break;
                    }
                    block[63] = binary_stream[binary_index++]; // Write binary value to the last position in the block
                    ++total_coefficients;
                }
            }
        }

        // Ensure all binary data is written
        if (binary_index < binary_stream.size()) {
            std::cerr << "Warning: Not all binary data was used! Remaining bits: "
                      << (binary_stream.size() - binary_index) << std::endl;
        }

        std::cout << "Total coefficients written: " << total_coefficients << std::endl;

    }

    static void write_dct_coefficients(const char* output_filename,
                                            const std::vector<std::vector<std::vector<std::array<short, 64>>>>& dct_coefficients,
                                            struct jpeg_decompress_struct& srcinfo) {
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;

        FILE* outfile = fopen(output_filename, "wb");
        if (!outfile) {
            std::cerr << "Error opening file: " << std::strerror(errno) << std::endl;
            return;
        }

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, outfile);

        jpeg_copy_critical_parameters(&srcinfo, &cinfo);

        jvirt_barray_ptr* coef_arrays_from_srcinfo = jpeg_read_coefficients(&srcinfo);
        jpeg_write_coefficients(&cinfo, coef_arrays_from_srcinfo);

        for (int comp = 0; comp < cinfo.num_components; comp++) {
            jpeg_component_info* comp_info = &cinfo.comp_info[comp];
            for (int row = 0; row < comp_info->height_in_blocks; row++) {
                JBLOCKARRAY coef_blocks = (cinfo.mem->access_virt_barray)(
                    (j_common_ptr)&cinfo, coef_arrays_from_srcinfo[comp], row, 1, TRUE);
                for (int col = 0; col < comp_info->width_in_blocks; col++) {
                    const std::array<short, 64>& block = dct_coefficients[comp][row][col];
                    std::memcpy(coef_blocks[0][col], block.data(), sizeof(std::array<short, 64>));
                }
            }
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        fclose(outfile);
    }

    static void encode_image(const char* input_filename, const char* output_filename, const std::vector<uint8_t>& data) {
        std::vector<std::vector<std::vector<std::array<short, 64>>>> dct_coefficients;
        struct jpeg_decompress_struct srcinfo = read_dct_coefficients(input_filename, dct_coefficients);

        // 传递DCT系数到处理方法
        process_dct_coefficients(dct_coefficients, data);

        // 根据处理后的DCT系数重构图像
        write_dct_coefficients(output_filename, dct_coefficients, srcinfo);

        jpeg_destroy_decompress(&srcinfo);
    }

    static void decode_image(const char* input_filename, std::vector<uint8_t>& data){
        std::vector<std::vector<std::vector<std::array<short, 64>>>> dct_coefficients;
        read_dct_coefficients(input_filename, dct_coefficients);
        uint16_t length = 0;
        size_t count = 0;

        for (const auto& plane : dct_coefficients) {
            for (const auto& row : plane) {
                for (const auto& block : row) {
                    if (count < 16) {
                        length |= (block[63] & 1) << (15 - count); // Correctly extract bits for length
                        ++count;
                    } else {
                        goto done_length; // Exit the loop once length is read
                    }
                }
            }
        }

    done_length:
        //length+=2;
        // Step 2: Read the subsequent bits based on the extracted length
        //std::vector<uint8_t> data;
        uint8_t current_byte = 0;
        uint8_t bit_position = 0;
        int t = 0;
        for (const auto& plane : dct_coefficients) {
            for (const auto& row : plane) {
                for (const auto& block : row) {
                    if (t >= 16) { // Skip the first 16 elements
                        current_byte = (current_byte << 1) | (block[63] & 1); // Extract the bit
                        ++bit_position;

                        if (bit_position == 8) { // A full byte is formed
                            data.push_back(current_byte);
                            current_byte = 0;
                            bit_position = 0;
                            --length;
                        }

                        if (length == 0) {
                            goto done_data; // Exit once all bits are read
                        }
                    }
                    ++t;
                }
            }
        }

    done_data:
        // Handle the case where the last byte is incomplete
        if (bit_position > 0) {
            current_byte <<= (8 - bit_position); // Left-align remaining bits in the last byte
            data.push_back(current_byte);
        }

    }

};
