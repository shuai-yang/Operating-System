//
// client.c: A very, very primitive HTTP client.
// 
// To run, try: 
//      client hostname portnumber filename
//
// Sends one HTTP request to the specified HTTP server.
// Prints out the HTTP response.
//
// For testing your server, you will want to modify this client.  
// For example:
// You may want to make this multi-threaded so that you can 
// send many requests simultaneously to the server.
//
// You may also want to be able to request different URIs; 
// you may want to get more URIs from the command line 
// or read the list from a file. 
//
// When we test your server, we will be using modifications to this client.
//

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

//
// Send an HTTP request for the specified file 
//
void client_send(int fd, char *filename) {
    char buf[MAXBUF];
    char hostname[15];
    
    gethostname(hostname, MAXBUF);
    
    /* Form and send the HTTP request */
    snprintf(buf, MAXBUF, "GET %s HTTP/1.1\r\n"
		    "host: %s\n\r\n",
		    filename, hostname);
    write(fd, buf, strlen(buf));
}

//
// Read the HTTP response and print it out
//
void client_print(int fd) {
    char buf[MAXBUF];  
    int n;
    
    // Read and display the HTTP Header 
    n = readline(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0)) {
	printf("Header: %s", buf);
	n = readline(fd, buf, MAXBUF);
	
	// If you want to look for certain HTTP tags... 
	// int length = 0;
	//if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
	//    printf("Length = %d\n", length);
	//}
    }
    
    // Read and display the HTTP Body 
    n = readline(fd, buf, MAXBUF);
    while (n > 0) {
	printf("%s", buf);
	n = readline(fd, buf, MAXBUF);
    }
}

int main(int argc, char *argv[]) {
    char *host, *filename;
	struct hostent *hp;
	struct sockaddr_in dest;
    int port;
	int sockfd;
    
    if (argc != 4) {
		fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
		exit(1);
    }
    
    host = argv[1];
    port = atoi(argv[2]);
    filename = argv[3];
    
    /* create a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* set the destination information */
	memset(&dest, 0, sizeof(struct sockaddr_in));
	dest.sin_family = AF_INET;
	hp = gethostbyname(host);
	bcopy((char *)hp->h_addr, (char *)&dest.sin_addr, hp->h_length);
	dest.sin_port = htons(port);

	/* connect to the server */
	connect(sockfd, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));

	/* send data to the server */
    client_send(sockfd, filename);

	/* print the response we get from the server */
    client_print(sockfd);
    
	/* close the connection */
	close(sockfd);

    return 0;
}

/* vim: set ts=4: */
