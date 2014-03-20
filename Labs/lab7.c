/* IS C462
 * NETWORK PROGRAMMING
 * Lab 7 - Assignment
 *
 * Debanshu Sinha
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>

int max(int a,int b)
{
	if (a>b)
		return a;
	return b;
}

int main(int argc,char *argv[])
{
	int conn1,conn2,listenfd,n,maxfd,clilen;
	if(argc<2)
	{
		printf("Usage: %s [port_number]\n",argv[0]);
		return 0;
	}
	int port = atoi(argv[1]);
	fd_set rset;
	char buf[1000];
	struct sockaddr_in client1,client2,server;
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	bzero(&server,sizeof(struct sockaddr_in));

	server.sin_family=AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	bind(listenfd,(struct sockaddr *)&server,sizeof(server));
	listen(listenfd,2);
	maxfd=listenfd;
	FD_ZERO(&rset);
	clilen = sizeof(struct sockaddr_in);
	char *end="Other client exited. Server exiting...";

	printf("Waiting for first client...\n");
	conn1 = accept(listenfd,(struct sockaddr *)&client1,&clilen);
	printf("First client connected..waiting for next client....\n");
	conn2 = accept(listenfd,(struct sockaddr *)&client2,&clilen);
	printf("Second client connected....time to chat...\n");

	maxfd = max(conn1,maxfd);
	maxfd = max(conn2,maxfd);
	FD_SET(conn1,&rset);
	FD_SET(conn2,&rset);

	for(;;)
	{
		select(maxfd+1,&rset,NULL,NULL,NULL);
		if(FD_ISSET(conn1,&rset))
		{
			if( (n=read(conn1,buf,1000))==0)
			{
				printf("First client exited. Server exiting..\n");
				write(conn2,end,strlen(end));
				return 0;
			}
			else
				write(conn2,buf,n);
		}
		else
			FD_SET(conn1,&rset);

		if(FD_ISSET(conn2,&rset))
		{
			if( (n=read(conn2,buf,1000))==0)
			{
				printf("Second client exited. Server exiting..\n");
				write(conn1,end,strlen(end));
				return 0;
			}
			else
				write(conn1,buf,n);
		}
		else
			FD_SET(conn2,&rset);
	}
	return 0;
}

