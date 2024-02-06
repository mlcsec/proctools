#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <winternl.h>
#include "beacon.h"

#pragma comment(lib, "ntdll.lib")

WINBASEAPI HANDLE WINAPI KERNEL32$OpenProcess (DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwProcessId);
WINBASEAPI WINBOOL WINAPI KERNEL32$CloseHandle (HANDLE hObject);
DECLSPEC_IMPORT NTSTATUS WINAPI NTDLL$NtQueryInformationProcess(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
WINBASEAPI BOOL WINAPI KERNEL32$ReadProcessMemory (HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead);
WINBASEAPI void* WINAPI MSVCRT$malloc(SIZE_T);
DECLSPEC_IMPORT void WINAPI MSVCRT$free(void*);
WINBASEAPI size_t __cdecl MSVCRT$wcslen(const wchar_t *_Str);

void go (char* args, int len) {

    datap parser;
    BeaconDataParse(&parser, args, len);
    int pid = BeaconDataInt(&parser);

    HANDLE hProcess = KERNEL32$OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        BeaconPrintf(CALLBACK_OUTPUT, "\n[-] Failed to open process");
        return;
    }

    // get process basic info
    PROCESS_BASIC_INFORMATION pbi;
    NTSTATUS status = NTDLL$NtQueryInformationProcess(
        hProcess,
        ProcessBasicInformation,
        &pbi,
        sizeof(PROCESS_BASIC_INFORMATION),
        NULL);

    if (!NT_SUCCESS(status)) {
        BeaconPrintf(CALLBACK_OUTPUT, "\n[-] Failed to query process information");
        KERNEL32$CloseHandle(hProcess);
        return;
    }

    // read PEB
    PEB peb;
    if (!KERNEL32$ReadProcessMemory(
        hProcess,
        pbi.PebBaseAddress,
        &peb,
        sizeof(PEB),
        NULL)) {
        BeaconPrintf(CALLBACK_OUTPUT, "\n[-] Failed to read PEB");
        KERNEL32$CloseHandle(hProcess);
        return;
    }

    // read the process parameters
    RTL_USER_PROCESS_PARAMETERS parameters;
    if (!KERNEL32$ReadProcessMemory(
        hProcess,
        peb.ProcessParameters,
        &parameters,
        sizeof(RTL_USER_PROCESS_PARAMETERS),
        NULL)) {
        BeaconPrintf(CALLBACK_OUTPUT, "\n[-] Failed to read process parameters");
        KERNEL32$CloseHandle(hProcess);
        return;
    }

    // read the command line buffer
    wchar_t* commandLineBuffer = (wchar_t*)MSVCRT$malloc(parameters.CommandLine.Length + sizeof(wchar_t));
    if (!KERNEL32$ReadProcessMemory(
        hProcess,
        parameters.CommandLine.Buffer,
        commandLineBuffer,
        parameters.CommandLine.Length,
        NULL)) {
        BeaconPrintf(CALLBACK_OUTPUT, "\n[-] Failed to read command line buffer");
        MSVCRT$free(commandLineBuffer);
        KERNEL32$CloseHandle(hProcess);
        return;
    }

    commandLineBuffer[parameters.CommandLine.Length / sizeof(wchar_t)] = L'\0';

    // output and cleanup
    BeaconPrintf(CALLBACK_OUTPUT, "\n[+] CmdLine Args: %ws\n", commandLineBuffer);
    MSVCRT$free(commandLineBuffer);
    KERNEL32$CloseHandle(hProcess);

    return;
}
