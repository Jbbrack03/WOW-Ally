# Cursor Rules: WoW 3.3.5a ROG Ally Controller Enhancement

## 1. Project Knowledge & Guiding Principles

*   **User-Centric Design:** The primary goal is a seamless and intuitive controller experience for the ROG Ally user. Every design and technical decision must prioritize this.
*   **Minimize External Dependencies:** Avoid solutions that require users to install or configure external software (like Steam Input, reWASD). The aim is an all-in-one integrated package.
*   **Consistency is Key:** UI navigation, button prompts, and interaction paradigms should be consistent across all custom UI elements.
*   **Performance Matters:** The solution must not introduce noticeable input lag or a decrease in game FPS.
*   **Clarity and Readability:** UI elements must be clear and easily readable on the ROG Ally's screen size.
*   **Modularity (DLL & Addon):** The C++ DLL should focus solely on raw input capture and providing a simple API. The Lua addon handles UI, game logic, and bindings. This separation aids development and maintenance.
*   **Leverage Existing WoW API:** Utilize the standard WoW 3.3.5a Lua API for game interactions wherever possible, rather than attempting more complex/fragile direct memory manipulation for game actions.

## 2. Important Patterns (As Established)

*   **Hybrid DLL + Lua Addon Architecture:** This is the core technical pattern for achieving deep controller integration.
*   **Cross Hotbar (XHB):** The chosen paradigm for primary combat and action input, inspired by FFXIV.
*   **Visual Controller Configuration UI:** The method for users to intuitively bind actions to controller buttons.
*   **Centralized System Menu:** Single point of entry for major game functions and our addon's configuration.
*   **Tabbed Navigation (LB/RB):** Standard for complex windows.
*   **Contextual Interact Prompts:** For simplified world interaction.

## 3. Important Preferences (User & Project)

*   **No Steam Input:** User explicitly requested avoiding Steam as a dependency.
*   **Integrated Solution:** Preference for a single installer package that sets up everything needed.
*   **Focus on 3.3.5a & ROG Ally:** The project is tightly scoped to this game version and hardware.
*   **Iterative Development:** PoC first, then core UI, then full features, then packaging.

## 4. Key Terminology / Concepts

*   **DLL:** Dynamic Link Library (our C++ XInput reader).
*   **Lua Addon:** Our WoW addon handling UI and controller logic.
*   **XHB:** Cross Hotbar.
*   **ROG Ally:** Target handheld PC device.
*   **XInput:** Microsoft API for controller input.
*   **Binding UI:** The custom in-game screen for assigning actions to controller buttons.

## 5. Evolution of Project Decisions (Future Entries)

*   *(This section will be updated as significant decisions are made, or if initial assumptions/plans change based on development findings.)*
    *   **Example ( hypothetical future entry):** "Decision made to use a specific Lua CFunction binding library for DLL-Lua communication due to performance benefits observed during PoC."

## 6. Tool Usage Patterns (Anticipated)

*   **IDE for C++:** (To be decided - e.g., Visual Studio, VS Code with C++ tools)
*   **IDE/Editor for Lua:** (e.g., VS Code with Lua extensions)
*   **Version Control:** Git, with regular commits and meaningful messages.
*   **WoW Addon Development Tools:** (e.g., packagers, TOC generators, or manual file management).
*   **Debugging:** In-game Lua error reporting, C++ debugger for the DLL, `print()` statements for quick checks.

## 7. Memory Bank Maintenance

*   These files MUST be read at the start of every session.
*   Update the Memory Bank, especially `activeContext.md` and `progress.md`, after significant changes or new discoveries.
*   `cursorrules.md` should be updated when new project-specific patterns, preferences, or critical insights are identified. 