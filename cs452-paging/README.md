## Project Number/Title 
* Author: Sharon YANG
* Class: CS452/CS552 [Operating Systems] Section #01

## Overview
I wrote a Kernel module named Infiniti, which implements the a **page fault handler** for a 64-bit x86 system in a multilevel paging scheme. 

## Manifest
--- fault.c</br>
--- fault.h</br>
--- infiniti.h</br>
--- infiniti_main.c</br>
--- user --- infiniti-test1.c</br>
--- user --- infiniti-test2.c</br>
--- user --- infiniti-test3.c</br>
--- user --- infiniti-test4.c</br>
--- user --- infiniti-test5.c</br>
--- user --- infiniti-test6.c</br>
I only modified fault.c. In user folder, there are 6 testing programs(infinit-test[1-6].c).

 
## Building the project
To build the program, run the followingn commands:</br>
make clean</br>
make </br>
sudo insmod infiniti.ko </br>
sudo rmmod infiniti </br>

## Features and usage
To use the program, run the followingn commands:</br>
cd user</br>
make</br>
./infiniti-test1 </br>
./infiniti-test2 </br>
./infiniti-test3 </br>
./infiniti-test4 </br>
./infiniti-test5 </br>
./infiniti-test6 </br>

## Testing
- In infiniti-test1.c, we use infiniti_malloc(4098). There should no sef fault. </br>
- In infiniti-test2.c, we use infiniti_malloc(1024*1024), and we try to access the buf when it's already freed, so a seg fault is expected.</br>
- In infiniti-test3.c, we setup the signal handler to take care of seg fault.</br>
- In infiniti-test4.c, we printed infiniti_malloced size, stored address, and infiniti_freed address. </br> 
- Infiniti-test[5-6].c are leetcode problems(No.1 twoSum and No.6 zigZag conversion)

## Known Bugs
None

## Reflection and Self Assessment
To me, there are two major challenges for this project. One is to distinguish virtual address and physical address, kernel space and user space. So I know where to use __pa() and where to use __va(). The other one is bit manipulation including using AND, OR, SHIFT to store bits, clear bits, and do multiplications, etc. For the first challenge, the instructor's animation, the 4-level page table graph on Canvas, and some Youtube videos explain the paging structure. For the second challenge, the instructor gave us the line of code to get the correct PML4E and explained it. This project is not as difficult as the previous one because once I figured out how to update the first table, I just copied the same code applied to the remaining tables. The free_pa() function also utilize code from the do_page_fault() function. The instructor added two leetcode problems in the test programs and discussed one of them during the class, which is a good teaching method because I know spending time and making efforts on the tough project is worthwhile - that may save my time when I start preparing classical technical interview topics such as sorting, array, linked-list, bit manipulation,etc in the future. Lastly, many people choose Python or Java as their programming language during the coding interview, the instructor may talk about the applications where using C language is more beneficial.

## Sources Used
The following resources are listed by importance to me for this project:</br>
- Page on Canvas (x8664-bit 4-level page tables)</br>
- README handout </br>
- Lectures</br>
- TA hours</br>
- https://youtu.be/hd8bYx7QCS0 </br>
