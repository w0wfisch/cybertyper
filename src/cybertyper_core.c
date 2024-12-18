#include "cybertyper_core.h"
#include "hal_interface.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>      // Added for timing functions

#define MAX_FILES 50
#define MAX_FILENAME_LEN 64
#define MAX_PATH_LEN 512
#define INPUT_BUFFER_SIZE 128
#define MAX_FILE_CONTENT_SIZE 1024
#define MAX_COLUMNS 10


//File Explorer

// Is a single column of the File Explorer
typedef struct {
    char directory[MAX_PATH_LEN];                // Current directory path
    char file_list[MAX_FILES][MAX_FILENAME_LEN]; // List of files/folders in the directory
    size_t file_count;                           // Number of files/folders
    size_t selected_index;                       // Currently selected index within the directory
} DirectoryColumn;

// Global arrays and counters manage multiple columns for navigation.
static DirectoryColumn columns[MAX_COLUMNS];
static size_t column_count = 1;      // Start with root directory
static size_t focused_column = 0;    // Currently focused column (0 = leftmost)

//Editor-related globals.
/*Improvement: 
Move editor state into a separate editor-focused module. Provide functions to initialize,
load, save, and manipulate files. This keeps core.c
smaller and more focused.*/
static char edit_filename[MAX_FILENAME_LEN];
static char edit_buffer[MAX_FILE_CONTENT_SIZE];
static size_t edit_length = 0;
static size_t edit_cursor_pos = 0; // Position within edit_buffer

//Application state machine.
/*Improvement:
Consider a dedicated state machine source file or a well-documented finite state machine.
Add comments explaining what each state means and what transitions are possible.*/
typedef enum {
    STATE_NORMAL,
    STATE_RENAME,
    STATE_NEW_FOLDER,
    STATE_NEW_FILE,    
    STATE_EDITING
} AppState;

// Restore the 'initialized' variable
static bool initialized = false;

// Current state
static AppState current_state = STATE_NORMAL;

// Buffers for rename or new folder or new file
static char input_buffer[INPUT_BUFFER_SIZE];
static size_t input_len = 0;

// Variables for cursor blinking
static bool cursor_visible = true;         // Cursor visibility state
static time_t last_toggle_time;            // Last time the cursor visibility was toggled

// Function prototypes
static void display_columns(void);
static void load_directory(size_t col, const char *dir);
static void enter_rename_mode(void);
static void enter_new_folder_mode(void);
static void enter_new_file_mode(void);      // NEW
static void commit_rename(void);
static void commit_new_folder(void);
static void commit_new_file(void);          // NEW
static void handle_text_input(KeyCode key);
static void display_rename_mode_screen(void);       // NEW
static void display_new_folder_screen(void);        // NEW
static void display_new_file_screen(void);          // NEW
static void handle_rename_input(KeyCode key);              // NEW: Separate handler
static void handle_new_folder_input(KeyCode key);           // NEW: Separate handler
static void handle_new_file_input(KeyCode key);             // NEW
static void enter_edit_mode(const char *filename);
static void display_editor_screen(void);
static void handle_editor_input(KeyCode key);
static void handle_normal_navigation(KeyCode key);           // NEW: Extracted handler




// 	Responsibility: Initializes the application, loads the root directory, clears the display, and sets initial state.
/*Improvement:
	•	Consider separating display initialization from directory loading.
	•	Document what the initialization does clearly.
	•	If waking from sleep vs. cold start is significant, consider a separate function to handle that scenario.*/
/*Refactoring:
	•	Move file/directory initialization into a navigation_init() function.
	•	Move display clearing and initial UI write into a ui_init() function.*/
void cybertyper_init(void) {
    hal_display_clear();

    if (hal_system_is_wakeup_from_sleep()) {
        hal_display_write("Woke from sleep\n");
    } else {
        hal_display_write("Cold start\n");
    }

    // Initialize the first column with the root directory
    load_directory(0, "/");
    column_count = 1;
    focused_column = 0;

    display_columns();

    // Initialize cursor blinking variables
    cursor_visible = true;
    last_toggle_time = time(NULL);

    initialized = true;
}

// Initialize the directory for a specific column  Populates a DirectoryColumn with files and subdirectories.
static void load_directory(size_t col, const char *dir) {
    if (col >= MAX_COLUMNS) return; // Safety check
    strncpy(columns[col].directory, dir, MAX_PATH_LEN);
    columns[col].directory[MAX_PATH_LEN - 1] = '\0';
    columns[col].file_count = hal_storage_list_files(dir, columns[col].file_list, MAX_FILES);
    columns[col].selected_index = 0;
}

// Loads a file into edit_buffer and transitions to STATE_EDITING.
/*Improvement:
	•	Consider moving file reading and writing to a dedicated file I/O module.
	•	Document what happens if hal_storage_read_file fails.
	•	Handle large files exceeding buffer size more gracefully.*/
static void enter_edit_mode(const char *filename) {
    size_t col = focused_column;
    strncpy(edit_filename, filename, MAX_FILENAME_LEN);
    edit_filename[MAX_FILENAME_LEN - 1] = '\0';

    // Read file content into edit_buffer
    int len = hal_storage_read_file(filename, edit_buffer, sizeof(edit_buffer));
    if (len < 0) {
        len = 0;
    }
    edit_buffer[len] = '\0';
    edit_length = (size_t)len;
    edit_cursor_pos = edit_length; // Start cursor at end of file

    current_state = STATE_EDITING;
    hal_display_clear();
    hal_display_write("Editing file: ");
    hal_display_write(edit_filename);
    hal_display_write("\nCtrl+S to save, Esc to exit editing.\n");
    hal_display_write(edit_buffer);

    // Initialize cursor blinking
    cursor_visible = true;
    last_toggle_time = time(NULL);
}

// Clears the display and shows the directory columns, including the current selection, and instructions.
/*Improvement:
	•	Consider a ui_renderer.c module that takes the columns array and prints it.
	•	Add comments explaining each step of the rendering process.
	•	Make the column width a constant defined at the top.
    */
static void display_columns(void) {
    hal_display_clear();
    char line[1024] = {0}; // Adjust size as needed

    // Define column width for uniform spacing
    const int column_width = 30;

    // Determine the maximum number of entries across all columns
    size_t max_entries = 0;
    for (size_t i = 0; i < column_count; i++) {
        if (columns[i].file_count > max_entries) {
            max_entries = columns[i].file_count;
        }
    }

    // Print header for each column (directory path)
    for (size_t i = 0; i < column_count; i++) {
        snprintf(line, sizeof(line), "Dir: %s", columns[i].directory);
        hal_display_write(line);
        // Add spacing between columns
        for (int j = strlen(line); j < column_width; j++) {
            hal_display_write(" ");
        }
    }
    hal_display_write("\n");

    // If all columns are empty, display a message
    bool all_empty = true;
    for (size_t i = 0; i < column_count; i++) {
        if (columns[i].file_count > 0) {
            all_empty = false;
            break;
        }
    }

    if (all_empty) {
        hal_display_write("This directory is empty.\n");
        hal_display_write("\nUse F2 to create a new folder or Ctrl+N to create a new file.\n");
        return;
    }

    // Print each row of the columns
    for (size_t entry = 0; entry < max_entries; entry++) {
        for (size_t col = 0; col < column_count; col++) {
            if (entry < columns[col].file_count) {
                // Check if this is the selected item in the focused column
                if (col == focused_column && entry == columns[col].selected_index) {
                    // Highlight the selected item (e.g., with a '>' marker)
                    snprintf(line, sizeof(line), "> %s", columns[col].file_list[entry]);
                } else {
                    snprintf(line, sizeof(line), "  %s", columns[col].file_list[entry]);
                }
            } else {
                // If the current column has fewer entries, display an empty line or a placeholder
                snprintf(line, sizeof(line), "   ");
            }

            // Ensure uniform column width
            size_t len = strlen(line);
            if (len < column_width) {
                memset(line + len, ' ', column_width - len);
                line[column_width] = '\0';
            }

            hal_display_write(line);
        }
        hal_display_write("\n");
    }

    // Instructions or status can be added here
    hal_display_write("\nUse Up/Down to navigate, Right to open folder/file, Left to go back.\n");
}

// Display Rename Mode
static void display_rename_mode_screen(void) {
    hal_display_clear();
    hal_display_write("Rename Mode:\n");
    hal_display_write("Current Item: ");
    hal_display_write(columns[focused_column].file_list[columns[focused_column].selected_index]);
    hal_display_write("\nType new name and press Enter. Esc to cancel.\n");
    hal_display_write(input_buffer);
}

// Display New Folder Mode
static void display_new_folder_screen(void) {
    hal_display_clear();
    hal_display_write("New Folder Mode:\n");
    hal_display_write("Type folder name and press Enter. Esc to cancel.\n");
    hal_display_write(input_buffer);
}

// Display New File Mode
static void display_new_file_screen(void) {
    hal_display_clear();
    hal_display_write("New File Mode:\n");
    hal_display_write("Type file name (without extension) and press Enter. Esc to cancel.\n");
    hal_display_write(input_buffer);
}

// Display the editor screen with the current file content
static void display_editor_screen(void) {
    hal_display_clear();
    hal_display_write("Editing: ");
    hal_display_write(edit_filename);
    hal_display_write("\nCtrl+S to save, Esc to exit.\n");

    // Create a copy of edit_buffer to modify for display
    char display_buffer[MAX_FILE_CONTENT_SIZE * 2 + 1]; // Increased size to accommodate ANSI codes
    memset(display_buffer, 0, sizeof(display_buffer));

    for (size_t i = 0; i < edit_length && i < MAX_FILE_CONTENT_SIZE; i++) {
        if (i == edit_cursor_pos && cursor_visible) {
            // Start underline
            strcat(display_buffer, "\033[4m"); // ANSI code to start underlining
            // Append the character to be underlined
            char temp[2] = { edit_buffer[i], '\0' };
            strcat(display_buffer, temp);
            // Reset formatting
            strcat(display_buffer, "\033[0m");
        } else {
            // Append the character as is
            size_t len = strlen(display_buffer);
            if (len < sizeof(display_buffer) - 2) { // Ensure space for the character and null terminator
                display_buffer[len] = edit_buffer[i];
                display_buffer[len + 1] = '\0';
            }
        }
    }

    // If cursor is at the end of the buffer, append an underline space
    if (edit_cursor_pos == edit_length && cursor_visible && edit_length < MAX_FILE_CONTENT_SIZE) {
        strcat(display_buffer, "\033[4m \033[0m"); // Underlined space
    }

    hal_display_write(display_buffer);
}

//  Processes keyboard input in edit mode, handling navigation, insertion, deletion, and saving.
/*	•	Improvement:
	•	Add comments before each block explaining what keys do.
	•	Check bounds carefully before inserting characters.
	•	Refactoring: Move editor logic into a separate editor.c.
    */
static void handle_editor_input(KeyCode key) {
    if (key == KEY_CTRL_S) {
        // Save file
        if (hal_storage_write_file(edit_filename, edit_buffer, edit_length)) {
            hal_display_write("\nFile saved!\n");
        } else {
            hal_display_write("\nError saving file!\n");
        }
        
    
        display_columns();
        return;
    }

    if (key == KEY_ESCAPE) {
        // Cancel editing, discard changes
        current_state = STATE_NORMAL;
        display_columns();
        return;
    }

    // Navigation in edit buffer
    if (key == KEY_ARROW_LEFT && edit_cursor_pos > 0) {
        edit_cursor_pos--;
    } else if (key == KEY_ARROW_RIGHT && edit_cursor_pos < edit_length) {
        edit_cursor_pos++;
    }

    // Backspace
    if (key == KEY_BACKSPACE && edit_cursor_pos > 0) {
        memmove(&edit_buffer[edit_cursor_pos - 1],
                &edit_buffer[edit_cursor_pos],
                edit_length - edit_cursor_pos + 1);
        edit_length--;
        edit_cursor_pos--;
    }

    // Printable chars
    if (key >= KEY_CHAR_BASE) {
        char c = (char)(key - KEY_CHAR_BASE);
        if (edit_length < MAX_FILE_CONTENT_SIZE - 1 && c >= 32 && c <= 126) {
            memmove(&edit_buffer[edit_cursor_pos + 1],
                    &edit_buffer[edit_cursor_pos],
                    edit_length - edit_cursor_pos + 1);
            edit_buffer[edit_cursor_pos] = c;
            edit_length++;
            edit_cursor_pos++;
        }
    }

    display_editor_screen();
}

// Enter rename mode for the selected file/folder
static void enter_rename_mode(void) {
    if (columns[focused_column].file_count == 0) return; // No file selected
    current_state = STATE_RENAME;
    input_len = 0;
    input_buffer[0] = '\0';

    display_rename_mode_screen();
}

// Enter new folder creation mode
static void enter_new_folder_mode(void) {
    current_state = STATE_NEW_FOLDER;
    input_len = 0;
    input_buffer[0] = '\0';

    display_new_folder_screen();
}

// NEW: Enter new file creation mode
static void enter_new_file_mode(void) {
    current_state = STATE_NEW_FILE;
    input_len = 0;
    input_buffer[0] = '\0';

    display_new_file_screen();
}

// Commit the rename operation
static void commit_rename(void) {
    if (column_count == 0) return; // Safety check

    size_t col = focused_column;
    // Construct old path and new path based on the focused column
    char oldpath[MAX_PATH_LEN];
    char newpath[MAX_PATH_LEN];
    snprintf(oldpath, sizeof(oldpath), "%s/%s", columns[col].directory, columns[col].file_list[columns[col].selected_index]);
    snprintf(newpath, sizeof(newpath), "%s/%s", columns[col].directory, input_buffer);

    if (hal_storage_rename_file(oldpath, newpath)) {
        hal_display_write("Rename successful!\n");
    } else {
        hal_display_write("Rename failed!\n");
    }

    // Reload the current column's file list
    columns[col].file_count = hal_storage_list_files(columns[col].directory, columns[col].file_list, MAX_FILES);
    columns[col].selected_index = 0;
    current_state = STATE_NORMAL;
    display_columns();
}

// Commit the new folder creation
static void commit_new_folder(void) {
    if (column_count == 0) return; // Safety check

    size_t col = focused_column;
    char newdir[MAX_PATH_LEN];
    snprintf(newdir, sizeof(newdir), "%s/%s", columns[col].directory, input_buffer);

    if (hal_storage_create_directory(newdir)) {
        hal_display_write("Folder created!\n");
    } else {
        hal_display_write("Failed to create folder.\n");
    }

    // Reload the current column's file list
    columns[col].file_count = hal_storage_list_files(columns[col].directory, columns[col].file_list, MAX_FILES);
    columns[col].selected_index = 0;
    current_state = STATE_NORMAL;
    display_columns();
}

// NEW: Commit the new file creation
static void commit_new_file(void) {
    if (column_count == 0) return; // Safety check

    size_t col = focused_column;
    char newfile[MAX_PATH_LEN];
    snprintf(newfile, sizeof(newfile), "%s/%s.txt", columns[col].directory, input_buffer);

    // Check if file already exists
    if (hal_storage_file_exists(newfile)) {
        hal_display_write("File already exists.\n");
    } else {
        // Create the new file
        if (hal_storage_create_file(newfile)) {
            hal_display_write("File created successfully!\n");
            // Optionally, open the new file in edit mode
            enter_edit_mode(newfile);
            return;
        } else {
            hal_display_write("Failed to create file.\n");
        }
    }

    // Reload the current column's file list
    columns[col].file_count = hal_storage_list_files(columns[col].directory, columns[col].file_list, MAX_FILES);
    columns[col].selected_index = 0;
    current_state = STATE_NORMAL;
    display_columns();
}

// Handle text input for rename, new folder, and new file modes
static void handle_text_input(KeyCode key) {
    if (key == KEY_BACKSPACE) {
        if (input_len > 0) {
            input_len--;
            input_buffer[input_len] = '\0';
        }
        return;
    }

    // Check for printable char
    if (key >= KEY_CHAR_BASE) {
        char c = (char)(key - KEY_CHAR_BASE);
        if (input_len < INPUT_BUFFER_SIZE - 1 && c >= 32 && c <= 126) {
            input_buffer[input_len++] = c;
            input_buffer[input_len] = '\0';
        }
    }
}

// Handle input while in rename mode
static void handle_rename_input(KeyCode key) {
    if (key == KEY_ENTER) {
        if (input_len == 0) {
            hal_display_write("New name cannot be empty.\n");
            return;
        }
        commit_rename();
        return;
    }

    if (key == KEY_ESCAPE) {
        // Cancel operation
        hal_display_write("Operation canceled.\n");
        current_state = STATE_NORMAL;
        display_columns();
        return;
    }

    // Handle typing characters
    handle_text_input(key);

    // Re-display prompt with current buffer and cursor
    display_rename_mode_screen();
}

// Handle input while in new folder mode
static void handle_new_folder_input(KeyCode key) {
    if (key == KEY_ENTER) {
        if (input_len == 0) {
            hal_display_write("Folder name cannot be empty.\n");
            return;
        }
        commit_new_folder();
        return;
    }

    if (key == KEY_ESCAPE) {
        // Cancel operation
        hal_display_write("Operation canceled.\n");
        current_state = STATE_NORMAL;
        display_columns();
        return;
    }

    // Handle typing characters
    handle_text_input(key);

    // Re-display prompt with current buffer and cursor
    display_new_folder_screen();
}

// NEW: Handle input while in new file mode
static void handle_new_file_input(KeyCode key) {
    if (key == KEY_ENTER) {
        if (input_len == 0) {
            hal_display_write("File name cannot be empty.\n");
            return;
        }
        commit_new_file();
        return;
    }

    if (key == KEY_ESCAPE) {
        // Cancel operation
        hal_display_write("Operation canceled.\n");
        current_state = STATE_NORMAL;
        display_columns();
        return;
    }

    // Handle typing characters
    handle_text_input(key);

    // Re-display prompt with current buffer and cursor
    display_new_file_screen();
}

// Handle normal navigation (extracted from original run_cycle)
static void handle_normal_navigation(KeyCode key) {
    size_t focused_col_file_count = columns[focused_column].file_count;

    switch (key) {
        case KEY_ARROW_UP:
            if (focused_col_file_count > 0 && columns[focused_column].selected_index > 0) {
                columns[focused_column].selected_index--;
                display_columns();
            }
            break;

        case KEY_ARROW_DOWN:
            if (focused_col_file_count > 0 && columns[focused_column].selected_index < columns[focused_column].file_count - 1) {
                columns[focused_column].selected_index++;
                display_columns();
            }
            break;

        case KEY_ARROW_RIGHT: // Open folder or process file
        case KEY_ENTER:       // **NEW: Handle Enter key the same way**
            if (focused_col_file_count > 0) {
                size_t selected = columns[focused_column].selected_index;
                char selected_path[MAX_PATH_LEN];

                size_t dir_len = strlen(columns[focused_column].directory);
                if (dir_len > 0 && columns[focused_column].directory[dir_len - 1] == '/') {
                    snprintf(selected_path, sizeof(selected_path), "%s%s", columns[focused_column].directory, columns[focused_column].file_list[selected]);
                } else {
                    snprintf(selected_path, sizeof(selected_path), "%s/%s", columns[focused_column].directory, columns[focused_column].file_list[selected]);
                }

                hal_display_write("DEBUG: Selected Path: ");
                hal_display_write(selected_path);
                hal_display_write("\n");

                if (hal_storage_is_directory(selected_path)) {
                    // Open the directory
                    hal_display_write("DEBUG: It's a directory.\n");
                    if (column_count < MAX_COLUMNS) {
                        load_directory(column_count, selected_path);
                        column_count++;
                        focused_column++;
                        display_columns();
                    } else {
                        hal_display_write("Maximum column limit reached.\n");
                    }
                } else {
                    // Open file in edit mode
                    hal_display_write("DEBUG: It's a file. Opening in edit mode...\n");
                    enter_edit_mode(selected_path);
                }
            } else {
                hal_display_write("No items to open in this directory.\n");
            }
            break;

        case KEY_ARROW_LEFT:
            if (focused_column > 0) {
                focused_column--;
                // Optionally, remove all columns to the right of the new focused column
                for (size_t i = focused_column + 1; i < column_count; i++) {
                    // Clear the directory path and file list
                    memset(&columns[i], 0, sizeof(DirectoryColumn));
                }
                column_count = focused_column + 1;
                display_columns();
            }
            break;

        case KEY_CTRL_R:
            if (focused_col_file_count > 0) {
                // Enter rename mode only if there are items to rename
                enter_rename_mode();
            } else {
                hal_display_write("No items to rename in this directory.\n");
            }
            break;

        case KEY_CTRL_N: // NEW: Keybinding for creating a new file
            // Enter new file creation mode
            enter_new_file_mode();
            break;

        default:
            break;
    }
}




//main loop handler for timing updates (cursor blinking) and dispatching key events to the appropriate state handler.
/*Improvement:
	•	Consider a state machine approach: one function per state that handles keys and updates UI.
	•	Make cursor blinking timing configurable.
	•	Add comments explaining each step (blink logic, input handling, display refresh).*/
void cybertyper_run_cycle(void) {
    if (!initialized) {
        return;
    }

    // Handle cursor blinking
    time_t current_time = time(NULL);
    if (difftime(current_time, last_toggle_time) >= 0.5) {
        cursor_visible = !cursor_visible;
        last_toggle_time = current_time;

        // Update the display based on current state
        switch (current_state) {
            case STATE_EDITING:
                display_editor_screen();
                break;
            case STATE_RENAME:
                display_rename_mode_screen();
                break;
            case STATE_NEW_FOLDER:
                display_new_folder_screen();
                break;
            case STATE_NEW_FILE:
                display_new_file_screen();
                break;
            default:
                display_columns();
                break;
        }
    }

    KeyCode key = hal_input_get_key();
    if (key == KEY_NONE) {
        return; 
    }

    // Handle different states
    switch (current_state) {
        case STATE_EDITING:
            handle_editor_input(key);
            break;
        case STATE_RENAME:
            handle_rename_input(key);
            break;
        case STATE_NEW_FOLDER:
            handle_new_folder_input(key);
            break;
        case STATE_NEW_FILE:
            handle_new_file_input(key);
            break;
        default:
            // Handle normal navigation
            handle_normal_navigation(key);
            break;
    }
}