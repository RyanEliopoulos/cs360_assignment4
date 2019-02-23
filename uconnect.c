
#define ARG_LEN_MAX 1000

/* Error codes */
#define PIPE_ERR 1   /* pipe() failure */
#define EXEC_ERR 2   /* execvp() failure */
#define DUP_ERR 3    /* dup2 failure */
#define READ_ERR 4   /* read failure */
#define FIN_COL 5    /* last command-line arg was a : */
#define ARG_ERR 6    /* command-line argument problem */



#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

int readWrapper(int, char*, int);

int nextArg(char *[], int);

void main(int argc, char*argv[]) {

    /* check for args */
    if (argc == 1) {
        fprintf(stderr, "Need more args\n");
        exit(1);
    }

    /* at least one command line arg to process */
    char **parent_argv;    
    parent_argv = argv + 1; // first argument not itself
    argc--;  //  argc tracks parent_argv now


    /* find index of next : or find out there isn't one */
    int nxt_arg;
    if ( (nxt_arg = nextArg(parent_argv, argc)) == -1 ) { // if there were no separators
        printf("exec time\n");
            if (execvp(*parent_argv, parent_argv)) {
                fprintf(stderr, "error occurred\n%s\n", strerror(errno));
                exit(EXEC_ERR);
            }
    }

    printf("next arg is %d\n", nxt_arg);    

    /* check if the first arg is a : */ 
    if (nxt_arg == 0) {
        fprintf(stderr, ": is not a valid command\n");
        exit(1);
    }

    /* there is at least one :, so forking begins */
    /* need to save stdout for the final parent */
    int fout; 
    if ((fout = dup2(1, 9)) == -1) {
        fprintf(stderr, "dup2 failed\n%s\n", strerror(errno));
        exit(DUP_ERR);
    }
    printf("prepping file descriptors\n");
    /* set file descriptors properly */
    close(0);
    close(1);    

    int fd[2];
    int rdr, wtr;

    if (pipe(fd) == -1) {
        fprintf(stderr, "error opening pipe\n%s\n", strerror(errno));
        exit(PIPE_ERR);
    }
    
    rdr = fd[0];
    wtr = fd[1];

    /* demarcate child args */
    char *child_argv[argc];
    child_argv[0] = *parent_argv; /* marking arg list beginning */
    child_argv[nxt_arg] = (char *)0; /* terminate args */
    
    /* initial fork. Parent argv will be updated afterward */  
    /* only get here if there are at least two command-line arguments */
    if (fork()) {

        close(wtr);
        /* now we determine if we begin a fork loop */
        /* adjust parent_argv and argc */
        argc -= nxt_arg; // new index for parent_argv
        
        // DEBUG
        dprintf(fout, "dprint: argc is %d\n", argc);
        /* check for edge case - last argv arg is a : */ 
        if (argc == 1) {
            fprintf(stderr, "Final argument cannot be a :\n");
            exit(FIN_COL);
        }

         
        /* at least one more set of strings to parse */
        /* begin fork loop to process the remainders */
        parent_argv += nxt_arg + 1;  // +1 because we don't actually want the : 
        dprintf(fout, "parent_argv points at %s\n", *parent_argv);

        if (!strcmp(":", *parent_argv)) {
            fprintf(stderr, "final argument cannot be a :\n");
            exit(ARG_ERR);
        }

        /* this is where the fork loop would be */
        /* skipping for now */

        
        ////////////////////////
        ////////////////////////
        ////////////////////////

        /* this is where we go after the fork loop ends */
        /* and the final program is exec'ed */
        dup2(fout, 1); // set fd 1 to stdout from reserved fd
        close(fout);
        execvp(*parent_argv, parent_argv);
        dprintf(1, "shouldn't ever get here\n");
        
        
        /* just for debuggin now */
        close(wtr);
        char word[10]; 
        while(readWrapper(rdr, word, 1)) {
            write(fout, word, 1);
        }
    }
    
    /* child (exec morphs) */
    else {
        close(rdr);
        execvp(child_argv[0], child_argv);
    }
}

int readWrapper(int fd, char *buf, int count) {

    int red;
    if ( (red = read(fd, buf, count)) == -1) {
        fprintf(stderr, "read error\n%s\n", strerror(errno));
        exit(READ_ERR);
    }
    return red;
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




