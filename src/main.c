/* PostScript Language Project
 * CS4121 - Programming Languages
 * Nathan Peterson
 * Connor Wlodarczak
 */

#include <stdio.h>

#include "eval.h"

// Version string
const char* version = "Development Build";

int main( int argc, char* argv[] ) {

    // Print program info
    printf( "interPS - PostScript Interpreter\n" );
    printf( "Version: %s\n", version );
    printf( "Enter a command, or 'help' to view available commands.\n" );

    // Start the interpreter
    if( argc == 2 ) {
        run(argv[1]);
    } else {
        run(NULL);
    }
}
