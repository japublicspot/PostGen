/* PostScript Language Project
 * CS4121 - Programming Languages
 * Nathan Peterson
 * Connor Wlodarczak
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
    if( argc > 2 ) {
        printf( "Too many arguments provided!\n" );
        printf( "Usage: interPS [filename] (optional)\n" );
        exit(EXIT_FAILURE);
    }

    // Print program info
    printf( "interPS - PostScript Generator\n" );
    printf( "Version: %s\n", version );
    printf( "Enter a command, or 'help' to view available commands.\n" );

    // Start the interpreter
    if( argc == 2 ) {
        run(argv[1]);
    } else {
        run(NULL);
    }
}
