#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include "beacon.h"

WINBASEAPI WINBOOL WINAPI KERNEL32$Module32First(HANDLE hSnapshot,LPMODULEENTRY32 lpme);
WINBASEAPI WINBOOL WINAPI KERNEL32$Module32Next(HANDLE hSnapshot,LPMODULEENTRY32 lpme);
WINBASEAPI WINBOOL WINAPI KERNEL32$CloseHandle (HANDLE hObject);
WINBASEAPI HANDLE WINAPI KERNEL32$CreateToolhelp32Snapshot(DWORD dwFlags,DWORD th32ProcessID);
DECLSPEC_IMPORT WINBOOL WINAPI VERSION$GetFileVersionInfoA(LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
DECLSPEC_IMPORT DWORD WINAPI VERSION$GetFileVersionInfoSizeA(LPCSTR lptstrFilenamea ,LPDWORD lpdwHandle);
DECLSPEC_IMPORT WINBOOL WINAPI VERSION$VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen);
WINBASEAPI void* WINAPI MSVCRT$malloc(SIZE_T);
DECLSPEC_IMPORT void WINAPI MSVCRT$free(void*);

void go (char* args, int len) {

    datap parser;
    BeaconDataParse(&parser, args, len);
    int pid = BeaconDataInt(&parser);

    formatp obj;
    int iii = 2048;
    BeaconFormatAlloc(&obj, iii);

    HANDLE snapshot = KERNEL32$CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        BeaconPrintf(CALLBACK_ERROR, "Unable to create snapshot of the process.");
        return;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (KERNEL32$Module32First(snapshot, &moduleEntry)) {
        do {
            if (moduleEntry.th32ProcessID == pid) {
                BeaconFormatPrintf(&obj, "\nName: %s\n", moduleEntry.szModule);
                BeaconFormatPrintf(&obj, "File Path: %s\n", moduleEntry.szExePath);

                // version info
                DWORD dwHandle;
                DWORD dwSize = VERSION$GetFileVersionInfoSizeA(moduleEntry.szExePath, &dwHandle);

                if (dwSize > 0) {
                    LPVOID lpVersionInfo = MSVCRT$malloc(dwSize);

                    if (VERSION$GetFileVersionInfoA(moduleEntry.szExePath, dwHandle, dwSize, lpVersionInfo)) {
                        VS_FIXEDFILEINFO* fileInfo;
                        UINT len;

                        if (VERSION$VerQueryValueA(lpVersionInfo, "\\", (LPVOID*)&fileInfo, &len)) {
                            BeaconFormatPrintf(&obj,"Version: %u.%u.%u.%u\n",
                                   HIWORD(fileInfo->dwFileVersionMS),
                                   LOWORD(fileInfo->dwFileVersionMS),
                                   HIWORD(fileInfo->dwFileVersionLS),
                                   LOWORD(fileInfo->dwFileVersionLS));
                        }
                    }

                    MSVCRT$free(lpVersionInfo);
                }

                break;
            }
        } while (KERNEL32$Module32Next(snapshot, &moduleEntry));
    }

    KERNEL32$CloseHandle(snapshot);

    int outSize = 0;
    char* dataOut = BeaconFormatToString(&obj, &outSize);
    BeaconOutput(CALLBACK_OUTPUT, dataOut, outSize);

    BeaconFormatFree(&obj);
    return;
}
