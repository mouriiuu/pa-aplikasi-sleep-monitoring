#ifndef USER_H
#define USER_H

#include <string>

const int MAX_USERS = 100;
const int MAX_SLEEP_RECORDS = 500;

struct User {
    std::string nama;
    std::string username;
    std::string password;
};

struct SleepRecord {
    std::string usernamePasien;
    std::string tanggal;
    std::string jamMulaiSesiMalam;
    std::string catatanMalam;
    std::string jamMulaiTidurFinal;
    std::string jamBangun;
    int jumlahTerbangun;
    int totalTerjagaMenit;
    int kualitasTidur;
    std::string kondisiBangun;
    bool sudahInputPagi;
};

struct AppData {
    // Array statis utama aplikasi.
    User users[MAX_USERS];
    int userCount;
    SleepRecord sleepRecords[MAX_SLEEP_RECORDS];
    int sleepRecordCount;
};

#endif
