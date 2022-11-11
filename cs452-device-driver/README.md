## Parallel Merge Sort 

* Author: Sharon Yang
* Class: CS452/CS552 [Operating Systems] Section #01

## Overview

I implemented a Linux kernel module called toyota which simulates a simple character device driver.

## Manifest

--- toyota.c</br>
--- toyota.h</br>
--- toyota_load.c</br>
--- toyota_unload.c</br>
--- toyota-test1.c</br>
--- toyota-test2.c</br>
--- toyota-test3.c</br>
--- toyota-test4.c</br>

I only modified _toyota.c_. _toyota_load.c_ and _toyota_unload.c_ are used load and unload the kernel module. Testing programs _toyota-test[1-4].c_ will test the functions (open, release, read, write, init, exit) of the program.

## Building the project

To build the program, run the followingn commands:</br>
make clean</br>
make</br>
sudo ./toyota_load </br>
sudo ./toyota_unload </br>

## Features and usage
To use the program, run the followingn commands:</br>
./toyota-test1 </br>
./toyota-test2 </br>
./toyota-test3 </br>
./toyota-test4 </br>

## Testing

The testing program _toyota-test1.c_ makes sure that the read fails when no write data stored to the internal buffer firstly. _toyota-test2.c_ testes the remove duplicate function using the cases where read bytes are less than write bytes. _toyota-test3.c_ testes the remove duplicate function using more cases including the one where read bytes are more than write bytes. _toyota-test4.c_ tests the case where the user count is huge (200000). 

## Known Bugs

None.

## Reflection and Self Assessment

My initial testing went crash because for several reasons and I fixed the bugs one by one. Firstly, I didn't check whether the internal buffer stores data or not at the very beginning in the read(). This made the test1 failed. Then for test2,  I missed the memset() after kmalloc() so the result string of test2 contains garbage values. Once these bugs fixed, I could passed all the tests but VM went crash immediately after test4. That is because I didn't set pointers to NULL after reclaiming the memory using kfree().Once that is fixed, I could successfully unload module after test4. 

## Sources Used
Class Notes
Leetcode Solution 
TA
