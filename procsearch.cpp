#include <windows.h>
#include <iostream>
#include <vector>

bool IsPrintableASCII(char ch) {
    return ch >= 32 && ch <= 126;
}

void SearchStringInProcessMemory(DWORD processId, const char* searchString, DWORD range){ //std::vector<std::string>& foundStrings) {
    HANDLE processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);

    if (processHandle == NULL) {
        printf("Failed to open process with ID %lu. Error code: %lu\n", processId, GetLastError());
        return;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    MEMORY_BASIC_INFORMATION memInfo;
    std::vector<char> buffer;

    char* currentAddress = reinterpret_cast<char*>(sysInfo.lpMinimumApplicationAddress);

    while (VirtualQueryEx(processHandle, currentAddress, &memInfo, sizeof(memInfo)) == sizeof(memInfo)) {

        if (memInfo.State == MEM_COMMIT && memInfo.Protect != PAGE_NOACCESS) {
            buffer.resize(memInfo.RegionSize);

            SIZE_T bytesRead;
            if (ReadProcessMemory(processHandle, memInfo.BaseAddress, buffer.data(), memInfo.RegionSize, &bytesRead) && bytesRead > 0) {
                std::string pageContent(buffer.begin(), buffer.begin() + bytesRead);

                size_t found = pageContent.find(searchString);
                while (found != std::string::npos) {

                    // get X chars of printable characters around search string
                    size_t startPos = (found > range) ? found - range : 0;
                    size_t endPos = (found + strlen(searchString) + range < pageContent.length()) ? found + strlen(searchString) + range : pageContent.length();

                    // extract  substring from buffer around found position
                    std::string printableChars;
                    for (size_t i = startPos; i < endPos; ++i) {
                        if (IsPrintableASCII(pageContent[i])) {
                            printableChars += pageContent[i];
                        }
                    }

                    // save to array
                    //foundStrings.push_back(printableChars);
                    printf("[+] Search string \"%s\" FOUND: %s\n", searchString, printableChars.c_str());


                    // search next occurance
                    found = pageContent.find(searchString, found + 1);
                }
            }
            else {
                printf("[-] Failed to read process memory. Error code: %lu\n", GetLastError());
            }
        }

        // next mem region
        currentAddress = reinterpret_cast<char*>(memInfo.BaseAddress) + memInfo.RegionSize;
    }

    CloseHandle(processHandle);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: procsearch.exe <processId> <searchString> <range>\n");
        return 1;
    }

    DWORD targetProcessId = atoi(argv[1]);
    const char* searchString = argv[2];
    DWORD range = atoi(argv[3]);

    SearchStringInProcessMemory(targetProcessId, searchString, range);

    /*std::vector<std::string> foundStrings;
    SearchStringInProcessMemory(targetProcessId, searchString, range, foundStrings);

    for (const auto& str : foundStrings) {
        printf("[+] Search string \"%s\" FOUND: %s\n", searchString, str.c_str());
    }*/

    return 0;
}
