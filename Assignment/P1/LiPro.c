/*
 * Network Programming
 * Assignment
 * 
 * P1
 * LiPro.c
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

/* structures */
typedef struct {
    char name[100];
    char ip[100];
    int port;
}Group;

/* functions */
void systemError(const char *msg) {
  perror(msg);
  exit(-1);
}

int main(int argc,char *argv[])
{
    /* parse command line */
    if(argc<2)
    {
	printf("Usage: %s port_number [group1-name] [group1-ip] [group1-port] [group2-name] ...\n",argv[0]);
	exit(-1);
    }
    
    int port = atoi(argv[1]);
    int numGroups = (argc - 2)/3;
    struct { 
	int n; 
	Group list[numGroups]; 
	}groupList;
    
    groupList.n = numGroups;
    int i;
    for(i=0;i<numGroups;i++)
    {
	strcpy(groupList.list[i].name,argv[2 + (i*3) + 0]);
	strcpy(groupList.list[i].ip,argv[2 + (i*3) + 1]);
	groupList.list[i].port = atoi(argv[2 + (i*3) + 2]);
    }
    
    
    
    
    //~ /* DEBUG CODE  */
    //~ for(i=0;i<numGroups;i++)
    //~ {
	//~ printf("Name: %s\tIp: %s\tPort: %d\n",list[i].name,list[i].ip,list[i].port);
    //~ }
    //~ printf("Size of group list: %d\n",sizeof(list));
    //~ printf("Size of group list: %d\n",sizeof(list));
    //~ printf("Size of grouplist list: %d\n",sizeof(groupList));
    //~ printf("Size of grouplist groups: %d\n",sizeof(groupList.groups));
    
    
    /* creating and binding to UDP socket server-side */
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;
    if( (sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	systemError("Socket creation falied");
	
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
  
    // Bind to the local address
    if(bind(sockfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) <0 )
	systemError("Bind to socket failed");
	
    /* Run forever loop for waiting & replying to nodes */
    ssize_t numBytesRcvd,numBytesSent;
    char buffer[MAX_BUFF_LENGTH];
    socklen_t clntAddrLen = sizeof(cliaddr);
    for(;;)
    {
	if( (numBytesRcvd = recvfrom(sockfd, buffer, MAX_BUFF_LENGTH, 0,(struct sockaddr *) &cliaddr, &clntAddrLen)) < 0 )
	    systemError("Error in receiving bytes");
	
	if( numBytesRcvd > 0 )
	{
	    buffer[numBytesRcvd]='\0';
	    printf("\nReceived Broadcast from Node with:\nIP: %s\nPort: %d\nMessage: %s\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port),buffer);
	    
	    
	    if((numBytesSent=sendto(sockfd,(void *) &groupList,sizeof(groupList),0,(struct sockaddr *) &cliaddr, sizeof(cliaddr)))<0)
		systemError("Error in sending group list");
		
	    if( numBytesSent!= (sizeof(groupList)) )
		systemError("Incorrect number of bytes sent");
		
	    
	}
    }
    
    
    return 0;
}

