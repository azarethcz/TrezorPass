## 📋 Požadavky

Před spuštěním projektu se ujisti, že máš v systému nainstalovaný **C++ překladač** (podporující standard C++17 nebo novější) a kryptografickou knihovnu **OpenSSL**.

---

## 🚀 Návod pro macOS

### 1. Instalace závislostí
> Pokud ještě nemáš potřebné nástroje, otevři terminál a spusť tyto příkazy:
> ```bash
> xcode-select --install
> brew install openssl
> ```

### 2. Kompilace (Build) projektu
> Otevři terminál ve složce s projektem a spusť tento příkaz:
> ```bash
> g++ -std=c++17 main.cpp -o main -I$(brew --prefix openssl)/include -L$(brew --prefix openssl)/lib -lcrypto
> ```

### 3. Spuštění programu
> Program spustíš příkazem:
> ```bash
> ./main
> ```

---

## 🪟 Návod pro Windows

### Varianta A: Pomocí MinGW (GCC)
> 1. Ujisti se, že máš nainstalované **MinGW-w64** a **OpenSSL**.
> 2. Otevři příkazový řádek (CMD nebo PowerShell) ve složce s projektem a zadej:
>    ```cmd
>    g++ -std=c++17 main.cpp -o main.exe -lssl -lcrypto
>    ```
> 3. Spuštění programu:
>    ```cmd
>    main.exe
>    ```

### Varianta B: Pomocí sady Visual Studio (MSVC)
> 1. Otevři **Developer Command Prompt for VS**.
> 2. Spusť překlad příkazem:
>    ```cmd
>    cl /std:c++17 main.cpp /link libcrypto.lib libssl.lib
>    ```
> 3. Spuštění programu:
>    ```cmd
>    main.exe
>    ```
