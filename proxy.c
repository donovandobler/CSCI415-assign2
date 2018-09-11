#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)/*error message generated*/
{
	perror(msg);
	exit(1);
}

void *run_thread(void* newsockfd)/*reading and writing messages on each thread*/
{
        int  client = (int) (long) newsockfd;
	struct sockaddr_in cli_addr;
	int socket;
	int address_length;
	socket = *((int*)newsockfd);
	char buffer[256];/*buffer for storing messages*/
	int n;
	address_length = sizeof(cli_addr);
	getpeername(socket, (struct sockaddr*)&cli_addr, &address_length);
	while(1)/*endless loop*/
	{
		bzero(buffer,256);
		n = read(socket,buffer,255);
		if (n < 0) 
		{
			error("ERROR reading from socket");
		}
		printf("%s\n",buffer);/*message from client*/
		n = write(socket,"I got your message!!!",18);/*response to client*/
		if (n < 0) /* error message if socket  not written to socket correctly*/
		{
			error("ERROR writing to socket");
		}
		if(strcmp(buffer,"EXIT\n") == 0)/*checks to see if the client wants to quit*/
		{
			break;/*if client wants to quit break loop*/
		}
	}
	close(socket);/*closes the client socket*/
}


int main(int argc, char *argv[])/*main method*/
{

	int sockfd, newsockfd, portno;
	pthread_t thread;/*create our pthread_t called thread*/
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 2) {
		error("ERROR, no port provided\n");
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)	{
		error("ERROR opening socket");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);/*changes a string the an integer*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,/*binding the socket*/
				sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}
	listen(sockfd,5);/*listens to the socket for client connections*/
	while(1)/*infinte loop*/
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, 
				(struct sockaddr *) &cli_addr, 
				&clilen); 
		if (newsockfd < 0) {
			error("ERROR on accept");
		}
		pthread_create(&thread,0,run_thread,&newsockfd);/*creates a new thread for each client*/
	}
	close(sockfd);
	exit(0);
}
