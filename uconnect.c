
#define ARG_LEN_MAX 1000

/* Error codes */
#define PIPE_ERR 1   /* pipe() failure */
#define EXEC_ERR 2   /* execvp() failure */
#define DUP_ERR 3    /* dup2 failure */
#define READ_ERR 4   /* read failure */
#define ARG_ERR 6    /* command-line argument problem */


#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

int readWrapper(int, char*, int);

void pipeWrapper(int[]);
/* finds the offset from char*[] of the next colon */
int nextArg(int, char*[]);

/* screens for syntax errors */
/* can't begin or end with : */
/* consecutive : are forbidden */
void screenArgs(int, char*[]);

void execvpWrapper(char *, char*[]);

void main(int argc, char*argv[]) {
   
    /* check syntax of command line arguments */ 
    screenArgs(argc, argv);
    /* at least one command line arg to process */

    /* index argv to track argument processing */ 
    char **parent_argv;    
    parent_argv = argv + 1; // first argument not itself
    argc--;  //  argc tracks parent_argv now


    /* first check if there are any pipes at all*/
    int pipe_index;
    if ( (pipe_index = nextArg(argc, parent_argv)) == 0 ) {  /* true if no pipes in command list */
        printf("dropping into the pipe\n");
        execvpWrapper(*parent_argv, parent_argv); 
    }

    printf("hello for reference\n"); 
    /* there is at least one pipe and two progs */
    /* prepare for forking */

    /* find next : */

    printf("next is at offset: %d\n", pipe_index);    

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

    pipeWrapper(fd);
    
    rdr = fd[0];
    wtr = fd[1];

    /* demarcate child args */
    char *child_argv[argc];
    child_argv[0] = *parent_argv; /* marking arg list beginning */
    child_argv[pipe_index] = (char *)0; /* terminate args */
    
    /* initial fork. Parent argv will be updated afterward */  
    /* only get here if there are at least two command-line arguments */
    if (fork()) {
        close(wtr);

        /* now update parent_arg info to see if there are further pipes */
        argc = --argc - pipe_index;  /* -- because the : is being skipped */
        parent_argv += pipe_index + 1;  // +1 because : is being skipped */

        dprintf(fout, "parent_argv points at %s\n", *parent_argv);
        dprintf(fout, "Argc is %d\n parent_argv:%s\n", argc, *parent_argv); 

        while(nextArg(argc, parent_argv)) {
            // fork loop here
        } 


        dprintf(fout, "here????\n");
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


/* checks argv for any syntax violations */
/* can't start or end with a : */
/* there can't be two : in a row */
void screenArgs(int argc, char*argv[]) {

    /* check for args */
    if (argc == 1) {
        fprintf(stderr, "Need at least one argument\n");
        exit(ARG_ERR);
    }

    /* check args don't begin with : */
    if (!strcmp(":", argv[1])) {
        fprintf(stderr, "Arguments cannot being with a ':'\n");
        exit(ARG_ERR);
    }

    /* check args dont end with : */
    if (!strcmp(":", argv[argc-1])) {
        fprintf(stderr, "Arguments cannot end with a ':'\n");
        exit(ARG_ERR);
    }  
  
    /* now check for consecutive : */  
    for (int i = 1; i < argc; i++) {
        if (!strcmp(":", argv[i]) && !strcmp(argv[i], argv[i-1])) {
            fprintf(stderr, "Cannot have consecutive ':' symbols\n");
            fprintf(stderr, "Check symbol at position %d\n", i-1);
            exit(ARG_ERR);
        } 
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
/* or 0 if there isn't one */
int nextArg(int argc, char* parent_argv[]) {
    printf("in nxt arg\n");
    for (int i = 1; i < argc; i++) { /* p_argv[0] will never be a colon */
        if (!strcmp(":", *(parent_argv + i))) return i;
    }
    return 0;
    /*
    int i = 0;
    while (parent_argv[i] != NULL) {
        if (!strcmp(":", parent_argv[i])) return i; 
        i++;
    }
    return -1; // means no more : in argv
    */
}


void pipeWrapper(int fd[]) {
    
    if (pipe(fd) == -1) {
        fprintf(stderr, "pipe error\n");
        exit(PIPE_ERR);
    }
}
void execvpWrapper(char *prog, char*prog_args[]) {

    if (execvp(prog, prog_args)) {
        fprintf(stderr, "Execvp failed\n%s\n", strerror(errno));
        exit(EXEC_ERR);
    }
}

