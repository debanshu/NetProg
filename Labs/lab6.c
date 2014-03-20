/* IS C462
 * NETWORK PROGRAMMING
 * Lab 6 - Assignment
 *
 * Debanshu Sinha
 */


#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
   int sd, psd;
   struct   sockaddr_in name,client;
   char   buf[1024],*ipaddress;
   int    cc,clilen;
   clilen=sizeof(client);
   if(argc<2)
   {
	   printf("Please give port number as command-line argument\n");
	   return 0;
   }

   sd = socket (AF_INET,SOCK_STREAM,0);
   name.sin_family = AF_INET;
   name.sin_addr.s_addr = htonl(INADDR_ANY);
   name.sin_port = htons(atoi(argv[1]));

   bind( sd, (struct sockaddr*) &name, sizeof(name) );
   listen(sd,3);
   for(;;) {
   psd = accept(sd, (struct sockaddr *)&client, &clilen);
   if( fork()==0)
   {
	   close(sd);
	   printf("Child Server pid: %d\n",getpid());
	   printf("Client's ip address: %s\n",inet_ntoa(client.sin_addr));
	   printf("Client's port: %d\n\n\n",ntohs(client.sin_port));

	   struct sockaddr_in server;
	   sd = socket (AF_INET,SOCK_STREAM,0);
	   server.sin_family = AF_INET;
	   server.sin_addr.s_addr=inet_addr("172.18.9.1");
	   server.sin_port = htons(80);

	   int cfd =connect(sd, (struct sockaddr*) &server, sizeof(server));

	   char* req="GET / HTTP/1.1\nHost:psd\n\n";
	   send(sd,req,strlen(req),0);
	   int rcvdata;
	   char data[1000];
	   while ((rcvdata = recv(sd,data,1000, 0))>0)
	   {
		   send(psd,data,rcvdata,0);
	   }

	   close(psd);
	   close(sd);
	   exit(0);

   }
   close(psd);
   }

   return 0;

}

