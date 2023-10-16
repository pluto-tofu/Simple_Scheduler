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
int index_rq = 0;//total no of jobs = index_rq-1
int n_jobs = 0;//no of jobs done therefore when execution done == true then increment 
// while loop condition should be index_rq - njobs != 1
typedef struct process {
    pid_t child_pid;
    long entry_time;
    long execution_time;
    bool execution_done; // important **
    char path[100];
    bool picked;
} process;

process ready_q[512];
process subarray[256];

void delete_elements(char **arr) {
    for (int i = 0; arr[i] != NULL; i++) {
        free(arr[i]);
        arr[i] = NULL;
    }
}

int count_subarray = 0;
void add_to_subarray(process p){
    subarray[count_subarray] = p;
    count_subarray +=1;
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
void show_paths(){
    for (int i = 0; i < index_rq ; i++){
        write(STDOUT_FILENO,ready_q[i].path,sizeof(ready_q[i].path));
        char msg5[] = "\n";
        write(STDOUT_FILENO, msg5, sizeof(msg5));
        setbuf(stdout, NULL);
    }
}
void sigint_handler(int signum) {
    print_pid();
    exit(0);
}
// if signal recienved from user : SIGQUIT
// schedular execution begin -> checkmark -> picked turns true -> execution block-> old picked turnes to false -> step 2 to 5 are in while loop with conditon indeq_rq - njobs != 0 

// okay tested 
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
// okay tested
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
// okay tested
void Create_process_node(char *args[]){
    //creating the node of ready queue
        strcpy(ready_q[index_rq].path,args[1]);
        ready_q[index_rq].child_pid = 0;
        ready_q[index_rq].entry_time = (long)clock() / CLOCKS_PER_SEC;
        ready_q[index_rq].execution_time = 0;
        ready_q[index_rq].execution_done = false;
        ready_q[index_rq].picked = false;
        //updating last ready queue pointer 
        index_rq +=1;
        write(STDOUT_FILENO,index_rq,sizeof(index_rq));
        setbuf(stdout, NULL);
        show_paths();
}
// okay tested
void prime_processes(){
    for(int i = 0 ; i < index_rq ; i++){
        int child_pid = fork();
        if (child_pid < 0){
            printf("child creation error");
            exit(0);
        }
        else if(child_pid == 0){
            // inside the child process
            execlp(ready_q[i].path,ready_q[i].path,NULL);
            exit(0);
        }
        else{
            // inside parent process
            usleep(10);
            ready_q[i].child_pid = child_pid;
            kill(ready_q[i].child_pid, SIGSTOP);
        }
    }
}
// important
// case in which waitpid returns something other that 0
void schedule_processes(){
    int current_index = 0;
    int completed_jobs = 0;
    while(true){
        if(waitpid(ready_q[current_index].child_pid,NULL,WNOHANG) == 0){
            add_to_subarray(ready_q[current_index]);
            if(count_subarray == ncpu){
                for(int i = 0 ; i < ncpu ; i++){
                    if(waitpid(subarray[i].child_pid,NULL,WNOHANG)==0){
                        kill(subarray[i].child_pid,SIGCONT);
                        subarray[i].execution_time+=tslice;
                    }
                    else{
                        completed_jobs +=1;
                        continue;
                    }
                }
                usleep(tslice*1000);
                for(int i = 0 ; i < ncpu ; i++){
                    if(waitpid(subarray[i].child_pid,NULL,WNOHANG)==0){
                        kill(subarray[i].child_pid,SIGSTOP);
                    }
                    else{
                        completed_jobs +=1;
                        continue;
                    }
                }
                delete_elements(subarray);
                count_subarray = 0;
            }
        }
        else{
            completed_jobs +=1;
            continue;
        }
        current_index +=1;
        if(current_index == index_rq){
            current_index = 0;
        }
        if(completed_jobs == index_rq + 1){
            break;
        }
    }
}

int launch (char *args[]) {
    int status;
    int i=0;
    if(strcmp(args[0],"submit")!=0){
        status=create_process_and_run(args);
    }
    else{
        Create_process_node(args);
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

void sigint_handler2(int signum) {
    prime_processes();
    schedule_processes();
    exit(0);
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
    signal(SIGQUIT,sigint_handler2);//press ctrl + '\' to begin schedular
    shell_loop();
    return 0;
}
