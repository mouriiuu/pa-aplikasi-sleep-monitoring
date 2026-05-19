#include "pasien_menu.h"

#include <climits>
#include <cctype>
#include <iostream>

#include "data_store.h"
#include "menu_common.h"
#include "sleep_metrics.h"

using namespace std;

static bool parseBilanganBulatNonNegatif(const string& teks, int& hasil, bool& terlaluBesar) {
    // Guard input angka + overflow.
    terlaluBesar = false;
    if (teks.empty()) {
        return false;
    }

    long long nilai = 0;
    for (char c : teks) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
        nilai = nilai * 10 + (c - '0');
        if (nilai > INT_MAX) {
            terlaluBesar = true;
            return false;
        }
    }

    hasil = static_cast<int>(nilai);
    return true;
}

static int cariJurnalMalamBelumLengkap(const AppData& data, const string& usernamePasien) {
    for (int i = data.sleepRecordCount - 1; i >= 0; i--) {
        if (data.sleepRecords[i].usernamePasien == usernamePasien && !data.sleepRecords[i].sudahInputPagi) {
            return i;
        }
    }
    return -1;
}

static bool adaJurnalMalamHariIniBelumLengkap(const AppData& data, const string& usernamePasien) {
    string tanggalHariIni = ambilTanggalSekarang();
    for (int i = data.sleepRecordCount - 1; i >= 0; i--) {
        const SleepRecord& record = data.sleepRecords[i];
        if (record.usernamePasien == usernamePasien &&
            record.tanggal == tanggalHariIni &&
            !record.sudahInputPagi) {
            return true;
        }
    }
    return false;
}

static bool sudahAdaJurnalDiTanggalYangSama(const AppData& data, const string& usernamePasien, const string& tanggal) {
    for (int i = 0; i < data.sleepRecordCount; i++) {
        const SleepRecord& record = data.sleepRecords[i];
        if (record.usernamePasien == usernamePasien && record.tanggal == tanggal) {
            return true;
        }
    }
    return false;
}

static bool sudahAdaInputPagiDiTanggalYangSama(const AppData& data, const string& usernamePasien, const string& tanggal, int abaikanIndex) {
    for (int i = 0; i < data.sleepRecordCount; i++) {
        if (i == abaikanIndex) {
            continue;
        }

        const SleepRecord& record = data.sleepRecords[i];
        if (record.usernamePasien == usernamePasien &&
            record.tanggal == tanggal &&
            record.sudahInputPagi) {
            return true;
        }
    }
    return false;
}

static void isiJurnalMalam(AppData& data, int userIndex, const string& sleepRecordFilePath) {
    if (data.sleepRecordCount >= MAX_SLEEP_RECORDS) {
        cout << "\n[ERROR] Kapasitas data tidur penuh.\n";
        return;
    }

    while (true) {
        string usernamePasien = data.users[userIndex].username;
        string tanggal = ambilTanggalSekarang();
        if (sudahAdaJurnalDiTanggalYangSama(data, usernamePasien, tanggal)) {
            cout << "\n[ERROR] Jurnal malam untuk tanggal " << tanggal << " sudah ada.\n";
            cout << "[INFO] Kamu hanya bisa isi 1 jurnal malam per hari.\n";
            return;
        }

        cout << "\n--- JURNAL MALAM ---\n";
        cout << "Tanggal otomatis            : " << tanggal << "\n";
        string catatanMalam = inputBarisMenu("Catatan sebelum tidur       : ");

        if (catatanMalam.empty()) {
            cout << "\n[ERROR] Catatan sebelum tidur wajib diisi.\n";
            continue;
        }

        if (berisiKarakterTerlarang(catatanMalam)) {
            cout << "\n[ERROR] Karakter '|' tidak boleh dipakai.\n";
            continue;
        }

        SleepRecord& record = data.sleepRecords[data.sleepRecordCount];
        record.usernamePasien = usernamePasien;
        record.tanggal = tanggal;
        record.jamMulaiSesiMalam = ambilJamSekarang();
        record.catatanMalam = catatanMalam;
        record.jamMulaiTidurFinal = "-";
        record.jamBangun = "-";
        record.jumlahTerbangun = 0;
        record.totalTerjagaMenit = 0;
        record.kualitasTidur = 0;
        record.kondisiBangun = "-";
        record.sudahInputPagi = false;
        data.sleepRecordCount++;

        if (!saveSleepRecordsToFile(data, sleepRecordFilePath)) {
            cout << "\n[ERROR] Data jurnal malam gagal disimpan.\n";
            data.sleepRecordCount--;
            continue;
        }

        cout << "\n[SUKSES] Jurnal malam tersimpan.\n";
        cout << "Jam mulai sesi malam otomatis: " << record.jamMulaiSesiMalam << "\n";
        return;
    }
}

static void inputDataPagi(AppData& data, int userIndex, const string& sleepRecordFilePath) {
    string usernamePasien = data.users[userIndex].username;
    int recordIndex = cariJurnalMalamBelumLengkap(data, usernamePasien);

    if (recordIndex == -1) {
        cout << "\n[ERROR] Belum ada jurnal malam yang perlu dilengkapi.\n";
        return;
    }

    SleepRecord& record = data.sleepRecords[recordIndex];
    if (sudahAdaInputPagiDiTanggalYangSama(data, usernamePasien, record.tanggal, recordIndex)) {
        cout << "\n[ERROR] Data pagi untuk tanggal " << record.tanggal << " sudah pernah diinput.\n";
        cout << "[INFO] Input data pagi hanya boleh 1 kali per hari.\n";
        return;
    }

    while (true) {
        cout << "\n--- INPUT DATA SETELAH BANGUN ---\n";
        cout << "Tanggal jurnal malam : " << record.tanggal << "\n";
        cout << "Jam sesi malam       : " << record.jamMulaiSesiMalam << "\n";

        string jamMulaiTidurFinal, jamBangun;

        cout << "Jam mulai tidur final        : ";
        cin >> jamMulaiTidurFinal;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Jam mulai tidur harus berupa teks.\n";
            continue;
        }
        if (!formatJamValid(jamMulaiTidurFinal)) {
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Format jam harus HH:MM (24 jam).\n";
            continue;
        }

        cout << "Jam bangun                   : ";
        cin >> jamBangun;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Jam bangun harus berupa teks.\n";
            continue;
        }
        if (!formatJamValid(jamBangun)) {
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Format jam harus HH:MM (24 jam).\n";
            continue;
        }
        if (konversiJamKeMenit(jamMulaiTidurFinal) == konversiJamKeMenit(jamBangun)) {
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Jam bangun harus lebih dari jam mulai tidur final (tidak boleh sama).\n";
            continue;
        }
        
        int jumlahTerbangun;
        int totalTerjagaMenit;
        int kualitasTidur;
        string inputJumlahTerbangun;
        string inputTotalTerjaga;
        string inputKualitas;
        bool terlaluBesar;

        cout << "Jumlah terbangun             : ";
        cin >> inputJumlahTerbangun;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Jumlah terbangun harus angka.\n";
            continue;
        }
        if (!parseBilanganBulatNonNegatif(inputJumlahTerbangun, jumlahTerbangun, terlaluBesar)) {
            cin.ignore(10000, '\n');
            if (terlaluBesar) {
                cout << "\n[ERROR] Jumlah terbangun terlalu besar.\n";
            } else {
                cout << "\n[ERROR] Jumlah terbangun harus angka bulat non-negatif.\n";
            }
            continue;
        }

        cout << "Total lama terjaga (menit)   : ";
        cin >> inputTotalTerjaga;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Total terjaga harus angka.\n";
            continue;
        }
        if (!parseBilanganBulatNonNegatif(inputTotalTerjaga, totalTerjagaMenit, terlaluBesar)) {
            cin.ignore(10000, '\n');
            if (terlaluBesar) {
                cout << "\n[ERROR] Total terjaga terlalu besar.\n";
            } else {
                cout << "\n[ERROR] Total terjaga harus angka bulat non-negatif.\n";
            }
            continue;
        }

        cout << "Kualitas tidur (1-10)        : ";
        cin >> inputKualitas;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Kualitas tidur harus angka.\n";
            continue;
        }
        if (!parseBilanganBulatNonNegatif(inputKualitas, kualitasTidur, terlaluBesar)) {
            cin.ignore(10000, '\n');
            if (terlaluBesar) {
                cout << "\n[ERROR] Kualitas tidur terlalu besar.\n";
            } else {
                cout << "\n[ERROR] Kualitas tidur harus angka bulat.\n";
            }
            continue;
        }
        if (kualitasTidur < 1 || kualitasTidur > 10) {
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Kualitas tidur harus antara 1-10.\n";
            continue;
        }

        cin.ignore(10000, '\n');
        string kondisiBangun = inputBarisMenu("Kondisi saat bangun          : ");

        if (jamMulaiTidurFinal.empty() || jamBangun.empty() || kondisiBangun.empty()) {
            cout << "\n[ERROR] Semua field wajib diisi.\n";
            continue;
        }

        if (!formatJamValid(jamMulaiTidurFinal) || !formatJamValid(jamBangun)) {
            cout << "\n[ERROR] Format jam harus HH:MM (24 jam).\n";
            continue;
        }

        if (berisiKarakterTerlarang(jamMulaiTidurFinal) || berisiKarakterTerlarang(jamBangun) || berisiKarakterTerlarang(kondisiBangun)) {
            cout << "\n[ERROR] Karakter '|' tidak boleh dipakai.\n";
            continue;
        }

        int mulaiTidurFinalMenit = konversiJamKeMenit(jamMulaiTidurFinal);
        int jamBangunMenit = konversiJamKeMenit(jamBangun);
        int durasiTidurDasar = hitungSelisihMenit(mulaiTidurFinalMenit, jamBangunMenit);
        if (totalTerjagaMenit > durasiTidurDasar) {
            cout << "\n[ERROR] Total terjaga melebihi durasi dari mulai tidur final hingga bangun.\n";
            continue;
        }

        record.jamMulaiTidurFinal = jamMulaiTidurFinal;
        record.jamBangun = jamBangun;
        record.jumlahTerbangun = jumlahTerbangun;
        record.totalTerjagaMenit = totalTerjagaMenit;
        record.kualitasTidur = kualitasTidur;
        record.kondisiBangun = kondisiBangun;
        record.sudahInputPagi = true;

        if (!saveSleepRecordsToFile(data, sleepRecordFilePath)) {
            cout << "\n[ERROR] Data pagi gagal disimpan.\n";
            continue;
        }

        cout << "\n[SUKSES] Data pagi tersimpan dan tergabung dengan jurnal malam.\n";
        tampilkanIndikatorSleepDiary(record);
        return;
    }
}

static void lihatsleepdairypasien(const AppData& data, int userIndex) {
    string usernamePasien = data.users[userIndex].username;
    bool adaRecord = false;

    cout << "\n--- DATA SLEEP DIARY PASIEN ---\n";
    for (int i = 0; i < data.sleepRecordCount; i++) {
        if (data.sleepRecords[i].usernamePasien == usernamePasien) {
            const SleepRecord& record = data.sleepRecords[i];
            cout << "\n================= DATA SLEEP DIARY [" << i + 1 << "] =================\n";
            cout << "\nTanggal          : " << record.tanggal << "\n";
            cout << "Jam sesi malam   : " << record.jamMulaiSesiMalam << "\n";
            cout << "Catatan malam    : " << record.catatanMalam << "\n";
            if (record.sudahInputPagi) {
                cout << "Jam mulai tidur  : " << record.jamMulaiTidurFinal << "\n";
                cout << "Jam bangun       : " << record.jamBangun << "\n";
                cout << "Jumlah terbangun : " << record.jumlahTerbangun << "\n";
                cout << "Total terjaga    : " << record.totalTerjagaMenit << " menit\n";
                cout << "Kualitas tidur   : " << record.kualitasTidur << "/10\n";
                cout << "Kondisi bangun   : " << record.kondisiBangun << "\n";
                if (recordSiapDihitung(record)) {
                    tampilkanIndikatorSleepDiary(record);
                } else {
                    cout << "[INFO] Indikator belum bisa dihitung karena format jam tidak valid.\n";
                }
            } else {
                cout << "[INFO] Data pagi belum diinput untuk tanggal ini.\n";
            }
            adaRecord = true;
        }
    }

    if (!adaRecord) {
        cout << "[INFO] Belum ada data sleep diary untuk pasien ini.\n";
    }
}

static void editjurnal(AppData& data, int userIndex, const string& sleepRecordFilePath) {
    while (true) {
        string usernamePasien = data.users[userIndex].username;

        int daftarIndex[MAX_SLEEP_RECORDS];
        int jumlah = 0;

        cout << "\n--- PILIH JURNAL YANG INGIN DIEDIT ---\n";

        for (int i = 0; i < data.sleepRecordCount; i++) {
            if (data.sleepRecords[i].usernamePasien == usernamePasien) {
                string status = data.sleepRecords[i].sudahInputPagi
                    ? "[Lengkap]"
                    : "[Belum ada data pagi]";

                cout << jumlah + 1 << ". Tanggal: "
                     << data.sleepRecords[i].tanggal
                     << " " << status << "\n";

                daftarIndex[jumlah++] = i;
            }
        }

        if (jumlah == 0) {
            cout << "\n[ERROR] Tidak ada jurnal yang ditemukan.\n";
            return;
        }

        int pilih;

        cout << "Pilih nomor jurnal (Enter untuk batal): ";
        string inputpilihan;
        getline(cin, inputpilihan);

        if (inputpilihan.empty()) {
            cout << "\n[INFO] Edit dibatalkan.\n";
            return;
        }

        bool terlaluBesar = false;
        if (!parseBilanganBulatNonNegatif(inputpilihan, pilih, terlaluBesar)) {
            if (terlaluBesar) {
                cout << "\n[ERROR] Pilihan terlalu besar.\n";
            } else {
                cout << "\n[ERROR] Pilihan harus berupa angka.\n";
            }
            continue;
        }

        if (pilih < 1 || pilih > jumlah) {
            cout << "\n[ERROR] Pilihan tidak valid.\n";
            continue;
        }

        SleepRecord& record = data.sleepRecords[daftarIndex[pilih - 1]];

        cout << "\n--- EDIT JURNAL TANGGAL: " << record.tanggal << " ---\n";
        cout << "1. Edit jurnal malam (catatan malam)\n";
        cout << "2. Edit data pagi (jam tidur, jam bangun, dll)\n";
        cout << "3. Batal\n";
        cout << "Pilihan: ";

        int pilihanEdit;
        cin >> pilihanEdit;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Pilihan harus angka.\n";
            continue;
        }

        cin.ignore(10000, '\n');

        if (pilihanEdit == 1) {

            cout << "\n--- EDIT JURNAL MALAM ---\n";
            cout << "Catatan saat ini     : " << record.catatanMalam << "\n";
            cout << "(Tekan Enter untuk tidak mengubah)\n\n";

            string catatanBaru = inputBarisMenu("Catatan malam baru   : ");

            if (!catatanBaru.empty()) {
                record.catatanMalam = catatanBaru;
            }

            if (!saveSleepRecordsToFile(data, sleepRecordFilePath)) {
                cout << "\n[ERROR] Gagal menyimpan perubahan.\n";
                continue;
            }

            cout << "\n[SUKSES] Jurnal malam berhasil diperbarui.\n";
            return;

        } else if (pilihanEdit == 2) {

            if (!record.sudahInputPagi) {
                cout << "\n[INFO] Data pagi belum pernah diisi. Gunakan menu 'Input data setelah bangun'.\n";
                continue;
            }

            cout << "\n--- EDIT DATA PAGI ---\n";
            cout << "Jam mulai tidur saat ini   : " << record.jamMulaiTidurFinal << "\n";
            cout << "Jam bangun saat ini        : " << record.jamBangun << "\n";
            cout << "Jumlah terbangun saat ini  : " << record.jumlahTerbangun << "\n";
            cout << "Total terjaga saat ini     : " << record.totalTerjagaMenit << " menit\n";
            cout << "Kualitas tidur saat ini    : " << record.kualitasTidur << "/10\n";
            cout << "Kondisi bangun saat ini    : " << record.kondisiBangun << "\n";
            cout << "(Tekan Enter jika tidak ingin mengubah)\n\n";

            string jamTidurBaru;
            while (true) {
                jamTidurBaru = inputBarisMenu("Jam mulai tidur baru       : ");

                if (jamTidurBaru.empty()) {
                    jamTidurBaru = record.jamMulaiTidurFinal;
                    break;
                }

                if (!formatJamValid(jamTidurBaru)) {
                    cout << "\n[ERROR] Format jam mulai tidur harus HH:MM.\n";
                    continue;
                }

                if (berisiKarakterTerlarang(jamTidurBaru)) {
                    cout << "\n[ERROR] Karakter '|' tidak boleh dipakai.\n";
                    continue;
                }

                break;
            }

            string jamBangunBaru;
            while (true) {
                jamBangunBaru = inputBarisMenu("Jam bangun baru            : ");

                if (jamBangunBaru.empty()) {
                    jamBangunBaru = record.jamBangun;
                    break;
                }

                if (!formatJamValid(jamBangunBaru)) {
                    cout << "\n[ERROR] Format jam bangun harus HH:MM.\n";
                    continue;
                }

                if (berisiKarakterTerlarang(jamBangunBaru)) {
                    cout << "\n[ERROR] Karakter '|' tidak boleh dipakai.\n";
                    continue;
                }

                break;
            }

            int terbangunBaru;
            while (true) {

                string inputTerbangun =
                    inputBarisMenu("Jumlah terbangun baru      : ");

                if (inputTerbangun.empty()) {
                    terbangunBaru = record.jumlahTerbangun;
                    break;
                }

                bool terlaluBesar = false;
                if (!parseBilanganBulatNonNegatif(inputTerbangun, terbangunBaru, terlaluBesar)) {
                    if (terlaluBesar) {
                        cout << "\n[ERROR] Jumlah terbangun terlalu besar.\n";
                    } else {
                        cout << "\n[ERROR] Jumlah terbangun harus angka bulat non-negatif.\n";
                    }
                    continue;
                }

                break;
            }

            int terjagaBaru;
            while (true) {

                string inputTerjaga =
                    inputBarisMenu("Total terjaga menit baru   : ");

                if (inputTerjaga.empty()) {
                    terjagaBaru = record.totalTerjagaMenit;
                    break;
                }

                bool terlaluBesar = false;
                if (!parseBilanganBulatNonNegatif(inputTerjaga, terjagaBaru, terlaluBesar)) {
                    if (terlaluBesar) {
                        cout << "\n[ERROR] Total terjaga terlalu besar.\n";
                    } else {
                        cout << "\n[ERROR] Total terjaga harus angka bulat non-negatif.\n";
                    }
                    continue;
                }

                break;
            }

            int kualitasBaru;
            while (true) {

                string inputKualitas =
                    inputBarisMenu("Kualitas tidur baru (1-10) : ");

                if (inputKualitas.empty()) {
                    kualitasBaru = record.kualitasTidur;
                    break;
                }

                bool terlaluBesar = false;
                if (!parseBilanganBulatNonNegatif(inputKualitas, kualitasBaru, terlaluBesar)) {
                    if (terlaluBesar) {
                        cout << "\n[ERROR] Kualitas tidur terlalu besar.\n";
                    } else {
                        cout << "\n[ERROR] Kualitas tidur harus angka bulat.\n";
                    }
                    continue;
                }

                if (kualitasBaru < 1 || kualitasBaru > 10) {
                    cout << "\n[ERROR] Kualitas tidur harus antara 1-10.\n";
                    continue;
                }

                break;
            }

            string kondisiBaru;
            while (true) {

                kondisiBaru =
                    inputBarisMenu("Kondisi bangun baru        : ");

                if (kondisiBaru.empty()) {
                    kondisiBaru = record.kondisiBangun;
                    break;
                }

                if (berisiKarakterTerlarang(kondisiBaru)) {
                    cout << "\n[ERROR] Karakter '|' tidak boleh dipakai.\n";
                    continue;
                }

                break;
            }

            int durasiDasar = hitungSelisihMenit(
                konversiJamKeMenit(jamTidurBaru),
                konversiJamKeMenit(jamBangunBaru)
            );

            if (konversiJamKeMenit(jamTidurBaru) == konversiJamKeMenit(jamBangunBaru)) {
                cout << "\n[ERROR] Jam bangun harus lebih dari jam mulai tidur final (tidak boleh sama).\n";
                continue;
            }

            if (terjagaBaru > durasiDasar) {
                cout << "\n[ERROR] Total terjaga melebihi durasi tidur.\n";
                continue;
            }

            record.jamMulaiTidurFinal = jamTidurBaru;
            record.jamBangun = jamBangunBaru;
            record.jumlahTerbangun = terbangunBaru;
            record.totalTerjagaMenit = terjagaBaru;
            record.kualitasTidur = kualitasBaru;
            record.kondisiBangun = kondisiBaru;

            if (!saveSleepRecordsToFile(data, sleepRecordFilePath)) {
                cout << "\n[ERROR] Gagal menyimpan perubahan.\n";
                continue;
            }

            cout << "\n[SUKSES] Data pagi berhasil diperbarui.\n";
            tampilkanIndikatorSleepDiary(record);
            return;
        }
        else if (pilihanEdit == 3) {
            cout << "\n[INFO] Edit dibatalkan.\n";
            return;
        } else {
            cout << "\n[ERROR] Pilihan edit tidak valid.\n";
        }
    }
}

static void hapusdatajurnal(AppData& data, int userIndex, const string& sleepRecordFilePath) {
    while (true) {
        string usernamePasien = data.users[userIndex].username;

        cout << "\n--- PILIH JURNAL YANG MAU DIHAPUS ---\n";

        int daftarIndex[MAX_SLEEP_RECORDS];
        int jumlah = 0;

        for (int i = 0; i < data.sleepRecordCount; i++) {
            if (data.sleepRecords[i].usernamePasien == usernamePasien) {
                cout << jumlah + 1 << ". Tanggal: "
                     << data.sleepRecords[i].tanggal << endl;

                daftarIndex[jumlah++] = i;
            }
        }

        if (jumlah == 0) {
            cout << "\n[ERROR] Tidak ada jurnal yang ditemukan.\n";
            return;
        }

        int pilih;

        cout << "Pilih nomor jurnal (Enter untuk batal): ";
        string inputpilihan;
        getline(cin, inputpilihan);

        if (inputpilihan.empty()) {
            cout << "\n[INFO] Hapus dibatalkan.\n";
            return;
        }

        bool terlaluBesar = false;
        if (!parseBilanganBulatNonNegatif(inputpilihan, pilih, terlaluBesar)) {
            if (terlaluBesar) {
                cout << "\n[ERROR] Pilihan terlalu besar.\n";
            } else {
                cout << "\n[ERROR] Pilihan harus berupa angka.\n";
            }
            continue;
        }

        if (pilih < 1 || pilih > jumlah) {
            cout << "\n[ERROR] Pilihan tidak valid.\n";
            continue;
        }

        int recordIndex = daftarIndex[pilih - 1];

        string inputKonfirmasi;

        cout << "Yakin ingin menghapus jurnal tanggal "
             << data.sleepRecords[recordIndex].tanggal
             << "? (y/n): ";

        do {
            getline(cin, inputKonfirmasi);

            if (inputKonfirmasi != "y" &&
                inputKonfirmasi != "Y" &&
                inputKonfirmasi != "n" &&
                inputKonfirmasi != "N") {

                cout << "[ERROR] Masukkan hanya y atau n: ";
            }

        } while (inputKonfirmasi != "y" &&
                 inputKonfirmasi != "Y" &&
                 inputKonfirmasi != "n" &&
                 inputKonfirmasi != "N");

        if (inputKonfirmasi == "n" ||
            inputKonfirmasi == "N") {

            cout << "\n[INFO] Penghapusan dibatalkan.\n";
            return;
        }

        for (int i = recordIndex; i < data.sleepRecordCount - 1; i++) {
            data.sleepRecords[i] = data.sleepRecords[i + 1];
        }

        data.sleepRecordCount--;

        if (!saveSleepRecordsToFile(data, sleepRecordFilePath)) {
            cout << "\n[ERROR] Gagal menyimpan perubahan setelah hapus.\n";
            continue;
        }

        cout << "\n[SUKSES] Jurnal berhasil dihapus.\n";
        return;
    }
}

void menuPasien(AppData& data, int userIndex, const string& sleepRecordFilePath) {
    int pilihan;
    bool selesai = false;

    do {
        string usernamePasien = data.users[userIndex].username;
        string tanggalHariIni = ambilTanggalSekarang();
        bool tampilIsiJurnalMalam = !sudahAdaJurnalDiTanggalYangSama(data, usernamePasien, tanggalHariIni);
        bool tampilInputPagi = adaJurnalMalamHariIniBelumLengkap(data, usernamePasien);

        int nomorMenu = 1;
        int opsiProfil = nomorMenu++;
        int opsiIsiJurnalMalam = tampilIsiJurnalMalam ? nomorMenu++ : -1;
        int opsiInputPagi = tampilInputPagi ? nomorMenu++ : -1;
        int opsiLihatSemua = nomorMenu++;
        int opsiEdit = nomorMenu++;
        int opsiHapus = nomorMenu++;
        int opsiLogout = nomorMenu++;

        cout << "\n========== MENU PASIEN ==========";
        cout << "\nLogin sebagai : " << data.users[userIndex].nama << endl;
        cout << "\n" << opsiProfil << ". Lihat profil singkat";
        if (tampilIsiJurnalMalam) {
            cout << "\n" << opsiIsiJurnalMalam << ". Isi jurnal malam";
        }
        if (tampilInputPagi) {
            cout << "\n" << opsiInputPagi << ". Input data setelah bangun";
        }
        cout << "\n" << opsiLihatSemua << ". Lihat seluruh data sleep diary";
        cout << "\n" << opsiEdit << ". Edit jurnal";
        cout << "\n" << opsiHapus << ". Hapus data jurnal";
        cout << "\n" << opsiLogout << ". Logout";
        cout << "\nPilihan: ";
        cin >> pilihan;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "[ERROR] Pilihan harus angka.\n";
            continue;
        }

        cin.ignore(10000, '\n');

        if (pilihan == opsiProfil) {
            cout << "\n--- PROFIL PASIEN ---\n";
            cout << "Nama     : " << data.users[userIndex].nama << "\n";
            cout << "Username : " << data.users[userIndex].username << "\n";
        } else if (pilihan == opsiIsiJurnalMalam) {
            isiJurnalMalam(data, userIndex, sleepRecordFilePath);
        } else if (pilihan == opsiInputPagi) {
            inputDataPagi(data, userIndex, sleepRecordFilePath);
        } else if (pilihan == opsiLihatSemua) {
            lihatsleepdairypasien(data, userIndex);
        } else if (pilihan == opsiEdit) {
            editjurnal(data, userIndex, sleepRecordFilePath);
        } else if (pilihan == opsiHapus) {
            hapusdatajurnal(data, userIndex, sleepRecordFilePath);
        } else if (pilihan == opsiLogout) {
            cout << "\nLogout pasien berhasil.\n";
            selesai = true;
        } else {
            cout << "\n[ERROR] Menu tidak tersedia.\n";
        }
    } while (!selesai);
}
