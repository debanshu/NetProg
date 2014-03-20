/*
 * Netprog Exam
 *
 * Debanshu Sinha
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/mman.h>

#define LISTENQ 5
#define MAXSIZE 1000

int N;
typedef struct{
	pthread_t thread_tid;
	long clients_handled;
	char *cliname;
	struct sockaddr *cliaddr;
	int connfd;
}Thread;

Thread* threads;
socklen_t addrlen;
int listenfd;

char *err1="You have already joined!";
char *err2="You must join before sending any other message";
char *err3 = "ERROR <client not online>";
char *err4 = "Unrecognized command";
char *msg  = "Listing all connected users\n";

int err1len,err2len,err3len,err4len,msglen;


pthread_mutex_t accept_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *client_lock;

void thread_init(int);
void* thread_main(void*);

int main(int argc,char* argv[])
{
	err1len = strlen(err1);
	err2len = strlen(err2);
	err3len = strlen(err3);
	err4len = strlen(err4);
	msglen = strlen(msg);
	
	if(argc<2)
	{
		printf("Usage: %s num_prethreads [port]\n",argv[0]);
		exit(-1);
	}
	
	N=atoi(argv[1]);
	int port;
	if(argc>2)
		port=atoi(argv[2]);
	else
		port= 5555;

	struct sockaddr_in servaddr;
	addrlen = sizeof(servaddr);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family  =AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	listenfd = socket(AF_INET,SOCK_STREAM,0);

	if(bind(listenfd,(struct sockaddr *)&servaddr,addrlen) < 0 )
	{
		perror("Bind failed!");
		exit(-1);
	}
	listen(listenfd,LISTENQ);

	threads = (Thread *)malloc(N*sizeof(Thread));
	client_lock = (pthread_mutex_t *)malloc(N*sizeof(pthread_mutex_t));
	bzero(threads,N*sizeof(Thread));
	bzero(client_lock, N*sizeof(pthread_mutex_t));

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	int idx;
	for(idx=0;idx<N;idx++)
	{
		pthread_mutex_init(&client_lock[idx],&attr);
		thread_init(idx);
	}
	
	for(;;)
		pause();	
	
	return 0;
}

void thread_init(int i)
{
	pthread_create(&threads[i].thread_tid,NULL,&thread_main,(void *)i);
}

void* thread_main(void* arg)
{
	int i = (int) arg;
	int j;
	free(threads[i].cliaddr);
	threads[i].cliaddr = malloc(addrlen);
	
	threads[i].cliname = NULL;
	socklen_t clilen = addrlen;
	char buff[MAXSIZE];
	int recvd,sent,joined;
	char *pos;
	char *saveptr;
	
	for(;;)
	{
		pthread_mutex_lock(&accept_lock);
		threads[i].connfd = accept(listenfd,threads[i].cliaddr,&clilen);
		pthread_mutex_unlock(&accept_lock);
		
		/* process client*/
		joined =0;
		for(;;)
		{
			if((recvd = read(threads[i].connfd,buff,MAXSIZE))<0)
			{
				printf("Error in receiving client Message. Client disconnected\n");
				break;
			}
			buff[recvd] = '\0';
			pos = strtok_r(buff," ",&saveptr);
			printf("%s\n",pos);
			if( strncmp(pos,"JOIN",4)==0)
			{
				if(joined == 1)
				{
					pthread_mutex_lock(&client_lock[i]);
					if((sent = write(threads[i].connfd,err1,err1len))<0)
					{
						printf("Send to client failed. Client disconnected\n");
						break;
					}
					pthread_mutex_unlock(&client_lock[i]);	
				}
				else
				{
					joined = 1;
					pos = strtok_r(NULL," ",&saveptr);
					printf("%s\n",pos);
					threads[i].cliname = strdup(pos);
				}
			}
			else if(joined == 0)
			{
				pthread_mutex_lock(&client_lock[i]);
				if((sent = write(threads[i].connfd,err2,err2len))<0)
				{
					printf("Send to client failed. Client disconnected\n");
					break;
				}
				pthread_mutex_unlock(&client_lock[i]);
			}
			else
			{
				if(strncmp(pos,"LIST",4)==0)
				{
					pthread_mutex_lock(&client_lock[i]);
					if((sent = write(threads[i].connfd,msg,msglen))<0)
					{
						printf("Send to client failed. Client disconnected\n");
						break;
					}
					pthread_mutex_unlock(&client_lock[i]);
					
					pthread_mutex_lock(&list_lock);
					for(j=0;j<N;j++)
					{
						pos = threads[j].cliname;
						
						if(pos!=NULL)
						{
							printf("%s\n",pos);
							pthread_mutex_lock(&client_lock[i]);
							if((sent = write(threads[i].connfd,pos,strlen(pos)))<0)
							{
								printf("Send to client failed. Client disconnected\n");
								break;
							}
							/*
							if((sent = write(threads[i].connfd,"\n",strlen(tmp)))<0)
							{
								printf("Send to client failed. Client disconnected\n");
								break;
							}
							*/
							pthread_mutex_unlock(&client_lock[i]);
						}
					}
					pthread_mutex_unlock(&list_lock);
				}
				else if(strncmp(pos,"BMSG",4)==0)
				{
					pos = strtok_r(NULL," ",&saveptr);
					printf("%s\n",pos);
					printf("Broadcasting message to all online users\n");
					pthread_mutex_lock(&list_lock);
					for(j=0;j<N;j++)
					{
						if(threads[j].cliname!=NULL)
						{
							pthread_mutex_lock(&client_lock[j]);
							if((sent = write(threads[j].connfd,pos,strlen(pos)))<0)
							{
								printf("Send to client failed. Client disconnected\n");
								break;
							}
							pthread_mutex_unlock(&client_lock[j]);
							
						}
					}
					pthread_mutex_unlock(&list_lock);
				}
				else if(strncmp(pos,"UMSG",4)==0)
				{
					int found = 0;
					pos = strtok_r(NULL," ",&saveptr);
					//printf("%s\n",pos);
					//int l = strlen(pos);
					
					pthread_mutex_lock(&list_lock);
					for(j=0;j<N;j++)
					{
					printf("%d ",j);
						if(strcmp(pos,threads[j].cliname)==0)
						{
							pos = strtok_r(NULL," ",&saveptr);
							//printf("%s\n",pos);
							pthread_mutex_lock(&client_lock[j]);
							if((sent = write(threads[j].connfd,pos,strlen(pos)))<0)
							{
								printf("Send to client failed. Client disconnected\n");
								break;
							}
							pthread_mutex_unlock(&client_lock[j]);
							found = 1;
							break;
						}
					}
					pthread_mutex_unlock(&list_lock);
					
					if(found == 0)
					{
						printf("not found\n");
						pthread_mutex_lock(&client_lock[i]);
						if((sent = write(threads[i].connfd,err3,err4len))<0)
						{
							printf("Send to client failed. Client disconnected\n");
							break;
						}
						pthread_mutex_unlock(&client_lock[i]);
					}
				}
				else if(strncmp(pos,"LEAVE",5)==0)
				{
					break;
				}
				else
				{
					pthread_mutex_lock(&client_lock[i]);
					if((sent = write(threads[i].connfd,err4,err4len))<0)
					{
						printf("Send to client failed. Client disconnected\n");
						break;
					}
					pthread_mutex_unlock(&client_lock[i]);
				}
			}
		}


		threads[i].clients_handled++;
		free(threads[i].cliname);
		close(threads[i].connfd);
	}
}
