# HideSlothQt

**HideSlothQt** is a cross-platform steganography and lightweight encryption tool, rebuilt from the legacy project [HideSloth](https://github.com/pingwcy/HideSloth), originally written in C# by the same author (no longer actively maintained). This new version is built using Qt and supports modern image formats and steganographic algorithms.

## Project Overview

- **Framework**: Built on Qt 5.15 with compatibility for Qt 6 (though Qt 6 is not fully tested).
- **Cross-Platform Support**: Designed to be fully hardware-independent and portable. Currently tested on:
  - Windows x64
  - Linux x64 / arm64
- **Current Status**: Alpha stage — potentially unstable and not recommended for production use until version 1.0 is released.

## Key Features

- 🖼️ **Image Format Support**:
  - JPEG (using DCT-based steganography)
  - PNG / BMP (using LSB steganography)
- 📁 **Batch Embedding and Splitting**:
  - Large files can be split and embedded across multiple images
  - Automatically detects embeddable images and distributes data accordingly
- 🔐 **Basic Encryption Functionality**:
  - Optional symmetric encryption to secure embedded data
  - Can function as a lightweight file encryption utility
- 🧪 **Flexible Architecture**:
  - Modular design allows for easy extension and integration of new steganographic methods

## Build Requirements

### Windows

- **Compiler**: MSVC 2019 or later
- **Dependencies**:
  - [OpenSSL 3.4+](https://www.openssl.org/) (tested with 3.5)
  - Qt 5.15.x or Qt 6.x

### Linux

- **Compiler**: Any C++17-compatible compiler (e.g., g++ 9+, clang 10+)
- **Dependencies**:
  - OpenSSL 3.2 or later
  - Qt 5.15 or Qt 6

### Build Instructions

```bash
git clone https://github.com/pingwcy/HideSlothQt.git
cd HideSlothQt
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Notes

JPEG embedding uses DCT domain, offering minimal visible changes but smaller capacity.

PNG and BMP use LSB (Least Significant Bit) embedding, offering larger capacity with tiny visual distortion.

Currently, video/audio format embedding is not supported.

### Development Status

Current version: 0.4-alpha

Planned feature freeze: 1.0-beta

Actively under development — contributions via Issues and Pull Requests are welcome.

