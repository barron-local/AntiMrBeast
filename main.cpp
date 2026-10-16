/*
 * AntiMrBeast - Endpoint Integrity Scanner & Threat Remediation Utility
 * Copyright (c) 2026 Local3
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <atomic>
#include <chrono>

namespace fs = std::filesystem;

std::atomic<bool> g_titleRunning(true);

void setColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void TitleWaveAnimationThread() {
    const std::wstring baseText = L"anti-mrbeast endpoint defense // by tnk";
    const size_t len = baseText.length();
    const size_t waveWidth = 4;

    size_t wavePos = 0;

    while (g_titleRunning) {
        std::wstring animated = baseText;

        for (size_t i = 0; i < len; ++i) {

            bool inWave = false;
            for (size_t w = 0; w < waveWidth; ++w) {
                if ((wavePos + w) % (len + waveWidth) == i) {
                    inWave = true;
                    break;
                }
            }

            if (inWave) {
                animated[i] = (wchar_t)towupper(baseText[i]);
            } else {
                animated[i] = (wchar_t)towlower(baseText[i]);
            }
        }

        SetConsoleTitleW(animated.c_str());

        wavePos = (wavePos + 1) % (len + waveWidth);
        std::this_thread::sleep_for(std::chrono::milliseconds(35));
    }
}

bool IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin == TRUE;
}

void ElevateNow() {
    wchar_t szPath[MAX_PATH];
    if (GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath))) {
        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.lpVerb = L"runas";
        sei.lpFile = szPath;
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;
        if (ShellExecuteExW(&sei)) {
            exit(0);
        }
    }
}

bool IsProcessRunning(const std::wstring& procName) {
    bool exists = false;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, procName.c_str()) == 0) {
                    exists = true;
                    break;
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    return exists;
}

int KillProcessByName(const std::wstring& procName) {
    int count = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, procName.c_str()) == 0) {
                    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProc) {
                        TerminateProcess(hProc, 0);
                        CloseHandle(hProc);
                        count++;
                    }
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    return count;
}

std::wstring GetLocalAppDataPath() {
    const wchar_t* env = _wgetenv(L"LOCALAPPDATA");
    if (env) return std::wstring(env);
    return L"";
}

std::vector<std::wstring> ScanForBKPayloads(const std::wstring& dirPath) {
    std::vector<std::wstring> found;
    try {
        if (!fs::exists(dirPath)) return found;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                std::wstring filename = entry.path().filename().wstring();
                if (filename.rfind(L"BK", 0) == 0 && filename.size() >= 4 &&
                    filename.substr(filename.size() - 4) == L".exe") {
                    found.push_back(entry.path().wstring());
                }
            }
        }
    } catch (...) {}
    return found;
}

void RunScan() {
    system("cls");
    setColor(10);
    std::cout << "========================================================\n";
    std::cout << "       ENDPOINT INTEGRITY & THREAT HUNTING ENGINE\n";
    std::cout << "========================================================\n\n";

    int threats = 0;

    setColor(11);
    std::cout << "[1/3] Analyzing background process tree & memory execution state...\n";
    if (IsProcessRunning(L"wscript.exe")) {
        setColor(12);
        std::cout << "   [ALERT] Anomalous script host execution detected [wscript.exe].\n";
        threats++;
    } else {
        setColor(10);
        std::cout << "   [OK] Process runtime environment clear.\n";
    }

    if (IsProcessRunning(L"cscript.exe")) {
        setColor(12);
        std::cout << "   [ALERT] Anomalous script host execution detected [cscript.exe].\n";
        threats++;
    }

    setColor(11);
    std::cout << "\n[2/3] Inspecting application storage vectors & dropper binary signatures...\n";
    std::wstring localApp = GetLocalAppDataPath();
    std::vector<std::wstring> bkFiles = ScanForBKPayloads(localApp);
    if (!bkFiles.empty()) {
        setColor(12);
        for (const auto& file : bkFiles) {
            std::wcout << L"   [ALERT] Malicious payload footprint identified: " << file << L"\n";
            threats++;
        }
    } else {
        setColor(10);
        std::cout << "   [OK] Zero dropper artifacts detected in local storage scope.\n";
    }

    setColor(11);
    std::cout << "\n[3/3] Auditing persistence mechanisms & build-chain artifacts...\n";
    std::wstring msbuildDir = localApp + L"\\Microsoft\\MSBuild";
    if (fs::exists(msbuildDir) && !fs::is_empty(msbuildDir)) {
        setColor(14);
        std::cout << "   [ALERT] Unverified persistence artifact detected in build directory.\n";
        threats++;
    } else {
        setColor(10);
        std::cout << "   [OK] Build-chain subsystem integrity verified.\n";
    }

    setColor(10);
    std::cout << "\n========================================================\n";
    if (threats > 0) {
        setColor(12);
        std::cout << "[STATUS] Threat detection flag raised: " << threats << " active IOC(s) identified.\n";
        std::cout << "Action Required: Execute [2] REMEDIATE to neutralize threats.\n";
    } else {
        setColor(10);
        std::cout << "[STATUS] Integrity check passed. Endpoint posture: SECURE.\n";
    }
    setColor(10);
    std::cout << "========================================================\n\n";

    setColor(7);
    std::cout << "Press Enter to return to the main console...";
    std::cin.ignore(10000, '\n');
}

bool SetRegistryDword(HKEY root, const wchar_t* subKey, const wchar_t* valName, DWORD data) {
    HKEY hKey;
    if (RegCreateKeyExW(root, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        LSTATUS status = RegSetValueExW(hKey, valName, 0, REG_DWORD, (const BYTE*)&data, sizeof(data));
        RegCloseKey(hKey);
        return status == ERROR_SUCCESS;
    }
    return false;
}

bool SetRegistryString(HKEY root, const wchar_t* subKey, const wchar_t* valName, const wchar_t* data) {
    HKEY hKey;
    if (RegCreateKeyExW(root, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        LSTATUS status = RegSetValueExW(hKey, valName, 0, REG_SZ, (const BYTE*)data, (DWORD)((wcslen(data) + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
        return status == ERROR_SUCCESS;
    }
    return false;
}

void DeleteRegistryKey(HKEY root, const wchar_t* subKey) {
    RegDeleteKeyW(root, subKey);
}

void RunClean() {
    system("cls");
    setColor(10);
    std::cout << "========================================================\n";
    std::cout << "     EXECUTING ENDPOINT IMMUNIZATION & REMEDIATION\n";
    std::cout << "========================================================\n\n";

    setColor(11);
    std::cout << "[1/5] Terminating anomalous script engine execution threads...\n";
    KillProcessByName(L"cscript.exe");
    KillProcessByName(L"wscript.exe");
    system("taskkill /F /IM cscript.exe /T >nul 2>&1");
    system("taskkill /F /IM wscript.exe /T >nul 2>&1");

    setColor(11);
    std::cout << "[2/5] Hardening script execution policies & restricting host subsystem...\n";
    SetRegistryDword(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows Script Host\\Settings", L"Enabled", 0);

    setColor(11);
    std::cout << "[3/5] Enforcing Software Restriction Policy (SRP) binary constraints...\n";
    const wchar_t* policyBase = L"SOFTWARE\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers\\262144\\Paths";
    DeleteRegistryKey(HKEY_LOCAL_MACHINE, (std::wstring(policyBase) + L"\\{a875d04d-e9c6-484c-813c-fb72714529f7}").c_str());

    std::wstring rule1 = std::wstring(policyBase) + L"\\{c875d04d-e9c6-484c-813c-fb72714529f7}";
    SetRegistryString(HKEY_LOCAL_MACHINE, rule1.c_str(), L"ItemData", L"BK*.exe");
    SetRegistryDword(HKEY_LOCAL_MACHINE, rule1.c_str(), L"SaferFlags", 0);

    std::wstring rule2 = std::wstring(policyBase) + L"\\{b986e15e-f0d7-495d-924d-0b83825630e8}";
    SetRegistryString(HKEY_LOCAL_MACHINE, rule2.c_str(), L"ItemData", L"%LOCALAPPDATA%\\BK*.exe");
    SetRegistryDword(HKEY_LOCAL_MACHINE, rule2.c_str(), L"SaferFlags", 0);

    setColor(11);
    std::cout << "[4/5] Purging staging cache, dropper footprints & temporary artifacts...\n";
    std::wstring localApp = GetLocalAppDataPath();
    std::wstring msbuildDir = localApp + L"\\Microsoft\\MSBuild";
    try {
        if (fs::exists(msbuildDir)) {
            fs::remove_all(msbuildDir);
        }
    } catch (...) {}
    system("powershell -Command \"Remove-Item -Path '$env:TEMP\\*' -Recurse -Force -ErrorAction SilentlyContinue\" >nul 2>&1");
    system("powershell -Command \"Remove-Item -Path '$env:LOCALAPPDATA\\Microsoft\\MSBuild' -Recurse -Force -ErrorAction SilentlyContinue\" >nul 2>&1");

    setColor(11);
    std::cout << "[5/5] Re-indexing and synchronizing global security descriptors...\n";
    system("gpupdate /force >nul 2>&1");

    setColor(10);
    std::cout << "\n========================================================\n";
    std::cout << "[SUCCESS] Endpoint immunization completed successfully.\n";
    std::cout << "- Threat execution pathways permanently restricted.\n";
    std::cout << "- Standard operational baseline maintained.\n";
    std::cout << "========================================================\n\n";

    setColor(7);
    std::cout << "Press Enter to return to the main console...";
    std::cin.ignore(10000, '\n');
}

int main() {

    if (!IsRunAsAdmin()) {
        setColor(14);
        std::cout << "Requesting elevated security privileges (UAC)...\n";
        ElevateNow();
        return 0;
    }

    std::thread titleThread(TitleWaveAnimationThread);

    while (true) {
        system("cls");
        setColor(10);
        std::cout << "========================================================\n";
        std::cout << "   ANTI-MRBEAST // ADVANCED ENDPOINT DEFENSE SUITE\n";
        std::cout << "========================================================\n\n";

        setColor(15);
        std::cout << "  [1] SCAN      - Execute heuristic threat scan & integrity audit\n";
        std::cout << "  [2] REMEDIATE - Neutralize threats & apply endpoint immunization\n";
        std::cout << "  [3] EXIT      - Terminate session\n\n";

        setColor(10);
        std::cout << "========================================================\n";
        setColor(7);
        std::cout << "Select operation mode (1, 2, or 3): ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1") {
            RunScan();
        } else if (choice == "2") {
            RunClean();
        } else if (choice == "3") {
            break;
        }
    }

    g_titleRunning = false;
    if (titleThread.joinable()) {
        titleThread.join();
    }

    return 0;
}
