#include "dokter_menu.h"

#include <cctype>
#include <iomanip>
#include <iostream>
#include <vector>

#include "auth/auth.h"
#include "data_store.h"
#include "menu_common.h"
#include "sleep_metrics.h"

using namespace std;

static string keHurufKecil(const string& teks) {
    string hasil = teks;
    for (size_t i = 0; i < hasil.length(); i++) {
        hasil[i] = static_cast<char>(tolower(static_cast<unsigned char>(hasil[i])));
    }
    return hasil;
}

static bool mengandungTanpaCase(const string& sumber, const string& kataKunci) {
    return keHurufKecil(sumber).find(keHurufKecil(kataKunci)) != string::npos;
}

static void tampilkanDaftarPasienByIndex(const AppData& data, const vector<int>& indeksPasien) {
    if (indeksPasien.empty()) {
        cout << "- Tidak ada pasien yang cocok.\n";
        return;
    }

    for (size_t i = 0; i < indeksPasien.size(); i++) {
        int idx = indeksPasien[i];
        cout << i + 1 << ". " << data.users[idx].nama << " (" << data.users[idx].username << ")\n";
    }
}

static void cariPasien(const AppData& data) {
    if (data.userCount == 0) {
        cout << "\n[ERROR] Belum ada pasien terdaftar.\n";
        return;
    }

    string kataKunci;
    while (true) {
        kataKunci = inputBarisMenu("\nKata kunci nama/username: ");
        if (kataKunci.empty()) {
            cout << "\n[ERROR] Kata kunci tidak boleh kosong. Silakan ulangi.\n";
            continue;
        }
        break;
    }

    vector<int> hasil;
    for (int i = 0; i < data.userCount; i++) {
        if (mengandungTanpaCase(data.users[i].nama, kataKunci) ||
            mengandungTanpaCase(data.users[i].username, kataKunci)) {
            hasil.push_back(i);
        }
    }

    cout << "\nHasil pencarian pasien:\n";
    tampilkanDaftarPasienByIndex(data, hasil);
}

static void urutkanDanTampilkanPasien(const AppData& data) {
    if (data.userCount == 0) {
        cout << "\n[ERROR] Belum ada pasien terdaftar.\n";
        return;
    }

    int pilihanSort;
    while (true) {
        cout << "\n--- SORT DAFTAR PASIEN ---\n";
        cout << "1. Nama A-Z\n";
        cout << "2. Nama Z-A\n";
        cout << "3. Username A-Z\n";
        cout << "4. Username Z-A\n";
        cout << "Pilihan sort: ";

        cin >> pilihanSort;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Pilihan sort harus angka. Silakan ulangi.\n";
            continue;
        }
        cin.ignore(10000, '\n');

        if (pilihanSort < 1 || pilihanSort > 4) {
            cout << "\n[ERROR] Pilihan sort tidak valid. Silakan ulangi.\n";
            continue;
        }
        break;
    }

    vector<int> indeksPasien;
    for (int i = 0; i < data.userCount; i++) {
        indeksPasien.push_back(i);
    }

    auto harusTukar = [&data, pilihanSort](int kiri, int kanan) {
        string nilaiKiri;
        string nilaiKanan;

        if (pilihanSort == 1 || pilihanSort == 2) {
            nilaiKiri = keHurufKecil(data.users[kiri].nama);
            nilaiKanan = keHurufKecil(data.users[kanan].nama);
        } else {
            nilaiKiri = keHurufKecil(data.users[kiri].username);
            nilaiKanan = keHurufKecil(data.users[kanan].username);
        }

        if (pilihanSort == 1 || pilihanSort == 3) {
            return nilaiKiri > nilaiKanan;
        }
        return nilaiKiri < nilaiKanan;
    };

    int n = static_cast<int>(indeksPasien.size());
    for (int i = 0; i < n - 1; i++) {
        bool adaTukar = false;
        for (int j = 0; j < n - i - 1; j++) {
            if (harusTukar(indeksPasien[j], indeksPasien[j + 1])) {
                int temp = indeksPasien[j];
                indeksPasien[j] = indeksPasien[j + 1];
                indeksPasien[j + 1] = temp;
                adaTukar = true;
            }
        }
        if (!adaTukar) {
            break;
        }
    }

    cout << "\nHasil urut daftar pasien:\n";
    tampilkanDaftarPasienByIndex(data, indeksPasien);
}

static int pilihPasienUntukMonitoring(const AppData& data) {
    if (data.userCount == 0) {
        cout << "\n[ERROR] Belum ada pasien terdaftar.\n";
        return -1;
    }

    while (true) {
        tampilkanDaftarPasienSingkat(data);
        cout << "Pilih nomor pasien: ";
        int pilihan;
        cin >> pilihan;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Nomor pasien harus angka.\n";
            continue;
        }

        cin.ignore(10000, '\n');

        if (pilihan < 1 || pilihan > data.userCount) {
            cout << "\n[ERROR] Nomor pasien tidak valid.\n";
            continue;
        }

        return pilihan - 1;
    }
}

static void tampilkanSemuaDataTidurPasien(const AppData& data, const User& pasien) {
    cout << "\n--- DATA TIDUR PASIEN ---\n";
    cout << "Nama     : " << pasien.nama << "\n";
    cout << "Username : " << pasien.username << "\n";

    int nomor = 0;
    for (int i = 0; i < data.sleepRecordCount; i++) {
        const SleepRecord& record = data.sleepRecords[i];
        if (record.usernamePasien != pasien.username) {
            continue;
        }

        nomor++;
        cout << "\n[" << nomor << "] Tanggal : " << record.tanggal << "\n";
        cout << "Jam sesi malam      : " << record.jamMulaiSesiMalam << "\n";
        cout << "Jam tidur final     : " << record.jamMulaiTidurFinal << "\n";
        cout << "Jam bangun          : " << record.jamBangun << "\n";
        cout << "Jumlah terbangun    : " << record.jumlahTerbangun << "\n";
        cout << "Total terjaga       : " << record.totalTerjagaMenit << " menit\n";
        cout << "Kualitas tidur      : " << record.kualitasTidur << "/10\n";
        cout << "Kondisi bangun      : " << record.kondisiBangun << "\n";

        if (recordSiapDihitung(record)) {
            tampilkanIndikatorSingkatRecord(record);
        } else {
            cout << "Indikator           : Belum bisa dihitung (data belum lengkap).\n";
        }
    }

    if (nomor == 0) {
        cout << "\nBelum ada data tidur untuk pasien ini.\n";
    }
}

static void tampilkanRingkasanPasien(const AppData& data, const User& pasien) {
    int jumlahRecordLengkap = 0;
    int totalTST = 0;
    double totalSE = 0.0;
    int totalKualitas = 0;
    int totalTerbangun = 0;

    for (int i = 0; i < data.sleepRecordCount; i++) {
        const SleepRecord& record = data.sleepRecords[i];
        if (record.usernamePasien != pasien.username || !recordSiapDihitung(record)) {
            continue;
        }

        jumlahRecordLengkap++;
        totalTST += hitungTST(record);
        totalSE += hitungSE(record);
        totalKualitas += record.kualitasTidur;
        totalTerbangun += record.jumlahTerbangun;
    }

    cout << "\n--- RINGKASAN PASIEN ---\n";
    cout << "Nama     : " << pasien.nama << "\n";
    cout << "Username : " << pasien.username << "\n";

    if (jumlahRecordLengkap == 0) {
        cout << "Belum ada data tidur lengkap untuk diringkas.\n";
        return;
    }

    cout << "Jumlah data lengkap          : " << jumlahRecordLengkap << "\n";
    cout << "Rata-rata TST                : "
         << fixed << setprecision(2) << static_cast<double>(totalTST) / jumlahRecordLengkap
         << " menit\n";
    cout << "Rata-rata SE                 : "
         << fixed << setprecision(2) << totalSE / jumlahRecordLengkap
         << "%\n";
    cout << "Rata-rata kualitas tidur     : "
         << fixed << setprecision(2) << static_cast<double>(totalKualitas) / jumlahRecordLengkap
         << "/10\n";
    cout << "Frekuensi terbangun (rata2)  : "
         << fixed << setprecision(2) << static_cast<double>(totalTerbangun) / jumlahRecordLengkap
         << " kali/malam\n";
    cout << "Frekuensi terbangun (total)  : " << totalTerbangun << " kali\n";
    cout << defaultfloat;
}

static int hapusSemuaDataTidurPasien(AppData& data, const string& usernamePasien) {
    int tulis = 0;
    int awal = data.sleepRecordCount;

    for (int baca = 0; baca < data.sleepRecordCount; baca++) {
        if (data.sleepRecords[baca].usernamePasien != usernamePasien) {
            data.sleepRecords[tulis] = data.sleepRecords[baca];
            tulis++;
        }
    }

    data.sleepRecordCount = tulis;
    return awal - tulis;
}

static bool hapusPasienDariDaftar(AppData& data, const string& usernamePasien) {
    int indexHapus = -1;
    for (int i = 0; i < data.userCount; i++) {
        if (data.users[i].username == usernamePasien) {
            indexHapus = i;
            break;
        }
    }

    if (indexHapus == -1) {
        return false;
    }

    for (int i = indexHapus; i < data.userCount - 1; i++) {
        data.users[i] = data.users[i + 1];
    }
    data.userCount--;
    return true;
}

static bool hapusPasienDanSemuaDataByUsername(
    AppData& data,
    const string& usernamePasien,
    const string& userFilePath,
    const string& sleepRecordFilePath,
    int& jumlahDataTidurDihapus
) {
    AppData backup = data;

    bool pasienTerhapus = hapusPasienDariDaftar(data, usernamePasien);
    jumlahDataTidurDihapus = hapusSemuaDataTidurPasien(data, usernamePasien);

    if (!pasienTerhapus) {
        data = backup;
        return false;
    }

    if (!saveUsersToFile(data, userFilePath) || !saveSleepRecordsToFile(data, sleepRecordFilePath)) {
        data = backup;
        return false;
    }

    return true;
}

static void hapusPasienDanSemuaDataTidurOlehDokter(
    AppData& data,
    const string& userFilePath,
    const string& sleepRecordFilePath
) {
    if (data.userCount == 0) {
        cout << "\n[ERROR] Belum ada pasien terdaftar.\n";
        return;
    }

    int pasienIndex = pilihPasienUntukMonitoring(data);
    if (pasienIndex == -1) {
        return;
    }

    const User pasienDipilih = data.users[pasienIndex];
    cout << "\nPasien yang akan dihapus: "
         << pasienDipilih.nama << " (" << pasienDipilih.username << ")\n";
    string konfirmasi = inputBarisMenu("Ketik Y untuk konfirmasi hapus akun pasien + semua data tidurnya: ");
    if (konfirmasi != "Y" && konfirmasi != "y") {
        cout << "\nAksi hapus dibatalkan.\n";
        return;
    }

    int jumlahDataTidurDihapus = 0;
    if (!hapusPasienDanSemuaDataByUsername(
            data,
            pasienDipilih.username,
            userFilePath,
            sleepRecordFilePath,
            jumlahDataTidurDihapus)) {
        cout << "\n[ERROR] Gagal menghapus akun pasien.\n";
        return;
    }

    cout << "\n[SUKSES] Akun pasien berhasil dihapus.\n";
    cout << "[SUKSES] " << jumlahDataTidurDihapus << " data tidur terkait juga dihapus.\n";
}

static void monitoringPasienOlehDokter(
    AppData& data,
    const string& userFilePath,
    const string& sleepRecordFilePath
) {
    int pasienIndex = pilihPasienUntukMonitoring(data);
    if (pasienIndex == -1) {
        return;
    }

    User pasienDipilih = data.users[pasienIndex];
    int pilihan;

    do {
        cout << "\n===== MONITORING PASIEN =====\n";
        cout << "Pasien: " << pasienDipilih.nama << " (" << pasienDipilih.username << ")\n";
        cout << "1. Lihat semua data tidur pasien\n";
        cout << "2. Lihat ringkasan pasien\n";
        cout << "3. Hapus akun pasien + semua data tidurnya\n";
        cout << "4. Kembali ke menu dokter\n";
        cout << "Pilihan: ";
        cin >> pilihan;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "[ERROR] Pilihan harus angka.\n";
            continue;
        }

        cin.ignore(10000, '\n');

        switch (pilihan) {
            case 1:
                tampilkanSemuaDataTidurPasien(data, pasienDipilih);
                break;
            case 2:
                tampilkanRingkasanPasien(data, pasienDipilih);
                break;
            case 3: {
                string konfirmasi = inputBarisMenu("Ketik Y untuk konfirmasi hapus akun pasien + semua data tidurnya: ");
                if (konfirmasi != "Y" && konfirmasi != "y") {
                    cout << "\nAksi hapus dibatalkan.\n";
                    break;
                }

                int jumlahDihapus = 0;
                if (!hapusPasienDanSemuaDataByUsername(
                        data,
                        pasienDipilih.username,
                        userFilePath,
                        sleepRecordFilePath,
                        jumlahDihapus)) {
                    cout << "\n[ERROR] Gagal menghapus akun pasien.\n";
                    break;
                }

                cout << "\n[SUKSES] Akun pasien berhasil dihapus.\n";
                cout << "[SUKSES] " << jumlahDihapus << " data tidur terkait juga dihapus.\n";
                cout << "\nKembali ke menu dokter.\n";
                return;
            }
            case 4:
                cout << "\nKembali ke menu dokter.\n";
                break;
            default:
                cout << "\n[ERROR] Menu monitoring tidak tersedia.\n";
                break;
        }
    } while (pilihan != 4);
}

void menuDokter(AppData& data, const string& userFilePath, const string& sleepRecordFilePath) {
    int pilihan;

    do {
        cout << "\n========== MENU DOKTER ==========";
        cout << "\n1. Buat akun pasien";
        cout << "\n2. Lihat daftar pasien";
        cout << "\n3. Cari pasien";
        cout << "\n4. Urutkan daftar pasien";
        cout << "\n5. Monitoring pasien";
        cout << "\n6. Hapus pasien + semua data tidurnya";
        cout << "\n7. Logout";
        cout << "\nPilihan: ";
        cin >> pilihan;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "[ERROR] Pilihan harus angka.\n";
            continue;
        }

        cin.ignore(10000, '\n');

        switch (pilihan) {
            case 1:
                buatAkunPasienOlehDokter(data, userFilePath);
                break;
            case 2:
                tampilkanDaftarPasienSingkat(data);
                break;
            case 3:
                cariPasien(data);
                break;
            case 4:
                urutkanDanTampilkanPasien(data);
                break;
            case 5:
                monitoringPasienOlehDokter(data, userFilePath, sleepRecordFilePath);
                break;
            case 6:
                hapusPasienDanSemuaDataTidurOlehDokter(data, userFilePath, sleepRecordFilePath);
                break;
            case 7:
                cout << "\nLogout dokter berhasil.\n";
                break;
            default:
                cout << "\n[ERROR] Menu tidak tersedia.\n";
                break;
        }
    } while (pilihan != 7);
}
