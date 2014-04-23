/* PostScript Language Project
 * CS4121 - Programming Languages
 * Nathan Peterson
 * Connor Wlodarczak
 */

/* PostGen Eval
 *
 * This is the core of PostGen.
 *
 * This file contains the interpreters main eval loop, and all logic for
 * interpreting and executing user inputted commands and script files.
 *
 * All supported commands have their own function that acts as a state.  When a
 * command is received by the eval loop, it is compared to a list of commands
 * and then executes the corresponding function from a list of function
 * pointers. The same is true of a script file as it is parsed.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <strings.h>

#include "eval.h"

// The number of commands in the interpreter
#define NUM_COMMANDS 17

// Index that the PostScript commands begin at
#define PS_CMD_START 5

// Private function prototypes:

// Evaluator for user input
static void eval( FILE* inStream, bool psOnly );
// Used to parse each command
static int parseCommand( char* line, int* argc, char* argv[] );

// Functions for each command/state:
static void path( int argc, char* argv[] );
static void closedPath( int argc, char* argv[] );
static void solidPath( int argc, char* argv[] );
static void curve( int argc, char* argv[] );
static void closedCurve( int argc, char* argv[] );
static void solidCurve( int argc, char* argv[] );
static void circle( int argc, char* argv[] );
static void solidCircle( int argc, char* argv[] );
static void polygon( int argc, char* argv[] );
static void solidPolygon( int argc, char* argv[] );
static void rotate( int argc, char* argv[] );
static void begin( int argc, char* argv[] );
static void end( int argc, char* argv[] );
static void loop( int argc, char* argv[] );
static void open( int argc, char* argv[] );
static void quit( int argc, char* argv[] );
static void help( int argc, char* argv[] );

// List of states for the interpreter
static void (*states[NUM_COMMANDS])(int argc, char* argv[]) =
        {
            help,
            begin,
            end,
            quit,
            open,
            path,
            closedPath,
            solidPath,
            curve,
            closedCurve,
            solidCurve,
            circle,
            solidCircle,
            polygon,
            solidPolygon,
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
            "open",
            "path",
            "closedpath",
            "solidpath",
            "curve",
            "closedcurve",
            "solidcurve",
            "circle",
            "solidcircle",
            "polygon",
            "solidpolygon",
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
 * char* filename - (optional) If filename of a script is provided when program
 *                             is executed, then open a evaluate the script.
 *
 * Returns:
 * None
 */
void run( char* filename ) {
    // Check if a script filename was provided
    if( filename == NULL ) {
        // Continue reading user input until the user quits
        while(1) {
            // Print interpreter promt
            printf( "\n>> " );

            // Get and evaluate user input
            eval( stdin, false );
        }
    } else {
        // Set up args
        char* argv[2] = { "open", filename };
        // Execute script file
        open( 2, argv );

        // Quit the interpreter
        char* argv2[2] = { "quit", "1" };
        quit( 2, argv2 );
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
    char* argv[30];

    // Get command from user
    char line[255];
    // Only proccess command if we successfully read something
    if( fgets( line, 255, input ) != NULL ) {
        // Parse the user command
        int parseErr = parseCommand( line, &argc, argv );

        // If a command was provided, try to execute it
        if( argc > 0 ) {
            // Used to detect if the command was unknown
            bool found = false;
            // Check if the command given is a known command
            // Only execute if parseCommand succeeded
            int start = 0;
            if(psOnly) { start = PS_CMD_START; }
            for( int i = start; i < NUM_COMMANDS && parseErr == 0; i++  ) {
                if( strcmp( commands[i], argv[0] ) == 0 ) {
                    // Ensure there is an active session prior to executing
                    // commands that require it.
                    if( session == NULL && i >= PS_CMD_START ) {
                        printf( "\nERROR:\tNo active session!\n" );
                        return;
                    }

                    // If we are executing a PS command, add this to file
                    if( i >= PS_CMD_START && !psOnly ) {
                        // Save coordinate system state
                        fprintf( session, "gsave\n" );
                    }

                    // Execute the command with provided args
                    (*states[i])(argc, argv);

                    // If we are executing a PS command, add this to file
                    if( i >= PS_CMD_START && !psOnly ) {
                        // Restore state
                        fprintf( session, "grestore\n" );
                    }

                    // We found a valid command
                    found = true;
                }
            }

            // Free the args list
            for( int i = 0; i < argc; i++ ) {
                free(argv[i]);
            }

            // If an invalid command was provided, display error
            if(!found) {
                printf( "\nERROR: Unknown command!\n" );
            }
        } else {
            // We weren't given a command. This should never happen.
            printf( "\nERROR: No command provided!\n" );
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
 * char* argv[] - Used to return the args found.
 */
int parseCommand( char* line, int* argc, char* argv[] ) {
    // Get the first argument
    char* current = strtok( line, " \n" );
    // Continue until there are no arguments left to read
    while( current != NULL ) {
        // Set the current argument in the args list
        argv[*argc] = (char*)malloc(strlen(current) + 1);
        if( argv[*argc] == NULL ) {
            printf( "\nERROR:\tUnable to allocate args list!\n" );
            return -1;
        }
        strcpy( argv[*argc], current );
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
 * int x, y   - Starting point for the path.
 * int closed - (optional) Whether the generated path will be closed or open.
 * int solid  - (optional) Whether the generated path should be filled or not.
 * int curve  - (optional) Whether the generated path is based on curves or lines.
 */
void path( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc < 3 || argc > 6 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tpath <start_x> <start_y>\n" );
    } else {
        if( session == NULL ) {
            printf( "\nERROR:\tNo active session!\n" );
            return;
        }

        printf( "\nEnter a series of points (one tuple per line), and 'done' when finished:\n" );

        // Get the starting point of the path
        int startX = atoi( argv[1] );
        int startY = atoi( argv[2] );

        // Get path options if provided
        int closed = 0;
        int solid  = 0;
        int curve  = 0;
        if( argc >= 4 ) {
            // If this path should be closed or open
            closed = atoi(argv[3]);
            if( argc >= 5 ) {
                // If this path should be filled or stroked
                solid = atoi(argv[4]);
                if( argc == 6 ) {
                    // If this path should use curves or lines
                    curve = atoi(argv[5]);
                }
            }
        }

        // Begin the path in the file
        fprintf( session, "newpath\n" );
        fprintf( session, "%d %d moveto\n", startX, startY );

        // Current cound of points entered
        int points = 0;

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
                    // Apply points as curves if option is set
                    if(curve) {
                        // Three points are required for a valid curve
                        if( points < 3 ) {
                            printf( "ERROR:\t Need at least %d more points to create a valid curve!\n", (3 - points) );
                            continue;
                        }
                        fprintf( session, "curveto\n" );
                    }

                    // Close the path if option is set
                    if(closed) {
                        fprintf( session, "closepath\n" );
                    }

                    // Apply the appropriate path finalizer
                    if(solid) {
                        fprintf( session, "fill\n" );
                    } else {
                        fprintf( session, "stroke\n" );
                    }

                    // End path construction
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
                            fprintf( session, "%d %d", x, y );
                            // Define points as lines if curve not set
                            if(!curve) {
                                fprintf( session, " lineto" );
                            }
                            fprintf( session, "\n" );

                            // Increment the number of points
                            points++;
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
 * Command state for drawing a user-defined closed path.
 *
 * Input:
 * int x, y - Starting point for the path.
 */
void closedPath( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 3 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tclosedpath <start_x> <start_y>\n" );
    } else {
        // Alloc space
        argv[3] = (char*)malloc(sizeof(char) + 1);
        memset( argv[3], 0, sizeof(char) + 1);

        // Make sure the alloc succeeded
        if( argv[3] != NULL ) {
            // Set the args for a solid path
            argv[3][0] = '1';
            argc++;

            // Call path with new args added
            path(argc, argv);
        } else {
            printf( "\nERROR:\tFailed to allocate args list!\n" );
            return;
        }

        // Free the extra args we added
        free(argv[3]);
    }
}

/*
 * Command state for drawing a user-defined solid path.
 *
 * Input:
 * int x, y - Starting point for the path.
 */
void solidPath( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 3 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tsolidpath <start_x> <start_y>\n" );
    } else {
        // Alloc space
        argv[3] = (char*)malloc(sizeof(char) + 1);
        argv[4] = (char*)malloc(sizeof(char) + 1);
        memset( argv[3], 0, sizeof(char) + 1);
        memset( argv[4], 0, sizeof(char) + 1);

        // Make sure the alloc succeeded
        if( argv[3] != NULL && argv[4] != NULL ) {
            // Set the args for a solid path
            argv[3][0] = '1';
            argv[4][0] = '1';
            argc += 2;

            // Call path with new args added
            path(argc, argv);
        } else {
            printf( "\nERROR:\tFailed to allocate args list!\n" );
            return;
        }

        // Free the extra args we added
        free(argv[3]);
        free(argv[4]);
    }
}

/*
 * Command state for drawing a user-defined bezier curve.
 *
 * Input:
 * int x, y - Starting point for the path.
 */
void curve( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 3 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tcurve <start_x> <start_y>\n" );
    } else {
        // Alloc space
        argv[3] = (char*)malloc(sizeof(char) + 1);
        argv[4] = (char*)malloc(sizeof(char) + 1);
        argv[5] = (char*)malloc(sizeof(char) + 1);
        memset( argv[3], 0, sizeof(char) + 1);
        memset( argv[4], 0, sizeof(char) + 1);
        memset( argv[5], 0, sizeof(char) + 1);


        // Make sure the alloc succeeded
        if( argv[3] != NULL && argv[4] != NULL && argv[5] != NULL ) {
            // Set the args for a solid path
            argv[3][0] = '0';
            argv[4][0] = '0';
            argv[5][0] = '1';
            argc += 3;

            // Call path with new args added
            path(argc, argv);
        } else {
            printf( "\nERROR:\tFailed to allocate args list!\n" );
            return;
        }

        // Free the extra args we added
        free(argv[3]);
        free(argv[4]);
        free(argv[5]);
    }
}

/*
 * Command state for drawing a user-defined, closed bezier curve.
 *
 * Input:
 * int x, y - Starting point for the path.
 */
void closedCurve( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 3 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tclosedcurve <start_x> <start_y>\n" );
    } else {
        // Alloc space
        argv[3] = (char*)malloc(sizeof(char) + 1);
        argv[4] = (char*)malloc(sizeof(char) + 1);
        argv[5] = (char*)malloc(sizeof(char) + 1);
        memset( argv[3], 0, sizeof(char) + 1);
        memset( argv[4], 0, sizeof(char) + 1);
        memset( argv[5], 0, sizeof(char) + 1);

        // Make sure the alloc succeeded
        if( argv[3] != NULL && argv[4] != NULL && argv[5] != NULL ) {
            // Set the args for a solid path
            argv[3][0] = '1';
            argv[4][0] = '0';
            argv[5][0] = '1';
            argc += 3;

            // Call path with new args added
            path(argc, argv);
        } else {
            printf( "\nERROR:\tFailed to allocate args list!\n" );
            return;
        }

        // Free the extra args we added
        free(argv[3]);
        free(argv[4]);
        free(argv[5]);
    }
}

/*
 * Command state for drawing a user-defined, filled bezier curve.
 *
 * Input:
 * int x, y - Starting point for the path.
 */
void solidCurve( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 3 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tcurve <start_x> <start_y>\n" );
    } else {
        // Alloc space
        argv[3] = (char*)malloc(sizeof(char) + 1);
        argv[4] = (char*)malloc(sizeof(char) + 1);
        argv[5] = (char*)malloc(sizeof(char) + 1);
        memset( argv[3], 0, sizeof(char) + 1);
        memset( argv[4], 0, sizeof(char) + 1);
        memset( argv[5], 0, sizeof(char) + 1);

        // Make sure the alloc succeeded
        if( argv[3] != NULL && argv[4] != NULL && argv[5] != NULL ) {
            // Set the args for a solid path
            argv[3][0] = '1';
            argv[4][0] = '1';
            argv[5][0] = '1';
            argc += 3;

            // Call path with new args added
            path(argc, argv);
        } else {
            printf( "\nERROR:\tFailed to allocate args list!\n" );
            return;
        }

        // Free the extra args we added
        free(argv[3]);
        free(argv[4]);
        free(argv[5]);
    }
}

/*
 * Command state for drawing a circle at center (x,y) and a given radius.
 *
 * Input:
 * int x, y - The center coordinates of the circle.
 * int r    - The radius of the circle.
 */
void circle( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 4 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tcircle <center_x> <center_y> <radius>\n" );
    } else {
        if( session == NULL ) {
            printf( "\nERROR:\tNo active session!\n" );
            return;
        }

        // Get the argument values
        int x = atoi(argv[1]);
        int y = atoi(argv[2]);
        int r = atoi(argv[3]);

        // Create the circle
        fprintf( session, "%d %d %d 0 360 arc\n", x, y, r );
        fprintf( session, "stroke\n");
    }
}

/*
 * Command state for drawing a filled circle at center (x,y) and a given radius.
 *
 * Input:
 * int x, y - The center coordinates of the circle.
 * int r    - The radius of the circle.
 */
void solidCircle( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 4 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tcircle <center_x> <center_y> <radius>\n" );
    } else {
        if( session == NULL ) {
            printf( "\nERROR:\tNo active session!\n" );
            return;
        }

        // Get the argument values
        int x = atoi(argv[1]);
        int y = atoi(argv[2]);
        int r = atoi(argv[3]);

        // Create the circle
        fprintf( session, "%d %d %d 0 360 arc\n", x, y, r );
        fprintf( session, "fill\n");
    }
}

/*
 * Command state for drawing an n-sided polygon.
 *
 * Input:
 * int x, y  - The center coordinates of the polygon.
 * int r     - The radius of the polygon.
 * int n     - The number of sides of the polygon.
 * int solid - (optional) Whether the polygon should be filled or not.
 */
void polygon( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc < 5 || argc > 6 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tpolygon <center_x> <center_y> <radius> <sides>\n" );
    } else {
        if( session == NULL ) {
            printf( "\nERROR:\tNo active session!\n" );
            return;
        }

        // Get the argument values
        float x = atof(argv[1]);
        float y = atof(argv[2]);
        float r = atof(argv[3]);
        float n = atof(argv[4]);

        // Get the solid setting arg
        int solid = 0;
        if( argc == 6 ) {
            solid = atoi(argv[5]);
        }

        // Calculate the all the points for the polygon
        for( int i = 0; i < n; i++ ) {
            // Get the x,y for the next point
            float curX = r * cos( 2.0 * M_PI * (i/n) ) + x;
            float curY = r * sin( 2.0 * M_PI * (i/n) ) + y;

            // Write the current point
            fprintf( session, "%f %f", curX, curY );
            if( i == 0 ) {
                // If this is the first point move into position
                fprintf( session, " moveto\n");
            } else {
                // Set lines for all other points
                fprintf( session, " lineto\n" );
            }
        }

        // Close the path to complete the polygon
        fprintf( session, "closepath\n" );

        // Draw the polygon
        if(solid) {
            fprintf( session, "fill\n" );
        } else {
            fprintf( session, "stroke\n" );
        }
    }
}

/* Command state for drawing a filled n-sided polygon.
 *
 * Input:
 * int x, y  - The center coordinates of the polygon.
 * int r     - The radius of the polygon.
 * int n     - The number of sides of the polygon.
 */
void solidPolygon( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 5 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tsolidpolygon <center_x> <center_y> <radius> <sides>\n" );
    } else {
        // Alloc space
        argv[5] = (char*)malloc(sizeof(char) + 1);
        memset( argv[5], 0, sizeof(char) + 1 );

        // Make sure the alloc succeeded
        if( argv[5] != NULL ) {
            // Set the args for a solid path
            argv[5][0] = '1';
            argc++;

            // Call path with new args added
            polygon(argc, argv);
        } else {
            printf( "\nERROR:\tFailed to allocate args list!\n" );
            return;
        }

        // Free the extra args we added
        free(argv[5]);
    }
}
/*
 * Command state to execute rotations
 *
 * Input:
 * int deg - degrees to rotate by
 */
void rotate( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 2 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\trotate <degrees>\n" );
    } else {
        if( session == NULL ) {
            printf( "\nERROR:\tNo active session!\n" );
            return;
        }

        int deg = strtol( argv[1], NULL, 10 );
        if( errno != 0 ) {
            printf( "\nERROR:\tArguments must be numbers!\n" );
            return;
        }

        // Apply the rotation
        fprintf( session, "%d rotate\n", deg );
        while(1) {
            printf( "\nEnter commands to construct a block of code to rotate:\n" );
            // Print repeat prompt
            printf( "+> " );

            // Evaluate input for body of the repeat block
            eval( input, true );

            // Ask the user if they wish to enter more commands
            printf( "\nFinished constructing rotate block? (y/n) " );
            char ans[255];
            // Check input and verify its validity
            if( fgets(ans, 255, input) != NULL && ans[0] == 'y' && strlen(ans) == 2 ) {
                break;
            }
        }

        printf( "Rotate block finished. Result of block will be rotated %d degrees.\n", deg );
    }
}

/*
 * Command state to begin a new session.
 *
 * Input:
 * char* name - The name of the session.
 */
void begin( int argc, char* argv[] ) {
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
        char* name = argv[1];

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
void end( int argc, char* argv[] ) {
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
void loop( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 2 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tloop <count>\n" );
    } else {
        if( session == NULL ) {
            printf( "\nERROR:\tNo active session!\n" );
            return;
        }

        int count = strtol( argv[1], NULL, 10 );
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
            eval( input, true );

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
 * Opens and evaluates a script file that conforms to this interpreter.
 *
 * Input:
 * char* filename - Name of the script file to open.
 */
void open( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 2 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\topen <filename>\n" );
    } else {
        // Check the file extension of the file
        char* extension = index( argv[1], '.' );
        if( extension == NULL || strcmp(extension, ".pscript") != 0 ) {
            printf( "\nERROR:\tUnsuppored filetype! Expected '.pscript' file!\n" );
            return;
        }

        // Close the session first
        if( session != NULL ) {
            printf( "Closing current session before loading script.\n" );
            end( 1, NULL );
        }

        // Open the file
        FILE* script = fopen( argv[1], "r" );

        // Check if open succeeded
        if( script == NULL ) {
            printf( "\nERROR:\tFailed to open script file!\n" );
            return;
        } else {
            printf( "\nExecuting user-defined script file: %s\n\n", argv[1] );

            // Evaluate the script
            while( !feof(script) ) {
                eval(script, false);
            }

            // Close the file
            fclose(script);
        }
    }
}

/*
 * Command state to quit the program.
 *
 * Input:
 * None
 */
void quit( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc < 1 || argc > 2 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\tquit\n" );
    } else {
        // Close the session first
        if( session != NULL ) {
            end( 1, NULL );
        }

        // Get the script setting arg
        int script = 0;
        if( argc == 2 ) {
            script = atoi(argv[1]);
        }

        // Free the args value for this call
        if(!script) {
            free(argv[0]);
        }

        printf( "Closing interpreter...\n" );

        // Close the input stream
        if(!script) {
            fclose(input);
        }

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
void help( int argc, char* argv[] ) {
    // Check if we have the correct number of arguments
    if( argc != 1 ) {
        printf( "\nERROR:\tInvalid number of arguments provided!\n" );
        printf( "Usage:\thelp\n" );
    } else {
        printf( "\nPostGen Manual\n" );
        printf( "--------------\n" );
        printf( "This interpreter behaves like a state machine, with each \n"
                "command mapped to its own state.  The commands given construct \n"
                "a PostScript file that can be opened in any PostScript viewer. \n"
                "Prior to executing any commands, a session must first be created \n"
                "which sets up the PostScript file that is being constructed.\n" );
        printf( "\nCommands:" );
        printf( "\nbegin [name]                         \tStarts a new session with the given name.\n" );
        printf( "                                       \tThis creates a PostScript file of the given name.\n" );
        printf( "\nend                                  \tEnds the current session and closes its file.\n" );
        printf( "\npath [x] [y]                         \tConstructs a user-defined, open path, starting at (x,y).\n"
                "                                       \tContinues to read tuples in until the user enters 'done'.\n" );
        printf( "\nclosedpath [x] [y]                   \tConstructs a user-defined, closed path, starting at (x,y).\n"
                "                                       \tContinues to read tuples in until the user enters 'done'.\n" );
        printf( "\nsolidpath [x] [y]                    \tConstructs a user-defined, filled path, starting at (x,y).\n"
                "                                       \tContinues to read tuples in until the user enters 'done'.\n" );
        printf( "\ncurve [x] [y]                        \tConstructs a user-defined, bezier curve, starting at (x,y).\n"
                "                                       \tContinues to read tuples in until the user enters 'done'.\n" );
        printf( "\nclosedcurve [x] [y]                  \tConstructs a user-defined, closed bezier curve, starting at (x,y).\n"
                "                                       \tContinues to read tuples in until the user enters 'done'.\n" );
        printf( "\nsolidcurve [x] [y]                   \tConstructs a user-defined, filled bezier curve, starting at (x,y).\n"
                "                                       \tContinues to read tuples in until the user enters 'done'.\n" );
        printf( "\ncircle [x] [y] [radius]              \tConstructs a circle with center at (x,y) and the given radius.\n" );
        printf( "\nsolidcircle [x] [y] [radius]         \tConstructs a filled circle with center at (x,y), and the given radius.\n" );
        printf( "\npolygon [x] [y] [radius] [sides]     \tConstructs an n-sided polygon centered at (x,y).\n"
                "                                       \tPolygon has given radius and number of sides.\n" );
        printf( "\nsolidpolygon [x] [y] [radius] [sides]\tConstructs a filled n-sided polygon centered at (x,y).\n"
                "                                       \tPolygon has given radius and number of sides.\n" );
        printf( "\nrotate [degrees]                     \tRotates the given construct by the given number of degrees.\n" );
        printf( "\nloop [count]                         \tRepeats the given construct count times.\n" );
        printf( "\nopen [filename]                      \tOpens the given script file and evaluates it.\n ");
        printf( "\nquit                                 \tCloses any open session and exits the interpreter.\n" );
        printf( "\nhelp                                 \tDisplays this dialog.\n" );
    }
}
