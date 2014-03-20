/* IS C462
 * NETWORK PROGRAMMING
 * Lab 8 - Assignment
 *
 * Debanshu Sinha
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>

    // global variable the number of active
    // threads (clients)
    int active_threads=0;

    // mutex used to lock active_threads
    pthread_mutex_t at_mutex = PTHREAD_MUTEX_INITIALIZER;

    // condition var. used to signal changes 
    pthread_cond_t at_cond = PTHREAD_COND_INITIALIZER;

static void	*threadfunc(void *);		/* each thread executes this function */

int main(int argc,char *argv[])
{   
    int connfd,listenfd,clilen;
    pthread_t tid1;
	if(argc<2)
	{
		printf("Usage: %s [port_number]\n",argv[0]);
		return 0;
	}
	int port = atoi(argv[1]);
	struct sockaddr_in client,server;
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	bzero(&server,sizeof(struct sockaddr_in));

	server.sin_family=AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	bind(listenfd,(struct sockaddr *)&server,sizeof(server));
	listen(listenfd,5);
	clilen = sizeof(struct sockaddr_in);
    
    while (1) {
	pthread_mutex_lock(&at_mutex);
	while (active_threads < 2 ) {
		active_threads++;
        connfd = accept(listenfd,(struct sockaddr *)&client,&clilen);
		pthread_create(&tid1, NULL, &threadfunc, (void *) connfd);
	}
	pthread_cond_wait( &at_cond, &at_mutex);
	pthread_mutex_unlock(&at_mutex);
    }    
    
	return 0;
}

static void *threadfunc(void *arg)
{
	pthread_detach(pthread_self());
    int sockfd  = (int)arg;
    ssize_t n;
    char buf[1000];
	while((n=read(sockfd,buf,1000))>0)
	{
	write(sockfd,buf,n);
	}
	close(sockfd);		/* we are done with connected socket */
    pthread_mutex_lock(&at_mutex);
	active_threads--;
	pthread_cond_signal(&at_cond);
	pthread_mutex_unlock(&at_mutex);
	
	return(NULL);
}


