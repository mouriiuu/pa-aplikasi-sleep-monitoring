#include "auth/auth.h"

#include <cstdlib>
#include <iostream>

#include "data_store.h"

using namespace std;

const string DOKTER_USERNAME = "dokter";
const string DOKTER_PASSWORD = "dokter123";

static string inputBaris(const string& pesan) {
    string nilai;
    cout << pesan;
    getline(cin, nilai);
    return nilai;
}

static bool lanjutBuatAkunSetelahError() {
    string jawaban = inputBaris("Ulangi input akun? (Y=ya, N/Enter=kembali): ");
    return jawaban == "Y" || jawaban == "y";
}

bool usernameSudahAda(const AppData& data, const string& username) {
    for (int i = 0; i < data.userCount; i++) {
        if (data.users[i].username == username) {
            return true;
        }
    }
    return false;
}

bool buatAkunPasienOlehDokter(AppData& data, const string& userFilePath) {
    if (data.userCount >= MAX_USERS) {
        cout << "\n[ERROR] Kapasitas user penuh.\n";
        return false;
    }

    while (true) {
        cout << "\n--- BUAT AKUN PASIEN (OLEH DOKTER) ---\n";

        string nama = inputBaris("Nama lengkap   : ");
        string username = inputBaris("Username       : ");
        string password = inputBaris("Password       : ");

        if (nama.empty() || username.empty() || password.empty()) {
            cout << "\n[ERROR] Semua field wajib diisi.\n";
            if (!lanjutBuatAkunSetelahError()) {
                cout << "[INFO] Pembuatan akun dibatalkan.\n";
                return false;
            }
            continue;
        }

        if (password.length() < 6) {
            cout << "\n[ERROR] Password minimal 6 karakter.\n";
            if (!lanjutBuatAkunSetelahError()) {
                cout << "[INFO] Pembuatan akun dibatalkan.\n";
                return false;
            }
            continue;
        }

        if (username.find('|') != string::npos || nama.find('|') != string::npos || password.find('|') != string::npos) {
            cout << "\n[ERROR] Karakter '|' tidak boleh dipakai.\n";
            if (!lanjutBuatAkunSetelahError()) {
                cout << "[INFO] Pembuatan akun dibatalkan.\n";
                return false;
            }
            continue;
        }

        if (usernameSudahAda(data, username)) {
            cout << "\n[ERROR] Username sudah dipakai. Gunakan username lain.\n";
            if (!lanjutBuatAkunSetelahError()) {
                cout << "[INFO] Pembuatan akun dibatalkan.\n";
                return false;
            }
            continue;
        }

        data.users[data.userCount].nama = nama;
        data.users[data.userCount].username = username;
        data.users[data.userCount].password = password;
        data.userCount++;

        if (!saveUsersToFile(data, userFilePath)) {
            cout << "\n[ERROR] Data user gagal disimpan ke file.\n";
            data.userCount--;
            if (!lanjutBuatAkunSetelahError()) {
                cout << "[INFO] Pembuatan akun dibatalkan.\n";
                return false;
            }
            continue;
        }

        cout << "\n[SUKSES] Akun pasien berhasil dibuat dan tersimpan di file TXT.\n";
        return true;
    }
}

int loginPasien(const AppData& data) {
    const int MAX_LOGIN_ATTEMPTS = 3;

    for (int percobaan = 1; percobaan <= MAX_LOGIN_ATTEMPTS; percobaan++) {
        cout << "\n--- LOGIN PASIEN ---\n";
        string username = inputBaris("Username : ");
        string password = inputBaris("Password : ");

        for (int i = 0; i < data.userCount; i++) {
            if (data.users[i].username == username && data.users[i].password == password) {
                cout << "\n[SUKSES] Login pasien berhasil.\n";
                return i;
            }
        }

        int sisaPercobaan = MAX_LOGIN_ATTEMPTS - percobaan;
        cout << "\n[ERROR] Login pasien gagal. Username/password salah.\n";
        if (sisaPercobaan > 0) {
            cout << "[INFO] Sisa percobaan login pasien: " << sisaPercobaan << "\n";
        }
    }

    cout << "\n[ERROR] Percobaan login pasien melebihi 3 kali. Program akan keluar.\n";
    exit(0);
    return -1;
}

bool loginDokter() {
    const int MAX_LOGIN_ATTEMPTS = 3;

    for (int percobaan = 1; percobaan <= MAX_LOGIN_ATTEMPTS; percobaan++) {
        cout << "\n--- LOGIN DOKTER ---\n";
        string username = inputBaris("Username : ");
        string password = inputBaris("Password : ");

        if (username == DOKTER_USERNAME && password == DOKTER_PASSWORD) {
            cout << "\n[SUKSES] Login dokter berhasil.\n";
            return true;
        }

        int sisaPercobaan = MAX_LOGIN_ATTEMPTS - percobaan;
        cout << "\n[ERROR] Login dokter gagal. Username/password salah.\n";
        if (sisaPercobaan > 0) {
            cout << "[INFO] Sisa percobaan login dokter: " << sisaPercobaan << "\n";
        }
    }

    cout << "\n[ERROR] Percobaan login dokter melebihi 3 kali. Program akan keluar.\n";
    exit(0);
    return false;
}


