#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <XInput.h>
#include <string>
#include <fstream> // For PoC logging to file
#include <iomanip> // For formatting output

// Link with XInput.lib
#pragma comment(lib, "XInput.lib")

// Deadzone for analog sticks (e.g., 20%)
const float ANALOG_STICK_DEADZONE = 0.20f;

// Helper function to normalize and apply deadzone to stick values
float NormalizeStickValue(SHORT value, SHORT deadzoneThreshold) {
    float normalized = static_cast<float>(value) / 32767.0f;
    if (abs(value) < deadzoneThreshold) {
        return 0.0f;
    }
    // Rescale to full range after deadzone
    if (normalized > 0) {
        normalized = (normalized - ANALOG_STICK_DEADZONE) / (1.0f - ANALOG_STICK_DEADZONE);
    } else {
        normalized = (normalized + ANALOG_STICK_DEADZONE) / (1.0f - ANALOG_STICK_DEADZONE);
    }
    // Clamp to [-1.0, 1.0] in case of any floating point inaccuracies
    if (normalized > 1.0f) normalized = 1.0f;
    if (normalized < -1.0f) normalized = -1.0f;
    return normalized;
}

// Function to be exported for PoC testing (writes to file)
extern "C" __declspec(dllexport) void CheckControllerStateToFile() {
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    // Simply get the state of the first controller
    DWORD dwResult = XInputGetState(0, &state);

    std::ofstream logFile("controller_state.txt", std::ios_base::app);
    logFile << std::fixed << std::setprecision(3);

    if (dwResult == ERROR_SUCCESS) {
        logFile << "Controller connected." << std::endl;
        logFile << "  Packet: " << state.dwPacketNumber << std::endl;

        // Buttons
        logFile << "  Buttons:" << std::endl;
        logFile << "    DPAD_UP: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? "1" : "0") << std::endl;
        logFile << "    DPAD_DOWN: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? "1" : "0") << std::endl;
        logFile << "    DPAD_LEFT: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? "1" : "0") << std::endl;
        logFile << "    DPAD_RIGHT: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? "1" : "0") << std::endl;
        logFile << "    START: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? "1" : "0") << std::endl;
        logFile << "    BACK: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? "1" : "0") << std::endl;
        logFile << "    LEFT_THUMB: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? "1" : "0") << std::endl;
        logFile << "    RIGHT_THUMB: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? "1" : "0") << std::endl;
        logFile << "    LEFT_SHOULDER: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? "1" : "0") << std::endl;
        logFile << "    RIGHT_SHOULDER: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? "1" : "0") << std::endl;
        logFile << "    A: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? "1" : "0") << std::endl;
        logFile << "    B: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? "1" : "0") << std::endl;
        logFile << "    X: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? "1" : "0") << std::endl;
        logFile << "    Y: " << ((state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? "1" : "0") << std::endl;

        // Triggers
        float leftTrigger = static_cast<float>(state.Gamepad.bLeftTrigger) / 255.0f;
        float rightTrigger = static_cast<float>(state.Gamepad.bRightTrigger) / 255.0f;
        logFile << "  Triggers:" << std::endl;
        logFile << "    Left: " << leftTrigger << std::endl;
        logFile << "    Right: " << rightTrigger << std::endl;

        // Sticks
        SHORT sThumbLX = state.Gamepad.sThumbLX;
        SHORT sThumbLY = state.Gamepad.sThumbLY;
        SHORT sThumbRX = state.Gamepad.sThumbRX;
        SHORT sThumbRY = state.Gamepad.sThumbRY;
        
        // Apply deadzone. XInput constants define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, etc.
        // For simplicity in PoC, we use a symmetrical deadzone calculation for values already normalized.
        // A more precise deadzone calculation would directly use XInput's defined deadzone constants like XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE.
        // The XInput deadzone is 7849 for left stick, 8689 for right stick.
        
        float normLX = NormalizeStickValue(sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        float normLY = NormalizeStickValue(sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE); // Using same deadzone for Y
        float normRX = NormalizeStickValue(sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        float normRY = NormalizeStickValue(sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE); // Using same deadzone for Y


        logFile << "  Sticks (Normalized & Deadzoned):" << std::endl;
        logFile << "    Left X: " << normLX << " Y: " << normLY << std::endl;
        logFile << "    Right X: " << normRX << " Y: " << normRY << std::endl;
        logFile << "------------------------------------" << std::endl;

    } else {
        logFile << "Controller not connected." << std::endl;
        logFile << "------------------------------------" << std::endl;
    }
    logFile.close();
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Code to run when the DLL is loaded
        {
            std::ofstream logFile("controller_state.txt", std::ios_base::app);
            logFile << "AllyInputPoC.dll Loaded." << std::endl;
            logFile.close();
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        // Code to run when the DLL is unloaded
        {
            std::ofstream logFile("controller_state.txt", std::ios_base::app);
            logFile << "AllyInputPoC.dll Unloaded." << std::endl;
            logFile.close();
        }
        break;
    }
    return TRUE;
} 