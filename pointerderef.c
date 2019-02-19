/* working out how to do assignment 4 with indefinite arg count */

#include<stdio.h>
#include<string.h>

void childArgs( char*[], char **);
void main (int argc, char *argv[]) {

    /* need to dup stdout a couple times so its ~ 5, outside the fork cycle */
    /* close the fd slots it opened on the way. */
    /* then once there are no more arguments, write to saved stdout */


    char **parent_arg = argv + 1;
    printf("so now parent_arg starts at %s\n", *parent_arg);

    char *child_arg[argc];

    //// if next arg //
    /* load the next set of strings for the child */
    childArgs(child_arg, parent_arg);
    
    /* readjust parent_arg to new position in argument stream */    
    //parentArgs(argc, parent_arg);

    /* child args fuel exec */
    //child_arg[0] = *parent_arg;
    int j = 0;
    while (child_arg[j] != NULL) printf("%s\n", child_arg[j++]);

    // Now we are pointing at the :
    // this is just in case there is nothing beyond the last colon
    parent_arg += j; // Point at colon now
    printf("%s\n", *parent_arg);

  
    if (!strcmp(*parent_arg, *(argv + argc - 1))) {
       printf("colon is the last argument\n"); 
       /* from here we print to std */
    }
    else {
        printf("%s is that thing \n", *(argv + argc - 1));
    }
    //parentArg(parent_arg, j); // because we use a nextArg fnx that is true contingent upon a next :
                                // we don't know of there is actually anything beyond that
 }


void parentArgs(char **parent_arg, int j) {

    

}



void childArgs (char *child_arg[], char **parent_arg) {

    int i = 0;
    /* copy the pointers until the separator */
    while (strcmp(":", parent_arg[i])) {
        child_arg[i] = *(parent_arg + i++);
        printf("new string %s\n", child_arg[i-1]);
    }
    child_arg[i] = NULL; 
}
