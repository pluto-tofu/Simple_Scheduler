#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#define CLOCKS_PER_SEC ((clock_t)1000000)
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGUMENTS 1024
#define MAX_CHILDREN 512
double start_times[MAX_CHILDREN];
double runtimes[MAX_CHILDREN];
char *cutt[100];
char *cutt2[100];
char *cutt3[100];
char *history_save[512][512];
int pidd[512];
int ab=0;
int cd=0;
void split_double_pipe(char *args[]) {
    int size_of = 0;
    int a = 0;
    int size = 0;
    int i = 0;
    while (args[i] != NULL) {
        size++;
        if (strcmp(args[i], "|") == 0) {
            if(a==0){
                size_of = i;
                a = i + 1;
            }    
        }
        i++;
    }
    while (args[a] != NULL) {
        if (strcmp(args[a], "|") == 0) {
            break;
        }
        a++;
    }
    for (int j = 0; j < size_of; j++) {
        cutt[j] = strdup(args[j]);
    }
    cutt[size_of] = NULL;
    for (int b=size_of+1 , j=0;b<a;b++,j++){
        cutt3[j]=strdup(args[b]);
    }
    cutt3[a-size_of-1]=NULL;
    for (int k = a+ 1, j = 0; k < size; k++, j++) {
        cutt2[j] = strdup(args[k]);
    }
    cutt2[size - a - 1] = NULL;    
}
char **split_single_pipe(char *args[]){
        int size_of=0;
        int size=0;
        int i=0;
        while (args[i]!=NULL){   
            size++;
            if (strcmp(args[i],"|")==0){
                size_of=i;
            }
            i++;
        }

        for (int j=0; j<size_of;j++){

            cutt[j]=strdup(args[j]);
        }

        cutt[size_of]=NULL;

        for (int k=size_of+1,j=0;k<size;k++,j++){

            cutt2[j]=strdup(args[k]);
        }

        cutt2[size-size_of-1]=NULL;
}
void delete_elements(char **arr) {
    for (int i = 0; arr[i] != NULL; i++) {
        free(arr[i]);
        arr[i] = NULL;
    }
}
void print_pid() {
    for (int i=0; i<=ab;i++){
        printf("\n%s\n",history_save[i]);
    }
    for(int i=0; i<cd;i++){
        printf("\nProcess ID: %d\n",pidd[i]);
        printf("Start Time: %f\n",start_times[i]);
        printf("Run Time:%f\n",runtimes[i]);
    }
}

void sigint_handler(int signum) {
    print_pid();
    exit(0);
}
int create_process_and_run_single_pipe(char* l1[],char* l2[]){
    int fd[2];
    if(pipe(fd) == -1){
        printf("Pipe creation error !\n");
        return 0;
    }
    start_times[cd] = (double)clock() / CLOCKS_PER_SEC;
    int pid1 = fork();
    if(pid1 < 0){
        printf("Child 1 creation error\n");
        return 0;
    }
    else if(pid1 ==0){
        dup2(fd[1],STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        if(execvp(l1[0],l1)==-1){
            printf("Command execution error or invalid command\n");
            return 1;
        }
    }
    runtimes[cd] = ((double)clock() / CLOCKS_PER_SEC) - start_times[cd];
    pidd[cd]=pid1;
    cd++;
    delete_elements(l1);
    close(fd[1]);
    start_times[cd] = (double)clock() / CLOCKS_PER_SEC;
    int pid2 = fork();
    if(pid2<0){
        printf("Child 2 creation error\n");
        return 0;
    }
    else if(pid2 == 0){
        dup2(fd[0],STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        if(execvp(l2[0],l2)==-1){
            printf("Command execution error or invalid command\n");
            return 1;
        }
    }
    runtimes[cd] = ((double)clock() / CLOCKS_PER_SEC) - start_times[cd];
    pidd[cd]=pid2;
    cd++;
    delete_elements(l2);
    close(fd[0]);
    if(waitpid(pid1,NULL,0)==-1){
        printf("Abnormal termination or execution of child\n");
        return 0;
    }
    if(waitpid(pid2,NULL,0)==-1){
        printf("Abnormal termination or execution of child\n");
        return 0;
    }
    return 1;
}
int create_process_and_run_double_pipe(char* l1[],char* l2[], char* l3[]){
    int fd1[2];
    int fd2[2];
    if(pipe(fd1) == -1){
        printf("Pipe 1 creation error\n");
        return 0;
    }
    if(pipe(fd2) == -1){
        printf("Pipe 2 creation error\n");
        return 0;
    }
    start_times[cd] = (double)clock() / CLOCKS_PER_SEC;
    int pid1 = fork();
    if(pid1 < 0){
        printf("Child 1 creation error\n");
        return 0;
    }
    else if(pid1 == 0){
        dup2(fd1[1],STDOUT_FILENO);
        close(fd1[0]);
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);
        if(execvp(l1[0],l1)==-1){
            printf("Command execution error or invalid command\n");
            return 1;
        }
    }
    runtimes[cd] = ((double)clock() / CLOCKS_PER_SEC) - start_times[cd];
    pidd[cd]=pid1;
    cd++;
    delete_elements(l1);
    start_times[cd] = (double)clock() / CLOCKS_PER_SEC;
    int pid2 = fork();
    if(pid2 < 0){
        printf("Child 2 creation error\n");
        return 0;
    }
    else if(pid2 == 0){
        dup2(fd1[0],STDIN_FILENO);
        dup2(fd2[1],STDOUT_FILENO);
        close(fd1[0]);
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);
        if(execvp(l2[0],l2)==-1){
            printf("Command execution error or invalid command\n");
            return 1;
        }
    }
    runtimes[cd] = ((double)clock() / CLOCKS_PER_SEC) - start_times[cd];
    pidd[cd]=pid2;
    cd++;
    delete_elements(l2);
    start_times[cd] = (double)clock() / CLOCKS_PER_SEC;
    int pid3 = fork();
    if(pid3 < 0){
        printf("Child 3 creation error\n");
        return 0;
    }
    else if(pid3==0){
       dup2(fd2[0],STDIN_FILENO);
       close(fd1[0]);
       close(fd1[1]);
       close(fd2[0]);
       close(fd2[1]);
       if(execvp(l3[0],l3)==-1){
           printf("Command execution error or invalid command\n");
           return 1;
       }
   }
    runtimes[cd] = ((double)clock() / CLOCKS_PER_SEC) - start_times[cd];
    pidd[cd]=pid3;
    cd++;
    delete_elements(l3);
    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);
    
    if(waitpid(pid1,NULL,0)==-1){
        printf("Abnormal termination or execution of child\n");
        return 0;
    }
    if(waitpid(pid2,NULL,0)==-1){
        printf("Abnormal termination or execution of child\n");
        return 0;
    }
    if(waitpid(pid3,NULL,0)==-1){
        printf("Abnormal termination or execution of child\n");
        return 0;
    }
    return 1;
}
 
int create_process_and_run(char *args[]) {
    start_times[cd] = (double)clock() / CLOCKS_PER_SEC;
    int status = fork();
    if (status < 0) {
        printf("Fork error\n");
        exit(EXIT_FAILURE);
    } else if (status == 0) {
        if(execvp(args[0], args)==-1){
            printf("Command execution error or invalid command\n");
            return 1;
        }
    } else {
        int ret;
        int pid = wait(&ret);
        if (WIFEXITED(ret)) {
            runtimes[cd] = ((double)clock() / CLOCKS_PER_SEC) - start_times[cd];
            pidd[cd]=pid;
            cd++;
            printf("%d Exit = %d\n", pid, WEXITSTATUS(ret));
        } else {
            printf("Abnormal termination of %d\n", pid);
        }
    }
    return 1;
}
char **read_user_input(int ab) {
    char userInput[MAX_COMMAND_LENGTH];
    fgets(userInput, sizeof(userInput), stdin);
    userInput[strcspn(userInput, "\n")] = '\0';
    strcpy(history_save[ab],userInput);
    char *args[MAX_ARGUMENTS + 1];
    int i = 0;
    char *token = strtok(userInput, " ");
    while (token != NULL && i < MAX_ARGUMENTS)  {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    char **result = (char **)malloc((i + 1) * sizeof(char *));
    if (result == NULL) {
        printf("Malloc error\n");
        exit(EXIT_FAILURE);
    }
    for (int j = 0; j <= i; j++) {
        result[j] = args[j];
    }
    return result;
}
int launch (char *args[]) {
    int status;
    int i=0;
    int pipe_count=0;
    while (args[i]!=NULL){   
        if (strcmp(args[i],"|")==0){
            pipe_count++;
        }
        i++;
    }
    if(strcmp(args[0],"history")==0){
        for (int i=0; i<=ab;i++){
        printf("%s",history_save[i]);
        printf("\n");
        }
        return 1;    
    }
    else{
        if (pipe_count==0){
            status=create_process_and_run(args);
        }else if (pipe_count==1){
            split_single_pipe(args);
            status=create_process_and_run_single_pipe(cutt,cutt2);
        }else if (pipe_count==2){
            split_double_pipe(args);
            status=create_process_and_run_double_pipe(cutt,cutt3,cutt2);
        }else{
            printf("Limitation of shell reached: Only 2 pipe operator allowed\n");
            return 1;
        }   
    }
    return status; 
}
void shell_loop() {
    int status;
    do {
        printf("device@user~$ ");
        char **args = read_user_input(ab);
        status = launch(args);
        free(args);
        ab++;
    } while (status);
}
int main() {
    signal(SIGINT, sigint_handler);
    shell_loop();
    return 0;
}
