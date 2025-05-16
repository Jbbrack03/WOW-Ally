#include <windows.h>
#include <iostream>
#include <string> // Required for std::string and std::getline

// Define a function pointer type for our exported DLL function
typedef void (*ProcessControllerInputFunc)();

int main() {
    HINSTANCE hDll = LoadLibrary("AllyMapperPoC.dll"); // Updated DLL name

    if (!hDll) {
        std::cerr << "Could not load AllyMapperPoC.dll! Error code: " << GetLastError() << std::endl;
        return 1;
    }

    // Resolve function address here
    ProcessControllerInputFunc processInput = (ProcessControllerInputFunc)GetProcAddress(hDll, "ProcessControllerInputAndSendEvents"); // Updated function name
    if (!processInput) {
        std::cerr << "Could not find function ProcessControllerInputAndSendEvents in DLL! Error code: " << GetLastError() << std::endl;
        FreeLibrary(hDll);
        return 1;
    }

    std::cout << "AllyMapperPoC.dll loaded successfully." << std::endl;
    std::cout << "The DLL will now attempt to translate controller input to keyboard/mouse events." << std::endl;
    std::cout << "Switch to a target application (e.g., Notepad, WoW)." << std::endl;
    std::cout << " - Controller 'A' button should send Spacebar." << std::endl;
    std::cout << " - Controller D-Pad Up should send 'W' key." << std::endl;
    std::cout << " - Controller Right Stick should move the mouse cursor." << std::endl;
    std::cout << "Press Enter in this window to process input once, or type 'q' and Enter to quit." << std::endl;
    std::cout << "For continuous processing, you'd typically call this in a loop (e.g., from a game engine or a dedicated thread)." << std::endl;

    std::string input_line;
    while (true) {
        // In a real scenario, processInput() would be called very frequently (e.g., every game frame or on a timer)
        // For this test app, we call it on demand or in a fast loop after a single Enter press.
        
        // To make it poll continuously after one Enter press (more like a real mapper):
        // std::cout << "Press Enter to START polling, then 'q' and Enter in this window to STOP." << std::endl;
        // std::getline(std::cin, input_line);
        // if (input_line == "q") break;
        // std::cout << "Polling started... switch to target window. Type 'q' and Enter here to stop." << std::endl;
        // while(true) {
        //    processInput();
        //    Sleep(16); // Poll roughly 60 times per second
        //    // Check for quit condition without blocking, e.g., PeekConsoleInput or a separate input thread.
        //    // For simplicity, this inner loop would need a more robust exit mechanism in a real app.
        //    // Or, just let it run and close the console window to stop.
        // }
        
        std::cout << "Press Enter to process controller input and send events (or q to quit): ";
        std::getline(std::cin, input_line);
        if (input_line == "q" || input_line == "Q") {
            break;
        }
        processInput(); // Call the function from the DLL
        // No direct output to check here, observe behavior in the target application.
    }
    
    FreeLibrary(hDll);
    std::cout << "DLL Unloaded. Exiting." << std::endl;

    return 0;
} 