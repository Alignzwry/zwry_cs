#pragma once
#include <windows.h>
#include <cstdio>
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>

using std::cout, std::endl;

namespace memory
{
    void EnableDebugPriv()
    {
        HANDLE hToken;
        LUID luid;
        TOKEN_PRIVILEGES tkp;

        OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Luid = luid;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);

        CloseHandle(hToken);
    }

    DWORD getPId(char const* name) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        DWORD pID = NULL;
        if (Process32First(snapshot, &entry) == TRUE)
        {
            while (Process32Next(snapshot, &entry) == TRUE)
            {
                if (_stricmp(entry.szExeFile, name) == 0)
                {
                    pID = entry.th32ProcessID;
                    break;
                }
            }
        }
        CloseHandle(snapshot);
        return pID;
    }

    uintptr_t GetModule(DWORD procId, const char* modName) {
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
        uintptr_t a = NULL;
        if (hSnap != INVALID_HANDLE_VALUE)
        {
            MODULEENTRY32 modEntry;
            modEntry.dwSize = sizeof(modEntry);
            if (Module32First(hSnap, &modEntry))
            {
                do
                {
                    if (strcmp(modEntry.szModule, modName) == 0)
                    {
                        a = (uintptr_t)modEntry.modBaseAddr;
                        break;
                    }
                } while (Module32Next(hSnap, &modEntry));
            }
            CloseHandle(hSnap);
        }
        return a;
    }
    /*
    uintptr_t getAddressFromOffsets(HANDLE pHandle, uintptr_t BaseAddress, std::vector<DWORD> Offsets) {
        DWORD64 Address = 0;

        if (Offsets.size() == 0)
            return BaseAddress;

        if (!_cs.ReadMemory<DWORD64>(BaseAddress, Address))
            return 0;

        for (int i = 0; i < Offsets.size() - 1; i++)
        {
            if (!_cs.ReadMemory<DWORD64>(Address + Offsets[i], Address))
                return 0;
        }
        return Address == 0 ? 0 : Address + Offsets[Offsets.size() - 1];
    }*/
}