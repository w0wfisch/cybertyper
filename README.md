# cybertyper v0.0.1

**cybertyper** is a minimalistic, terminal-based environment designed for navigating directories and editing files through a purely keyboard-driven interface. By leveraging a Hardware Abstraction Layer (HAL), it simulates file operations and user input handling, making it easy to explore how file systems and text editing could function in a streamlined, extensible environment. Initially conceived as a proof-of-concept, **cybertyper** aims to provide a solid foundation for experimenting with UI rendering, state management, and HAL integration.

**Version:** 0.0.1  
**Status:** Proof-of-concept / Early development

## Overview

cybertyper currently allows you to:  
- Browse directories and navigate through multiple column views.  
- Create and rename files and folders.  
- Enter an editing mode to modify file contents directly.  
- Interact with the application via keyboard input.

The codebase is minimal and relies on a mock HAL implementation and a simple core loop. The UI is text-based, with display updates and navigation handled in the main loop.

## Project Structure

- **hal_interface.h** and **hal_mock.c**  
  Provide a Hardware Abstraction Layer for storage operations and, in this early version, mock out hardware interactions.
  - **hal_interface.h:** Declares functions for listing files, reading/writing files, and other platform-agnostic I/O operations.
  - **hal_mock.c:** Implements the HAL functions in a mock manner, simulating file and directory behaviors in memory for testing and demonstration.
  
- **cybertyper_core.c** and **cybertyper_core.h**  
  Contain the main application logic and state management.
  - **cybertyper_core.c:** Implements initialization, state handling, directory loading, file editing logic, and UI display updates. It forms the “core” of the application loop and manages modes like normal, rename, new file/folder, and editing.
  - **cybertyper_core.h:** Exposes the main lifecycle functions (`cybertyper_init` and `cybertyper_run_cycle`).
  
- **main.c**  
  The entry point of the application.
  - Calls `cybertyper_init` to set up the state and then enters a loop calling `cybertyper_run_cycle` periodically.
  - Prints instructions on how to interact and updates the screen continuously until the user terminates the program (e.g., with Ctrl+C).

## Building and Running

**Prerequisites:**  
- A C compiler (e.g., gcc or clang).
- A Unix-like environment (for `usleep` and terminal-based interaction).

**Build Steps:**  
1. Place the code in a working directory.  
2. Compile using a C compiler.  
3. Run the resulting executable.

**Interaction:**  
- **Navigation:** Use arrow keys to move through directories and files.  
- **Enter Key:** Select files/folders or initiate rename/new file/folder modes.  
- **Typing Keys:** In editing or input modes, typed characters modify file names or contents.  
- **Ctrl+C:** Exit the application at any time.

## Current Limitations & Future Improvements

**Limitations:**  
- The UI is rudimentary and purely text-based.  
- Editing mode has a fixed-size buffer and lacks line wrapping or large file handling.  
- Error handling, configuration, and HAL implementations are minimal.

**Planned Improvements:**  
- **Refactoring & Modularization:** Move UI rendering, navigation logic, and editor logic into separate modules.  
- **Commenting & Documentation:** Improve code-level documentation and inline comments.  
- **Error Handling & State Management:** Introduce better handling of HAL failures, invalid inputs, and robust state machines.  
- **Enhanced Features:** Search functionality, scrollable file content editing, larger file handling, and configurable timing/UI layouts.

## Versioning

**v0.0.1** is a preliminary version showcasing basic navigation, editing, and renaming capabilities. Future versions will add more functionality, better modularization, and an improved user experience.

## Contributing & Feedback

Feedback on code structure, commenting style, and feature requests are welcome. The focus is on building a solid foundation for modularity, readability, and maintainability. Submit issues, pull requests, or comments if you have suggestions or improvements.

**Thank you for trying out cybertyper v0.0.1!**