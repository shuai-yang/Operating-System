## Project Number/Title 
* Author: Sharon YANG
* Class: CS452/CS552 [Operating Systems] Section #01

## Overview
I wrote a Kernel module named Lexus, which implements the **lottery scheduling** for a single core processor in a Linux system.

## Manifest
--- lexus.c</br>
--- lexus.h</br>
--- test-lexus.c</br>
--- lexus-test1.sh</br>
--- lexus-test2.sh</br>
--- lexus-test3.sh</br>

Test-lexus.c is a testing program with corresonding testing scripts(lexus-test*.sh) which start a number of the test program simultaneously, and pass different parameters to the testing program. Each testing program will run as a seperate process, which holds a number of tickets. For example, lexus-test1 tries to launch three processes, each runs the test program and each of the three processes of different number of tickets attempts to accomplish the same task, which is calcuating lucas number 44.

## Building the project
To build the program, run the followingn commands:</br>
make clean</br>
make </br>
sudo insmod lexus.ko </br>
sudo rmmod lexus.ko </br>

## Features and usage
To use the program, run the followingn commands:</br>
./lexus-test1.sh </br>
./lexus-test2.sh </br>
./lexus-test3.sh </br>

## Testing
- Firstly, after finishing the ioctl(), lexus_register() and lexus_unregister(). I proved that process registering and unregistering are successful by adding pringk comments in both register() and unregister() and making sure they are printed out in the log.
- Next, after finishing the schedule(), I ran the given testing scripts(lexus-test*.sh) and observe the results making sure the results are resonable. </br>
</br>Based on the lottery scheduling, when every process tries to complete the same task, processes which hold more tickets of course will be more likely to be scheduled, and thus are expected to finish faster. Therefore, when running the test scripts, processes with more tickets finish faster. 
</br></br>Following are the snapshots of my testing results, proving that my lottery scheduling produces reasonable scheduling results.</br> 
![](https://github.com/shuai-yang/cs452-scheduler/blob/main/my_test1_result.PNG)
![](https://github.com/shuai-yang/cs452-scheduler/blob/main/my_test2_result.PNG)
![](https://github.com/shuai-yang/cs452-scheduler/blob/main/my_test3_result.PNG)

## Known Bugs
None

## Reflection and Self Assessment
There are a few bugs I fixed during the development: </br>
- In iotcl(), be careful to distinguish variables and pointers, so to use address of and char* casting accordingly. 
- In lexus_register(), no need to initialize the list because we could use global variable lexus_task_struct.list direclty.
- In lexus_unregister(), once the to-be-unregistered task found by pid, we need to check 1) whether the to-be-unregisted task is the currently-running task AND 2) whether the currently-running task is NOT NULL. If both conditions are met, set currently-runningn task NULL.  
- In schedule(), one thing is that I should loop the list by the same way using the given list_for_each_safe(); another thing is I should **reset counter to zero once the winner is found**(This is one place where caused my computer crashed); the last thing is I should **unlock before getting out of a locked area**(This is the other place where caused my computer crashed).<br/>
Note that I didn't make the divide-by-zero mistake and dereference a null pointer mistake becuase the instructor's pseudo code provided in the class has avoided these pitfalls.  

## Sources Used
The following resources are listed by importance to me for this project:</br>
- README
- Pseudo code provided during the Canvas discussion part in the class
- TA hours
