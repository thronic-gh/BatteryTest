#pragma once

std::string RapportMottaker = "REDACTED";

int BatteriTestStatus = 0;
bool RunningOnBattery = false;
std::mutex mutexlock;
std::mutex mutexlock2;
unsigned long mWhStartCapacity = 0;
int rundeteller = 0;
std::vector<std::wstring> statustekster;
int MaxCapByDesignPerc = 0;
int MaxCapCurrentlyPerc = 0;
unsigned long MaxCapByDesignVal = 0;
unsigned long MaxCapCurrentlyVal = 0;
std::vector<long> AvgRate;
int ratecounter = 0;
time_t tid_fra_start = 0;
tm *lokaltid_fra_start = 0;
HWND ReportButtonHwnd;
HWND ReportButtonTooltip;
HANDLE ReportButtonImg;

// For batterirapport:
struct BatteriRapportData {
	std::string TestStartet = u8"N/A.";
	std::string TestStoppet = u8"N/A.";
	std::string TestMaksKapasitetHelse = u8"N/A.";
	std::string TestBurdeVareTil = u8"N/A.";
	std::string TestVarerFaktiskTil = u8"N/A.";
	std::string TestKapasitetVedStart = u8"N/A.";
	std::string TestKapasitetVedStopp = u8"N/A.";
	std::string TestUtladningsSnitt = u8"N/A.";
	std::string TestProsentResterende = u8"N/A.";
	std::string TestVarighet = u8"N/A.";
} RapportDetaljer;
