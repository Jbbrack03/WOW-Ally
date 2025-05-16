#include <windows.h>
#include <iostream>
#include <string>
#include <tlhelp32.h> // Required for process snapshotting

// Define a function pointer type for our exported DLL function
typedef void (*ProcessControllerInputFunc)();

// Function to check if Wow.exe is running
bool IsWowProcessRunning() {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        // Optionally, log an error or handle it. For now, assume not running.
        return false;
    }

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            // Compare process name (case-insensitive)
            if (_wcsicmp(pe32.szExeFile, L"Wow.exe") == 0) {
                CloseHandle(hProcessSnap);
                return true;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
    return false;
}

int main() {
    HINSTANCE hDll = NULL;
    ProcessControllerInputFunc processInput = NULL;
    bool dllLoaded = false;

    std::cout << "TestAllyInput: Waiting for Wow.exe to start..." << std::endl;

    while (true) { // Outer loop: monitors WoW process state
        if (IsWowProcessRunning()) {
            if (!dllLoaded) {
                hDll = LoadLibrary("AllyMapperPoC.dll");
                if (!hDll) {
                    std::cerr << "Could not load AllyMapperPoC.dll! Error code: " << GetLastError() << std::endl;
                    Sleep(5000); // Wait before retrying WoW check or DLL load
                    continue;    // Go back to checking if WoW is running
                }

                processInput = (ProcessControllerInputFunc)GetProcAddress(hDll, "ProcessControllerInputAndSendEvents");
                if (!processInput) {
                    std::cerr << "Could not find function ProcessControllerInputAndSendEvents in DLL! Error code: " << GetLastError() << std::endl;
                    FreeLibrary(hDll);
                    hDll = NULL;
                    Sleep(5000);
                    continue;
                }
                std::cout << "TestAllyInput: Wow.exe detected. AllyMapperPoC.dll loaded. Controller input active." << std::endl;
                dllLoaded = true;
            }

            // WoW is running and DLL is loaded, process input
            if (processInput) {
                processInput();
            }
            Sleep(16); // Approx 60 FPS polling

        } else { // Wow.exe is not running
            if (dllLoaded) {
                std::cout << "TestAllyInput: Wow.exe closed or not found. Deactivating controller input." << std::endl;
                if (hDll) {
                    FreeLibrary(hDll);
                    hDll = NULL;
                }
                processInput = NULL;
                dllLoaded = false;
            }
            // No need to print "waiting" every 5 seconds if it was already printed or just closed.
            // A single message when it first starts waiting and when WoW closes is enough.
            // However, to prevent a tight loop if it was already waiting, add a sleep.
            if (!dllLoaded) { // Only print waiting if we haven't just unloaded the DLL.
                 // To avoid spamming, let's only print if it was not running in the previous check.
                 // This needs a 'wasWowRunning' flag, or simply accept a small delay in printing "Waiting...".
                 // For simplicity, we'll just let it sleep. The main message is printed at startup.
            }
            Sleep(5000); // Check for Wow.exe every 5 seconds
        }
    }

    // Cleanup if the loop somehow exits (e.g., manual termination of TestAllyInput)
    // This part is less likely to be reached with the current infinite outer loop.
    if (hDll) {
        FreeLibrary(hDll);
    }

    return 0;
} 