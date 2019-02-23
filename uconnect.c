
#define ARG_LEN_MAX 1000

/* Error codes */
#define PIPE_ERR 1
#define EXEC_ERR 2
#define DUP_ERR 3
#define READ_ERR 4



#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>


int nextArg(char *[], int);

void main(int argc, char*argv[]) {

    if (argc == 1) {
        fprintf(stderr, "Need more args\n");
        exit(1);
    }

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
    if (!nxt_arg) fprintf(stderr, ": is not a valid command\n");

    /* there is at least one :, so forking begins */
    
    /* need to save stdout for the final parent */
    int fout; 
    if ((fout = dup2(2, 9)) == -1) {
        fprintf(stderr, "dup2 failed\n%s\n", strerror(errno));
        exit(DUP_ERR);
    }

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
    char **child_argv = malloc (sizeof (char *) * argc);
    child_argv = parent_argv; /* marking arg list beginning */
    child_argv[nxt_arg] = (char *)0; /* terminate args */

    /* initial fork. Parent argv will be updated afterward */  
    if (fork()) {

        /* now we determine if we begin a fork loop */
        /* 
        /* just for debuggin now */
        close(wtr);
        char word[10]; 
        printf("in parent, can we hear child?\n");
        while(read(rdr, word, 1)) {
            printf("%s", word);
        }
    }
    
    /* child (exec morphs) */
    else {
        close(rdr);
        execvp(*child_argv, child_argv);
    }
    /* this is just debug now */
    //if (execvp(*child_argv, child_argv) == -1) fprintf(stderr, "second exec failed\n%s\n", strerror(errno));


}

int readWrapper(int fd, char *buf, int count) {

    if (read(fd, buf, count) == -1) {
        fprintf(stderr, "read error\n%s\n", strerror(errno));
        exit(READ_ERR);
    }
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




