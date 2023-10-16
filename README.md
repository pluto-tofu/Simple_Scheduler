# Simple_Scheduler
this implementation of the simple scheduler integrates the scheduler into the code file 
of the simple shell. The shell as instructed has now been stripped of any functionality 
containing pipes. The user inputs the path of the binary files that they want to execute 
using the submit command, the submit command pushes all these processes into the ready queue.
when the user is done submitting, they input a custom signal (ctrl + \ in this implementaion)
to start the scheduling process. the scheduler performs round robin execution based on the 
number of cpu resources and the time slice as inputted by the user at the beginning of the program.
The output is then shown and a summary which contains the process id (pid), execution time and wait 
time is also shown. After this the shell resumes execution as normal.
