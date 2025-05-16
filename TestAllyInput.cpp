#include <windows.h>
#include <iostream>
#include <string> // Still useful for potential future input, though not strictly for the loop

// Define a function pointer type for our exported DLL function
typedef void (*ProcessControllerInputFunc)();

int main() {
    HINSTANCE hDll = LoadLibrary("AllyMapperPoC.dll");

    if (!hDll) {
        std::cerr << "Could not load AllyMapperPoC.dll! Error code: " << GetLastError() << std::endl;
        return 1;
    }

    ProcessControllerInputFunc processInput = (ProcessControllerInputFunc)GetProcAddress(hDll, "ProcessControllerInputAndSendEvents");
    if (!processInput) {
        std::cerr << "Could not find function ProcessControllerInputAndSendEvents in DLL! Error code: " << GetLastError() << std::endl;
        FreeLibrary(hDll);
        return 1;
    }

    std::cout << "AllyMapperPoC.dll loaded successfully." << std::endl;
    std::cout << "DLL will now continuously translate controller input to keyboard/mouse events." << std::endl;
    std::cout << "Switch to your target application (e.g., Notepad, WoW)." << std::endl;
    std::cout << "Press Ctrl+C in this window or close it to quit." << std::endl;
    std::cout << "Expected basic mappings for testing:" << std::endl;
    std::cout << "  - Left Stick: W, A, S, D" << std::endl;
    std::cout << "  - Right Stick: Mouse Movement" << std::endl;
    std::cout << "  - A Button (Xbox): F11 key" << std::endl;
    std::cout << "  - D-Pad Up: F1 key" << std::endl;
    std::cout << "  - Left Trigger: Left Shift (modifier)" << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;

    // Continuous polling loop
    while (true) {
        processInput(); // Call the function from the DLL
        Sleep(16);      // Poll roughly 60 times per second (1000ms / 60hz ~= 16ms)
                        // Adjust timing if needed (e.g., Sleep(33) for ~30hz)
        
        // To make this truly robust for a real application, you might add
        // a way to signal this loop to terminate gracefully, e.g., by checking
        // a global flag that could be set by another thread or a console input handler.
        // For this PoC, Ctrl+C in the console or closing the window is the way to stop.
    }

    // The following lines will not be reached in this simple continuous loop model
    // unless the loop is broken by some other means (e.g. an error inside processInput if it could terminate the app).
    // In a more complete app, you'd ensure FreeLibrary is called on exit.
    // FreeLibrary(hDll); 
    // std::cout << "DLL Unloaded. Exiting." << std::endl;

    return 0; 
} 