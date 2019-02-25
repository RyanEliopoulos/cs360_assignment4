able to use as many pipes as you want, or whatever the shell/OS limit is.
use argument switches/input as normal.
    example -- ./connect ls -lrt : sort -r : uniq : wc - l


the pipe symbol is only considered as a single character string.
That is, the string "::" is not interpreted as a pipe.

Syntax restrictions: 

    Argument list cannot begin with a pipe
    Consecutive pipe symbols (unique argv elements) are not allowed (e.g. : :)
    A trailing pipe is ignored, as long as it doesn't violate the above two rules


To run the program with the Makefile be sure to use 'make run invoke="{program/pipe string}"'

    Example -- ~make run invoke="ls -lrt : sort -r : wc -c"
