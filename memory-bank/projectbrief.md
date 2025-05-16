# Project Brief: WoW 3.3.5a ROG Ally Controller Enhancement

## 1. Project Goal

To significantly enhance the playability of World of Warcraft (specifically version 3.3.5a for AzerothCore) on the ROG Ally handheld device by implementing a comprehensive, intuitive, and seamless controller-first user experience. The aim is to make the game feel as if it were natively designed for controller input, minimizing clunkiness and the need for external configuration tools.

## 2. Scope

*   **Custom Controller Input System:** Develop a hybrid solution involving a C++ DLL for direct XInput controller reading and a Lua addon for UI and game logic integration.
*   **Revamped User Interface (UI):** Design and implement a full suite of controller-navigable UI screens for all essential game functions (character, inventory, spellbook, talents, quests, map, vendor, loot, social, etc.). This includes a Cross Hotbar (XHB) system inspired by modern console MMOs.
*   **Intuitive Binding Configuration:** Create a dedicated in-game UI for players to easily view available spells/items/macros and assign them to controller buttons/combinations without needing a keyboard and mouse.
*   **Streamlined Installation:** Package all enhancements (client, DLL, addon, pre-configured settings) into an installer for easy setup.
*   **Target Platform:** ROG Ally (Windows-based handheld with Xbox-style controller).
*   **Game Version:** World of Warcraft 3.3.5a (AzerothCore).

## 3. Key Objectives

*   Eliminate reliance on external controller mapping software (e.g., Steam Input, reWASD) for core functionality.
*   Provide a user interface that is clear, readable, and efficient on the ROG Ally's screen size.
*   Ensure all essential game interactions can be performed fluidly with a controller.
*   Simplify the process of customizing controller layouts for different abilities and items.
*   Create a "plug-and-play" experience through a unified installer.

## 4. Exclusions (Initial Scope)

*   Modifications to server-side AzerothCore code.
*   Advanced client-side modifications beyond the DLL and Lua addons (e.g., deep MPQ editing for core asset replacement, unless strictly necessary for UI functionality and agreed upon).
*   Support for other game versions or controller types beyond standard XInput.
*   Automated game update/patching mechanisms for the custom client. 