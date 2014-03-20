/*
 * Network Programming
 * Assignment
 * 
 * P1
 * Node.c
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
#include <sys/select.h>

/* constants */
#define MAX_BUFF_LENGTH 1000
#define MAX_GROUPS 20

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

int max(int a,int b)	{
    return( a>b ? a:b);
}

int main(int argc,char *argv[])
{
    /* parse command line */
    if(argc<2)
    {
	printf("Usage: %s LiPro_server_port_number \n",argv[0]);
	exit(-1);
    }
    
    int port = atoi(argv[1]);
    
    /* creating socket */
    int sockfd;
    struct sockaddr_in servaddr;
    if( (sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	systemError("Socket creation falied");
	
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    servaddr.sin_port = htons(port);
    
    // Set socket to allow broadcast
    int broadcastPerm = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastPerm, sizeof(broadcastPerm)) < 0)
	systemError("Socket could not be set for broadcasting");
	
    // broadcast request
    ssize_t numBytesRcvd,numBytesSent;
    if((numBytesSent=sendto(sockfd,"Send List",10,0,(struct sockaddr *) &servaddr, sizeof(servaddr)))<0)
	systemError("Failed to broadcast message");
	
    //receive group list
    struct { 
	int n; 
	Group list[MAX_GROUPS]; 
	}groupList;
    if((numBytesRcvd = recvfrom(sockfd,(void *) &groupList, sizeof(groupList), 0, NULL,NULL))<=0)
	systemError("Failed to receive group list");
	
    // remove broadcast flag
    broadcastPerm = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastPerm, sizeof(broadcastPerm)) < 0)
	systemError("Socket could not be RESET against broadcasting");
    
    /* ask for groups to join */
    char* joined = (char *)malloc(groupList.n * sizeof(char) );
    char ans[3];
    int i,numJoined=0;
    for(i=0;i<groupList.n;i++)
    {
	printf("\nGroup%d:\nName: %s\nDo you want to join this group? (y/n): ",(i+1),groupList.list[i].name);
	scanf("%s",ans);
	if(strcmp(ans,"y")==0)
	{
	    joined[i]=1;
	    numJoined++;
	}
	else
	{ joined[i]=0; }
	
    }
	
    /* DEBUG CODE  */
    //~ for(i=0;i<groupList.n;i++)
    //~ {
	//~ printf("Name: %s\tJoined: %d\n",groupList.list[i].name,(int)joined[i]);
    //~ }
    
    /* fork to handle received information */
    int ret = fork();
    if(ret<0)
	systemError("Fork() failed");
    
    /* child code: receive and print data */
    if(ret==0)
    {
	struct ip_mreq mreq;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/* create one socket for every multicast group & bind it */
	int maxfd=0;
	fd_set rset;
	int sockfds[numJoined];
	char joinedGroups[numJoined][100];
	FD_ZERO(&rset);	
	
	
	numJoined =0;
	for(i=0;i<groupList.n;i++)
	{
	    if(joined[i]==1)
	    {
		if( (sockfds[numJoined++] = socket(AF_INET,SOCK_DGRAM,0)) < 0)
		    systemError("Socket creation falied");
		    
		FD_SET(sockfds[numJoined-1],&rset);
		maxfd = max(maxfd,sockfds[numJoined-1]);
		
		strcpy(joinedGroups[numJoined-1],groupList.list[i].name);
		
		mreq.imr_multiaddr.s_addr = inet_addr(groupList.list[i].ip);
		servaddr.sin_port = htons(groupList.list[i].port);
		
		if(bind(sockfds[numJoined-1],(struct sockaddr *) &servaddr,sizeof(servaddr)) <0 )
		    systemError("Bind to socket failed");
		    
		if (setsockopt(sockfds[numJoined-1], IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof(mreq)) < 0)
		    systemError("Failed to add to multicast group");		
				
	    }	    
	}
	
	/* use select for readability */
	char msg[1000];
	struct sockaddr_in cliaddr;
	socklen_t clilen = sizeof(cliaddr);
	for(;;)
	{
	    select(maxfd+1,&rset,NULL,NULL,NULL);
	    for(i=0;i<numJoined;i++)
	    {
		if(FD_ISSET(sockfds[i],&rset))
		{
		    if((numBytesRcvd = recvfrom(sockfds[i],msg, sizeof(msg), 0, (struct sockaddr *) &cliaddr,&clilen))<=0)
			systemError("Failed to receive group list");
		    
		    if( numBytesRcvd > 0 )
		    {
			msg[numBytesRcvd]='\0';
			printf("\nReceived message from Group: %s\nMessage = %s\n",joinedGroups[i], msg);
		    }
		}
		else
		{
		    FD_SET(sockfds[i],&rset);
		}
	    }
	}	
    }
    /* parent code to send multicast data */
    else
    {
	char name[100],msg[1000],c;
	/* infinitely run node */		
	for(;;)
	{
	    printf("\nWhich group do you want to send message to ?\nGroup name: ");
	    scanf("%s",name);
	    
	    for(i=0;i<groupList.n;i++)
	    {
		if(strcmp(groupList.list[i].name,name)==0)
		{
		    if(joined[i]==0)
		    {
			printf("\nPlease a group name you have joined!\n");
		    }
		    else
		    {
			printf("Enter Message to send: ");
			while ((c = getchar()) != '\n' && c != EOF);
			fgets(msg,1000,stdin);
			servaddr.sin_addr.s_addr = inet_addr(groupList.list[i].ip);
			servaddr.sin_port = htons(groupList.list[i].port);
			
			if((numBytesSent=sendto(sockfd,msg,sizeof(msg),0,(struct sockaddr *) &servaddr, sizeof(servaddr)))<0)
			    systemError("Failed to multicast message");
			 			
		    }
		    break;
		}
		    
	    }
	}
    }
	
    
    return 0;
}
		
    

