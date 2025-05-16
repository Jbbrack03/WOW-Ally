#define WIN32_LEAN_AND_MEAN // Keep this for general Windows programming
#include <windows.h>
// Removed iostream as we are making it silent. For logging, we'd need a different approach.
#include <string>
#include <tlhelp32.h> 
#include <filesystem> 

// Function to get Wow.exe PID (remains largely the same, error reporting would change if logging was added)
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

std::wstring GetDllPath() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::filesystem::path injectorPath = buffer;
    return injectorPath.parent_path() / L"AllyMapperPoC.dll";
}

bool InjectDll(DWORD processId, const std::wstring& dllPath) {
    if (!std::filesystem::exists(dllPath)) {
        // Error: DLL not found. In a silent app, this would fail without user notification
        // unless we implement file logging or another error reporting mechanism.
        return false;
    }

    HANDLE hWowProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processId);
    if (hWowProcess == NULL) return false; // Error: Could not open process

    size_t dllPathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID pRemoteDllPath = VirtualAllocEx(hWowProcess, NULL, dllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteDllPath == NULL) {
        CloseHandle(hWowProcess);
        return false; // Error: Could not allocate memory
    }

    if (!WriteProcessMemory(hWowProcess, pRemoteDllPath, dllPath.c_str(), dllPathSize, NULL)) {
        VirtualFreeEx(hWowProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hWowProcess);
        return false; // Error: Could not write memory
    }

    HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
    FARPROC pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");
    if (pLoadLibraryW == NULL) {
        VirtualFreeEx(hWowProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hWowProcess);
        return false; // Error: Could not get LoadLibraryW address
    }

    HANDLE hRemoteThread = CreateRemoteThread(hWowProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pRemoteDllPath, 0, NULL);
    if (hRemoteThread == NULL) {
        VirtualFreeEx(hWowProcess, pRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hWowProcess);
        return false; // Error: Could not create remote thread
    }

    CloseHandle(hRemoteThread);
    VirtualFreeEx(hWowProcess, pRemoteDllPath, 0, MEM_RELEASE); 
    CloseHandle(hWowProcess);
    // Success: In a silent app, this just means it worked. No output.
    return true;
}

// Entry point for a Windows (non-console) application
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::wstring dllPath = GetDllPath();
    bool wowIsRunning = false;
    DWORD lastWowPid = 0;

    if (!std::filesystem::exists(dllPath)) {
        // CRITICAL ERROR: DLL not found. Injector cannot function.
        // In a silent app, this means it will just exit or fail to start properly.
        // For robustness, an installer should ensure the DLL is present.
        return 1; // Exit if DLL is critically missing
    }

    // Prevent multiple instances of the injector (optional but good practice)
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"WoWAllyInjectorMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Another instance is already running
        if(hMutex) CloseHandle(hMutex);
        return 0; // Exit silently
    }

    while (true) {
        DWORD currentWowPid = GetWowProcessId();

        if (currentWowPid != 0) { 
            if (!wowIsRunning || currentWowPid != lastWowPid) {
                InjectDll(currentWowPid, dllPath);
                // No console output for success/failure here in a silent app
                wowIsRunning = true;
                lastWowPid = currentWowPid;
            }
        } else { 
            if (wowIsRunning) {
                wowIsRunning = false;
                lastWowPid = 0;
            }
        }
        Sleep(3000); 
    }

    // Cleanup mutex if loop somehow exits (not expected in this design)
    if(hMutex) CloseHandle(hMutex);
    return 0; // Should not be reached
} 