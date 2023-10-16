# Simple_Scheduler

This implementation of the simple scheduler integrates the scheduler into the code file <br>
of the simple shell. The shell as instructed has now been stripped of any functionality <br>
containing pipes. The user inputs the path of the binary files that they want to execute <br>
using the submit command, the submit command pushes all these processes into the ready queue.<br>
when the user is done submitting, they input a custom signal (ctrl + \ in this implementaion)<br>
to start the scheduling process. the scheduler performs round robin execution based on the <br>
number of cpu resources and the time slice as inputted by the user at the beginning of the program.<br>
The output is then shown and a summary which contains the process id (pid), execution time and wait <br>
time is also shown. After this the shell resumes execution as normal.
