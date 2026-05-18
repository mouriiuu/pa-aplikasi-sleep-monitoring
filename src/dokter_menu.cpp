#include "dokter_menu.h"

#include <iomanip>
#include <iostream>

#include "auth/auth.h"
#include "data_store.h"
#include "menu_common.h"
#include "sleep_metrics.h"

using namespace std;

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

static void monitoringPasienOlehDokter(AppData& data, const string& sleepRecordFilePath) {
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
        cout << "3. Hapus semua data tidur pasien\n";
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
                string konfirmasi = inputBarisMenu("Ketik Y untuk konfirmasi hapus semua data tidur pasien: ");
                if (konfirmasi != "Y" && konfirmasi != "y") {
                    cout << "\nAksi hapus dibatalkan.\n";
                    break;
                }

                int jumlahDihapus = hapusSemuaDataTidurPasien(data, pasienDipilih.username);
                if (jumlahDihapus == 0) {
                    cout << "\nTidak ada data tidur yang bisa dihapus.\n";
                    break;
                }

                if (!saveSleepRecordsToFile(data, sleepRecordFilePath)) {
                    cout << "\n[ERROR] Gagal menyimpan penghapusan data tidur.\n";
                    break;
                }

                cout << "\n[SUKSES] " << jumlahDihapus << " data tidur pasien berhasil dihapus.\n";
                break;
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
        cout << "\n3. Monitoring pasien";
        cout << "\n4. Logout";
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
                monitoringPasienOlehDokter(data, sleepRecordFilePath);
                break;
            case 4:
                cout << "\nLogout dokter berhasil.\n";
                break;
            default:
                cout << "\n[ERROR] Menu tidak tersedia.\n";
                break;
        }
    } while (pilihan != 4);
}
