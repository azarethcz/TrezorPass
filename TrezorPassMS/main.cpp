#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstddef>
#include <algorithm>
#include <random>

// OpenSSL knihovny
#include <openssl/evp.h>
#include <openssl/rand.h>

#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================================
// 1. TERMINÁLOVÉ ROZHRANÍ (UI)
// ============================================================================
namespace TerminalUI {
    const std::string RESET        = "\033[0m";
    const std::string BOLD         = "\033[1m";
    const std::string DIM          = "\033[2m";

    const std::string CYAN         = "\033[36m";
    const std::string BRIGHT_CYAN  = "\033[96m";
    const std::string GREEN        = "\033[32m";
    const std::string BRIGHT_GREEN = "\033[92m";
    const std::string YELLOW       = "\033[33m";
    const std::string RED          = "\033[31m";
    const std::string WHITE        = "\033[97m";
    const std::string GRAY         = "\033[90m";

    const int SIRKA_OBSAHU = 68; 

    void vycisti() {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
#endif
        std::cout << "\033[H";
    }

    void smažVse() {
        std::cout << "\033[2J\033[1;1H";
    }

    std::string opakujZnak(const std::string& str, int pocet) {
        if (pocet <= 0) return "";
        std::string res = "";
        for (int i = 0; i < pocet; ++i) res += str;
        return res;
    }

    size_t vizualniDelka(const std::string& text) {
        size_t visibleChars = 0;
        bool vSekvenci = false;

        for (size_t i = 0; i < text.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(text[i]);

            if (c == '\033') { 
                vSekvenci = true; 
                continue; 
            }
            if (vSekvenci) {
                if (c == 'm') vSekvenci = false;
                continue;
            }

            if ((c & 0xC0) != 0x80) {
                visibleChars++;
            }
        }
        return visibleChars;
    }

    void vytiskniRadekRamecku(const std::string& obsah) {
        size_t visLen = vizualniDelka(obsah);
        int mezery = SIRKA_OBSAHU - static_cast<int>(visLen);
        if (mezery < 0) mezery = 0;

        std::cout << BRIGHT_CYAN << "│ " << RESET << obsah 
                  << opakujZnak(" ", mezery) << BRIGHT_CYAN << " │\033[K\n" << RESET;
    }

    void nakresliHlavicku() {
        vycisti();
        std::cout << BRIGHT_CYAN << "┌" << opakujZnak("─", SIRKA_OBSAHU + 2) << "┐\033[K\n" << RESET;
        
        std::string radek1 = BOLD + WHITE + "            BEZPEČNÝ SPRÁVCE HESEL (TREZOR v3.4)" + RESET;
        std::string radek2 = GRAY + "               Autor aplikace: " + BRIGHT_GREEN + "Martin Strnad" + RESET;

        vytiskniRadekRamecku(radek1);
        vytiskniRadekRamecku(radek2);

        std::cout << BRIGHT_CYAN << "└" << opakujZnak("─", SIRKA_OBSAHU + 2) << "┘\033[K\n" << RESET;
    }

    void nakresliHorniRamecek(const std::string& titulText) {
        size_t delkaTitulu = vizualniDelka(titulText);
        int zbyvajiciCarky = (SIRKA_OBSAHU + 4) - 9 - static_cast<int>(delkaTitulu);
        if (zbyvajiciCarky < 0) zbyvajiciCarky = 0;

        std::cout << CYAN << "┌─ [ " << BOLD << WHITE << titulText << RESET << CYAN << " ] " 
                  << opakujZnak("─", zbyvajiciCarky) << "┐\033[K\n" << RESET;
    }

    void nakresliPatuRamecku() {
        std::cout << CYAN << "└" << opakujZnak("─", SIRKA_OBSAHU + 2) << "┘\033[K\n" << RESET;
    }
}

// ============================================================================
// 2. STRUKTURY A KRYPTOGRAFIE
// ============================================================================
struct PolozkaHesla {
    int id;
    std::string sluzba;
    std::string uzivatel;
    std::string heslo;
};

namespace Crypto {
    std::string vygenerujHeslo(int delka = 16) {
        const std::string znaky = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distrib(0, znaky.size() - 1);

        std::string heslo = "";
        for (int i = 0; i < delka; ++i) {
            heslo += znaky[distrib(generator)];
        }
        return heslo;
    }

    std::string sha256(const std::string& input) {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int lengthOfHash = 0;

        EVP_MD_CTX* context = EVP_MD_CTX_new();
        if (context != nullptr) {
            if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) == 1) {
                EVP_DigestUpdate(context, input.c_str(), input.size());
                EVP_DigestFinal_ex(context, hash, &lengthOfHash);
            }
            EVP_MD_CTX_free(context);
        }

        std::stringstream ss;
        for (unsigned int i = 0; i < lengthOfHash; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    // AES-256 šifrování dat pro uložení do passwords.enc
    std::vector<unsigned char> encryptData(const std::string& plainText, const std::string& password) {
        unsigned char salt[8];
        RAND_bytes(salt, sizeof(salt));

        unsigned char key[32], iv[32];
        EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt, 
                       reinterpret_cast<const unsigned char*>(password.c_str()), 
                       password.size(), 1, key, iv);

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        std::vector<unsigned char> cipherText(plainText.size() + EVP_MAX_BLOCK_LENGTH);
        int len = 0, encryptedLen = 0;

        EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);
        EVP_EncryptUpdate(ctx, cipherText.data(), &len, 
                          reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.size());
        encryptedLen = len;
        EVP_EncryptFinal_ex(ctx, cipherText.data() + len, &len);
        encryptedLen += len;
        cipherText.resize(encryptedLen);
        EVP_CIPHER_CTX_free(ctx);

        // Vytvoříme výstupní paket obsahující hlavičku Salt + zašifrovaná data
        std::vector<unsigned char> output;
        output.insert(output.end(), "Salted__", "Salted__" + 8);
        output.insert(output.end(), salt, salt + 8);
        output.insert
