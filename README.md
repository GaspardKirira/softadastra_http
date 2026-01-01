# Softadastra HTTP (Legacy)

⚠️ **Legacy project — no longer actively developed**

**Softadastra HTTP** is an early experimental HTTP server written in **C++**, created before the conception of **Vix.cpp**.  
This repository is preserved **for historical and learning purposes only**.

Since **2024**, the project has been **officially superseded by Vix.cpp** — a modern, high-performance, offline-first C++ runtime and backend framework.

---

## Project Status

- 🟡 **Status**: Legacy / Archived
- 🧪 **Purpose**: Early experimentation with C++ HTTP servers
- ❌ **Not recommended for production**
- ✅ **Kept for reference and historical context**

If you are looking for an actively maintained, production-ready C++ backend framework, use **Vix.cpp** instead.

👉 **Vix.cpp (official successor)**  
🔗 https://vixcpp.com  
🔗 https://github.com/vixcpp

---

## Historical Context

Softadastra HTTP was part of the early Softadastra experiments exploring:

- Low-level HTTP servers in C++
- Manual threading and request handling
- Integration with:
  - Boost
  - OpenSSL
  - MySQL Connector/C++
  - spdlog

These experiments directly influenced the architectural decisions that later shaped **Vix.cpp**, including:

- Clear module separation
- Strong focus on performance
- Developer-friendly tooling
- Modern C++ design (C++20+)
- Runtime-oriented approach instead of ad-hoc servers

---

## Technical Overview (Legacy)

- **Language**: C++
- **Build System**: CMake
- **Dependencies**:
  - Boost
  - OpenSSL
  - MySQL Connector/C++
  - spdlog

The project implements a basic HTTP server with custom threading and configuration logic.

⚠️ The codebase does **not** reflect current best practices used in Vix.cpp.

---

## Build (Not Recommended)

If you still want to build it for educational purposes:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

Executable name and behavior may vary depending on your system.

---

## Why This Repository Exists

This repository remains public to:

- Preserve project history
- Document the evolution toward Vix.cpp
- Provide learning material for early C++ backend experiments

It is **not maintained** and **will not receive updates**.

---

## Official Successor: Vix.cpp

**Vix.cpp** is the continuation and evolution of everything learned in this repository.

### Key differences:

- Modern C++20 / C++23
- High-performance runtime architecture
- Offline-first & P2P foundations
- Integrated CLI tooling (`vix`)
- Modular ecosystem
- Production-grade design

👉 https://vixcpp.com  
👉 https://github.com/vixcpp

---

## Author

**Gaspard Kirira**  
Founder & Creator — Softadastra, Vix.cpp  
📧 gaspardkirira@outlook.com
