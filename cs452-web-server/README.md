## Project Number/Title 

* Author: Sharon Yang   
* Class: CS452/CS552 [Operating Systems] Section #01

## Overview
I converted an existing single-threaded web server into a multi-threaded web server (a well-known concurrent producer-consumer problem) using locks and condition variables.

## Manifest
--- item.h</br>
--- item.c</br>
--- client.c</br>
--- server.c</br>
--- concurrent_server.c</br>
--- spin.c</br>
--- ./list: include  lib  Makefile  README.md  SimpleTestList.c</br>
--- ./list/include: List.h  Node.h</br>
--- ./list/lib: libmylib.a  libmylib.so</br>

client.c implements a web client, which can send http requests to the web server. spin.c will be used to generate a cgi script, which will be the only file hosted on the web server. The script does nothing but just occupy the server for 5 seconds. We use this behavior to simulate the server handling an http request which takes 5 seconds. libmylib is a  pre-compiled doubly linked list library, which does not support multiple threads. That's why we will use locks to avoid race conditions. Note that the linked list should have no more than 5 nodes. </br>


## Building the project

To build the program, run the following commands:</br>
make clean</br>
make

## Features and usage
To test the program, open two terminals and run following commands on server and client side respectively:
- server side </br>
  Usage: ./concurrent_server [-p port] [-t numThreads]</br>

  For example:  run this command to start the web server listening on port 8080, with 4 (consumer) threads.</br>
  ./concurrent_server -p 8080 -t 4 </br>

- client side</br>
  For example: run this command to send 1 http request</br>
  time ./client localhost 8080 /spin.cgi?5

## Testing
We will test the results using different number of consumer threads and number of http requests and observe how long it takes the server handles http requests. The results will show real/user/sys time as below:</br>
</br>
real	0m5.010s</br>
user	0m0.000s</br>
sys	0m0.004s</br>
</br>
Specifically, our test cases and expected results are: 
- when testing with 2 consumer threads and 4 http requests, the testing result is approximately 10 seconds.
- when testing with 4 consumer threads and 2 http requests, the testing result is approximately 5 seconds.
- when testing with 4 consumer threads and 4 http requests, the testing result is approximately 5 seconds.
- when testing with 4 consumer threads and 8 http requests, the testing result is approximately 10 seconds.

## Known Bugs

None

## Reflection and Self Assessment

Before I started, I read the Condition Variables chapter particularly the producer-consumer problem. Then, I wrote my  consumer() and producer() and checked that I used lock correctly. The other day, after Tuesday's class, I modified my main() function according to the pesudo code given in the class making sure I called consumer and producer functions in the right placce in main() and closed the socketfd in right way as discussed in the class. Finally, I test my first running version, but it failed by segment fault. I debugged the main() and checked if the input arguments were taken. Then I found I should use "p:t:" in getopt() instead getopt(argc, argv, "pt"). Once I fixed this error. The seg fault was gone and I was able to pass all tests>

## Sources Used

- the producer-consumer problem covered in the [Condition Variables](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-cv.pdf) chapter 

- pesudo code given in the class
- leetcode 1115, 1117 (code shared on Slack)