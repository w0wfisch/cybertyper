// hal_mock.c

#include "hal_interface.h"
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>  // for opendir, readdir, closedir
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/time.h>

#define SDCARD_DIR "./sdcard" // Root directory for the mock "SD card"

/* 
Hello, It's me on day one, i understand maybe 5% of this. I made this file to get me
basic hardware emulation. Have mercy on my code

w0wfisch, 2024_12_18
*/

// -----------------------------------------------------------------------------
/* Helper Functions */
// -----------------------------------------------------------------------------

// Helper function to build full path
// Converts a virtual (device-level) path into a host filesystem path for the mock environment.

static void build_full_path(const char *virtual_path, char *full_path, size_t size) {
    if (strcmp(virtual_path, "/") == 0) {
        snprintf(full_path, size, "%s", SDCARD_DIR);
    } else {
        // Remove leading slash to prevent double slashes
        if (virtual_path[0] == '/') {
            snprintf(full_path, size, "%s%s", SDCARD_DIR, virtual_path);
        } else {
            snprintf(full_path, size, "%s/%s", SDCARD_DIR, virtual_path);
        }
    }
}

static struct termios orig_termios;

// Disable raw mode on exit
static void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// Enable raw mode for terminal
static void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    // Disable canonical mode, echoing, and signals
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    // Set minimum number of input characters to 0
    raw.c_cc[VMIN] = 0;
    // Set timeout to 1 decisecond
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

// Check if a key has been pressed
static int kbhit(void) {
    fd_set set;
    struct timeval timeout = {0, 0};
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    return select(STDIN_FILENO+1, &set, NULL, NULL, &timeout) == 1;
}

// -----------------------------------------------------------------------------
/* Key Reading Logic */
// -----------------------------------------------------------------------------

/*
 * Reads a single raw character from stdin and translates it into a KeyCode.
 * Handles special keys like arrow keys, Enter, Backspace, and some Ctrl combinations.
 * Returns KEY_NONE if no key is available.
 */

static KeyCode read_key() {
    unsigned char c;
    int n = read(STDIN_FILENO, &c, 1);
    if (n == 0) return KEY_NONE;

    printf("DEBUG: Key Pressed: 0x%02x\n", c); // Debug output for key pressed

    if (c == '\r' || c == '\n') return KEY_ENTER;      // Enter key
    if (c == 127 || c == '\b') return KEY_BACKSPACE;  // Backspace key

    // Handle arrow keys and escape sequences
    if (c == '\x1b') { // Escape character
        unsigned char next_c;
        if (read(STDIN_FILENO, &next_c, 1) == 0) return KEY_ESCAPE;

        if (next_c == '[') { // Arrow keys
            unsigned char seq;
            if (read(STDIN_FILENO, &seq, 1) == 0) return KEY_NONE;
            switch (seq) {
                case 'A': return KEY_ARROW_UP;
                case 'B': return KEY_ARROW_DOWN;
                case 'C': return KEY_ARROW_RIGHT;
                case 'D': return KEY_ARROW_LEFT;
                default: return KEY_NONE;
            }
        }
        return KEY_ESCAPE;
    }

    // Ctrl keys: ASCII 1-26 (Ctrl+A to Ctrl+Z)
    if (c == 14) return KEY_CTRL_N; // Ctrl+N
    if (c == 18) return KEY_CTRL_R; // Ctrl+R
    if (c == 19) return KEY_CTRL_S; // Ctrl+S

    // Printable characters
    if (c >= 32 && c <= 126) return (KeyCode)(KEY_CHAR_BASE + c);

    return KEY_NONE;
}

// Public HAL function: Retrieves a pressed key or returns KEY_NONE if none.
KeyCode hal_input_get_key(void) {
    if (kbhit()) {
        return read_key();
    }
    return KEY_NONE;
}



// -----------------------------------------------------------------------------
/* Display Handling */
// -----------------------------------------------------------------------------

// Clears the display. In this mock, it just prints a message to stdout.
void hal_display_clear(void) {
    printf("\n--- DISPLAY CLEARED ---\n");
}

// Writes the given text to the (mock) display (stdout).
void hal_display_write(const char *text) {
    printf("%s", text);
}

// Sets the display cursor to the specified line and column. 
void hal_display_set_cursor(int line, int column) {
    (void)line;
    (void)column;
}



// -----------------------------------------------------------------------------
/* Storage Handling */
// -----------------------------------------------------------------------------

/* Lists files from the given virtual directory into the 'files' array. Returns the number of files found. */
size_t hal_storage_list_files(const char *directory, char files[][64], size_t max_files) {
    char full_path[512];
    build_full_path(directory, full_path, sizeof(full_path));

    DIR *dir = opendir(full_path);
    if (!dir) {
        perror("opendir");
        return 0;
    }

    size_t count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < max_files) {
        // Ignore '.' and '..'
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct filepath to check if it's a file or directory
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", full_path, entry->d_name);

        struct stat st;
        if (stat(filepath, &st) == 0) {
            // We list both files and directories, core logic handles directories separately.
            strncpy(files[count], entry->d_name, 64);
            files[count][63] = '\0';
            count++;
        }
    }

    closedir(dir);
    return count;
}

// Reads the file at 'filepath' into 'buffer' (up to buffer_size-1 bytes). Returns bytes read or -1 on error.
int hal_storage_read_file(const char *filepath, char *buffer, size_t buffer_size) {
    char fullpath[512];
    build_full_path(filepath, fullpath, sizeof(fullpath));

    FILE *f = fopen(fullpath, "rb");
    if (!f) {
        return -1;
    }

    // Read file into buffer
    size_t bytesRead = fread(buffer, 1, buffer_size - 1, f);
    fclose(f);
    buffer[bytesRead] = '\0'; // null terminate

    return (int)bytesRead;
}

// Checks if the given file exists in the mock file system.
bool hal_storage_file_exists(const char *filepath) {
    struct stat buffer;
    return (stat(filepath, &buffer) == 0);
}

// Creates a new empty file. Returns true on success, false on failure.
bool hal_storage_create_file(const char *filepath) {
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        perror("hal_storage_create_file fopen");
        return false;
    }
    fclose(file);
    printf("File created successfully at '%s'\n", filepath); // Debug statement
    return true;
}

// Checks if the given virtual path represents a directory in the mock environment.
bool hal_storage_is_directory(const char *virtual_path) {
    char full_path[512];
    build_full_path(virtual_path, full_path, sizeof(full_path));

    struct stat st;
    if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        printf("DEBUG: Path: %s | Is Directory: Yes\n", full_path);
        return true;
    }

    if (stat(full_path, &st) != 0) {
        perror("stat");
        printf("DEBUG: Failed to stat path: %s\n", full_path);
    } else {
        printf("DEBUG: Path: %s | Is Directory: No\n", full_path);
    }
    return false;
}

// Renames a file or directory from oldpath to newpath. Returns true on success.
bool hal_storage_rename_file(const char *oldpath, const char *newpath) {
    int result = rename(oldpath, newpath);
    if (result != 0) {
        perror("hal_storage_rename_file rename");
        return false;
    }
    printf("Renamed '%s' to '%s' successfully\n", oldpath, newpath); // Debug statement
    return true;
}

// Creates a directory at 'dirpath'. Returns true on success.
bool hal_storage_create_directory(const char *dirpath) {
    int result = mkdir(dirpath, 0777);
    if (result != 0) {
        perror("hal_storage_create_directory mkdir");
        return false;
    }
    printf("Directory created successfully at '%s'\n", dirpath); // Debug statement
    return true;
}

// Writes 'length' bytes from 'buffer' to the file at 'filepath'. Returns true on success.
bool hal_storage_write_file(const char *filepath, const char *buffer, size_t length) {
    char fullpath[512];
    build_full_path(filepath, fullpath, sizeof(fullpath));
    
    FILE *f = fopen(fullpath, "wb");
    if (!f) {
        perror("hal_storage_write_file fopen");
        return false;
    }
    size_t written = fwrite(buffer, 1, length, f);
    fclose(f);
    if (written != length) {
        printf("Warning: Only wrote %zu of %zu bytes to '%s'\n", written, length, fullpath);
        return false;
    }
    printf("Successfully wrote %zu bytes to '%s'\n", written, fullpath);
    return true;
}



// -----------------------------------------------------------------------------
/* Power / Sleep Events (Mocked) */
// -----------------------------------------------------------------------------

// Checks if the system just woke up from a sleep state. Returns false in this mock.
bool hal_system_is_wakeup_from_sleep(void) {
    return false;
}

// Prepares the system for sleep. (No-op in the mock environment.)
void hal_system_prepare_for_sleep(void) {
    // No-op in mock
}

// Simulates putting the system into a sleep state.
void hal_system_sleep(void) {
    printf("Going to sleep... (mock)\n");
}



// -----------------------------------------------------------------------------
/* Initialization */
// -----------------------------------------------------------------------------

// Automatically called at program start to set up the mock HAL environment.
__attribute__((constructor))
static void init_mock_hal() {
    enable_raw_mode();
    printf("--- Mock HAL Initialized ---\n");
}
// Automatically called at program exit to restore terminal state and clean up.
__attribute__((destructor))
static void cleanup_mock_hal() {
    disable_raw_mode();
    printf("--- Mock HAL Cleanup ---\n");
}