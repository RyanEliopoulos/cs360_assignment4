/*
 *  Ryan Paulos
 *  CS 360, Assignment 4
 *  Spring 2019 
 *
 *
 *
 *
 */



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

/* point char*[] elements to first int */
/* elements in char **                 */
/* punctuates char*[] with (char *)0   */
void populateChildArgs(char*[], char**, int);

/* returns offset of next pipe  */
/* or 0 if there isn't one      */
int nextArg(int, char*[]);

/* screens for syntax errors             */
/* can't start with a pipe               */
/* there can't be two pipes in a row     */

/* trailing pipes are ignored, as long   */
/* as it doesn't violate the above rules */    

/* returns 1 if trailing pipe, else 0    */
int screenArgs(int, char*[]);

/* magic happens here                            */
/* handles all exec calls, pipes, and fork logic */
void connect(int, char*[]);



/* wrappers */
void execvpWrapper(char *, char*[]);
void argPrinter(char *[]);
int readWrapper(int, char*, int);
void pipeWrapper(int[]);



/**********   Program Begins   *********/

void main(int argc, char*argv[]) {

    /* check for a trailing pipe */
    /* screenArgs returns 1 when */
    /* pipe is present */
    if (screenArgs(argc, argv)) {

        /* omit the trailing pipe in a new array */
        char *new_argv[argc];
        for (int i = 0; i < (argc - 1); i++) new_argv[i] = argv[i];
        new_argv[argc-1] = (char *)0;
    
        /* begin piping */
        connect(--argc, new_argv);
    }

    /* no trailing pipe present */
    /* no adjustments needed */
    /* begin piping */
    connect(argc, argv);
}


int screenArgs(int argc, char*argv[]) {

    /* check for args */
    if (argc == 1) {
        fprintf(stderr, "Need at least one argument\n");
        exit(ARG_ERR);
    }

    /* check args don't begin with : */
    if (!strcmp(":", argv[1])) {
        fprintf(stderr, "Arguments cannot begin with a ':'\n");
        exit(ARG_ERR);
    }
    
    /* now check for consecutive pipes  */  
    for (int i = 1; i < argc; i++) {
        if (!strcmp(":", argv[i]) && !strcmp(argv[i], argv[i-1])) {
            fprintf(stderr, "Cannot have consecutive ':' symbols\n");
            fprintf(stderr, "Check symbol at position %d\n", i-1);
            exit(ARG_ERR);
        } 
    }

    /* check for trailing pipe */
    if (!strcmp(":", argv[argc-1])) {
        return 1;
    }  
    return 0;
}


int readWrapper(int fd, char *buf, int count) {

    int red;
    if ( (red = read(fd, buf, count)) == -1) {
        fprintf(stderr, "read error\n%s\n", strerror(errno));
        exit(READ_ERR);
    }
    return red;
}


int nextArg(int argc, char* parent_argv[]) {

    for (int i = 1; i < argc; i++) { /* p_argv[0] will never be a colon */
        if (!strcmp(":", *(parent_argv + i))) {
            return i;
        }
    }
    return 0;
}


void connect(int argc, char*argv[]) {

    /* index argv to track argument processing */ 
    char **parent_argv;    
    parent_argv = argv + 1; 
    argc--;  /*  argc tracks parent_argv now */
    

    /* first check if there are any pipes at all*/
    int pipe_index;
    if ( !(pipe_index = nextArg(argc, parent_argv)) ) {  /* true if no pipes in command list */
        execvpWrapper(*parent_argv, parent_argv); 
    }

    /* there is at least one pipe and two progs */
    /* prepare for forking */

    /* need to save stdout for the final parent */
    int fout; 
    if ((fout = dup2(1, 9)) == -1) {
        fprintf(stderr, "dup2 failed\n%s\n", strerror(errno));
        exit(DUP_ERR);
    }

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
    populateChildArgs(child_argv, parent_argv, pipe_index);

    /* initial fork. Parent argv will be updated afterward */  
    /* only get here if there are at least two command-line arguments */
    if (fork()) {

        /* file descriptors */
        /* 0: current reader */
        /* 1: curent writer */
        /* 2: stderr */

        /* now update parent_arg info to see if there are further pipes */
        argc = --argc - pipe_index;  /* -- because the : is being skipped */
        parent_argv += pipe_index + 1;  // +1 because : is being skipped */

        /* fork loop */
        while( (pipe_index = nextArg(argc, parent_argv)) > 0) {
            close(wtr);

            pipeWrapper(fd);
            rdr = fd[0];
            wtr = fd[1];

            /* file descriptors */
            /* 0: reading from last from last command */
            /* 1: current reader */
            /* 2: stderr */
            /* 3: current writer */

            
            /* need to adjust parent_arg and child_arg values */
            populateChildArgs(child_argv, parent_argv, pipe_index);

            /* adjust arg values */
            /* ignoring the pipe */
            argc = --argc - pipe_index;  
            parent_argv += pipe_index + 1;

            /* fork parent prepares file descriptors */
            /* for the next loop                     */
            if (fork()) {

                /* fd maintenance for next loop */
                /* modify reader position */
                close(0); /* this is second before last command now */
                dup(rdr); /* now 0 */
                close(rdr); /* 1: NOTHING */
                
                /* modify writer position */
                dup(wtr);  /* 1: current writer*/
                close(wtr); /* 3: nothing */
                wtr = 1; 

                /* file descriptors */
                /* 0: current reader */
                /* 1: current writer */
                /* 2: stderr */
            }
            
            /* fork child     */
            /* exec to stdout */
            else {
                close(rdr); /* this is fd 1 */
                dup(wtr); /* moves writer to position 1 */
                close(wtr); 

                /* file descriptors */  
                /* 0: reading from last command */
                /* 1: current writer */
                /* 2: stderr         */

                execvpWrapper(*child_argv, child_argv);
            }
        } 

        /* this is where we go after the fork loop ends */
        /* only a single program invocation remains     */

        close(wtr);
        dup2(fout, 1); // set fd 1 to stdout from reserved fd
        close(fout);

        execvp(*parent_argv, parent_argv);
    }
   
    /* logic of first child process */
    else {
        close(rdr);
        execvp(child_argv[0], child_argv);
    }
}


void populateChildArgs(char *child_argv[], char **parent_argv, int index) {

    for (int i = 0; i < index; i++) {
        child_argv[i] = *(parent_argv + i);
    }
    child_argv[index] = (char *)0;
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

/* debug tool */
void argPrinter(char *args[]) {
    
    int i = 0;
    while (args[i] != NULL) {
        dprintf(9, "argPrinter i:%d and arg: <%s>\n", i, args[i++]);
    }  
}
