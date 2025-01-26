
# Softadastra HTTP Server

Welcome to **Softadastra HTTP**, a HTTP server built using C++, Boost, MySQL Connector/C++, spdlog, and OpenSSL. This document will guide you through cloning, setting up, building, and running the project.

## Prerequisites

Before you begin, ensure the following dependencies are installed on your system:

1. **CMake** (version ≥ 3.10)
2. **C++ Compiler** supporting C++17 (GCC, Clang, etc.)
3. **Boost** (version ≥ 1.78)
4. **MySQL Connector/C++**
5. **spdlog** (logging library)
6. **OpenSSL** (security and encryption library)

### Installing Dependencies (Ubuntu)
```bash
sudo apt update && sudo apt install -y cmake g++ libboost-all-dev libssl-dev libmysqlcppconn-dev

# For spdlog, you can use vcpkg or install it manually:
git clone https://github.com/gabime/spdlog.git
cd spdlog && mkdir build && cd build
cmake .. && make -j$(nproc)
sudo make install
```

## Installation

1. **Clone the GitHub Repository:**
```bash
git clone https://github.com/GaspardKirira/softadastra_http.git
cd softadastra_http
```

2. **Create a Build Directory and Generate Build Files:**
```bash
mkdir build && cd build
cmake ..
```

3. **Build the Project:**
```bash
make -j$(nproc)
```

4. **Run the Generated Executable:**
```bash
./prog
```

## Configuration

Configuration files are located in the `src/config` directory. Ensure you update relevant files (such as `Config.hpp` or `Config.cpp`) to suit your requirements, including database connection settings or SSL certificates.

## Project Structure

Here is an overview of the project organization:
- **src/**: Contains the source files for the HTTP server.
  - **config/**: Includes configuration files like `Config.hpp`.
- **CMakeLists.txt**: CMake configuration file for building the project.

## Common Issues

### Boost Not Found
If Boost is not found, verify that it is correctly installed and meets the minimum version requirements.

### MySQL Connector/C++ Not Found
Ensure `libmysqlcppconn-dev` is installed, and paths are correctly configured in the `CMakeLists.txt` file.

### spdlog Not Found
Install `spdlog` using vcpkg or compile it manually following the installation section above.

### OpenSSL Not Found
Install `libssl-dev` or configure the include paths in `CMakeLists.txt` if OpenSSL is installed in a non-standard location.

## Contributions

Contributions are welcome! Create a pull request or open an issue to report a bug or suggest a feature.

## License

This project is licensed for Softadastra Company. See the LICENSE file for more details.

---

**Author:** Gaspard Kirira  
**Contact:** gaspardkirira@outlook.com
