## Parallel Merge Sort 

* Author: Sharon Yang
* Class: CS452/CS552 [Operating Systems] Section #01

## Overview

I implemented a multi-threaded version of merge sort algorithm (also known as parallel merge sort) using the pthread library.

## Manifest

--- mergesort.c</br>
--- mergesort.h</br>
--- runval.sh</br>
--- test-mergesort.c</br>

I only modified _mergesort.c_. Bash script _runval.sh_ called valgrind to test if my memory is correctly released, if not, valgrind will report memory leaks. Testing program _test-mergesort.c_ will take three parameters from users - size of the array(at least 100); cutoff value; seed number - and call _parallel_mergesort()_ to sort a randomly-generated array and time the sorting.

## Building the project

To build the program, run the followingn commands:</br>
make clean</br>
make</br>

## Features and usage
Usage: ./test-mergesort \<input size> \<cutoff level> \<seed></br>
For example ./test-mergesort 100000000 0 1234</br></br>
Usage: bash runval.sh</br>

## Testing

The testing program _test-mergesort.c_ takes 3 parameters. The first parameter is the size of the array. 
The second parameter is the cutoff value. The third parameter is a seed number. Assume the size of the array is denoted as n (n>=100), then the testing program will call ```struct argument *arg=buildArgs(0, n-1, 0);``` and ```parallel_mergesort(arg);``` to sort a randomly-generated array and time it.</br>
The testing program will show timing results for the single thread merge sort when the number of level is 0, and show timing results of the parallel merge sort when the number of level is larger than 0.
Compared to serial merge sort, I expect my parallel merge sort get a speedup of at least 2 on onyx using array size n = 100 millions. </br>
I also tested my memory using bash script _runval.sh_. 

## Known Bugs

None.

## Reflection and Self Assessment

My initial testing failed, then I debugged by just testing the sequential merge sort using mergesort(0, n-1). It failed, so I knew something wrong in my mergesort() or merge() functions. Then I found in my merge() function, my index was wrongly initialized to 0, and I copied the entire array B into array A at the end. Those are incorrect and caused multiple threads overwrote the same subarray of array B and the entire array A was also overwritten over and over. That is not how multithreading works. Once I corrected my index initialization to leftstart and at the end copied the &B[ leftstart ] into &A[ leftstart ], I passed the testing. Note that for this project, there's no overlap of subarrays, so each thread works on its own assigned subarray starting from its leftstart. 

## Sources Used
[Youtube video - Algorithms: Merge Sort](https://www.youtube.com/watch?v=KF2j-9iSf4Q&t=372s)</br>
[Leetcode Problem 88 - Merge Sorted Array](https://leetcode.com/problems/merge-sorted-array/)</br>
[Stackoverflow - memcpy with startindex](https://stackoverflow.com/questions/1163624/memcpy-with-startindex)</br>