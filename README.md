1. Návod pro macOS (pro tvůj README.md)
Pro zbuildění projektu na macOS je nutné mít nainstalované vývojové nástroje Xcode Command Line Tools a knihovnu OpenSSL (dostupnou přes Homebrew).
Otevři terminál ve složce projektu a spusť následující příkaz:
Bash
g++ -std=c++17 main.cpp -o main -I$(brew --prefix openssl)/include -L$(brew --prefix openssl)/lib -lcrypto
Spuštění zkompilovaného programu:
Bash
./main
2. Návod pro Windows (pro tvůj README.md)
Na Windows záleží, jaké vývojové prostředí používáš. Nejčastější a nejspolehlivější cestou je použití MinGW-w64 (GCC pro Windows).
Varianta A: Pomocí MinGW / příkazového řádku (CMD / PowerShell)
Ujisti se, že máš nainstalovaný MinGW (s podporou C++17) a knihovnu OpenSSL (např. přes balíčkový systém vuntu/MSYS2 nebo staženou předkompilovanou verzi).
Otevři příkazový řádek ve složce s projektem.
Spusť překlad (pokud máš OpenSSL ve standardních cestách, nebo uprav cesty podle své instalace):
DOS
g++ -std=c++17 main.cpp -o main.exe -lssl -lcrypto
Spuštění programu:
DOS
main.exe
Varianta B: Pomocí Visual Studio (MSVC)
Otevři Developer Command Prompt for VS (nebo vytvoř projekt přímo ve Visual Studiu).
Přelož kód příkazem:
DOS
cl /std:c++17 main.cpp /link libcrypto.lib libssl.lib
