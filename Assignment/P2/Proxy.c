/*
 * Network Programming
 * Assignment
 * 
 * P2
 * proxy.c
 * 
 * Debanshu Sinha
 */

/* headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

/* constants */
#define MAX_BUFF_LENGTH 1000
#define LISTENQ 5

/* structures */
typedef struct node{
    char host[100];
    struct node* next;    
}Node;

/* functions */
void systemError(const char *msg) {
  perror(msg);
  exit(-1);
}

int blocklisted(char host[],Node* list)
{
    Node* curr;
    for(curr=list;curr!=NULL;curr = curr->next)
    {
	if(strcmp(host,curr->host)==0)
	    return 1;
    }
    
    return 0;
}

int main(int argc,char* argv[])
{
    /* parse command line */
    if(argc<2)
    {
	printf("Usage: %s port_number\n",argv[0]);
	exit(-1);
    }
    int port = atoi(argv[1]);
    
    /* setting up blocklist.txt file */
    Node* blocklist=NULL;
    FILE* fp=fopen("blocklist.txt","r");
    Node* curr;    
    
    while(!feof(fp))
    {
	curr = (Node*)malloc(sizeof(Node));
	fscanf(fp,"%s",curr->host);
	curr->next = blocklist;
	blocklist = curr;
	printf("added to blocklist: %s\n",blocklist->host);
    }	
    
    /* creating and binding to TCP socket server-side */
    int sockfd,connfd,ret;
    struct sockaddr_in servaddr,cliaddr;
    if( (sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	systemError("Socket creation falied");
	
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
  
    // Bind to the local address
    if(bind(sockfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) <0 )
	systemError("Bind to socket failed");
	
    // Listen for incoming requests
    if(listen(sockfd,LISTENQ)<0)
	systemError("Error in listening to bound socket");
	
    socklen_t clilen = sizeof(cliaddr);
    //run accept loop
    for(;;)
    {
	if((connfd = accept(sockfd,(struct sockaddr *) &cliaddr,&clilen))<0)
	    systemError("Error in accepting connection");
	    
	ret=fork();
	
	if(ret<0)
	    systemError("Error in forking");
	    
	/* child to handle connections */
	if(ret==0)
	{
	    close(sockfd);
	    
	    /* receive and parse browser request */
	    char buff[MAX_BUFF_LENGTH],hostname[100],url[100],tmp;
	    ssize_t numBytesRcvd,numBytesSent;
	    
	    if((numBytesRcvd = read(connfd,buff,MAX_BUFF_LENGTH))<0)
		systemError("Error in reading");
		
	    buff[numBytesRcvd]='\0';
	    sscanf(buff,"%s%s%s%s%s",&tmp,url,&tmp,&tmp,hostname);
	    
	    //~ /* DEBUG CODE */
	    //~ printf("hostname: %s\n",hostname);
	    //~ printf("url: %s\n",url);
	    //~ printf("tmp1: %s\n",tmp1);
	    //~ printf("tmp2: %s\n",tmp2);
	    //~ printf("tmp3: %s\n",tmp3);
	    
	    /* check whether host in blocklist */
	    if( blocklisted(hostname,blocklist) )
	    {
		printf("Sending Forbidden 403 Error\n");
		/* send 403 Forbidden error */
		char* err="HTTP/1.1 403 Forbidden\nKeep-Alive: timeout=15, max=100\nConnection: Keep-Alive\nContent-Type: text/html\n\n<!DOCTYPE HTML><html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1><p>You don't have permission to access this page via this proxy.</p><hr><address>Network Programming Assignment P3 Server</address></body></html>";
		if((numBytesSent = write(connfd,err,strlen(err)))<0)
		    systemError("Error in writing to connected socket");
		    
		if(numBytesSent < sizeof(err))
		    systemError("Complete message not sent");
		
	    }
	    else
	    {
		
		/* get actual host to connect to */
		/* setup getaddrinfo */
		struct addrinfo hints,*res;
		bzero(&hints,sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		int rv;
		if ((rv = getaddrinfo(hostname, "80", &hints, &res)) != 0)
		    systemError("getaddrinfo failed");
		
		struct sockaddr_in *addr;
		
		/* try connecting to proper socket */	
		if( (sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		    systemError("Socket creation falied");	
		while(res!=NULL)
		{
		    //~ printf("Trying to connect to:\n");
		    //~ addr = (struct sockaddr_in *)res->ai_addr;
		    //~ printf("\nIP: %s\n",inet_ntoa(addr->sin_addr));
		    //~ printf("\nPort: %d\n",ntohs(addr->sin_port));
		    
		    if((connect(sockfd,(res->ai_addr),sizeof(*addr)))==0)
			break;
			
		    res = res->ai_next;
		}	
		
		if(res==NULL)
		    systemError("could not get proper TCP socket to connect to for http service");
		
		//~ printf("Connected..sending request!\n");
		sprintf(buff,"GET %s HTTP/1.1\nHost: %s\n\n",url,hostname);
		//~ write(1,buff,strlen(buff));
		if((numBytesSent = write(sockfd,buff,strlen(buff))<0))
		    systemError("Error in wrting to connected socket");
		    
		/* read from actual host & write to client */
		while((numBytesRcvd = read(sockfd,buff,MAX_BUFF_LENGTH))>0)
		{
		    buff[numBytesRcvd]='\0';
		    if((numBytesSent = write(connfd,buff,numBytesRcvd))<0)
			systemError("Error in wrting to connected socket");
		    
		    if(numBytesSent < numBytesRcvd)
			systemError("Complete message not sent");
		}
		
		if(numBytesRcvd<0)
		    systemError("Error in reading from actual host");
		    
	    }	    
	    
	    close(connfd);
	    exit(0);
	}
	
	close(connfd);
    }
    
    return 0;
}
	
	    
