## 📋 Požadavky

Před spuštěním projektu se ujisti, že máš v systému nainstalovaný **C++ překladač** (podporující standard C++17 nebo novější) a kryptografickou knihovnu **OpenSSL**.

---

## 🚀 Návod pro macOS

1. **Instalace závislostí** (pokud je ještě nemáš):
   ```bash
   xcode-select --install
   brew install openssl
Kompilace (Build) projektu:
Otevři terminál ve složce s projektem a spusť tento příkaz:
Bash
g++ -std=c++17 main.cpp -o main -I$(brew --prefix openssl)/include -L$(brew --prefix openssl)/lib -lcrypto
Spuštění programu:
Bash
./main

🪟 Návod pro Windows
Varianta A: Pomocí MinGW (GCC)
Ujisti se, že máš nainstalované MinGW-w64 a OpenSSL.
Otevři příkazový řádek (CMD nebo PowerShell) ve složce s projektem a zadej:
DOS
g++ -std=c++17 main.cpp -o main.exe -lssl -lcrypto
Spuštění programu:
DOS
main.exe
Varianta B: Pomocí sady Visual Studio (MSVC)
Otevři Developer Command Prompt for VS.
Spusť překlad příkazem:
DOS
cl /std:c++17 main.cpp /link libcrypto.lib libssl.lib
Spuštění programu:
DOS
main.exe
