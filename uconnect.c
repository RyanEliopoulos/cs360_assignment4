
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

void populateChildArgs(char*[], char**, int);
void argPrinter(char *[]);
int readWrapper(int, char*, int);

void pipeWrapper(int[]);
/* finds the offset from char*[] of the next colon */
int nextArg(int, char*[]);

/* screens for syntax errors */
/* can't begin or end with : */
/* consecutive : are forbidden */
/* returns 1 if trailing pipe  */
/* present else returns 0 */
int screenArgs(int, char*[]);


/* magic happens here */
void connect(int, char*[]);

void execvpWrapper(char *, char*[]);

void main(int argc, char*argv[]) {

    /* fixed main */   
    /* to deal with trailing colons */
    
    /* if argc == 1 fprintf "insufficient arguments" */
    /* if lead with colon fprintf ": cannot be first argument: */ 
    /* if two colons in a row fprintf("Can't have two consecutive ':' arguments" */

    /* if trailingColon(argc, argv) {                       */
    /*      create new_argv to hold all but the last string */
    /*      decrement argc                                  */

    /* connect(argc, argv)                                  */

    /* trailing pipe present */
    /* copy all but last argv into a new array */
    if (screenArgs(argc, argv)) {
        printf("Trailing pipe present\n");
        char *new_argv[argc];
        for (int i = 0; i < (argc - 1); i++) new_argv[i] = argv[i];
        new_argv[argc-1] = (char *)0;
        
        connect(--argc, new_argv);
    }
    /* no trailing pipe present */
    /* no further adjustments needed */
    connect(argc, argv);
    
    /* DONE */
    printf("never Here, right?\n");
}


/* checks argv for any syntax violations */
/* can't start or end with a : */
/* there can't be two : in a row */
/* returns 1 if trailing pipe, else 0 */
int screenArgs(int argc, char*argv[]) {

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
    
    /* now check for consecutive : */  
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
        //fprintf(stderr, "Arguments cannot end with a ':'\n");
        //exit(ARG_ERR);
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

/* returns offset of next : separator */
/* or 0 if there isn't one */
int nextArg(int argc, char* parent_argv[]) {
    dprintf(9, "in nxt arg\n");
    dprintf(9, "argc is %d\n", argc);
    dprintf(9, "parent_argv is %s\n", *parent_argv); 

    for (int i = 1; i < argc; i++) { /* p_argv[0] will never be a colon */
        if (!strcmp(":", *(parent_argv + i))) {
            dprintf(9, "nextArg is returning %d\n", i);
            return i;
        }
    }
    dprintf(9, "nextArg is returning 0\n");
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


void connect(int argc, char*argv[]) {

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
    //child_argv[0] = *parent_argv; /* marking arg list beginning */
    //child_argv[pipe_index] = (char *)0; /* terminate args */

    /* testing new copy arg */
    populateChildArgs(child_argv, parent_argv, pipe_index);
    /* end test */

    /* need a "copyArgs" command to populate child_args with everything from parent */ 


    argPrinter(child_argv); 
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

        dprintf(fout, "parent_argv points at %s\n", *parent_argv);
        dprintf(fout, "Argc is %d\n parent_argv:%s\n", argc, *parent_argv); 

        while( (pipe_index = nextArg(argc, parent_argv)) > 0) {
            // fork loop here
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
            argPrinter(child_argv);

            argc = --argc - pipe_index; 
            parent_argv += pipe_index + 1;

            dprintf(fout, "parent argv is %s\n", *parent_argv);

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
            
            /* fork child */
            else {
                close(rdr); /* this is fd 1 */
                dup(wtr); /* moves writer to position 1 */
                close(wtr); 
                dprintf(fout, "make it here, eh?\n");       
                /* file descriptors */  
                /* 0: reading from last command */
                /* 1: current writer */
                /* 2: stderr         */

                execvpWrapper(*child_argv, child_argv);
            }
        } 

        /* this is where we go after the fork loop ends */
        /* and the final program is exec'ed */

        close(wtr);
        dprintf(fout, "Final chapter ????\n");
        dprintf(fout, "parent_argv is %s\n", *parent_argv);
        dup2(fout, 1); // set fd 1 to stdout from reserved fd
        close(fout);
        execvp(*parent_argv, parent_argv);
        dprintf(fout, "shouldn't ever get here\n");
         

        /* this is the final forked parent printing to stdout */        
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

void argPrinter(char *args[]) {
    
    int i = 0;
    while (args[i] != NULL) {
        dprintf(9, "argPrinter i:%d and arg: <%s>\n", i, args[i++]);
    }  
}
