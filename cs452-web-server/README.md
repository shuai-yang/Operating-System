# Overview

In this assignment, we will convert an existing single-threaded web server into a multi-threaded web server. Note this is NOT a kernel project, and you should just develop your code on onyx, not in your virtual machine. Submissions fail to compile or run on onyx, will not be graded.

## Learning Objectives

- Gain more experience writing concurrent code.
- Understand a well-known concurrent programming problem - the producer-consumer problem.
- Explore the pthread library, and learn how to use locks and condition variables.

## Book References

Read these chapters carefully in order to prepare yourself for this assignment:

- [Threads API](http://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf) 
- [Locks](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf)
- [Condition Variables](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-cv.pdf)

In particular, the producer-consumer problem covered in the [Condition Variables](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-cv.pdf) chapter is directly related to this assignment. The chapter actually used the multi-threaded web server as an example of the producer-consumer problem.

## Starter Code

The starter code looks like this:

```console
(base) [jidongxiao@onyx cs452-web-server]$ ls -R
.:
client.c  concurrent_server.c  Item.c  Item.h  list  Makefile  README.md  server.c  spin.c

./list:
include  lib  Makefile  README.md  SimpleTestList.c

./list/include:
List.h  Node.h

./list/lib:
libmylib.a  libmylib.so
```

As of now, *concurrent_server.c* and *server.c* are completely the same, and they now just work as a single-threaded web server. You will be modifying *concurrent_server.c*, so as to convert it from a single-threaded web server into a multi-threaded web server. You should not modify other files in the same folder.

*client.c* implements a web client, which can send http requests to the web server.

*spin.c* will be used to generate a cgi script, which will be the only file hosted on the web server. When the web client attempts to access this cgi script, the script does nothing but just occupy the server for 5 seconds. We use this behavior to simulate the server handling an http request which takes 5 seconds. Therefore, if two such requests are sent to the server, and if the server only has one single thread to handle requests, then we expect the server to spend about 10 seconds to handle these 2 requests; but if the server has 2 threads, then handling 2 requests may still just take 5 seconds. This shows the benefit of using multiple threads.

## Specification

You are required to implement the following two functions:

```c
void producer(int fd);
void *consumer(void *ptr);
```

You also need to modify the *main*() function, so that the *producer*() and *consumer*() will be called. Plus, you are required to extend *main*() so that users (who run the web server) can specify (from the command line with a -*t* flag) how many consumer threads they want to run. Users do not need to specify how many producer threads they want to run: in this assignment, we only need to support one single producer thread.

To facilitate your implementation of the producer/consumer functions, a doubly linked list library is provided. As shown in the starter code, *./list/lib/libmylib.a* is the provided library. It is pre-compiled, meaning that you can use the library but you have no access to its source code. Coming with the library are two of its header files: List.h and Node.h, both are located in the list/include subfolder. In this assignment, you are asked to create a linked list using API functions provided by this library. When the web server is running, producers add nodes to this linked list, whereas consumers remove nodes from this linked list.

This library does not support multiple threads, thus when a program with multiple threads attempts to access this library - manipulating the doubly linked list, there will be race conditions and the results may not be deterministic, and that is why in this assignment we need to use locks so as to avoid race conditions.

**Constraints**: your linked list should have no more than 5 nodes, in other words, the producer should stop producing (i.e., adding nodes to the linked list) if the number of nodes on the linked list is already 5 - when the linked list has 5 nodes, we consider the list to be full.

## Data Structures Defined in the Doubly Linked List Library

The provided linked list library defines *struct node* in *list/include/Node.h*:

```c
typedef struct node * NodePtr;

struct node {
        void *obj;
        struct node *next;
        struct node *prev;
};
```

And it defines *struct list* in *list/include/List.h*:

```c
typedef struct list * ListPtr;

struct list {
  int size;
  struct node *head;
  struct node *tail;
  int (*equals)(const void *, const void *);
  char * (*toString)(const void *);
  void (*freeObject)(void *);
};
```

The internal of these two structures are not really important to you. But you need to be aware of the *size* field of the *struct list*, which gives you the current size of the list. Knowing the current size of the list tells you if the list is full or not, and if the list is empty or not. So how do you create the linked list? Use the following function:

```c
struct list* createList(int (*equals)(const void *, const void *),
                        char *(*toString)(const void *),
                        void (*freeObject)(void *));
```

You do not really need to understand what these arguments are doing, but you can call *createList*() as following:

```c
struct list *list;
list=createList(compareToItem, toStringItem, freeItem); // yes, the name of the three arguments of this createList() must be exactly the same as what are used here, as they are actually three pre-defined functions, defined in *Item.c*.
```

After these two lines, you now have a doubly linked list pointed to by *list*. Note that the linked list library itself does not enforce a capacity to the linked list, which means the library itself allows you to add as many nodes as you want to the linked list. Therefore it is your job to check if the size of this linked list now reaches the capacity (which is 5 in this assignment) or not, so that you can avoid adding nodes to the linked list when the size has already reached the capacity.

## What Exactly Does the Producer Produce?

Everytime the client attempts to send an http request to the server, a special pipe will be established between the client and the server. The request will then be associated with this pipe and its data will be sent over this pipe. To differentiate multiple pipes, the server assign a file descriptor (which is just an integer) to each pipe. Therefore, we can also say each request is associated with a file descriptor. We can also say each request is associated with a pipe.

To describe what exactly is produced by the producer, a data structure called *struct item* is defined in the starter code (in Item.h).

```c
struct item {
	int fd;
	int producer;
};
```

Each item has two fields. The *fd* field allows us to track that this item is corresponding to which http request - remember each request is associated with a file descriptor. If we have many producers, then we use the *producer* field to track this item is produced by which producer. In other words, the *producer* field here is like the producer id.

Everytime the *producer*() is called, it produces an item, encapsulates this item in a node, and add the node into the linked list. The following lines show how you can do so:

```c
struct item *item;
struct node *node;
```

```c
item = createItem(fd, 1);
```

this above line produces an item, stores the file descriptor in the *fd* field of struct item, and stores the producer id in the *producer* field of struct item. In this assignment, you will only have one producer (but multiple consumers), thus the producer id is just 1.

```c
node = createNode(item);
addAtRear(list, node);
```
this above line assumes the doubly linked list you created is named as **list**.

In summary, you should have the above 5 lines in your *producer*() function. And if now you look at the protoypte of the *producer*() function:

```c
void producer(int fd);
```

you will see why *fd* is passed as an argument to the *producer*() function.

## Other APIs Provided by the Doubly Linked List Library

You are recommended to use the following two functions from the doubly linked list library:

```c
struct node *removeFront(struct list *list);
void freeNode(struct node *node, void (*freeObject)(void *));
```

You should only use these two functions in your *consumer*() function. The following lines show how you can use them:

```c
struct node *node;
struct item *item;
/* consumer removes nodes from the front */
node = removeFront(list);
if(node){
	item=(struct item *)(node->obj);

	/* now that you have the item, add your code here to consume the item. */

	/* now that the item is removed and consumed, no longer need this node, free the node and free the item. */
	freeNode(node, freeItem);
}
```

once again, this above code assumes the doubly linked list you created is named as **list**.

So what does consume mean? Consumers consume items, and remember we mentioned above, each item is corresponding to an http request. As a web server, eventually what you really want to do is handle these http requests. Each item contains a file descriptor, and each file descriptor is associated with an http request. Now that we have the item, how do we handle the request? Look at the *main*() function in the starter code, you see *request_handle*() is the function we should call to handle http requests. And this function has the following prototype:

```c
void request_handle(int fd);
```

Up to this point, you should be able to figure out how you should call *request_handle*() in your *consumer*() function.

## Pthread APIs

I used the following pthread APIs:

- pthread_mutex_init()
- pthread_mutex_lock()
- pthread_mutex_unlock()
- pthread_cond_init()
- pthread_cond_wait()
- pthread_cond_signal()

For each pthread API, read its man page to find out how to use it. 

## Testing and Expected Results

You can test your code using one onyx node only. Just open two terminals: one is used as the web server, and the other is used as the web client. Make sure you run the following commands while you are in the stater code directory.

1. When running the default single thread web server, we get the following results:


- server side, run this command to start the server and listen on port 8080.

```console
(base) [jidongxiao@onyxnode60 webserver]$ ./server -p 8080
```

- client side, run this command to send one http request to the server's port 8080:

```console
(base) [jidongxiao@onyxnode60 webserver]$ time ./client localhost 8080 /spin.cgi?5
Header: HTTP/1.0 200 OK
Header: Server: Web Server
Header: Content-Length: 127
Header: Content-Type: text/html
<p>Welcome to the CGI program</p>
<p>My only purpose is to waste time on the server!</p>
<p>I spun for 5.00 seconds</p>

real	0m5.010s
user	0m0.000s
sys	0m0.004s
```

As can be seen that it takes the server about 5 seconds to handle this one single http request.

now, run this command to send 4 http requests:

```console
(base) [jidongxiao@onyxnode60 webserver]$ time seq 4 | xargs -n 1 -P 4 -I{} ./client localhost 8080 /spin.cgi?5
Header: HTTP/1.0 200 OK
Header: Server: Web Server
Header: Content-Length: 127
Header: Content-Type: text/html
<p>Welcome to the CGI program</p>
Header: HTTP/1.0 200 OK
Header: Server: Web Server
<p>My only purpose is to waste time on the server!</p>
<p>I spun for 5.00 seconds</p>
Header: Content-Length: 127
Header: Content-Type: text/html
<p>Welcome to the CGI program</p>
Header: HTTP/1.0 200 OK
Header: Server: Web Server
<p>My only purpose is to waste time on the server!</p>
<p>I spun for 5.00 seconds</p>
Header: Content-Length: 127
Header: Content-Type: text/html
<p>Welcome to the CGI program</p>
Header: HTTP/1.0 200 OK
Header: Server: Web Server
<p>My only purpose is to waste time on the server!</p>
<p>I spun for 5.00 seconds</p>
Header: Content-Length: 127
Header: Content-Type: text/html
<p>Welcome to the CGI program</p>
<p>My only purpose is to waste time on the server!</p>
<p>I spun for 5.00 seconds</p>

real	0m20.038s
user	0m0.007s
sys	0m0.021s
```

And we can see that to satisfy these 4 http requests, in total it takes the server about 20 seconds.

2. When running the multiple-threaded web server, we expect to get results similar to this:

- server side, run this command to start the web server listening on port 8080, with 4 (consumer) threads.

```console
(base) [jidongxiao@onyxnode60 webserver]$ ./concurrent_server -p 8080 -t 4
```

- client side, run this command to send one http request:

```console
(base) [jidongxiao@onyxnode60 webserver]$ time ./client localhost 8080 /spin.cgi?5
Header: HTTP/1.0 200 OK
Header: Server: Web Server
Header: Content-Length: 127
Header: Content-Type: text/html
<p>Welcome to the CGI program</p>
<p>My only purpose is to waste time on the server!</p>
<p>I spun for 5.00 seconds</p>

real	0m5.013s
user	0m0.001s
sys	0m0.005s
```

it again takes about 5 seconds.

now, run this command to send 4 http requests:

```console
(base) [jidongxiao@onyxnode60 webserver]$ time seq 4 | xargs -n 1 -P 4 -I{} ./client localhost 8080 /spin.cgi?5
Header: HTTP/1.0 200 OK
Header: Server: Web Server
Header: HTTP/1.0 200 OK
Header: Server: Web Server
Header: HTTP/1.0 200 OK
Header: Server: Web Server
Header: HTTP/1.0 200 OK
Header: Server: Web Server
Header: Content-Length: 127
Header: Content-Length: 127
Header: Content-Length: 127
Header: Content-Length: 127
Header: Content-Type: text/html
Header: Content-Type: text/html
Header: Content-Type: text/html
Header: Content-Type: text/html
<p>Welcome to the CGI program</p>
<p>Welcome to the CGI program</p>
<p>Welcome to the CGI program</p>
<p>Welcome to the CGI program</p>
<p>My only purpose is to waste time on the server!</p>
<p>My only purpose is to waste time on the server!</p>
<p>My only purpose is to waste time on the server!</p>
<p>My only purpose is to waste time on the server!</p>
<p>I spun for 5.00 seconds</p>
<p>I spun for 5.00 seconds</p>
<p>I spun for 5.00 seconds</p>
<p>I spun for 5.00 seconds</p>

real	0m5.021s
user	0m0.001s
sys	0m0.025s
```

this time we can see the improvements - even for 4 http requests, our web server can still serve all of them in about 5 seconds, as opposed to 20 seconds.

3. additional testing. if you want to test the client program with 2 http requests, you should run:

```console
(base) [jidongxiao@onyxnode60 webserver]$ time seq 2 | xargs -n 1 -P 2 -I{} ./client localhost 8080 /spin.cgi?5
```

if you want to test the client program with 8 http requests, you should run:

```console
(base) [jidongxiao@onyxnode60 webserver]$ time seq 8 | xargs -n 1 -P 8 -I{} ./client localhost 8080 /spin.cgi?5
```

## Submission 

23:59pm, October 20th, 2022. Late submissions will not be accepted/graded. 

## Project Layout

All files necessary for compilation and testing need to be submitted, this includes source code files, header files, linked list library files, and Makefile. The structure of the submission folder should be the same as what was given to you.

## Grading Rubric (for Undergraduate and Graduate students)

All grading will be executed on onyx.boisestate.edu. Submissions that fail to compile on onyx will not be graded.
                                                                                     
- [80 pts] Functional Requirements:
  - [20 pts] testing program produces expected results: when testing with 2 consumer threads and 4 http requests, the testing result is approximately 10 seconds.
  - [20 pts] testing program produces expected results: when testing with 4 consumer threads and 2 http requests, the testing result is approximately 5 seconds.
  - [20 pts] testing program produces expected results: when testing with 4 consumer threads and 4 http requests, the testing result is approximately 5 seconds.
  - [20 pts] testing program produces expected results: when testing with 4 consumer threads and 8 http requests, the testing result is approximately 10 seconds.
- [10 pts] Compiling
  - Each compiler warning will result in a 3 point deduction.
  - You are not allowed to suppress warnings.
- [10 pts] Documentation:
  - README.md file (rename this current README file to README.orig and rename the README.template to README.md).
  - You are required to fill in every section of the README template, missing 1 section will result in a 2-point deduction.
