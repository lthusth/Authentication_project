#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "util.h"


int authentication(char *string)
{	
	char line[256];
	FILE *file;
	file = fopen("user.txt", "r");
	if (!file) {
		printf("File did not exist!\n");
		return 0;
	}
	while (fgets(line, sizeof(line), file)) {
		line[strlen(line)-1] = '\0';	//replace last character(assume a tab) by a nul terminator
		
		if(strcmp(line, string) == 0){
			printf("Matched!");
			return 1;
		}
	}

	fclose(file);
	return 0;
}


int main(int argc, char const *argv[])
{
	int sockfd, clientfd;
	socklen_t clen;
	struct sockaddr_in saddr, caddr;
	unsigned short port = 8784;
	char mess[1000];

	//creating socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0) {
		printf("Error: Creating socket failed\n");
		return 1;
	}
	//binding
	
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) 
	{
		printf("Error: Binding failed\n");
		return 1;
	}else{
		printf("Successfully binded\n");
	}

	//listening
	if (listen(sockfd, 5) < 0) 
	{
		printf("Error: Listening failed\n");
		return 1;
	}

	while(1){
		clen = sizeof(caddr);
		clientfd = accept(sockfd, (struct sockaddr *) &caddr, &clen);
		if (clientfd < 0) 
		{
			printf("Error: Accepting connection failed\n");
			return 1;
		}else{
			printf("Client connected success\n");
		}
		recv_wrapper(clientfd, mess, sizeof(mess), "Authen received");
		if(authentication(mess)){
			send_wrapper(clientfd, "valid", "Authen send back");
		}else{
			send_wrapper(clientfd, "invalid", "Authen send back");
		}
		
		close(clientfd);
	}

	close(sockfd);
	return 0;
}

void send_wrapper(int sockfd, char *msg, char *errmsg)
{
	fprintf(stderr, "%s: Sending...\n", errmsg);
	fprintf(stderr, "%s: Content: %s\n", errmsg, msg);
	if (send(sockfd, msg, strlen(msg), 0) < 0) {
		perror(errmsg);
		exit(1);
	}
	fprintf(stderr, "%s: Sent.\n", errmsg);
}

void recv_wrapper(int sockfd, char *msg, int size, char *errmsg)
{
	fprintf(stderr, "%s: Receiving...\n", errmsg);
	memset(msg, 0, size);
	int status;
	if ((status = recv(sockfd, msg, size, 0)) < 0) {
		perror(errmsg);
		exit(1);
	} else if (status == 0) {
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
		msg = NULL;
		fprintf(stderr, "Socket hung up.\n");
		exit(1);
	}
}