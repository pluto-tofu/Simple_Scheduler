#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
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
int ncpu;
int tslice;
int index_rq = 0;
int n_jobs = 0;
typedef struct process {
    pid_t child_pid;
    long entry_time;
    long execution_time;
    bool execution_happening;
    bool execution_done; // important **
    char path[100];
    bool picked;
} process;
process ready_q[512];
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
    if(strcmp(args[0],"submit")!=0){
        status=create_process_and_run(args);
    }
    else{
        strcpy(ready_q[index_rq].path,args[1]);
        ready_q[index_rq].child_pid = 0;
        ready_q[index_rq].entry_time = (long)clock() / CLOCKS_PER_SEC;
        ready_q[index_rq].execution_time = 0;
        ready_q[index_rq].execution_happening = false;
        ready_q[index_rq].execution_done = false;
        ready_q[index_rq].picked = false;
        
        index_rq +=1;
        int i=0;
        int j=0;

        while((index_rq - n_jobs != 0)){
            while(true){
                if(ready_q[i].execution_done == false){
                    j+=1;
                    if(ready_q[i].child_pid == 0){
                        ready_q[i].child_pid = fork();
                        if(ready_q[i].child_pid == 0){
                            //char buff1[] = "secret in child";
                            //write(STDOUT_FILENO,buff1,sizeof(buff1));
                            //setbuf(stdout, NULL);
                            execlp(ready_q[i].path,ready_q[i].path,NULL);
                        }
                        else{
                            usleep(100);
                            // char buff[] = "secret";
                            // write(STDOUT_FILENO,buff,sizeof(buff));
                            // setbuf(stdout, NULL);
                            kill(ready_q[i].child_pid , SIGSTOP);
                        }
                    }
                    if(ready_q[i].child_pid != 0){
                        ready_q[i].picked = true;
                        // if signal recienved from user :
                        // kill(ready_q[i].child_pid , SIGCONT);
                        // ready_q[i].execution_happening = true;
                        // ready_q[i].execution_time += tslice;
                        // usleep(tslice*1000);
                        // kill(ready_q[i].child_pid , SIGSTOP);
                        // ready_q[i].execution_happening = false;
                    }
                }
                else{
                    i += 1;
                    continue;
                }






                if(j==ncpu){
                    break;
                }
                else{
                    i+=1;
                }


                if( i == index_rq){
                    i = 0;
                }


            }
        }

    }
    return status; 
}
void show_paths(){
    for (int i = 0; i < index_rq ; i++){
        write(STDOUT_FILENO,ready_q[i].path,sizeof(ready_q[i].path));
        char msg5[] = "\n";
        write(STDOUT_FILENO, msg5, sizeof(msg5));
        setbuf(stdout, NULL);
    }
}
void shell_loop() {
    int status;
    do {
        printf("device@user~$ ");
        char **args = read_user_input(ab);
        char msg3[10];
        strcpy(msg3,args[0]);
        write(STDOUT_FILENO, msg3, sizeof(msg3));
        char msg4[] = "\n";
        write(STDOUT_FILENO, msg4, sizeof(msg4));
        setbuf(stdout, NULL);
        status = launch(args);
        show_paths();
        free(args);
        ab++;
    } while (status);
}
int main() {

    char msg1[] = "Enter the number of CPU resources: ";
    write(STDOUT_FILENO, msg1, sizeof(msg1));
    if (scanf("%d", &ncpu) != 1) {
        printf("Invalid input for CPU resources. Please enter an integer.\n");
        exit(EXIT_FAILURE);
    }
    printf("You entered: %d\n", ncpu);
    
    char msg2[] = "Enter the number of Time Slice: ";
    write(STDOUT_FILENO, msg2, sizeof(msg2));
    if (scanf("%d", &tslice) != 1) {
        printf("Invalid input for CPU resources. Please enter an integer.\n");
        exit(EXIT_FAILURE);
    }
    printf("You entered: %d\n", tslice);

    setbuf(stdin, NULL);  // Set input stream to unbuffered mode
    setbuf(stdout, NULL); // Set output stream to unbuffered mode
    
    signal(SIGINT, sigint_handler);
    shell_loop();
    return 0;
}