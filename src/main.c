/* PostGen
 *
 * PostGen is a basic interpreter that acts as a state machine to handle user
 * inputted commands in order to generate a corresponding PostScript file.
 */

#include <stdio.h>
#include <stdlib.h>

#include "eval.h"

// Version string
const char* version = "Development Build";

/*
 * Starts the interpreter.
 * If a script file is provided at runtime, then it will be evaluated.
 */
int main( int argc, char* argv[] ) {
    // Handle args
    if( argc > 2 ) {
        printf( "Too many arguments provided!\n" );
        printf( "Usage: postgen [filename] (optional)\n" );
        exit(EXIT_FAILURE);
    }

    // Print program info
    printf( "PostGen - PostScript Generator\n" );
    printf( "Version: %s\n", version );
    printf( "Enter a command, or 'help' to view available commands.\n" );

    // Start the interpreter
    if( argc == 2 ) {
        // Evaluate the contents of the defined script file
        run(argv[1]);
    } else {
        // Run the interpreter interactively
        run(NULL);
    }
}
