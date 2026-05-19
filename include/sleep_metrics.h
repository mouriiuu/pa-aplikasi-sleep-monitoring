#ifndef SLEEP_METRICS_H
#define SLEEP_METRICS_H

#include <string>

#include "user.h"

bool formatJamValid(const std::string& jam);
int konversiJamKeMenit(const std::string& jam);
int konversiJamKeMenit(int jamAngka, int menitAngka);
int hitungSelisihMenit(int menitMulai, int menitSelesai);
int hitungSOL(const SleepRecord& record);
int hitungWASO(const SleepRecord& record);
int hitungNumberOfAwakenings(const SleepRecord& record);
int hitungWaktuDiTempatTidur(const SleepRecord& record);
int hitungTST(const SleepRecord& record);
double hitungSE(const SleepRecord& record);
bool recordSiapDihitung(const SleepRecord& record);
void tampilkanIndikatorSleepDiary(const SleepRecord& record);
void tampilkanIndikatorSingkatRecord(const SleepRecord& record);

#endif
