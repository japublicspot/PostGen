/* PostScript Language Project
 * CS4121 - Programming Languages
 * Nathan Peterson
 * Connor W.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "eval.h"

// Version string
const char* version = "Development Build";

int main( int argc, char* arg[] ) {

    // Print program info
    printf( "interPS - PostScript Interpreter\n" );
    printf( "Version: %s\n", version );

    // Start the interpreter
    eval();
}
