#include "eval.h"

// Private function prototypes:
// Used to parse each command
static void parseCommand( char* line, int* argc, char* args[] );
// Functions for each command/state:
static void path( int argc, char* args[] );
static void begin( int argc, char* args[] );
static void end( int argc, char* args[] );
static void loop( int argc, char* args[] );
static void quit( int argc, char* args[] );

// List of states for the interpreter
static void (*states[NUM_COMMANDS])(int argc, char* argsp[]) =
        {
            path,
            begin,
            end,
            loop,
            quit
        };

// List of supported commands. These have a 1-to-1 mapping to the states above.
static char* commands[NUM_COMMANDS] =
        {
            "path",
            "begin",
            "end",
            "loop",
            "quit"
        };

// The PostScript file currently being operated on
static FILE* session = NULL;

/*
 * Reads and evaluates user input.
 * Continues indefinitely, until the user quits the interpreter.
 *
 * Input:
 * None
 *
 * Returns:
 * None
 */
void eval() {
    // Continue reading user input until the user quits
    while(1) {
        // Used by parseCommand
        int argc = 0;
        char* args[30];

        printf( "\n>> " );

        // Get command from user
        char line[255];
        // Only proccess command if we successfully read something
        if( fgets( line, 255, stdin ) != NULL ) {
            // Parse the user command
            parseCommand( line, &argc, args );

            // If a command was provided, try to execute it
            if( argc > 0 ) {
                // Check if the command given is a known command
                bool found = false;
                for( int i = 0; i < NUM_COMMANDS; i++  ) {
                    if( strcmp( commands[i], args[0] ) == 0 ) {
                        // Execute the command with provided args
                        (*states[i])(argc, args);
                        found = true;
                    }
                }

                // If an invalid command was provided, display error
                if(!found) {
                    printf( "ERROR: Unknown command!\n" );
                }
            } else {
                // We weren't given a command. This should never happen.
                printf( "ERROR: No command provided!\n" );
            }
        }
    }
}

/*
 * Parses the command provided by the user to the interpreter.
 *
 * Input:
 * char* line   - The line of input to be parsed.
 * int* argc    - Used to return the count of args found.
 * char* args[] - Used to return the args found.
 */
void parseCommand( char* line, int* argc, char* args[] ) {
    // Get the first argument
    char* current = strtok( line, " \n" );
    // Continue until there are no arguments left to read
    while( current != NULL ) {
        // Set the current argument in the args list
        args[*argc] = (char*)malloc(strlen(current));
        strcpy( args[*argc], current );
        // Increment args count
        (*argc)++;
        // Get the next argument
        current = strtok( NULL, " \n" );
    }
}

/*
 * Command state for drawing a user-defined path.
 *
 * Input:
 * int x, y - Starting point for the path.
 */
void path( int argc, char* args[] ) {
    // Check if we have the correct number of arguments
    if( argc != 3 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tpath <start_x> <start_y>\n" );
    } else {
        if( session == NULL ) {
            printf( "\nERROR:\tNo active session!\n" );
            return;
        }

        printf( "Enter a series of points (one tuple per line), and 'done when finished:\n" );

        // Get the starting point of the path
        int startX = atoi( args[1] );
        int startY = atoi( args[2] );

        // Begin the path in the file
        fprintf( session, "newpath\n" );
        fprintf( session, "%d %d moveto\n", startX, startY );

        // Continue reading points until the user is finished
        while(1) {
            printf("\n: ");

            char point[255];
            if( fgets( point, 255, stdin ) != NULL ) {
                // Get the first argument
                char* first = strtok( point, " \n" );

                // If the argument is 'done', exit this state
                if( strcmp( first, "done" ) == 0 ) {
                    // Close the path in the file
                    fprintf( session, "closepath\nstroke\n" );
                    break;
                } else {
                    // Retreive the arguments
                    char* xC = first;
                    char* yC = strtok( NULL, " \n" );

                    // Ensure that both arguments were provided
                    if( xC != NULL && yC != NULL ) {
                        // Reset errno to 0 to check for error on the following calls
                        errno = 0;

                        // Convert the provided strings to numbers
                        int x = strtol( xC, NULL, 10 );
                        int y = strtol( yC, NULL, 10 );

                        // NOTE: The behavior of strtol setting errno when not provided
                        // with a number is implementation specific. Thus, it is
                        // possible to not receive an error when provided non-numeric
                        // arguments. In such a case the argument with simply resolve
                        // to 0.
                        if( !errno ) {
                            // Add the next point to the path
                            fprintf( session, "%d %d lineto\n", x, y );
                        } else {
                            printf( "\nERROR:\tArguments must be numbers!\n" );
                        }
                    } else {
                        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
                        printf( "Usage:\t<x> <y>\n");
                    }
                }
            }
        }
        printf( "Path finished.\n" );
    }
}

/*
 * Command state to begin a new session.
 *
 * Input:
 * char* name - The name of the session.
 */
void begin( int argc, char* args[] ) {
    // Check if we have the correct number of arguments
    if( argc != 2 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tbegin <session_name>\n" );
    } else {
        // Check if we already have an active session
        if( session != NULL ) {
            printf( "Active session exists! Do wish to close this session and start a new one? (y/n) " );
            char ans[255];
            // Check input and verify its validity
            if( fgets(ans, 255, stdin) != NULL && ( ans[0] == 'n' || strlen(ans) != 2 ) ) {
                // Abort session creation
                printf( "Aborting session creation...\n" );
                return;
            } else {
                // Close the current session
                end( 1, NULL );
            }
        }
        // Get the name of the session to create
        char* name = args[1];

        // File extension
        char* ext = ".ps";
        // The new filename
        char filename[strlen(name) + strlen(ext)];

        // Copy session name
        strcpy( filename, name );
        // Add the file extension
        strcpy( filename + strlen(filename), ext );

        // Open/create the file
        session = fopen(filename, "w+");

        // Check if open succeeded
        if( session == NULL ) {
            printf( "\nERROR:\tFailed to create session!\n" );
        } else {
            // Write PostScript metadata to file
            char* head = "%!PS\n";
            fprintf( session, "%s", head );
            printf( "Created session: %s\n", name );
        }
    }
}

/*
 *  Command state to end the current session.
 *
 *  Input:
 *  None
 */
void end( int argc, char* args[] ) {
    // Check if we have the correct number of arguments
    if( argc != 1 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tend\n" );
    } else {
        // Check if session is null
        if( session != NULL ) {
            // Close session and check for errors
            if( fclose( session ) != 0 ) {
                printf( "\nERROR: Failed to close session file!\n" );
            } else {
                session = NULL;
                printf( "Session ended.\n" );
            }
        } else {
            printf( "\nERROR: No open session to end!\n" );
        }
    }
}

/*
 * Command state for looping construct.
 *
 * Input:
 * int r - Number of times to repeat loop.
 */
void loop( int argc, char* args[] ) {
    // TODO: Implement loop state
    printf( "TODO: Loop not yet implemented!\n" );
}

/*
 * Command state to quit the program.
 *
 * Input:
 * None
 */
void quit( int argc, char* args[] ) {
    // Check if we have the correct number of arguments
    if( argc != 1 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tquit\n" );
    } else {
        // Close the session first
        if( session != NULL ) {
            end( 1, NULL );
        }

        printf( "Closing interpreter...\n" );

        // Exit the program successfully
        exit(EXIT_SUCCESS);
    }
}
