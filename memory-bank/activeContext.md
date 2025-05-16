# Active Context: WoW 3.3.5a ROG Ally Controller Enhancement

## 1. Current Focus

*   **Phase 0: Project Initialization & Planning.**
    *   Defining project scope, goals, and core requirements.
    *   Establishing the technical approach (Hybrid DLL + Lua Addon).
    *   Designing the high-level UI/UX for a controller-first experience.
    *   Initializing the Memory Bank for ongoing project documentation and knowledge capture.

## 2. Recent Activities & Decisions

*   **Decision:** Adopted a hybrid approach: a C++ DLL for XInput controller reading, communicating with a comprehensive Lua addon that manages the UI and game logic for controller input.
*   **Decision:** Ruled out reliance on external mapping tools like Steam Input to provide a more integrated, streamlined solution.
*   **UI Design Direction:** Agreed on a UI philosophy inspired by modern console MMOs (e.g., FFXIV's Cross Hotbar, ESO's clarity), focusing on:
    *   Direct controller navigation for all elements.
    *   A centralized System Menu.
    *   Redesigned core game windows (Character, Inventory, Spellbook, Talents, Quest Log, Map, etc.) for controller usability and ROG Ally screen readability.
    *   A custom "Controller Setup" UI for intuitive in-game binding of actions to controller buttons/combinations.
*   **Initial Memory Bank Setup:** Created `projectbrief.md` and `productContext.md`.

## 3. Next Steps (Short-Term)

1.  **Complete Memory Bank Initialization:** Create `systemPatterns.md`, `techContext.md`, `progress.md`, and `cursorrules.md`.
2.  **Deep Dive into DLL-Lua Communication:** Specify the API between the C++ DLL and the Lua addon. How will controller states be exposed? How will Lua query this data?
3.  **Detailed UI Mockups/Wireframes:** Start creating more detailed visual mockups or wireframes for the key UI screens (e.g., XHB, Controller Setup, Inventory, Character Sheet).
4.  **Proof-of-Concept (PoC) Development:**
    *   Develop a basic C++ DLL that can read XInput and log controller states.
    *   Develop a minimal Lua addon that can communicate with the PoC DLL and display rudimentary controller input in the WoW chat window or a simple frame.

## 4. Active Considerations & Open Questions

*   **Specific XInput features to leverage:** Beyond basic button presses/analog sticks, consider trigger pressure sensitivity if useful, or vibration feedback.
*   **Error handling and stability:** How will the DLL and Lua addon handle potential errors, disconnections, or unexpected game states?
*   **Performance implications:** Ensure the DLL and frequent Lua updates do not negatively impact game performance.
*   **Talent Tree Navigation:** How to best adapt the visual talent tree for easy point allocation with a controller (e.g., cursor mode emulation vs. direct node snapping).
*   **Text Input Strategy:** While a custom on-screen keyboard via DLL is planned, further define its features and integration (e.g., for chat, character creation, macro editing). 