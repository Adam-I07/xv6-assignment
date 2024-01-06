#include "types.h"
#include "user.h"
int main(int argc, char *argv[]){
    // Check if there are command-line arguments
    if (argc > 1) {
        // Check if the first argument is "-r"
        if (strcmp("-r", argv[1]) == 0) { 
            // If "-r" is provided, call the shutdown system call with restart flag
            shutdown(1);
        }
        // Exit the program
        exit();
    }

    // If no arguments or the first argument is not "-r", call the shutdown system call with shutdown flag
    shutdown(0);

    // Exit the program
    exit();
}