#include <windows.h>
#include <iostream>
#include <string>
#include <tlhelp32.h> // Required for process snapshotting
#include <vector>      // For storing DLL path as wchar_t
#include <filesystem>  // For getting full path

// Function to get Wow.exe PID
DWORD GetWowProcessId() {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) return 0;

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, L"Wow.exe") == 0) {
                CloseHandle(hProcessSnap);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);
    return 0;
}

// Gets the full path of the DLL, assuming it's in the same directory as the injector
std::wstring GetDllPath() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::filesystem::path injectorPath = buffer;
    return injectorPath.parent_path() / L"AllyMapperPoC.dll";
}

bool InjectDll(DWORD processId, const std::wstring& dllPath) {
    if (!std::filesystem::exists(dllPath)) {
        std::wcerr << L"Injector: AllyMapperPoC.dll not found at: " << dllPath << std::endl;
        return false;
    }

    HANDLE hWowProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processId);
    if (hWowProcess == NULL) {
        std::cerr << "Injector: Could not open Wow.exe process (PID: " << processId << "). Error: " << GetLastError() << std::endl;
        return false;
    }

    size_t dllPathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID pRemoteDllPath = VirtualAllocEx(hWowProcess, NULL, dllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteDllPath == NULL) {
        std::cerr << "Injector: Could not allocate memory in Wow.exe. Error: " << GetLastError() << std::endl;
        CloseHandle(hWowProcess);
        return false;
    }

    if (!WriteProcessMemory(hWowProcess, pRemoteDllPath, dllPath.c_str(), dllPathSize, NULL)) {
        std::cerr << "Injector: Could not write DLL path to Wow.exe memory. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hWowProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hWowProcess);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
    FARPROC pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");
    if (pLoadLibraryW == NULL) {
        std::cerr << "Injector: Could not get address of LoadLibraryW. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hWowProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hWowProcess);
        return false;
    }

    HANDLE hRemoteThread = CreateRemoteThread(hWowProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pRemoteDllPath, 0, NULL);
    if (hRemoteThread == NULL) {
        std::cerr << "Injector: Could not create remote thread in Wow.exe. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hWowProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hWowProcess);
        return false;
    }

    // Don't wait for remote thread indefinitely, but give it a moment. 
    // If LoadLibraryW hangs for some reason in the target, we don't want the injector to hang.
    // WaitForSingleObject(hRemoteThread, 2000); // Wait up to 2 seconds

    CloseHandle(hRemoteThread);
    VirtualFreeEx(hWowProcess, pRemoteDllPath, 0, MEM_RELEASE); 
    CloseHandle(hWowProcess);
    std::wcout << L"Injector: Successfully initiated injection of " << dllPath << L" into Wow.exe (PID: " << processId << L")" << std::endl;
    return true;
}

int main() {
    std::cout << "WoW Ally Injector: Starting. Will monitor for Wow.exe." << std::endl;
    std::cout << "This window can be minimized. Close it to stop the injector." << std::endl;
    std::wstring dllPath = GetDllPath();
    bool wowIsRunning = false;
    DWORD lastWowPid = 0;

    if (!std::filesystem::exists(dllPath)) {
        std::wcerr << L"CRITICAL ERROR: AllyMapperPoC.dll not found at expected location: " << dllPath << std::endl;
        std::wcerr << L"Injector cannot function and will exit. Ensure AllyMapperPoC.dll is in the same directory as this injector." << std::endl;
        Sleep(10000);
        return 1;
    }

    while (true) {
        DWORD currentWowPid = GetWowProcessId();

        if (currentWowPid != 0) { // Wow.exe is currently running
            if (!wowIsRunning || currentWowPid != lastWowPid) {
                // WoW just started, or PID changed (e.g. game restarted)
                std::cout << "Injector: Wow.exe detected (PID: " << currentWowPid << "). Attempting to inject DLL..." << std::endl;
                if (InjectDll(currentWowPid, dllPath)) {
                    std::cout << "Injector: DLL injected. Monitoring Wow.exe for exit." << std::endl;
                } else {
                    std::cout << "Injector: DLL injection failed. Will retry if Wow.exe restarts." << std::endl;
                }
                wowIsRunning = true;
                lastWowPid = currentWowPid;
            }
        } else { // Wow.exe is not currently running
            if (wowIsRunning) {
                // WoW just closed
                std::cout << "Injector: Wow.exe (PID: " << lastWowPid << ") closed. Waiting for it to start again." << std::endl;
                wowIsRunning = false;
                lastWowPid = 0;
            }
        }
        Sleep(3000); // Poll every 3 seconds
    }
    return 0; // Should not be reached
} 