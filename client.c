#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include<signal.h>
#include <unistd.h>


char *buffer = NULL;
int sockfd = -1;





void sig_handler(int signo){
	if (signo == SIGINT || signo == SIGTERM || signo == SIGPIPE)  {
		if (sockfd >= 0) {
			shutdown(sockfd,2);
			close(sockfd);
			free(buffer);
		}
		exit(0);
	}
}

int main(int argc, char *argv[])
{
    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

	
	/*Setting up signal handler */
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		printf("\ncan't catch SIGINT and SIGTERM\n");
	}

	if (signal(SIGTERM, sig_handler) == SIG_ERR) {
		printf("\ncan't catch SIGINT and SIGTERM\n");
	}

	if (signal(SIGPIPE, sig_handler) == SIG_ERR) {
		printf("\ncan't catch SIGINT and SIGTERM\n");
	}
   
    if (argc != 3) {
       fprintf (stderr, "usage %s hostname port\n", argv[0]);
       exit(1);
    }

    /* create socket */
    portno = atoi (argv[2]);
    if ((portno < 0) || (portno > 65535)) {
		fprintf (stderr, "%s: Illegal port number, exiting!\n", argv[0]);
		exit(1);
    }

    sockfd = socket (AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf (stderr,"ERROR opening socket");
	exit(1);
	}
	
    /* enter connection data */
    server = gethostbyname (argv[1]);
    if (server == NULL) {
        fprintf (stderr, "ERROR, no such host\n");
        exit (1);
    }
    bzero ((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy ((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons (portno);

    /* connect to the server */
    if (connect (sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        fprintf (stderr,"ERROR connecting\n");
		exit(1);;
	}
	

    
	/* While messages are being inputted send them to server */
	while (1) {
	    	buffer = NULL;
		size_t hat = 0;
		int m = getline (&buffer, &hat,stdin);
		if (m < 0) {
			free(buffer);
			break;
		}
		n = write (sockfd, buffer, strlen(buffer));
		if (n < 0) {
			fprintf (stderr,"ERROR writing to socket");
			free(buffer);
			exit(1);
		}
			
		free(buffer);
	}


    return 0;
}

