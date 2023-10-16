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
int rounds;
typedef struct process {
    pid_t child_pid;
    long entry_time;
    long execution_time;
    bool execution_done;
    long exit_time;
    char path[100];
    bool picked;
} process;
process ready_q[512];
process ready_qq[512];
//printing pid of shell commands
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
//print details of schedular commands
int index_rqq=0;
void show_paths(){
    for (int i = 0; i < index_rqq ; i++){
        write(STDOUT_FILENO,ready_qq[i].path,sizeof(ready_qq[i].path));
        char numstr[15];
        long wait = ready_qq[i].exit_time-ready_qq[i].entry_time-(ready_qq[i].execution_time/CLOCKS_PER_SEC);
        char msg5[] = "\n";
        write(STDOUT_FILENO, msg5, sizeof(msg5));
        printf("%ld",wait);
        printf(" %ld", ready_qq[i].execution_time);
        printf(" %u",ready_qq[i].child_pid);
        write(STDOUT_FILENO, msg5, sizeof(msg5));
        setbuf(stdout, NULL);
    }
}
//deleting the elements of the ready queue and preparing for second run
void delete() {
    for (int i = index_rqq; i < index_rq + index_rqq; i++) {
        // Clear path
        strcpy(ready_qq[i].path, ready_q[i].path);
        // Clear child_pid
        ready_qq[i].child_pid = ready_q[i].child_pid;
        // Clear entry time
        ready_qq[i].entry_time = ready_q[i].entry_time;
        // Clear exit time
        ready_qq[i].exit_time = ready_q[i].exit_time;
        // Clear execution time
        ready_qq[i].execution_time = ready_q[i].execution_time;
        // Clear picked
        ready_qq[i].picked = ready_q[i].picked;
        // Clear execution done
        ready_qq[i].execution_done = ready_q[i].execution_done;
        
        // Clear data in the ready_q array
        strcpy(ready_q[i].path, "");
        ready_q[i].child_pid = 0;
        ready_q[i].entry_time = 0;
        ready_q[i].exit_time = 0;
        ready_q[i].execution_time = 0;
        ready_q[i].picked = false;
        ready_q[i].execution_done = false;
    }
    // Move the pointer to the end
    index_rqq += index_rq;
    index_rq = 0;
}
// signal handler for killing the program
void sigint_handler(int signum) {
    show_paths();
    print_pid();
    exit(0);
}
// if signal recienved from user : SIGQUIT
// schedular execution begin -> checkmark -> picked turns true -> execution block-> old picked turnes to false -> step 2 to 5 are in while loop with conditon indeq_rq - njobs != 0 

// execution of shell commands 
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
// tokanising input of user
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
// creation of process from data given and adding to ready queue
void Create_process_node(char *args[]){
    //creating the node of ready queue
        strcpy(ready_q[index_rq].path,args[1]);
        ready_q[index_rq].child_pid = 0;
        ready_q[index_rq].entry_time = (long)clock() / CLOCKS_PER_SEC;
        ready_q[index_rq].execution_time = 0;
        ready_q[index_rq].exit_time = 0;
        ready_q[index_rq].execution_done = false;
        ready_q[index_rq].picked = false;
        //updating last ready queue pointer 
        //shared_mem->
        write(STDOUT_FILENO,ready_q[index_rq].path,sizeof(ready_q[index_rq].path));
        setbuf(stdout,NULL);
        index_rq +=1;
}
// inititalising the processes
void prime_processes(){
    ncpu=index_rq;
    for(int i = 0 ; i < index_rq ; i++){
        int child_pid = fork();
        if (child_pid < 0){
            printf("child creation error");
            exit(0);
        }
        else if(child_pid == 0){
            // inside the child process
            if(execlp(ready_q[i].path,ready_q[i].path,NULL)==-1){
                perror("exec failed");
                exit(0);
            }
            exit(0);
        }
        else{
            // inside parent process
            usleep(5);
            ready_q[i].child_pid = child_pid;
            //sharedm_mem->child[i]=child_pid
            kill(ready_q[i].child_pid, SIGSTOP);
        }
    }
}
// actual round robin scheduling algo
int index_of = 0;
void schedule_processes() {
    bool termination_condition = false;
    int i = 0;
    while (!termination_condition) {
        termination_condition = true;
        for (i = index_of; i < index_of + ncpu; i++) {
            if (i == index_rq) {
                break;
            }
            int result2 = waitpid(ready_q[i].child_pid, NULL, WNOHANG);
            if (result2 == 0) {
                termination_condition = false;
                kill(ready_q[i].child_pid, SIGCONT);
                ready_q[i].execution_time += tslice;
            } else {
                continue;
            }
        }
        usleep(tslice * 1000);
        for (i = index_of; i < index_of + ncpu; i++) {
            if (i == index_rq) {
                break;
            }
            int result = waitpid(ready_q[i].child_pid, NULL, WNOHANG);
            if (result == 0) {
                kill(ready_q[i].child_pid, SIGSTOP);
            } else {
                ready_q[i].exit_time =(long)clock() / CLOCKS_PER_SEC;
                n_jobs += 1;
            }
        }
        index_of += ncpu;
        if (index_of >= index_rq) {
            index_of = 0;
        }
    }
}
// launching the shell 
int launch (char *args[]) {
    int status;
    int i=0;
    if(strcmp(args[0],"submit")!=0){
        status=create_process_and_run(args);
        prime_processes();
    }
    else{
        Create_process_node(args);
    }
    return status; 
}
// loop for shell
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
//signal handler for schedular begining
void sigint_handler2(int signum) {
    prime_processes();
    schedule_processes();
    delete();
    shell_loop();
}
int main() {
    // taking ncpu as input
    char msg1[] = "Enter the number of CPU resources: ";
    write(STDOUT_FILENO, msg1, sizeof(msg1));
    if (scanf("%d", &ncpu) != 1) {
        printf("Invalid input for CPU resources. Please enter an integer.\n");
        exit(EXIT_FAILURE);
    }
    printf("You entered: %d\n", ncpu);
    // taking tslice
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
