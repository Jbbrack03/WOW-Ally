#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0500 // Important for SendInput and associated structures
#include <windows.h>
#include <XInput.h>
#include <vector> // Not strictly needed for current version, but good for potential future use
#include <process.h> // For _beginthreadex

// Link with XInput.lib and User32.lib
#pragma comment(lib, "XInput.lib")
#pragma comment(lib, "User32.lib")

// Deadzones and sensitivity
const SHORT ANALOG_STICK_DEADZONE_LEFT = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
const SHORT ANALOG_STICK_DEADZONE_RIGHT = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
const float STICK_NORMALIZATION_FACTOR = 32767.0f;
const float STICK_DEADZONE_FLOAT_LEFT = static_cast<float>(ANALOG_STICK_DEADZONE_LEFT) / STICK_NORMALIZATION_FACTOR;
const float STICK_DEADZONE_FLOAT_RIGHT = static_cast<float>(ANALOG_STICK_DEADZONE_RIGHT) / STICK_NORMALIZATION_FACTOR;
const float MOUSE_SENSITIVITY = 15.0f; // Adjusted sensitivity

// Global state
XINPUT_STATE g_prevState = {0};
bool g_prevControllerConnected = false;
HANDLE g_hInputThread = NULL;
volatile bool g_bRunInputThread = false; // Volatile as it's accessed by multiple threads

// Helper to send key events (press or release) using scan codes
void SendScanKeyEvent(WORD virtualKeyCode, bool press) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = static_cast<WORD>(MapVirtualKey(virtualKeyCode, MAPVK_VK_TO_VSC));
    input.ki.dwFlags = KEYEVENTF_SCANCODE | (press ? 0 : KEYEVENTF_KEYUP);
    if (input.ki.wScan == 0) { // Fallback if scancode is 0 (e.g. for some special keys if not mapped correctly)
        input.ki.wVk = virtualKeyCode;
        input.ki.dwFlags = press ? 0 : KEYEVENTF_KEYUP;
    }
    SendInput(1, &input, sizeof(INPUT));
}

// Helper to send mouse button events
void SendMouseButtonEvent(DWORD eventFlag) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = eventFlag;
    SendInput(1, &input, sizeof(INPUT));
}

// Helper to send mouse movement
void SendMouseMove(long dx, long dy) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dx = dx;
    input.mi.dy = dy;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}

// Helper to manage button state and send key events
void ManageButtonState(WORD gamepadButton, WORD virtualKey, const XINPUT_STATE& current, const XINPUT_STATE& prev) {
    bool currentPressed = (current.Gamepad.wButtons & gamepadButton);
    bool prevPressed = (prev.Gamepad.wButtons & gamepadButton);
    if (currentPressed && !prevPressed) SendScanKeyEvent(virtualKey, true);
    else if (!currentPressed && prevPressed) SendScanKeyEvent(virtualKey, false);
}

// Helper to manage stick to key conversion for movement (W,A,S,D)
void ManageStickToKey(SHORT stickVal, SHORT deadzone, WORD positiveKey, WORD negativeKey, const XINPUT_STATE& current, const XINPUT_STATE& prev, bool isYAxis) {
    // Determine previous state for positive and negative keys based on previous stick value
    // This is a simplification; true prev key state would need more tracking if other inputs could press these keys.
    bool prevPositiveKeyPressed = false;
    bool prevNegativeKeyPressed = false;
    if (isYAxis) {
        prevPositiveKeyPressed = prev.Gamepad.sThumbLY > deadzone;
        prevNegativeKeyPressed = prev.Gamepad.sThumbLY < -deadzone;
    } else {
        prevPositiveKeyPressed = prev.Gamepad.sThumbLX > deadzone;
        prevNegativeKeyPressed = prev.Gamepad.sThumbLX < -deadzone;
    }

    if (stickVal > deadzone) { // Positive direction (e.g., W or D)
        if (!prevPositiveKeyPressed) SendScanKeyEvent(positiveKey, true); // Press if wasn't pressed
        if (prevNegativeKeyPressed) SendScanKeyEvent(negativeKey, false); // Release opposite if it was pressed
    } else if (stickVal < -deadzone) { // Negative direction (e.g., S or A)
        if (!prevNegativeKeyPressed) SendScanKeyEvent(negativeKey, true); // Press if wasn't pressed
        if (prevPositiveKeyPressed) SendScanKeyEvent(positiveKey, false); // Release opposite if it was pressed
    } else { // Stick is in deadzone for this axis
        if (prevPositiveKeyPressed) SendScanKeyEvent(positiveKey, false); // Release if was pressed
        if (prevNegativeKeyPressed) SendScanKeyEvent(negativeKey, false); // Release if was pressed
    }
}

// Renamed and no longer exported - this is the core input loop run by its own thread
unsigned int __stdcall ControllerInputProcessingThread(void* pArguments) {
    while (g_bRunInputThread) {
        XINPUT_STATE currentState = {0};
        DWORD dwResult = XInputGetState(0, &currentState);
        bool currentControllerConnected = (dwResult == ERROR_SUCCESS);

        if (currentControllerConnected) {
            // Face Buttons
            ManageButtonState(XINPUT_GAMEPAD_A, VK_F11, currentState, g_prevState);       // A -> F11 (CP_R_DOWN)
            ManageButtonState(XINPUT_GAMEPAD_B, VK_F10, currentState, g_prevState);       // B -> F10 (CP_R_RIGHT)
            ManageButtonState(XINPUT_GAMEPAD_X, VK_F12, currentState, g_prevState);       // X -> F12 (CP_R_LEFT)
            ManageButtonState(XINPUT_GAMEPAD_Y, VK_F9, currentState, g_prevState);        // Y -> F9  (CP_R_UP)

            // D-Pad (Updated based on ConsolePort.lua)
            ManageButtonState(XINPUT_GAMEPAD_DPAD_UP, VK_F1, currentState, g_prevState);    // D-Pad Up    -> F1 (CP_L_UP)
            ManageButtonState(XINPUT_GAMEPAD_DPAD_DOWN, VK_F3, currentState, g_prevState);  // D-Pad Down  -> F3 (CP_L_DOWN)
            ManageButtonState(XINPUT_GAMEPAD_DPAD_LEFT, VK_F4, currentState, g_prevState);  // D-Pad Left  -> F4 (CP_L_LEFT)
            ManageButtonState(XINPUT_GAMEPAD_DPAD_RIGHT, VK_F2, currentState, g_prevState); // D-Pad Right -> F2 (CP_L_RIGHT)

            // Shoulder Buttons
            ManageButtonState(XINPUT_GAMEPAD_LEFT_SHOULDER, VK_F7, currentState, g_prevState);  // LB -> F7 (CP_T1)
            ManageButtonState(XINPUT_GAMEPAD_RIGHT_SHOULDER, VK_F8, currentState, g_prevState); // RB -> F8

            // Triggers as Modifiers
            // Note: Direct XINPUT_GAMEPAD_LEFT_TRIGGER in ManageButtonState won't work as it's not a bitmask. 
            // A refined ManageButtonState for triggers or direct handling is needed.
            // For now, this is a placeholder logic for triggers and will need fixing.
            // Corrected Trigger Logic:
            bool ltPressed = currentState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
            bool prevLtPressed = g_prevState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
            if (ltPressed && !prevLtPressed) SendScanKeyEvent(VK_LSHIFT, true);
            else if (!ltPressed && prevLtPressed) SendScanKeyEvent(VK_LSHIFT, false);

            bool rtPressed = currentState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
            bool prevRtPressed = g_prevState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
            if (rtPressed && !prevRtPressed) SendScanKeyEvent(VK_LCONTROL, true);
            else if (!rtPressed && prevRtPressed) SendScanKeyEvent(VK_LCONTROL, false);

            // Start/Back Buttons
            ManageButtonState(XINPUT_GAMEPAD_START, VK_F6, currentState, g_prevState);      // Start -> F6 (CP_X_RIGHT)
            ManageButtonState(XINPUT_GAMEPAD_BACK, VK_F5, currentState, g_prevState);       // Back  -> F5 (CP_X_LEFT)

            // Stick Clicks (Reverted to Mouse Buttons as per user request)
            bool lStickClick = (currentState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            bool prevLStickClick = (g_prevState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            if (lStickClick && !prevLStickClick) SendMouseButtonEvent(MOUSEEVENTF_LEFTDOWN);    // L3 -> Left Mouse Click
            else if (!lStickClick && prevLStickClick) SendMouseButtonEvent(MOUSEEVENTF_LEFTUP);

            bool rStickClick = (currentState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
            bool prevRStickClick = (g_prevState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
            if (rStickClick && !prevRStickClick) SendMouseButtonEvent(MOUSEEVENTF_RIGHTDOWN);  // R3 -> Right Mouse Click
            else if (!rStickClick && prevRStickClick) SendMouseButtonEvent(MOUSEEVENTF_RIGHTUP);
            
            // Left Analog Stick for Movement (W, A, S, D)
            ManageStickToKey(currentState.Gamepad.sThumbLY, ANALOG_STICK_DEADZONE_LEFT, 'W', 'S', currentState, g_prevState, true); // Y-axis: W/S
            ManageStickToKey(currentState.Gamepad.sThumbLX, ANALOG_STICK_DEADZONE_LEFT, 'D', 'A', currentState, g_prevState, false); // X-axis: D/A

            // Right Analog Stick for Mouse Movement
            float normRX = 0.0f, normRY = 0.0f;
            if (abs(currentState.Gamepad.sThumbRX) > ANALOG_STICK_DEADZONE_RIGHT) {
                normRX = static_cast<float>(currentState.Gamepad.sThumbRX) / STICK_NORMALIZATION_FACTOR;
                if (normRX > 0) normRX = (normRX - STICK_DEADZONE_FLOAT_RIGHT) / (1.0f - STICK_DEADZONE_FLOAT_RIGHT); else normRX = (normRX + STICK_DEADZONE_FLOAT_RIGHT) / (1.0f - STICK_DEADZONE_FLOAT_RIGHT);
            }
            if (abs(currentState.Gamepad.sThumbRY) > ANALOG_STICK_DEADZONE_RIGHT) {
                normRY = static_cast<float>(currentState.Gamepad.sThumbRY) / STICK_NORMALIZATION_FACTOR;
                if (normRY > 0) normRY = (normRY - STICK_DEADZONE_FLOAT_RIGHT) / (1.0f - STICK_DEADZONE_FLOAT_RIGHT); else normRY = (normRY + STICK_DEADZONE_FLOAT_RIGHT) / (1.0f - STICK_DEADZONE_FLOAT_RIGHT);
                normRY = -normRY; // Invert Y for mouse
            }
            if (normRX != 0.0f || normRY != 0.0f) {
                SendMouseMove(static_cast<long>(normRX * MOUSE_SENSITIVITY), static_cast<long>(normRY * MOUSE_SENSITIVITY));
            }

            g_prevState = currentState;
        } else if (g_prevControllerConnected) {
            // Controller disconnected: release all potentially held keys/buttons
            SendScanKeyEvent(VK_F11, false); SendScanKeyEvent(VK_F10, false); SendScanKeyEvent(VK_F12, false); SendScanKeyEvent(VK_F9, false);
            SendScanKeyEvent(VK_F1, false); SendScanKeyEvent(VK_F3, false); SendScanKeyEvent(VK_F4, false); SendScanKeyEvent(VK_F2, false); // D-Pad F-keys
            SendScanKeyEvent(VK_F7, false); SendScanKeyEvent(VK_F8, false); SendScanKeyEvent(VK_LSHIFT, false); SendScanKeyEvent(VK_LCONTROL, false);
            SendScanKeyEvent(VK_F6, false); SendScanKeyEvent(VK_F5, false);
            // SendScanKeyEvent(VK_DIVIDE, false); SendScanKeyEvent(VK_MULTIPLY, false); // No longer Numpad keys for stick clicks
            // Ensure mouse buttons are up if they were pressed by stick clicks
            if (g_prevState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) SendMouseButtonEvent(MOUSEEVENTF_LEFTUP);
            if (g_prevState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) SendMouseButtonEvent(MOUSEEVENTF_RIGHTUP);
            SendScanKeyEvent('W', false); SendScanKeyEvent('A', false); SendScanKeyEvent('S', false); SendScanKeyEvent('D', false);
            ZeroMemory(&g_prevState, sizeof(XINPUT_STATE));
        }
        g_prevControllerConnected = currentControllerConnected;
        Sleep(16); // Approx 60 FPS polling
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule); // Optimization: not expecting DLL_THREAD_ATTACH/DETACH
        ZeroMemory(&g_prevState, sizeof(XINPUT_STATE));
        {
             XINPUT_STATE initState;
             g_prevControllerConnected = (XInputGetState(0, &initState) == ERROR_SUCCESS);
             if(g_prevControllerConnected) g_prevState = initState;
        }
        g_bRunInputThread = true;
        g_hInputThread = (HANDLE)_beginthreadex(NULL, 0, &ControllerInputProcessingThread, NULL, 0, NULL);
        if (g_hInputThread == NULL) {
            // Failed to create thread, handle error (e.g., log, but can't do much else from DllMain)
            return FALSE; // Prevent DLL from loading if thread creation fails
        }
        break;
    case DLL_PROCESS_DETACH:
        g_bRunInputThread = false;
        if (g_hInputThread != NULL) {
            WaitForSingleObject(g_hInputThread, INFINITE); // Wait for thread to finish
            CloseHandle(g_hInputThread);
            g_hInputThread = NULL;
        }
        // Release any held keys if the process is detaching (e.g. WoW closing)
        // This is a simplified version, assumes g_prevState reflects the last "active" input state.
        if (g_prevControllerConnected) { // Only if controller was connected before detaching
            SendScanKeyEvent(VK_F11, false); SendScanKeyEvent(VK_F10, false); SendScanKeyEvent(VK_F12, false); SendScanKeyEvent(VK_F9, false);
            SendScanKeyEvent(VK_F1, false); SendScanKeyEvent(VK_F3, false); SendScanKeyEvent(VK_F4, false); SendScanKeyEvent(VK_F2, false); 
            SendScanKeyEvent(VK_F7, false); SendScanKeyEvent(VK_F8, false); SendScanKeyEvent(VK_LSHIFT, false); SendScanKeyEvent(VK_LCONTROL, false);
            SendScanKeyEvent(VK_F6, false); SendScanKeyEvent(VK_F5, false);
            if (g_prevState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) SendMouseButtonEvent(MOUSEEVENTF_LEFTUP);
            if (g_prevState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) SendMouseButtonEvent(MOUSEEVENTF_RIGHTUP);
            SendScanKeyEvent('W', false); SendScanKeyEvent('A', false); SendScanKeyEvent('S', false); SendScanKeyEvent('D', false);
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
} 