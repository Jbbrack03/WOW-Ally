# Tech Context: WoW 3.3.5a ROG Ally Controller Enhancement

## 1. Core Technologies

*   **Game Client:** World of Warcraft 3.3.5a (WotLK)
    *   **Client Base:** AzerothCore (private server context implied)
*   **Operating System:** Windows (specifically for ROG Ally)
*   **Controller Input API:** Microsoft XInput API (for Xbox-style controllers)
*   **DLL Development:** C++
    *   **Compiler/Toolchain:** MinGW or Visual Studio (to be decided, MinGW might be simpler for standalone DLLs if not heavily reliant on MS-specific libraries beyond XInput).
    *   **Key Libraries:** `XInput.h` (standard part of Windows SDK).
*   **Addon Development:** Lua 5.1 (as used by WoW 3.3.5a client)
    *   **WoW API:** Standard WoW 3.3.5a Lua API functions and XML for UI layout.
*   **Installer Technology (Phase 2):** To be determined. Options include:
    *   Simple batch/PowerShell scripts for file copying.
    *   Dedicated installer software like Inno Setup or NSIS for a more polished user experience.

## 2. Development Environment & Setup

*   **WoW 3.3.5a Client:** A clean, working client installation for testing.
*   **AzerothCore Server:** Access to a local or remote AzerothCore server for development and testing (assuming self-hosted setup by user).
*   **C++ Development Environment:**
    *   IDE: Visual Studio Code, Visual Studio Community/Professional, or other C++ capable IDE.
    *   Compiler and Linker setup for creating a 32-bit DLL compatible with the WoW 3.3.5a client.
*   **Lua Development Environment:**
    *   Text editor with Lua syntax highlighting (e.g., VS Code with Lua extensions).
    *   WoW Addon development tools/helpers (e.g., WoW Addon Studio, BigWigs Packager, or manual setup).
*   **Version Control:** Git (recommended).
*   **Target Hardware for Testing:** ROG Ally device.

## 3. Key Technical Challenges & Considerations

*   **DLL Injection Method:** Determining the safest and most reliable method for loading the custom DLL into the `WoW.exe` process (e.g., standard DLL injection techniques, potentially a custom launcher if necessary, though aiming for direct integration).
*   **DLL-Lua Communication Interface:**
    *   Defining a robust and efficient API for Lua to query controller states from the DLL.
    *   This might involve Lua CFunctions exposed by the DLL or a shared memory approach (though the former is likely simpler and safer to start).
    *   Ensuring data types are correctly marshaled between C++ and Lua.
*   **Performance:**
    *   The DLL's input polling and the Lua addon's frequent checks must be highly performant to avoid any noticeable input lag or FPS drops.
    *   Optimizing Lua code, especially in `OnUpdate` handlers.
*   **Stability & Error Handling:**
    *   Robust error handling in both the DLL and Lua addon to prevent client crashes.
    *   Handling controller disconnection/reconnection gracefully.
*   **WoW API Limitations:** Working within the constraints of the 3.3.5a Lua API for UI manipulation and action execution.
*   **UI Asset Creation/Modification:** If custom UI graphics are needed beyond what XML/Lua can style, this might involve creating/editing BLP (Blizzard Picture) files, which adds another layer of complexity.
*   **Client Compatibility:** Ensuring the solution works with standard 3.3.5a clients and doesn't rely on non-standard client modifications (beyond our DLL).
*   **32-bit vs 64-bit:** WoW 3.3.5a is a 32-bit application, so the DLL must be compiled as 32-bit.

## 4. Dependencies

*   **External (for users):** None ideally, beyond the provided installer package. The goal is an all-in-one solution.
*   **Development:**
    *   Windows SDK (for XInput).
    *   C++ compiler/linker.
    *   WoW 3.3.5a client for testing. 