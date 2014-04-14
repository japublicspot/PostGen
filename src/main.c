/* PostScript Language Project
 * CS4121 - Programming Languages
 * Nathan Peterson
 * Connor W.
 */

#include "eval.h"

// Version string
const char* version = "Development Build";

int main( int argc, char* arg[] ) {

    // Print program info
    printf( "interPS - PostScript Interpreter\n" );
    printf( "Version: %s\n", version );
    printf( "Enter a command, or 'help' to view available commands.\n" );

    // Start the interpreter
    eval();
}
