#include "cybertyper_core.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    cybertyper_init();

    // Run the core logic in a loop.
    // Press arrow keys and type characters to interact.
    // Press Ctrl+C to exit the program.
    while (1) {
        cybertyper_run_cycle();
        // Add a small sleep to avoid busy-waiting; adjust as needed.
        usleep(50000); // 50ms
    }

    return 0;
}