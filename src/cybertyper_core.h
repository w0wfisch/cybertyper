#ifndef CYBERTYPER_CORE_H
#define CYBERTYPER_CORE_H

#include <stdbool.h>

/**
 * @brief Initialize the CyberTyper application logic.
 * 
 * This function sets up the initial state of the application, including UI,
 * file system state, and any required resources. It should be called once
 * before entering the main loop.
 */
void cybertyper_init(void);

/**
 * @brief Execute a single iteration (tick) of the main application logic.
 * 
 * This function should be called repeatedly, typically in a loop, to drive 
 * the CyberTyper’s application state forward. Each call:
 *  - Processes user input events.
 *  - Updates application state and UI if needed.
 *  - Handles file I/O operations pending for this cycle.
 * 
 * Ensure that `cybertyper_init()` has been called before invoking this function.
 */
void cybertyper_run_cycle(void);

#endif // CYBERTYPER_CORE_H