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
// 1. TERMINÁLOVÉ ROZHRANÍ (UI) - Hladké vykreslování bez problikávání
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
}

// ============================================================================
// 3. SOUBOROVÉ OPERACE PRO UKLÁDÁNÍ
// ============================================================================
std::vector<PolozkaHesla> nactiTrezor(const std::string& nazevSouboru) {
    std::vector<PolozkaHesla> trezor;
    std::ifstream soubor(nazevSouboru);
    
    if (!soubor.is_open()) {
        // Výchozí hodnoty při prvním spuštění
        return {
            {1, "Gmail", "martin.strnad@gmail.com", "mojeTajneHeslo123!"},
            {2, "GitHub", "mstrnad", "GitSecureP@ss2026"},
            {3, "Facebook", "martin.strnad", "123456"}
        };
    }

    PolozkaHesla p;
    while (soubor >> p.id >> p.sluzba >> p.uzivatel >> p.heslo) {
        trezor.push_back(p);
    }
    
    soubor.close();
    return trezor;
}

void ulozTrezor(const std::string& nazevSouboru, const std::vector<PolozkaHesla>& trezor) {
    std::ofstream soubor(nazevSouboru);
    if (soubor.is_open()) {
        for (const auto& p : trezor) {
            soubor << p.id << " " << p.sluzba << " " << p.uzivatel << " " << p.heslo << "\n";
        }
        soubor.close();
    }
}

// ============================================================================
// 4. HLAVNÍ LOGIKA
// ============================================================================
void zobrazMenu() {
    TerminalUI::nakresliHorniRamecek("HLAVNÍ MENU");
    TerminalUI::vytiskniRadekRamecku(TerminalUI::BRIGHT_CYAN + "[1] " + TerminalUI::WHITE + "Zobrazit přehled služeb " + TerminalUI::GRAY + "(bez zobrazení hesel)");
    TerminalUI::vytiskniRadekRamecku(TerminalUI::BRIGHT_CYAN + "[2] " + TerminalUI::WHITE + "Odtajnit konkrétní heslo " + TerminalUI::GRAY + "(podle ID)");
    TerminalUI::vytiskniRadekRamecku(TerminalUI::BRIGHT_CYAN + "[3] " + TerminalUI::WHITE + "Přidat nové heslo");
    TerminalUI::vytiskniRadekRamecku(TerminalUI::BRIGHT_CYAN + "[4] " + TerminalUI::RED   + "Odstranit heslo z trezoru " + TerminalUI::GRAY + "(podle ID)");
    TerminalUI::vytiskniRadekRamecku(TerminalUI::BRIGHT_CYAN + "[5] " + TerminalUI::WHITE + "Vygenerovat silné heslo");
    TerminalUI::vytiskniRadekRamecku(TerminalUI::BRIGHT_CYAN + "[6] " + TerminalUI::WHITE + "Spustit Bezpečnostní Audit " + TerminalUI::YELLOW + "(Chytrá analýza)");
    TerminalUI::vytiskniRadekRamecku(TerminalUI::RED         + "[7] " + TerminalUI::WHITE + "Zamknout trezor a ukončit");
    TerminalUI::nakresliPatuRamecku();
}

int main() {
    TerminalUI::smažVse();

    const std::string jmenoSouboru = "trezor_data.txt";
    std::vector<PolozkaHesla> trezor = nactiTrezor(jmenoSouboru);

    bool bezi = true;

    while (bezi) {
        TerminalUI::nakresliHlavicku();
        zobrazMenu();

        std::cout << "\n" << TerminalUI::BRIGHT_GREEN << "Volba ❯ " << TerminalUI::WHITE;
        int volba = 0;
        if (!(std::cin >> volba)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            TerminalUI::smažVse();
            continue;
        }

        std::cin.ignore(10000, '\n'); 

        switch (volba) {
            case 1: {
                TerminalUI::smažVse();
                TerminalUI::nakresliHorniRamecek("PŘEHLED ULOŽENÝCH SLUŽEB");
                if (trezor.empty()) {
                    TerminalUI::vytiskniRadekRamecku(TerminalUI::YELLOW + "Trezor je aktuálně prázdný." + TerminalUI::RESET);
                } else {
                    for (const auto& p : trezor) {
                        std::string radek = TerminalUI::BRIGHT_CYAN + "ID " + std::to_string(p.id) + ": " 
                                          + TerminalUI::WHITE + p.sluzba 
                                          + TerminalUI::GRAY + " (" + p.uzivatel + ")";
                        TerminalUI::vytiskniRadekRamecku(radek);
                    }
                }
                TerminalUI::nakresliPatuRamecku();
                break;
            }
            case 2: {
                TerminalUI::smažVse();
                std::cout << "Zadejte ID položky: ";
                int id;
                std::cin >> id;
                std::cin.ignore(10000, '\n');

                bool nalezeno = false;
                for (const auto& p : trezor) {
                    if (p.id == id) {
                        TerminalUI::nakresliHorniRamecek("DETAILY HESLA");
                        TerminalUI::vytiskniRadekRamecku("Služba:   " + TerminalUI::WHITE + p.sluzba);
                        TerminalUI::vytiskniRadekRamecku("Uživatel: " + TerminalUI::GRAY + p.uzivatel);
                        TerminalUI::vytiskniRadekRamecku("Heslo:    " + TerminalUI::BOLD + TerminalUI::YELLOW + p.heslo);
                        TerminalUI::nakresliPatuRamecku();
                        nalezeno = true;
                        break;
                    }
                }
                if (!nalezeno) {
                    std::cout << TerminalUI::RED << "Položka s tímto ID neexistuje!\n" << TerminalUI::RESET;
                }
                break;
            }
            case 3: {
                TerminalUI::smažVse();
                PolozkaHesla nova;
                nova.id = trezor.empty() ? 1 : trezor.back().id + 1;
                std::cout << "Název služby (bez mezer): ";
                std::cin >> nova.sluzba;
                std::cout << "Uživatelské jméno / e-mail: ";
                std::cin >> nova.uzivatel;
                std::cout << "Heslo: ";
                std::cin >> nova.heslo;
                std::cin.ignore(10000, '\n');

                trezor.push_back(nova);
                ulozTrezor(jmenoSouboru, trezor); // Uložení změn na disk
                std::cout << TerminalUI::BRIGHT_GREEN << "\n[✓] Položka byla úspěšně přidána a uložena!\n" << TerminalUI::RESET;
                break;
            }
            case 4: {
                TerminalUI::smažVse();
                std::cout << "Zadejte ID k odstranění: ";
                int id;
                std::cin >> id;
                std::cin.ignore(10000, '\n');

                auto it = std::remove_if(trezor.begin(), trezor.end(), [id](const PolozkaHesla& p) {
                    return p.id == id;
                });

                if (it != trezor.end()) {
                    trezor.erase(it, trezor.end());
                    ulozTrezor(jmenoSouboru, trezor); // Uložení změn na disk
                    std::cout << TerminalUI::BRIGHT_GREEN << "\n[✓] Položka s ID " << id << " byla úspěšně odstraněna.\n" << TerminalUI::RESET;
                } else {
                    std::cout << TerminalUI::RED << "\n[×] Položka s ID " << id << " nebyla nalezena.\n" << TerminalUI::RESET;
                }
                break;
            }
            case 5: {
                TerminalUI::smažVse();
                std::string noveHeslo = Crypto::vygenerujHeslo(18);
                TerminalUI::nakresliHorniRamecek("VYGENEROVANÉ SILNÉ HESLO");
                TerminalUI::vytiskniRadekRamecku("Heslo: " + TerminalUI::BOLD + TerminalUI::BRIGHT_GREEN + noveHeslo);
                TerminalUI::nakresliPatuRamecku();
                break;
            }
            case 6: {
                TerminalUI::smažVse();
                TerminalUI::nakresliHorniRamecek("BEZPEČNOSTNÍ AUDIT");
                if (trezor.empty()) {
                    TerminalUI::vytiskniRadekRamecku(TerminalUI::YELLOW + "Trezor je prázdný, nebyly nalezeny žádné položky k auditu." + TerminalUI::RESET);
                } else {
                    for (const auto& p : trezor) {
                        std::string stav = (p.heslo.length() < 8) 
                            ? (TerminalUI::RED + "SLABÉ (krátké)" + TerminalUI::RESET) 
                            : (TerminalUI::BRIGHT_GREEN + "BEZPEČNÉ" + TerminalUI::RESET);
                        TerminalUI::vytiskniRadekRamecku(p.sluzba + " ❯ " + stav);
                    }
                }
                TerminalUI::nakresliPatuRamecku();
                break;
            }
            case 7:
                bezi = false;
                TerminalUI::smažVse();
                std::cout << TerminalUI::CYAN << "\n[✓] Trezor byl bezpečně uzamčen. Na shledanou!\n\n" << TerminalUI::RESET;
                continue;
            default:
                TerminalUI::smažVse();
                std::cout << TerminalUI::RED << "Neplatná volba!\n" << TerminalUI::RESET;
                break;
        }

        std::cout << TerminalUI::GRAY << "\nStiskněte Enter pro pokračování..." << TerminalUI::RESET;
        std::cin.get();
        TerminalUI::smažVse();
    }

    return 0;
}
