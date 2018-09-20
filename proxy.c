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

char *parse_headers(char* raw, char* element) {
	if(strcmp(element,"method") == 0) {
		char* token  = strtok(raw, " ");
		return token;
	}
	else if (strcmp(element, "host") == 0 ) {
		char* token  = strtok(raw, " ");
		int i = 0;
		while(token) {
			if( i == 1) {
				token = strtok(token, "://");
				token = strtok(NULL, "://");
				return token;
			}
			token = strtok(NULL, " ");
			i++;
		}
	}
	else if (strcmp(element, "uri") == 0) {
		char* token = strtok(raw, " ");
		token = strtok(NULL, " ");
		token +=7;
		char *e;
		int index;
		e = strchr(token, '/');
		index = (int) (e - token);
		char* uri = index+token;
	}
}

char *proxy_request(char* host, char* uri, char* method) {
	printf("made it into method\n");
	if ((strcmp("CONNECT",method) != 0) && (method != NULL) ){
		int sockfd, n, comp, total, sent, bytes, received;
		int portno = 80;
	        struct sockaddr_in serv_addr;
	        struct hostent *server;
	        char request[2048],response[8192];
		bzero(response,8192);
		bzero(request,2048);
	        sockfd = socket(AF_INET, SOCK_STREAM, 0);
	        if (sockfd < 0)
	                error("ERROR opening socket");
	        server = gethostbyname(host);
	        if (server == NULL) {
	                fprintf(stderr,"ERROR, no such host\n");
	                return "NO SUCH HOST";
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
		strcpy(request, method);
		strcat(request, " ");
		strcat(request, uri);
		strcat(request, " HTTP/1.1\r\nHOST: ");
		strcat(request, host);
		strcat(request, ":80\r\nConnection: close\r\n\r\n");
		printf("Request String: %s\n", request);
		total = strlen(request);
		sent = 0;
		bytes = write(sockfd,request,sizeof(request));
		if( bytes < 0) {
			error("ERROR Writing to socket\n");
		}
		puts("request sent off");
		sent+=bytes;
		memset(response,0,sizeof(response));
		total = sizeof(response)-1;
		received = 0;
                char* response_return =(char *) malloc(sizeof(response)*100000);
		do {
			puts("start of response");
			bytes = read(sockfd,response,8191);
			if (bytes < 0) {
				error("ERROR Reading response from socket\n");
			}
			if (bytes == 0) { 
				break;
			}
			received+=bytes;
                        sprintf(response_return + strlen(response_return),"%s", response);
        //                memcpy(response_return, response, sizeof(response));
                        bzero(response,8192);
		} while(bytes !=0);
		if (received == total) {
			error("ERROR Storing response from socket\n");
		}
		printf("%s", response);
		printf("%i", received);
	        close(sockfd);/*close socket*/
//		char* response_return =(char *) malloc(sizeof(response));
//		memcpy(response_return, response, sizeof(response));
		return response_return;
	}
	else {
		return "SSL NOT IMPLEMENTED";
	}
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
//        printf("%s\n",buffer);/*message from client*/
	time_string = ctime(&current_time);
	f = fopen("proxy.log", "a+");
	// parse headers
	char temp_buffer[sizeof(buffer)];
	memcpy(temp_buffer,buffer,sizeof(buffer));
	char* request_method = parse_headers(buffer,"method");
	bzero(buffer,254);
	memcpy(buffer,temp_buffer,sizeof(temp_buffer));
	char* token = strtok(buffer, " ");
        token = strtok(NULL, " ");
	char host_string[100];
	strcpy(host_string,token);
        token +=7;
        char *e;
        int index;
        e = strchr(token, '/');
        index = (int) (e - token);
        char* uri = index+token;
	char* host = parse_headers(temp_buffer,"host");
	if(uri == NULL) {
		uri = "";
	}
	if(host == NULL ) {
		host = "";
	}
	if (request_method == NULL ) {
		request_method = "GET";
	}
	printf("method=%s uri=%s host=%s\n", request_method, uri , host);
	char* server_response=proxy_request(host, uri, request_method);
	bzero(buffer,254);
        bzero(temp_buffer, 254);
//        printf("%s\n",server_response);
	n = write(socket,server_response,strlen(server_response));/*response to client*/
	if (n < 0) /* error message if socket  not written to socket correctly*/
	{
		error("ERROR writing to socket");
	}
	printf("%s %s %s\n\n", time_string, ipstr, host_string);
	fprintf(f, "%s %s %s\n\n", time_string, ipstr, host_string);
	fclose(f);
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
//		pthread_create(&thread,0,run_thread,&newsockfd);/*creates a new thread for each client*/
		if(fork() == 0) {
			run_thread(&newsockfd);
			exit(1);
		}
	}
	close(sockfd);
	exit(0);
}
