/* A threaded server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include<signal.h>
#include <pthread.h>


FILE* file;
int *lastLine;

pthread_mutex_t mut; /* the lock */

/* displays error messages from system calls */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

/*Handle Signals */
void sig_handler(int signo){
	if (signo == SIGINT || signo == SIGTERM)  {
		free(lastLine);
		fclose(file);
		exit(0);
	}
}



/* the procedure called for each request */
void *processRequest (void *args) {
  	int *newsockfd = (int *) args;
	int temp = 0;
	FILE *f = fdopen(*newsockfd,"r");  
	if (f) {
	  while (1) {
		char *buffer = NULL;
		char **line = NULL;
		line = &buffer;	
		size_t hat = 0;
		
		int m = getline (line, &hat,f);
	
		if (m < 0) {
			free(*line);
			break;
		}
		pthread_mutex_lock (&mut);
		temp = *lastLine;
		fprintf(file, "%d %s", *lastLine, *line);
		*lastLine = temp +1;
		pthread_mutex_unlock (&mut); /* release the lock */
		fflush(file);		
		free(*line);	
	  }
	     
	}
	fclose(f); 
  	close (*newsockfd); /* important to avoid memory leak */  
  	free(newsockfd);


  pthread_exit (NULL);
}



int main(int argc, char *argv[])
{
     socklen_t clilen;
     int sockfd, portno;
 
     struct sockaddr_in serv_addr, cli_addr;
     int result;
	
	 


     
     if (argc !=  3) {
         fprintf (stderr,"usage: %s <port> <fileName>\n", argv[0]);
         exit(1);
     }

     /* open file for writing and reading TODO change back to append*/
	file = fopen(argv[2], "wb+"); 
	if (!file) {
		/* TODO Check what to do if file failed to open */
		fprintf (stderr, "Failed to open/create log file. Logs will not be saved.\n");
	}
	
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		printf("\ncan't catch SIGINT and SIGTERM\n");
	}

	if (signal(SIGTERM, sig_handler) == SIG_ERR) {
		printf("\ncan't catch SIGINT and SIGTERM\n");
	}

	

/*Counts lines in file, so log can continue, incorrect if blank line is added*/
	char *line = malloc (sizeof(char));
	lastLine = malloc(sizeof(int));
	*lastLine = 0;
	while (fgets(line,12,file)) {
		lastLine++;
	}
	free(line);;
	

     /* check port number */
     portno = atoi(argv[1]);
     if ((portno < 0) || (portno > 65535)) {
	 fprintf (stderr, "%s: Illegal port number, exiting!\n", argv[0]);
	 exit(1);
     }
     /* create socket */
     sockfd = socket (AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero ((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons (portno);

     /* bind it */
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     /* ready to accept connections */
     listen (sockfd,5);
     clilen = sizeof (cli_addr);
     
     /* now wait in an endless loop for connections and process them */
     while (1) {

       pthread_t server_thread;;
	
       pthread_attr_t pthread_attr; /* attributes for newly created thread */

   
       
	/* waiting for connections */
	int temp = accept( sockfd, 
			  (struct sockaddr *) &cli_addr, 
			  &clilen);
	       
	if (temp < 0) {
		error ("ERROR on accept \n");
		continue;	
	}

	int *newsockfd = malloc(sizeof(int));
	if (newsockfd == 0) {
		error("Could not malloc \n");
		continue;	
	}
	*newsockfd = temp;

	

     /* create separate thread for processing */
     if (pthread_attr_init (&pthread_attr)) {
	free(newsockfd);
	 fprintf (stderr, "Creating initial thread attributes failed!\n");
	 continue;
     }
	
     if (pthread_attr_setdetachstate (&pthread_attr, PTHREAD_CREATE_DETACHED)) {
free(newsockfd);
       	 fprintf (stderr, "setting thread attributes failed!\n");
	 continue;
     }
	
     result = pthread_create (&server_thread, &pthread_attr, processRequest, (void *) newsockfd);
       if (result != 0) {
free(newsockfd);
	 fprintf (stderr, "Thread creation failed!\n");
	 continue;
       }

       
     }
fclose(file);
     return 0; 
}
