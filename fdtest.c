#include<unistd.h>
#include<errno.h>
#include<stdio.h>
#include<string.h>
// execvp(program, const char *args[])

void main() {

    char *args[100];
    char arg2[100] = "Yo sup bitch?";
    char arg1[100] = "ls";

    args[0] = arg1;
    args[1] = arg2;
    args[2] = (char *)0;
    
    //execlp("ls", "ls", arg, (char *)0);    
    if ( execvp("ls", args) == -1) {
        printf("fucking faggot\n");
        
    }
    printf("here??\n");
}
