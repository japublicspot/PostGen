#include "eval.h"

// The number of commands in the interpreter
#define NUM_COMMANDS 7
// Index that the PostScript commands begin at
#define PS_CMD_START 4

// Private function prototypes:
// Evaluator for user input
static void eval( FILE* inStream, bool psOnly );
// Used to parse each command
static int parseCommand( char* line, int* argc, char* args[] );
// Functions for each command/state:
static void path( int argc, char* args[] );
static void rotate( int argc, char* args[] );
static void begin( int argc, char* args[] );
static void end( int argc, char* args[] );
static void loop( int argc, char* args[] );
static void quit( int argc, char* args[] );
static void help( int argc, char* args[] );

// List of states for the interpreter
static void (*states[NUM_COMMANDS])(int argc, char* argsp[]) =
        {
            help,
            begin,
            end,
            quit,
            path,
            rotate,
            loop
        };

// List of supported commands. These have a 1-to-1 mapping to the states above.
static char* commands[NUM_COMMANDS] =
        {
            "help",
            "begin",
            "end",
            "quit",
            "path",
            "rotate",
            "loop"
        };

// The PostScript file currently being operated on
static FILE* session = NULL;

// The input stream the interpreter will retrieve commands from
static FILE* input = NULL;

/*
 * Main run loop of the interpreter.
 * Continues indefinitely, until the user quits the interpreter.
 *
 * Input:
 * None
 *
 * Returns:
 * None
 */
void run() {
    // Continue reading user input until the user quits
    while(1) {
        // Print interpreter promt
        printf( "\n>> " );

        // Get and evaluate user input
        eval( stdin, false );
    }
}

/*
 * Reads and evaluates user input.
 *
 * Input:
 * FILE* input - Set the input stream.
 * bool psOnly - Set if only PS commands should be recognized, or all commands.
 *
 * Returns:
 * None
 */
void eval( FILE* inStream, bool psOnly ) {
    // Check that a valid input stream was provided
    if( inStream == NULL ) {
        printf( "\nERROR:\tInvalid input stream specified!\n" );
        return;
    } else {
        // Set the input stream
        input = inStream;
    }

    // Used by parseCommand
    int argc = 0;
    char* args[30];

    // Get command from user
    char line[255];
    // Only proccess command if we successfully read something
    if( fgets( line, 255, input ) != NULL ) {
        // Parse the user command
        int parseErr = parseCommand( line, &argc, args );

        // If a command was provided, try to execute it
        if( argc > 0 ) {
            // Used to detect if the command was unknown
            bool found = false;
            // Check if the command given is a known command
            // Only execute if parseCommand succeeded
            int start = 0;
            if(psOnly) { start = PS_CMD_START; }
            for( int i = start; i < NUM_COMMANDS && parseErr == 0; i++  ) {
                if( strcmp( commands[i], args[0] ) == 0 ) {
                    // Execute the command with provided args
                    (*states[i])(argc, args);
                    found = true;
                }
            }

            // Free the args list
            for( int i = 0; i < argc; i++ ) {
                free(args[i]);
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

/*
 * Parses the command provided by the user to the interpreter.
 * NOTE: This function mallocs each element of the args list individually.
 *       It is the responsibility of the caller to free each element manually.
 *
 * Input:
 * char* line   - The line of input to be parsed.
 * int* argc    - Used to return the count of args found.
 * char* args[] - Used to return the args found.
 */
int parseCommand( char* line, int* argc, char* args[] ) {
    // Get the first argument
    char* current = strtok( line, " \n" );
    // Continue until there are no arguments left to read
    while( current != NULL ) {
        // Set the current argument in the args list
        args[*argc] = (char*)malloc(strlen(current) + 1);
        if( args[*argc] == NULL ) {
            printf( "\nERROR:\tUnable to allocate args list!\n" );
            return -1;
        }
        strcpy( args[*argc], current );
        // Increment args count
        (*argc)++;
        // Get the next argument
        current = strtok( NULL, " \n" );
    }

    return 0;
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

        printf( "\nEnter a series of points (one tuple per line), and 'done' when finished:\n" );

        // Get the starting point of the path
        int startX = atoi( args[1] );
        int startY = atoi( args[2] );

        // Begin the path in the file
        fprintf( session, "newpath\n" );
        fprintf( session, "%d %d moveto\n", startX, startY );

        // Continue reading points until the user is finished
        while(1) {
            // Print state prompt
            printf(": ");

            char point[255];
            if( fgets( point, 255, input ) != NULL ) {
                // Get the first argument
                char* first = strtok( point, " \n" );

                // If the argument is 'done', exit this state
                if( strcmp( first, "done" ) == 0 ) {
                    // Close the path and apply the stroke
                    fprintf( session, "closepath\n" );
                    fprintf( session, "stroke\n" );
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
                            printf( "ERROR:\tArguments must be numbers!\n" );
                        }
                    } else {
                        printf( "ERROR:\tInvalid number of arguments provided!\n" );
                        printf( "Usage:\t<x> <y>\n");
                    }
                }
            }
        }
        printf( "Path finished.\n" );
    }
}

/*
 * Command state to execute rotations
 *
 * Input:
 * int deg - degrees to rotate by
 */
void rotate( int argc, char* args[] ) {
    // Check if we have the correct number of arguments
    if( argc != 2 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\trotate <degrees>\n" );
    } else {
        if( session == NULL ) {
            printf( "\nERROR:\tNo active session!\n" );
            return;
        }

        int deg = strtol( args[1], NULL, 10 );
        if( errno != 0 ) {
            printf( "\nERROR:\tArguments must be numbers!\n" );
            return;
        }

        // Save coordinate system state
        fprintf( session, "gsave\n" );
        // Apply the rotation
        fprintf( session, "%d rotate\n", deg );
        while(1) {
            printf( "\nEnter commands to construct a block of code to rotate:\n" );
            // Print repeat prompt
            printf( "+> " );

            // Evaluate input for body of the repeat block
            eval( stdin, true );

            // Ask the user if they wish to enter more commands
            printf( "\nFinished constructing rotate block? (y/n) " );
            char ans[255];
            // Check input and verify its validity
            if( fgets(ans, 255, input) != NULL && ans[0] == 'y' && strlen(ans) == 2 ) {
                break;
            }
        }
        // Restore coordinate system state
        fprintf( session, "grestore\n" );

        printf( "Rotate block finished. Result of block will be rotated %d degrees.\n", deg );
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
            if( fgets(ans, 255, input) != NULL && ans[0] == 'y' && strlen(ans) == 2 ) {
                // Close the current session
                end( 1, NULL );
            } else {
                // Abort session creation
                printf( "Aborting session creation...\n" );
                return;
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
            // Dump the generated page
            fprintf( session, "showpage\n" );

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
    // Check if we have the correct number of arguments
    if( argc != 2 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tloop <count>\n" );
    } else {
        if( session == NULL ) {
            printf( "\nERROR:\tNo active session!\n" );
            return;
        }

        int count = strtol( args[1], NULL, 10 );
        if( errno != 0 ) {
            printf( "\nERROR:\tArguments must be numbers!\n" );
            return;
        }

        // Apply the rotation
        fprintf( session, "%d {\n", count );
        while(1) {
            printf( "\nEnter commands to construct a block of code to loop:\n" );
            // Print repeat prompt
            printf( "#> " );

            // Evaluate input for body of the repeat block
            eval( stdin, true );

            // Ask the user if they wish to enter more commands
            printf( "\nFinished constructing loop block? (y/n) " );
            char ans[255];
            // Check input and verify its validity
            if( fgets(ans, 255, input) != NULL && ans[0] == 'y' && strlen(ans) == 2 ) {
                break;
            }
        }
        // Set to repeat
        fprintf( session, "} repeat\n" );

        printf( "Loop block finished. Result of block will be looped %d times.\n", count );
    }
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

        // Free the args value for this call
        free(args[0]);

        printf( "Closing interpreter...\n" );

        // Exit the program successfully
        exit(EXIT_SUCCESS);
    }
}

/*
 * Displays the help dialog.
 *
 * Input:
 * None
 */
void help( int argc, char* args[] ) {
    // Check if we have the correct number of arguments
    if( argc != 1 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\thelp\n" );
    } else {
        printf( "\ninterPS Manual\n" );
        printf( "--------------\n" );
        printf( "This interpreter behaves like a state machine, with each \n"
                "command mapped to its own state.  The commands given construct \n"
                "a PostScript file that can be opened in any PostScript viewer. \n"
                "Prior to executing any commands, a session must first be created \n"
                "which sets up the PostScript file that is being constructed.\n" );
        printf( "\nCommands:" );
        printf( "\nbegin [name]    \tStarts a new session with the given name.\n" );
        printf( "                  \tThis creates a PostScript file of the given name.\n" );
        printf( "\nend             \tEnds the current session and closes its file.\n" );
        printf( "\npath [x] [y]    \tConstructs a user-defined path, starting at (x,y).\n"
                "                  \tContinues to read tuples in until the user enters 'done'.\n" );
        printf( "\nrotate [degrees]\tRotates the given construct by the given number of degrees.\n" );
        printf( "\nloop [count]    \tRepeats the given construct count times.\n" );
        printf( "\nquit            \tCloses any open session and exits the interpreter.\n" );
        printf( "\nhelp            \tDisplays this dialog.\n" );
    }
}
