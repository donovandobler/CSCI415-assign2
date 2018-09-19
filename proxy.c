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
#include <assert.h>
void error(const char *msg)/*error message generated*/
{
	perror(msg);
	exit(1);
}

char *proxy_request(char* host, char* uri) {
	int sockfd, n, comp, total, sent, bytes, received;
	int portno = 80;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        char request[2048],response[10000];
	bzero(response,10000);
	bzero(request,2048);
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
        //request
	printf("host=%s uri=%s\n", host, uri);
	strcpy(request, "GET /");
	strcat(request, uri);
	strcat(request, " HTTP/1.1\r\nHOST: ");
	strcat(request, host);
	strcat(request, ":80\r\nConnection: close\r\n\r\n");
	total = strlen(request);
	sent = 0;

	puts("start of request");
	bytes = write(sockfd,request,sizeof(request));
	if( bytes < 0) {
		error("ERROR Writing to socket\n");
	}
	sent+=bytes;
	memset(response,0,sizeof(response));
	total = sizeof(response)-1;
	received = 0;
	do {
		puts("start of response");
		bytes = read(sockfd,response+received,total-received);
		if (bytes < 0) {
			error("ERROR Reading response from socket\n");
		}
		if (bytes == 0) { 
			break;
		}
		received+=bytes;
	} while(received < total);
	if (received == total) {
		error("ERROR Storing response from socket\n");
	}
//	printf("%s", response);
        close(sockfd);/*close socket*/
	char* response_return =(char *) malloc(sizeof(response));
	memcpy(response_return, response, sizeof(response));
	return response_return;
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
	bzero(buffer,256);
	n = read(socket,buffer,255);
	if (n < 0) 
	{
		error("ERROR reading from socket");
	}
	printf("%s\n",buffer);/*message from client*/
	int buffer_size = sizeof(buffer);
	printf("Buffer size in bytes is %d\n",buffer_size);
	time_string = ctime(&current_time);
//	f = fopen("proxy.log", "a+");
	const char s[2] = " ";
	const char split[2] = "/";
	char *buffer_token;
	int i = 0;
	char host[256];
	char uri[256];
	char* temp_uri;
	char* temp;
	char* temp_token;
	char* protocol_version;
//	char response[10000];
//	bzero(response,100000);
	bzero(uri,256);
	bzero(host,256);
	buffer_token = strtok(buffer, s);
	while (buffer_token != NULL) {
		if(i == 1) {
			temp = buffer_token;
			temp += 7;
			temp_token = strtok(temp, split);
//			url_token = strtok(NULL, split);
//			strcpy(url, "/");
			strcat(host, temp_token);
//			printf("%s\n", url);
			
		}
		else if(i == 2) {
			temp_uri = buffer_token;
			strcpy(uri, temp_uri);
		}
		else if(i == 3) {
			break;
		}
		buffer_token = strtok(NULL, s);
		i++;
	}
//	printf("host=%s uri=%s\n", host, uri);
//	fprintf(f,"%s %s %s %d",time_string,ipstr,url,buffer_size);
//	fprintf(f,"\n\n");
//	fclose(f);
//	url +=  7;
//	char* uri;
//	char* uri_temp;
//	uri_temp = strtok(url, split);
//	printf("%s", uri_temp);
//	while (url != NULL) {
//		uri_temp = strtok(NULL, split);
//		uri=uri_temp;
//		break;
//	}
//	printf("%s", uri);
//		char uri[256];
//		strncpy(uri, host_token, sizeof host_token);
//		prepend(uri,&uri_token);
//		printf("\n%s", uri);
//		printf("\n%s",uri);
		//n = write(socket,"I got your message!!!",18);/*response to client*/
		//Make call fot proxy_request
//	printf("in thread host=%s uri=%s", host, uri);
//	response = 
	char* server_response=proxy_request(host, uri);
	n = write(socket,server_response,strlen(server_response));/*response to client*/
	if (n < 0) /* error message if socket  not written to socket correctly*/
	{
		error("ERROR writing to socket");
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
