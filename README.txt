able to use as many pipes as you want, or whatever the shell/OS limit is.
use argument switches/input as normal.
    example -- ./connect ls -lrt : sort -r : uniq : wc - l


the pipe symbol is only considered as a single character string.
That is, the string "::" is not interpreted as a pipe.

Syntax restrictions: 

    Argument list cannot begin with a pipe
    a pipe can only terminate the argument list if it is the first pipe symbol used.
        example of acceptable -- ./connect ls -lrt :  
        example of unacceptable -- ./connect ls -lrt : sort -r :

    Consecutive pipe symbols (unique argv elements) are not allowed (e.g. : :)

