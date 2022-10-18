#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "Item.h"
#include <Node.h>
#include <List.h>

char default_root[] = ".";

#define MAXBUF (8192)

/* this is not the standard readline function. */
ssize_t readline(int fd, void *buf, size_t maxlen) {
    char c;
    char *bufp = buf;
    int n;
    for (n = 0; n < maxlen - 1; n++) { // leave room at end for '\0'
	int rc;
	if ((rc = read(fd, &c, 1)) == 1) {
	    *bufp++ = c;
	    if (c == '\n')
		break;
	} else if (rc == 0) {
	    if (n == 1)
		return 0; /* EOF, no data read */
	    else
		break;    /* EOF, some data was read */
	} else
	    return -1;    /* error */
    }
    *bufp = '\0';
    return n;
}

/* when something goes wrong, we call this function to show an error (such as the famous 404 error) to the web client. */
void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXBUF], body[MAXBUF];
    
    // create the body of error message first (have to know its length for header)
    snprintf(body, MAXBUF, ""
	    "<!doctype html>\r\n"
	    "<head>\r\n"
	    "  <title>Web Server Error</title>\r\n"
	    "</head>\r\n"
	    "<body>\r\n"
	    "  <h2>%s: %s</h2>\r\n" 
	    "  <p>%s: %s</p>\r\n"
	    "</body>\r\n"
	    "</html>\r\n", errnum, shortmsg, longmsg, cause);
    
    // write out the header information for this response
    snprintf(buf, MAXBUF, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    write(fd, buf, strlen(buf));
    
    snprintf(buf, MAXBUF, "Content-Type: text/html\r\n");
    write(fd, buf, strlen(buf));
    
    snprintf(buf, MAXBUF, "Content-Length: %lu\r\n\r\n", strlen(body));
    write(fd, buf, strlen(buf));
    
    // write out the body last
    write(fd, body, strlen(body));
}

// handle a request
void request_handle(int fd) {
    struct stat sbuf;
    char buf[MAXBUF], method[MAXBUF], uri[MAXBUF], version[15];
	/* this is the only file we serve. */
    char filename[MAXBUF] = "spin.cgi";
    char *argv[] = { NULL };
    
    /* make sure we do have the spin.cgi file on the server side */
	if (stat(filename, &sbuf) < 0) {
		request_error(fd, filename, "404", "Not found", "server could not find this file");
		return;
    }
    
    /* read the first line */
    readline(fd, buf, MAXBUF);

	/* read information from buf, which basically contains whatever information the client sent to the server. */
    sscanf(buf, "%s %s %s", method, uri, version);
    printf("method:%s uri:%s version:%s\n", method, uri, version);
    
    /* we only support get requests */
    if (strcasecmp(method, "GET")) {
		request_error(fd, method, "501", "Not Implemented", "server does not implement this method");
		return;
    }

	/* serve the file */
    
    /* set up the http header. */
    snprintf(buf, MAXBUF, ""
	    "HTTP/1.0 200 OK\r\n"
	    "Server: Web Server\r\n");
    
	/* write the http header, which is stored in buf, to the file descriptor, thus the above message will be displayed to the client. */
    write(fd, buf, strlen(buf));
    
    if (fork() == 0) {                        // child
		/* make cgi writes go to socket (not screen), doing so makes sure the cgi output will be displayed to the client. */
		dup2(fd, STDOUT_FILENO);              
		/* let the child process run the cgi script */
		execvp(filename, argv);
    } else {
		/* parent just waits for the child to finish */
		wait(NULL);
    }
}


// create a doubly linked list pointed to by list
struct *list;
list = createList(compareToItem, toStringItem, freeItem); 

void producer(int fd){
	while(size(list) < 5){
		struct item *item;
		struct node *node;
		pthread_mutex_t lock;
		item = createItem(fd, 1); 
		node = createNode(item);
		
		pthread_mutex_lock(&lock);
		addAtRear(list, node);
		pthread_mutex_unlock(&lock);
	}
}

void *consumer(void *ptr){
	struct item *item;
	struct node *node;
	node = removeFront(list);
	if(node){
		item=(struct item *)(node->obj);
		
	//request_handle();
	return null;
}

int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    
    while ((c = getopt(argc, argv, "p:")) != -1){
		switch (c) {
		/* the user specifies port number. the web server will then listen on this port. */
		case 'p':
	    	port = atoi(optarg);
	    	break;
		default:
	    	fprintf(stderr, "usage: ./server [-p port]\n");
	    	exit(1);
		}
	}

    // run out of this directory
    chdir(root_dir);
	
	int sockfd, newsockfd;
	struct sockaddr_in server_addr, client_addr;

	/* create a socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* bind to a port number */
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));

	/* listen for connections */
	listen(sockfd, 1024);

    while (1) {
		/* accept a connection request */
		int client_len = sizeof(client_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *) &client_len);

		/* read data from the connection. we have two file descriptors here,
		 * we use sockfd to listen to the port, and we use newsockfd to actually transfer data. */
		request_handle(newsockfd);

		/* close this one connection */
		close(newsockfd);
	}
	/* if we ever get here, then close the pipe and don't listen anymore. */
	close(sockfd);

	return 0;
}

/* vim: set ts=4: */
