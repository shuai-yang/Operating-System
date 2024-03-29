## Project 3 Buddy System

* Author: Sharon Yang
* Class: CS452/CS552 [Operating Systems] Section #1

## Overview

This project is to write our own malloc() and free() functions according to buddy system algorithm. 

## Manifest

--- buddy.c</br>
--- buddy.h</br>
--- buddy-unit-test.c</br>
--- buddy-test.c</br>
--- malloc-test.c</br>
Buddy-unit-test tests allocating 1 byte. Buddy-test.c and malloc-test.c test the performance.

## Building the project

To build the program, run the followingn commands:</br>
make clean</br>
make </br>

## Features and usage
To use the program, run the followingn commands:</br>
./buddy-unit-test s </br>
./buddy-unit-test v </br>
./buddy-test 100 1234 s </br>
./malloc-test 100 1234 s </br>

## Testing

Firstly, after finishing the buddy_init() and print(), run the buddy-test-unit to make sure the initialization shows the correct avail[] list and blocks if there. I tried ./buddy-test-unit v to get more information.
Then, during the search and split part in malloc(), I printed out the distance_bytes to make sure each distance between p's address and buddy's address is a power of 2. Then, once both allocation and free functions are done, run buddy unit test again
to track the process of searching and splitting. Lastly, run the formance test files with larger number of iteration to see if it runs into errors.

## Known Bugs
The largest iteration I tried is 20000000 and there's no error, so I don't have a known bug to me. 

## Reflection and Self Assessment

This is the most challenging project to me so far. I completed the first and second projects with less help but for this projec, I have to ask for more help from TA, online resources(for buddy system algo), and discuss with classmates(for debug).
A big headache for me is handling the points of the doubly-linked list particulary the merge process. The other troubles I have are bit manipulation, size casting, and debugging.
I have trouble to use virtual machine on my computer this time (network issue..) and have trouble to  run program in VS code directly, so  environment for C. So I wrote code in VS code but run it on onyx. 
It's quite unconvenient... For next kernel project, I have to fix my VM issue...

Although given two weeks, I didn't really start coding in the first week until this Tuesday... Once under some pressue but not too much, I feel more productive and could focuse more on one thing. 
But I should really start earlier next time to avoid overwhelmed on the due day.  

## Sources Used
TA hours (very helpful : many students have questions to ask, some are same questions, some are personal bugs)
Discuss with classmats (very helpful: I discussed with Bree, Sam, and Jonathan)
Buddy system animation (helpful to know the procedures on the high level)
Disucssion (helpful, but the instructor went over them too fast and I didn't fully understand until I started coding. Once I started coding, I went back to look at my notes but sometimes, I can't recoganize what I wrote......)
README (not as useful as the previous two projects)
Slack (not very helpful, because the difficutly of this project to me is handling the points, that details are not what instructor could help)
