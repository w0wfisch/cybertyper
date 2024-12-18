#ifndef HAL_INTERFACE_H
#define HAL_INTERFACE_H

#include <stdbool.h>
#include <stddef.h>


/**
 * @enum KeyCode
 * @brief Represents all possible keys that can be read from the input device.
 *
 * This enum covers standard character keys, function keys, navigation keys,
 * modifier keys, and other special input sequences. It provides a hardware-
 * independent way of representing input from keyboards.
 */
typedef enum {
    KEY_NONE = 0,
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_ESCAPE,
    KEY_SPACE,
    
    KEY_SHIFT_LEFT,
    KEY_SHIFT_RIGHT,
    KEY_CTRL_LEFT,
    KEY_CTRL_RIGHT,
    KEY_ALT_LEFT,
    KEY_ALT_RIGHT,
    KEY_GUI_LEFT,
    KEY_GUI_RIGHT,
    KEY_CAPS_LOCK,

    KEY_ARROW_UP,
    KEY_ARROW_DOWN,
    KEY_ARROW_LEFT,
    KEY_ARROW_RIGHT,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_INSERT,
    KEY_DELETE,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_PRINT_SCREEN,
    KEY_SCROLL_LOCK,
    KEY_PAUSE,

    KEY_NUM_LOCK,
    KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY,
    KEY_KP_MINUS,
    KEY_KP_PLUS,
    KEY_KP_ENTER,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_0,
    KEY_KP_DECIMAL,

    KEY_CTRL,
    // Add Ctrl+R and Ctrl+N for your operations
    KEY_CTRL_R,
    KEY_CTRL_N,
    KEY_CTRL_S,
    KEY_CTRL_ALT_N,

    KEY_CHAR_BASE
} KeyCode;

/**
 * @brief Returns the next pressed key, or KEY_NONE if no key is pressed.
 *
 * This function should be non-blocking and return immediately. If a key has been
 * pressed since the last call, it returns the corresponding KeyCode; otherwise,
 * it returns KEY_NONE.
 *
 * @return KeyCode representing the key pressed or KEY_NONE if none.
 */
KeyCode hal_input_get_key(void);

/**
 * @brief Clears the display or screen.
 *
 * On real hardware, this might send a command to a display controller to
 * clear its buffer. In a mock or test environment, it may simply output
 * a newline or reset a simulated display buffer.
 */
void hal_display_clear(void);

/**
 * @brief Writes text to the display.
 *
 * @param text The null-terminated string to display.
 */
void hal_display_write(const char *text);

/**
 * @brief Moves the display cursor to the specified line and column.
 *
 * @param line   The vertical position (e.g. row number).
 * @param column The horizontal position (e.g. column number).
 */
void hal_display_set_cursor(int line, int column);

/**
 * @brief Lists files in a directory.
 *
 * Scans the given directory and returns up to max_files entries in the provided
 * 'files' array. Each entry in 'files' can hold up to 63 characters plus a null terminator.
 *
 * @param directory The virtual directory path to list.
 * @param files     A 2D array where each element is a file name buffer.
 * @param max_files The maximum number of files to list.
 * @return The number of files found.
 */
size_t hal_storage_list_files(const char *directory, char files[][64], size_t max_files);

/**
 * @brief Reads the contents of a file into a buffer.
 *
 * @param filepath    The path of the file to read.
 * @param buffer      The output buffer where file content will be stored.
 * @param buffer_size The size of the output buffer.
 * @return Number of bytes read, or -1 on failure.
 */
int hal_storage_read_file(const char *filepath, char *buffer, size_t buffer_size);

/**
 * @brief Checks if a file exists.
 *
 * @param filepath The path of the file to check.
 * @return true if the file exists, false otherwise.
 */
bool hal_storage_is_directory(const char *filepath);


bool hal_storage_rename_file(const char *oldpath, const char *newpath);

bool hal_storage_create_directory(const char *dirpath);
bool hal_storage_write_file(const char *filepath, const char *buffer, size_t length);
// Add the four new storage functions:
bool hal_storage_file_exists(const char *filepath);      // NEW
bool hal_storage_create_file(const char *filepath);      // NEW
bool hal_storage_is_directory(const char *filepath);
bool hal_storage_rename_file(const char *oldpath, const char *newpath);
bool hal_storage_create_directory(const char *dirpath);
bool hal_storage_write_file(const char *filepath, const char *buffer, size_t length);

bool hal_system_is_wakeup_from_sleep(void);
void hal_system_prepare_for_sleep(void);
void hal_system_sleep(void);

#endif // HAL_INTERFACE_H