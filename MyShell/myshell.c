#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <ctype.h>

#define lineSize 1024
#define historySize 20
#define variableSize 20

void sig_handler(int sig)
{
  printf("\nYou typed Control-C!\n");
}

typedef struct{
    char name[20];
    char value[20];
} var;

char command[lineSize];
char commandtmp[lineSize];
char commandtmp2[lineSize];
char keywords[5];
char thenCommand[lineSize];
char elseCommand[lineSize];

char error_message[60];

char *saveptr1, *saveptr2;

char ifOutput[lineSize];

char path[lineSize];
char prompt[lineSize] = "hello:";
char history[historySize][lineSize];
var variables[variableSize];

char *token, *token2;
int i;
char *infile, *outfile, *errfile;
int isHistory=0;
int fd, amper, redirectIndexes[3], append, retid, status, argc, historyIndex=0, lastStatus;
int fildes[2], fildes2[2];
int originalStdin, originalStdout, originalStderr;
char *argv[30];
int pipeCount;

int* argc_ptr = &argc;

int historyAmount=0;
int isTest=0;

char* cmdptr = command;

char*** pipeArgs;
int* pipeArgsCounts;
void performRedirect();

void shellCommand(char** argv, int argc){
    //is command empty
    if(!argc){
        return;
    }
    
    if(argc == 2 && !strcmp(argv[0], "cd")){
        strcpy(path, "./");
        strcat(path,argv[1]);
        printf("changing directory to %s\n", path);

        chdir(path);
        return;
    }

    if(argc == 3 && !strcmp(argv[1], "=")){
        if(!strcmp(argv[0], "prompt")){
            strcpy(prompt, argv[2]);
            return;
        }
        
        if(argv[0][0]=='$'){
            for(int i=0;i<variableSize;i++){
                if(!strcmp(variables[i].name, &argv[0][1]) || !strcmp(variables[i].name, "")){
                    strcpy(variables[i].name, &argv[0][1]);
                    strcpy(variables[i].value, argv[2]);
                    return;
                }
            }
            return;
        }
    }

    if(!strcmp(argv[0], "read")){
        for(int i=0;i<variableSize;i++){
            if(!strcmp(variables[i].name, argv[1])){
                printf("%s\n", variables[i].value);
                return;
            }
        }
        printf("cant find variable %s\n", argv[1]);
        return;
    }
    
    if(fork() == 0){
        if(!strcmp(argv[0], "echo")){
            if(!strcmp(argv[1], "$?"))
                printf("%d",lastStatus);
            else{
                for(int i=1;i<argc;i++)
                    if(argv[i]!=NULL)
                        printf("%s ",argv[i]);

            }
            printf("\n");
            exit(0);
        }
    
        execvp(argv[0], argv);
        exit(0);
    }
    else{
        while (wait(&status) != -1) {}
    }
    
}

void findRedirectIndexes(char** argv, int argc){
    for(int i=0;i<3;i++)
        redirectIndexes[i]=-1;

    for(int i=0;i<argc;i++){
        if(!strcmp(argv[i], "<"))
            redirectIndexes[0]=i;

        if(!strcmp(argv[i], ">") || !strcmp(argv[i],">>"))
            redirectIndexes[1]=i;

        if(!strcmp(argv[i], "2>"))
            redirectIndexes[2]=i;
    }
}

void getRedirectNames(char** argv, int argc){
    if (redirectIndexes[0]!=-1) {
        infile = argv[redirectIndexes[0]+1];
        argv[redirectIndexes[0]] = NULL;
        argv[redirectIndexes[0]+1] = NULL;

        *argc_ptr = *argc_ptr - 2;
    }

    if (redirectIndexes[1]!=-1) {
        outfile = argv[redirectIndexes[1]+1];

        if(!strcmp(argv[redirectIndexes[1]], ">>"))
            append=1;
        
        argv[redirectIndexes[1]] = NULL;
        argv[redirectIndexes[1]+1] = NULL;

        *argc_ptr = *argc_ptr - 2;
    }

    if (redirectIndexes[2]!=-1) {
        errfile = argv[redirectIndexes[2]+1];
        argv[redirectIndexes[2]] = NULL;
        argv[redirectIndexes[2]+1] = NULL;

        *argc_ptr = *argc_ptr - 2;
    }
}

void performRedirect(){
    /* redirection of IO ? */
    if(redirectIndexes[0]!=-1){
        fd = open(infile,O_RDONLY,0660);
        close(STDIN_FILENO); 
        dup(fd); 
        close(fd); 
    }

    if (redirectIndexes[1]!=-1) {
        if(append)
            fd = open(outfile,O_WRONLY|O_APPEND|O_CREAT,0660); 
        else
            fd = creat(outfile, 0660); 
        
        close (STDOUT_FILENO) ; 
        dup(fd); 
        close(fd);
        
        /* stdout is now redirected */
    } 
    
    if(redirectIndexes[2]!=-1){
        fd = creat(errfile, 0660); 
        close (STDERR_FILENO) ; 
        dup(fd); 
        close(fd); 
    }
}

void redirect(char** argv, int argc){
    findRedirectIndexes(argv, argc);
    getRedirectNames(argv, argc);
    performRedirect();
}

void findAmperIndex(char** argv, int argc){
    amper=-1;

    for(int i=0;i<argc;i++)
        if(!strcmp(argv[i], "&")){
            amper = i;
            break;
        }
}

void parseCommandString(char* command){
    /* parse command line */
    i = 0;
    isHistory=0;
    token = strtok_r(strcpy(commandtmp, command)," ", &saveptr1);
    while (token != NULL)
    {
        if(!strcmp(token, "!!")){
            isHistory=1;
            int historyIDX = (historyIndex-1)%historySize;
            if(historyIDX<0)
                historyIDX+=historySize;

            token2 = strtok_r(strcpy(commandtmp2, history[historyIDX]), " ", &saveptr2);
            while(token2 != NULL){
                argv[i] = token2;
                token2 = strtok_r(NULL, " ", &saveptr2);
                i++;
                if (token2 && ! strcmp(token2, "|")) {
                    pipeCount++;
                }
            }
        }else{
            argv[i] = token;
            i++;
            if (token && ! strcmp(token, "|")) {
                pipeCount++;
            }
        }
        token = strtok_r(NULL, " ", &saveptr1);
    }
    int equalsIndex = 0;
    for(int j=0;j<i;j++){
        if(!strcmp(argv[j], "=")){
            equalsIndex = j;
            break;
        }
    }

    for(int j=equalsIndex;j<argc;j++){
        if(argv[j]!=NULL && argv[j][0]=='$'){
            for(int k=0;k<variableSize;k++){
                if(!strcmp(&argv[j][1], variables[k].name)){
                    strcpy(argv[j], variables[k].value);
                    break;
                }
            }
        }
            
    }

    argv[i] = NULL;
    argc = i;
}

void evalCommandString(char* command){
    pipeCount = 0;
    append=0;

    parseCommandString(command);
    if(isHistory){
        for(int i=0;i<argc;i++)
            printf("%s ", argv[i]);
        printf("\n");
    }
    findAmperIndex(argv, argc);

    if(pipeCount){
        if(amper!=-1){
            perror("& not allowed in command with pipes");
            return;
        }
        
        pipeArgs = (char***) calloc(pipeCount+1, sizeof(char**));
        pipeArgsCounts = (int*) malloc((pipeCount+1)*sizeof(int));
        if(!pipeArgs)
            perror("char*** calloc error");

        if(!pipeArgsCounts)
            perror("int* calloc error");
            
        for(int i=0;i<pipeCount+1;i++){
            pipeArgs[i]=(char**)calloc(10, sizeof(char*));
            if(!pipeArgs[i])
                perror("char** calloc error");
        }

        int j=0;
        int k=0;
        char** currentArgs = pipeArgs[j];
        for(int i=0; i<argc; i++){
            if(!strcmp(argv[i], "|")){
                pipeArgsCounts[j]=k;
                currentArgs = pipeArgs[++j];
                k=0;
            }else
                currentArgs[k++]=argv[i];
        }
        pipeArgsCounts[j]=k;
        
        
        int old_stdin = dup(STDIN_FILENO);
        
        for(i=0;i<pipeCount;i++){
            pipe (fildes);
            if (fork() == 0) {
                dup2(fildes[1], 1);
                shellCommand(pipeArgs[i], pipeArgsCounts[i]);
                exit(0);
            }else
                while (wait(&status) != -1) {}

            dup2(fildes[0], 0);
            close(fildes[1]);
        }

        if (fork() == 0) {
            shellCommand(pipeArgs[i], pipeArgsCounts[i]);
            exit(0);
        }else
            while (wait(&status) != -1) {}

        dup2(old_stdin, STDIN_FILENO);
        close(old_stdin);

        for(int i=0;i<pipeCount+1;i++)
            free(pipeArgs[i]);
        

        free(pipeArgs);
        free(pipeArgsCounts);
    }else{
        
        if(amper!=-1){
            if(amper == argc-1){
                argv[argc - 1] = NULL;
                argc--;
            }
            else
                perror("Found & not in final position in command, aborting.");
        }

        originalStdin = dup(STDIN_FILENO);
        originalStdout = dup(STDOUT_FILENO);
        originalStderr = dup(STDERR_FILENO);

        redirect(argv, argc);
        
        shellCommand(argv, argc);

        dup2(originalStdin, STDIN_FILENO);
        dup2(originalStdout, STDOUT_FILENO);
        dup2(originalStderr, STDERR_FILENO);
    }
}


void trim(char* str){
    int l=0;
    int r=strlen(str)-1;
    
    while(l<strlen(str)-1 && !(str[l]>= '!' && str[l] <= '~'))
        l++;
    while(r>=0 && !(str[r]>= '!' && str[r] <= '~'))
        r--;
    
    strcpy(str, &str[l]);
    str[r-l+1]='\0';
}

int main(int cmdargc, char *cmdargv[]) {
    signal(SIGINT, sig_handler);

    size_t size =lineSize;
    if(cmdargc>1){
        if(!strcmp(cmdargv[1], "-t"))
            isTest=1;
    }

    int chars=-50;
    
    while (1)
    {   
        chars=-50;
        if(!isTest)
            printf("%s ", prompt);

        historyAmount=0;

        chars = getline(&cmdptr, &size, stdin);
         
        fflush(stdout);
        
        if(command[0] == '\033'){
            for(int i=0;i<strlen(command)-2;i++){
                if(command[i] == '\033' && command[i+1] == '[')
                    switch(command[i+2]){
                        case 'A':
                            historyAmount++;
                            break;
                        case 'B':
                            if(historyAmount>0)
                                historyAmount--;
                            break;
                    }
                    
            }
            
            if(!historyAmount)
                continue;
            else{
                int hi = (historyIndex - historyAmount)%historySize;
                if(hi<0)
                    hi+=historySize;
                
                strcpy(command, history[hi]);
                
                if(command[0] == '\0'){
                    sprintf(error_message, "No command in history at index %d", hi);
                    perror(error_message);
                    continue;
                }else{
                    printf("\033[1A");
                    printf("\x1b[2K");
                    printf("%s %s\n", prompt , command);
                }
                
            }
        }

        trim(command);

        if(command[0] == 'i' && command[1] == 'f'){
            fgets(keywords,lineSize, stdin);

            trim(keywords);
            if(strcmp(keywords, "then")){
                perror("Expected `then` after `if`");
                exit(1);
            }

            fgets(thenCommand,lineSize, stdin);

            trim(thenCommand);

            fgets(keywords,lineSize, stdin);
            trim(keywords);

            if(strcmp(keywords, "else")){
                perror("Expected `else` after `then`");
                exit(1);
            }

            fgets(elseCommand,lineSize, stdin);
            trim(elseCommand);

            fgets(keywords,lineSize, stdin);
            trim(keywords);
            if(strcmp(keywords, "fi")){
                printf("keywords: %s\n", keywords);
                perror("Expected `fi` after `else`");
                exit(1);
            }

            strcpy(command, &command[3]);
            pipe(fildes2);
            char tmp[1] = {EOF};
            write(fildes2[1], tmp, 1);
            
            if(fork() == 0){
                dup2(fildes2[1], STDOUT_FILENO);
                evalCommandString(command);
                exit(0);
            }else{
                while (wait(&status) != -1) {}

                int nbytes = read(fildes2[0], ifOutput, sizeof(ifOutput));
                if(nbytes-1)
                    evalCommandString(thenCommand);
                else
                    evalCommandString(elseCommand);
                
            }
        }

        if(!strcmp(command, "quit")){
            printf("Exiting...\n");
            exit(0);
        }
        evalCommandString(command);

        if(strcmp(command, "!!") && strcmp(history[(historyIndex-1)%historySize], command)){
            strcpy(history[historyIndex], command);
            historyIndex = (historyIndex+1)%historySize;
        }

        lastStatus = status;
    }
}
