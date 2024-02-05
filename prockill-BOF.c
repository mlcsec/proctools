#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include "beacon.h"

WINBASEAPI HANDLE WINAPI KERNEL32$OpenProcess (DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwProcessId);
WINBASEAPI BOOL WINAPI KERNEL32$TerminateProcess (HANDLE hProcess, UINT uExitCode);
WINBASEAPI WINBOOL WINAPI KERNEL32$CloseHandle (HANDLE hObject);

void go (char* args, int len) {

    datap parser;
    BeaconDataParse(&parser, args, len);

    int pid = BeaconDataInt(&parser);

    HANDLE processHandle = KERNEL32$OpenProcess(PROCESS_TERMINATE, FALSE, pid);

    if (processHandle != NULL) {
        if (KERNEL32$TerminateProcess(processHandle, 0)) {
            BeaconPrintf(CALLBACK_OUTPUT, "\n[+] PID: %lu TERMINATED", pid);
        } else {
            BeaconPrintf(CALLBACK_ERROR,"\nFailed to kill process");
        }

        KERNEL32$CloseHandle(processHandle);

    } else {
        BeaconPrintf(CALLBACK_ERROR, "\nFailed to open process");
    }
}
