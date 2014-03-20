/*
 * Network Programming
 * Assignment
 * 
 * P3
 * DownloadFaster.c
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
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

/* constants */
#define MAX_BUFF_LENGTH 1000

/* functions */
void systemError(const char *msg) {
  perror(msg);
  exit(-1);
}

int max(int a,int b)	{
    return( a>b ? a:b);
}

int main(int argc,char* argv[])
{
    /* parse command line */
    if(argc<2)
    {
	printf("Usage: %s number_of_parts\n",argv[0]);
	exit(-1);
    }
    long N = atol(argv[1]);
    
    /* get url from user */
    char url[500],*hostname,*filename,urlcopy[500];
    printf("Enter url/download link (no spaces allowed): ");
    scanf("%s",url);
    strcpy(urlcopy,url);
    
    /* parse url */
    char* delim = "/";
    char* tmp = strtok(url,delim);
    if(strcmp(tmp,"http:")==0)
    {
	//tmp = strtok(NULL,delim);
	hostname = strtok(NULL,delim);
    }
    else
    {
	hostname = tmp;
    }
    
    do{
	filename = tmp;
	tmp = strtok(NULL,delim);
    }while(tmp!=NULL);
    
    //~ printf("hostname: %s\tfilename: %s\n",hostname,filename);
    /* setup getaddress infor for getting ip for hostname */
    struct addrinfo hints,*res;
    bzero(&hints,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int rv,sockfd;
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
    /* get content length meta-information */
    char buff[MAX_BUFF_LENGTH];
    ssize_t numBytesRcvd,numBytesSent;
    sprintf(buff,"HEAD %s HTTP/1.1\nHost: %s\n\n",urlcopy,hostname);
    printf("%s\n",buff);
        
    if((numBytesSent = write(sockfd,buff,strlen(buff)))<0)
	systemError("Error requesting meta-information");
	
    if((numBytesRcvd = read(sockfd,buff,MAX_BUFF_LENGTH))<0)
	systemError("Error in reading");
	
    buff[numBytesRcvd]='\0';    
    printf("Response: %s\n",buff);     
    close(sockfd);
    
    /* store this socket structure */
    addr = (struct sockaddr_in *)res->ai_addr;
    
    /* get content-length */
    long contentLength;
    char* pos = strstr(buff,"Content-Length:");
    if(pos==NULL)
    {
	printf("Content length not received from server. Cannot proceed.\n");
	exit(-1);
    }
    else
    {
	char tmp[20];
	sscanf(pos+15,"%s",tmp);
	contentLength = atol(tmp);
	//~ printf("Length: %ld\n",contentLength);
    }
    
    /* allocating buffers/memory/etc */
    char * file = (char *)malloc(contentLength*sizeof(char));
    bzero(file,sizeof(*file));
    char* parts[N+1];
    long partLength = contentLength/N;
    int i;
    parts[0]=file;
    for(i=1;i<N;i++)
    {
	parts[i]=parts[i-1]+partLength;
    }
    parts[N] = (file+contentLength);
    
    //~ /* debug */
    //~ for(i=0;i<N;i++)
    //~ {
	//~ printf("%d\n",parts[i]-file);
    //~ }
	
    /* setting up non-blocking call variables */
    int sockfds[N];
    char connected[N],removed[N];
    char* friptr[N];
    int flags,maxfd=0,connStat,done=0;
    fd_set rset,wset;
    
    /* initialize variables, make connect calls */
    bzero(&connected,sizeof(connected));
    bzero(&removed,sizeof(removed));
    //bzero(&complete,sizeof(complete));
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    
    for(i=0;i<N;i++)
    {
	friptr[i]=parts[i];
	
	if( (sockfds[i] = socket(AF_INET,SOCK_STREAM,0)) < 0)
	    systemError("Socket creation falied");
	    
	maxfd = max(maxfd,sockfds[i]);
	FD_SET(sockfds[i],&rset);
	FD_SET(sockfds[i],&wset);
	
	flags = fcntl(sockfds[i],F_GETFL,0);
	fcntl(sockfds[i],F_SETFL,flags | O_NONBLOCK);
		
	if((connStat = connect(sockfds[i],res->ai_addr,sizeof(*addr)))<0)
	    if(errno!= EINPROGRESS)
		systemError("Non-blocking connect failed");
		
	if(connStat == 0)
	{
	    connected[i]=1;
	    sprintf(buff,"GET %s HTTP/1.1\nHost: %s\nConnection: Keep-Alive\nRange: bytes=%ld-%ld\n\n",urlcopy,hostname,(parts[i]-file),(parts[i+1]-file-1));
	    printf("%s\n",buff);
	    if((numBytesSent = write(sockfds[i],buff,strlen(buff)))<0)
		systemError("Error requesting part-information");
	
	    FD_CLR(sockfds[i],&wset);
	}
	    
    }
    
    /*DEBUG */
    for(i=0;i<N;i++)
    {
	printf("part %d connected: %d\n",(i),connected[i]);
    }
    
    //~ FILE* dp=fopen("test","w");
    struct timeval tv;
    tv.tv_sec=3;
    tv.tv_usec=0;
    int dbg;
    while( done< N)
    {
	
	select(maxfd+1,&rset,&wset,NULL,NULL);
	//printf("%d ",dbg);
	//printf("%d ",done);
	for(i=0;i<N;i++)
	{
	    if(connected[i]==0)
	    {
		// failed connect, try again
		if(FD_ISSET(sockfds[i],&rset) && FD_ISSET(sockfds[i],&wset))
		{
		    if((connStat = connect(sockfds[i],res->ai_addr,sizeof(*addr)))<0)
			if(errno != EINPROGRESS)
			    systemError("Non-blocking connect failed");
		
		    if(connStat == 0)
		    {
			connected[i]=1;
			sprintf(buff,"GET %s HTTP/1.1\nHost: %s\nConnection: Keep-Alive\nRange: bytes=%ld-%ld\n\n",urlcopy,hostname,(parts[i]-file),(parts[i+1]-file));
			printf("%s\n",buff);
			if((numBytesSent = write(sockfds[i],buff,strlen(buff)))<0)
			    systemError("Error requesting part-information");
			
			FD_CLR(sockfds[i],&wset);
		    }
		    
		    continue;
			
		}
		else if(FD_ISSET(sockfds[i],&wset))	//connected properly
		{
		    connected[i]=1;
		    sprintf(buff,"GET %s HTTP/1.1\nHost: %s\nConnection: Keep-Alive\nRange: bytes=%ld-%ld\n\n",urlcopy,hostname,(parts[i]-file),(parts[i+1]-file-1));
		    printf("%s\n",buff);
		    if((numBytesSent = write(sockfds[i],buff,strlen(buff)))<0)
			systemError("Error requesting part-information");
		    
		    FD_CLR(sockfds[i],&wset);
		    FD_SET(sockfds[i],&rset);
		}
		else
		{
		    FD_SET(sockfds[i],&rset);
		    FD_SET(sockfds[i],&wset);
		}
	    }
	    else
	    {
		// read data for part
		if(FD_ISSET(sockfds[i],&rset))
		{
		    if(removed[i]==0)
		    {
			if((numBytesRcvd = read(sockfds[i],buff,MAX_BUFF_LENGTH))<0)
			    if( errno != EWOULDBLOCK )
				systemError("Error in reading part");
				
			buff[numBytesRcvd]='\0';
			
			pos = strstr(buff,"\n\n");
			if(pos!=NULL)
			{
			    removed[i]=1;
			    pos=pos+2;
			    strncpy(friptr[i],pos,(buff+numBytesRcvd-pos));
			    friptr[i] += (buff+numBytesRcvd-pos);
			    *pos = '\0';
			    //printf("%s\n",buff);	
			}
		    }
		    else
		    {
				
			
			
		    if((numBytesRcvd = recv(sockfds[i],buff,MAX_BUFF_LENGTH,0))<0)
			if( errno != EWOULDBLOCK )
			    systemError("Error in reading part"); 
			    
		    buff[numBytesRcvd]='\0';
		    strcpy(friptr[i],buff);
		    
		    //~ *(friptr[i] + numBytesRcvd)='\0';
		    //~ printf("%s",friptr[i]);
		    
		    friptr[i] += numBytesRcvd;
		    //printf("I: %d\tptr: %d\n",i,numBytesRcvd);
		    //~ if(numBytesRcvd==0 || (friptr[i]==(parts[i+1])))
		    //~ {
			if((friptr[i]<parts[i+1]) && (numBytesRcvd==0))
			{
			sprintf(buff,"GET %s HTTP/1.1\nHost: %s\nConnection: Keep-Alive\nRange: bytes=%ld-%ld\n\n",urlcopy,hostname,(friptr[i]-file),(parts[i+1]-file-1));
			printf("%s\n",buff);
			if((numBytesSent = write(sockfds[i],buff,strlen(buff)))<0)
			    systemError("Error requesting part-information");
			    
			    removed[i]=0;
			}
			
			
			if((friptr[i]==parts[i+1]) && (numBytesRcvd==0))
			{
			    done++;
			    printf("done: %d\n",done);
			    printf("%d : %ld %ld\n",i,friptr[i]-file,friptr[i]-parts[i]);
			    
			    
			    FD_CLR(sockfds[i],&rset);
			    //close(sockfds[i]);
			}
			
		    }
		    
		}
		else
		{
		    if(friptr[i]<parts[i+1])
			FD_SET(sockfds[i],&rset);
		}
	    }
	}
    }
    
    /* writing input data to file */
    FILE* fp =fopen(filename,"w");
    fwrite(file,contentLength,1,fp);
    fclose(fp);	    
	

    
    return 0;
}
    
