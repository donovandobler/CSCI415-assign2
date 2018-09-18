#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
void error(const char *msg)/*error message generated*/
{
	perror(msg);
	exit(1);
}

char *proxy_request(char* host, int iport) {
	int sockfd, portno, n, comp;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        char bufferread[256];
        char bufferwrite[256];
        portno = iport;/*change a string to an integer*/
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                error("ERROR opening socket");
        server = gethostbyname(host);
        if (server == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
                        (char *)&serv_addr.sin_addr.s_addr,
                        server->h_length);
        serv_addr.sin_port = htons(portno);
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
                error("ERROR connecting");
        while(strcmp(bufferwrite, "EXIT\n") != 0)/*checks to see if the host wants to quit*/
        {
                printf("Please enter the message: ");/*message to be sent*/
                bzero(bufferwrite,256);
                fgets(bufferwrite,255,stdin);/*get from the console*/
                n = write(sockfd,bufferwrite,strlen(bufferwrite));
                if (n < 0)
                {
                        error("ERROR writing to socket");
                }
                bzero(bufferread,256);/*clears the bufferread array*/
                n = read(sockfd,bufferread,255);/*read from the socket*/
                if (n < 0)
                        error("ERROR reading from socket");
                printf("%s\n",bufferread);
        }
        close(sockfd);/*close socket*/
        return 0;

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
	time_t current_time;
	char* time_string;
	current_time = time(NULL);
	address_length = sizeof(cli_addr);
	getpeername(socket, (struct sockaddr*)&cli_addr, &address_length);
	char ipstr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(cli_addr.sin_addr), ipstr, INET_ADDRSTRLEN);
	FILE *f;

	if(f == NULL){/*log file failure*/}
	while(1)/*endless loop*/
	{
		bzero(buffer,256);
		n = read(socket,buffer,255);
		if (n < 0) 
		{
			error("ERROR reading from socket");
		}
		printf("%s\n",buffer);/*message from client*/
		int buffer_size = sizeof(buffer);
		printf("Buffer size in bytes is %d",buffer_size);
		time_string = ctime(&current_time);
		f = fopen("proxy.log", "a+");
		const char s[2] = " ";
		char *buffer_token;
		buffer_token = strtok(buffer, s);
		buffer_token = strtok(NULL, s);
		fprintf(f,"%s %s %s %d",time_string,ipstr,buffer_token,buffer_size);
		fprintf(f,"\n\n");
		fclose(f);
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
