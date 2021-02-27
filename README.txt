This program emulates shell program invocation and pipe syntax where ':' replaces '|'
 *  
 *  Example -- ./connect ls -lrt : sort -r : uniq : wc -c
 *  
 *  -program arguments cannot begin with a pipe 
 *  -program arguments cannot contain consecutive pipes 
 *  -a string of two pipes, ::, is not interpreted as pipes
 *  -a trailing pipe is ignored if not violating the above rules
 *
 *  The user may use as many program invocations and 
 *  pipe redirections as they so desire
 *  The only limiting factors I can think of would be
 *  a process creation limit or exceeding the max
 *  length allowed for argv.

To run the program with the Makefile be sure to use 'make run invoke="{program/pipe string}"'

    Example -- ~make run invoke="ls -lrt : sort -r : wc -c"
