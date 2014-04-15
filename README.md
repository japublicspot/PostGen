#PostScript Language Project
CS4121

## Group
Nathan Peterson

Connor Wlodarczak

##Description
This is a basic interpreter that acts as a state machine to generate
a PostScript file from a series of user-inputted commands.

##Commands
begin [name]                            Starts a new session with the given name.
                                        This creates a PostScript file of the given name.

end                                     Ends the current session and closes its file.

path [x] [y]                            Constructs a user-defined, open path, starting at (x,y).
                                        Continues to read tuples in until the user enters 'done'.

closedpath [x] [y]                      Constructs a user-defined, closed path, starting at (x,y).
                                        Continues to read tuples in until the user enters 'done'.

solidpath [x] [y]                       Constructs a user-defined, filled path, starting at (x,y).
                                        Continues to read tuples in until the user enters 'done'.

curve [x] [y]                           Constructs a user-defined, bezier curve, starting at (x,y).
                                        Continues to read tuples in until the user enters 'done'.

closedcurve [x] [y]                     Constructs a user-defined, closed bezier curve, starting at (x,y).
                                        Continues to read tuples in until the user enters 'done'.

solidcurve [x] [y]                      Constructs a user-defined, filled bezier curve, starting at (x,y).
                                        Continues to read tuples in until the user enters 'done'.

circle [x] [y] [radius]                 Constructs a circle with center at (x,y) and the given radius.

solidcircle [x] [y] [radius]            Constructs a filled circle with center at (x,y) and the given radius.

polygon [x] [y] [radius] [sides]        Constructs an n-sided polygon centered at (x,y).
                                        Polygon has given radius and number of sides.

solidpolygon [x] [y] [radius] [sides]   Constructs an n-sided polygon centered at (x,y).
                                        Polygon has given radius and number of sides.

rotate [degrees]                        Rotates the given construct by the given number of degrees.

loop [count]                            Repeats the given construct count times.

quit                                    Closes any open sessions and exits the interpreter.

help                                    Displays the help dialog.


