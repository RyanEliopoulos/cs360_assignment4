#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>


int nextArg(char *[], int);

void main(int argc, char*argv[]) {

    char **parent_argv;    

    parent_argv = argv + 1; // first argument not itself
    argc--;  //  argc tracks parent_argv now


    /* find index of next : or find out there isn't one */
    int nxt_arg;
    if ( (nxt_arg = nextArg(parent_argv, argc)) == -1 ) { // if there were no separators
        printf("exec time\n");
            if (execvp(*parent_argv, parent_argv)) {
                fprintf(stderr, "error occurred\n%s\n", strerror(errno));
            }
    }
    printf("next arg is %d\n", nxt_arg);    
    /* check if the first arg is a : */ 
    if (!nxt_arg) fprintf(stderr, ": is not a valid command\n");

    /* there is at least one :, so forking begins */
    char **child_argv = malloc (sizeof (char *) * argc);

    child_argv = parent_argv; /* marking arg list beginning */
    child_argv[nxt_arg] = (char *)0; /* terminate args */


    /* this is just debug now */
    //if (execvp(*child_argv, child_argv) == -1) fprintf(stderr, "second exec failed\n%s\n", strerror(errno));


}


/* returns offset of next : separator */
/* if 0 is returned the caller should adjust  */
/* parent_argv + 1 and call again */
int nextArg(char* parent_argv[], int argc) {

    int i = 0;
    while (parent_argv[i] != NULL) {
        if (!strcmp(":", parent_argv[i])) return i; 
        i++;
    }
    return -1; // means no more : in argv
}
