#ifndef MENU_COMMON_H
#define MENU_COMMON_H

#include <string>

#include "user.h"

std::string inputBarisMenu(const std::string& pesan);
std::string ambilJamSekarang();
std::string ambilTanggalSekarang();
bool berisiKarakterTerlarang(const std::string& nilai);
void tampilkanDaftarPasienSingkat(const AppData& data);

#endif
