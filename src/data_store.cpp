#include "data_store.h"

#include <cstdlib>
#include <fstream>
#include <vector>

using namespace std;

void initAppData(AppData& data) {
    data.userCount = 0;
    data.sleepRecordCount = 0;
}

static vector<string> splitLine(const string& line) {
    vector<string> parts;
    size_t start = 0;
    size_t pos = line.find('|');

    while (pos != string::npos) {
        parts.push_back(line.substr(start, pos - start));
        start = pos + 1;
        pos = line.find('|', start);
    }

    parts.push_back(line.substr(start));
    return parts;
}

void loadUsersFromFile(AppData& data, const string& filePath) {
    ifstream file(filePath.c_str());
    if (!file.is_open()) {
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        size_t p1 = line.find('|');
        if (p1 == string::npos) {
            continue;
        }

        size_t p2 = line.find('|', p1 + 1);
        if (p2 == string::npos) {
            continue;
        }

        if (data.userCount >= MAX_USERS) {
            break;
        }

        data.users[data.userCount].nama = line.substr(0, p1);
        data.users[data.userCount].username = line.substr(p1 + 1, p2 - p1 - 1);
        data.users[data.userCount].password = line.substr(p2 + 1);
        data.userCount++;
    }
}

bool saveUsersToFile(const AppData& data, const string& filePath) {
    ofstream file(filePath.c_str());
    if (!file.is_open()) {
        return false;
    }

    for (int i = 0; i < data.userCount; i++) {
        file << data.users[i].nama << '|'
             << data.users[i].username << '|'
             << data.users[i].password << '\n';
    }

    return true;
}

void loadSleepRecordsFromFile(AppData& data, const string& filePath) {
    ifstream file(filePath.c_str());
    if (!file.is_open()) {
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        if (data.sleepRecordCount >= MAX_SLEEP_RECORDS) {
            break;
        }

        vector<string> parts = splitLine(line);
        if (parts.size() != 11) {
            continue;
        }

        SleepRecord& record = data.sleepRecords[data.sleepRecordCount];
        record.usernamePasien = parts[0];
        record.tanggal = parts[1];
        record.jamMulaiSesiMalam = parts[2];
        record.catatanMalam = parts[3];
        record.jamMulaiTidurFinal = parts[4];
        record.jamBangun = parts[5];
        record.jumlahTerbangun = atoi(parts[6].c_str());
        record.totalTerjagaMenit = atoi(parts[7].c_str());
        record.kualitasTidur = atoi(parts[8].c_str());
        if (record.jumlahTerbangun < 0) {
            record.jumlahTerbangun = 0;
        }
        if (record.totalTerjagaMenit < 0) {
            record.totalTerjagaMenit = 0;
        }
        if (record.kualitasTidur < 0) {
            record.kualitasTidur = 0;
        }
        record.kondisiBangun = parts[9];
        record.sudahInputPagi = parts[10] == "1";
        data.sleepRecordCount++;
    }
}

bool saveSleepRecordsToFile(const AppData& data, const string& filePath) {
    ofstream file(filePath.c_str());
    if (!file.is_open()) {
        return false;
    }

    for (int i = 0; i < data.sleepRecordCount; i++) {
        const SleepRecord& record = data.sleepRecords[i];
        file << record.usernamePasien << '|'
             << record.tanggal << '|'
             << record.jamMulaiSesiMalam << '|'
             << record.catatanMalam << '|'
             << record.jamMulaiTidurFinal << '|'
             << record.jamBangun << '|'
             << record.jumlahTerbangun << '|'
             << record.totalTerjagaMenit << '|'
             << record.kualitasTidur << '|'
             << record.kondisiBangun << '|'
             << (record.sudahInputPagi ? 1 : 0) << '\n';
    }

    return true;
}
