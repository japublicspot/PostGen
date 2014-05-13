#PostGen

##Description
This program was created as a project for a Programming Languages class at Michigan Tech (CS4121) as
a way to research and learn about the PostScript language.

This is a basic interpreter that acts as a state machine to generate
a PostScript file from a series of user-inputted commands.

##Compilation
Simply execute `make` in the root project directory. (ensure that you have clang installed)

An executable will be created at `./bin/postgen`.

##Usage
Run the interpreter by executing `./postgen`

When executing the intperpreter a filename may also optionally be provided:
```
./postgen script.pscript
```

When a filename is provided, the interpreter will open and evaluate the contents of that file.
The file must be of type `.pscript`, and must be implemented using only commands supported by the interpreter as defined below.

There are a several script examples in the folder `sample-scripts` that can be executed as described above.

If no file is specified, then the interpreter executes normally and enters an eval loop.
You must start by issueing `begin` with a session name.
This will create the PostScript file of that name to be constructed by the subsequent commands.

Proceed by entering any of the commands that are supported by the interpreter, as defined below.

When finished with a session, simply call `end`.  Likewise, when you wish to exit the interpreter completely, call `quit`.

Once you are done generating a PostScript file, you may then open it with any PostScript viewer.

##Commands
###begin [name]
Starts a new session with the given name.

This creates a PostScript file of the given name.

###end
Ends the current session and closes its file.

###path [x] [y]
Constructs a user-defined, open path, starting at (x,y).

Continues to read tuples in until the user enters 'done'.

###closedpath [x] [y]
Constructs a user-defined, closed path, starting at (x,y).

Continues to read tuples in until the user enters 'done'.

###solidpath [x] [y]
Constructs a user-defined, filled path, starting at (x,y).

Continues to read tuples in until the user enters 'done'.

###curve [x] [y]
Constructs a user-defined, bezier curve, starting at (x,y).

Continues to read tuples in until the user enters 'done'.

###closedcurve [x] [y]
Constructs a user-defined, closed bezier curve, starting at (x,y).

Continues to read tuples in until the user enters 'done'.

###solidcurve [x] [y]
Constructs a user-defined, filled bezier curve, starting at (x,y).

Continues to read tuples in until the user enters 'done'.

###circle [x] [y] [radius]
Constructs a circle with center at (x,y) and the given radius.

###solidcircle [x] [y] [radius]
Constructs a filled circle with center at (x,y) and the given radius.

###polygon [x] [y] [radius] [sides]
Constructs an n-sided polygon centered at (x,y).

Polygon has given radius and number of sides.

###solidpolygon [x] [y] [radius] [sides]
Constructs an n-sided polygon centered at (x,y).

Polygon has given radius and number of sides.

###rotate [degrees]
Rotates the given construct by the given number of degrees.

###loop [count]
Repeats the given construct count times.

###open [filename]
Opens the given script file and evaluates it.

###quit
Closes any open sessions and exits the interpreter.

###help
Displays the help dialog.

##Repository
Repository for this project exists at:

https://github.com/ntpeters/PostGen
