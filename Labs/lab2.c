/* IS C462
 * NETWORK PROGRAMMING
 * Lab 2 - Assignment
 *
 * Debanshu Sinha
 */


#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//datastructure to store executed commandlist
typedef struct node{
	char* name;
	struct node* next;
	int status;
}NODE;

typedef NODE* process;
process commands = NULL,current;
pid_t retval=0;

//custom program prompt
char* prompt = "#: ";

void pr_print(process);
void pr_exit(int);

//handler for SIGINT
void sig_printlist(int signo)
{
	pr_print(commands);
}

//handler for SIGQUIT
void sig_killcurrent(int signo)
{
	if(retval == 0)
	{	
		//printf("No current command running\n");
	}
	else
	{
		if( kill(retval,SIGKILL)!=0)
		{
			printf("Could not kill current command. Either command has exited or No command running\n");
		}
	}
}

//print executed command list
void pr_print(process list)
{
	printf("\nPrinting all Executed Commands and their Exit Status'\n\n");
	process curr=list;
	while(curr!=NULL)
	{
		printf("\nName: %s\n",curr->name);
		pr_exit(curr->status);
		printf("\n");
		curr = curr->next;
		//can't print prompt after signal interrupt
		//if( curr == NULL)
		//printf("#: ");
	}

}


//print status description
void pr_exit(int status)
{
  if (WIFEXITED (status))
      printf ("normal termination, exit status = %d\n", WEXITSTATUS (status));
  else if (WIFSIGNALED (status))
      printf ("abnormal termination, signal number = %d \n", WTERMSIG (status));
  else if (WIFSTOPPED (status))
      printf ("child stopped, signal number = %d\n", WSTOPSIG (status));
}


int main(int argc,char *argv[])
{
	current = commands;


	//variables
	char line[100];
	line[0]='\0';
	char* cmd[20];

	char* delim = " ";
	int i,status;

	//signal handlers 
	signal(SIGINT, sig_printlist);
	signal(SIGQUIT, sig_killcurrent);

	printf("Starting program, Enter commands at the prompt '%s' ,Enter 'exit' to end\n\n",prompt); 
	//running program in infinite loop
		printf("%s",prompt);
	while(1)
	{
		//scanf("%[^\n]s",line);
		gets(line);
		i=0;
		if(strcmp(line,"exit")==0)
			break;

		if( current == NULL)
		{
			current = (process)malloc(sizeof(NODE));
		}
		else
		{
			current->next = (process)malloc(sizeof(NODE));
			current = current->next;
		}	       
		if( commands == NULL) commands = current;
		current->name = strdup(line);
		current->next = NULL;

		//getting prompt input
		cmd[i] = strtok(line,delim);
		while(cmd[i]!=NULL)
		{
			i++;
			cmd[i]=strtok(NULL,delim);
		}

		if( (retval = fork()) <0 )
			perror("Fork Error");
		else if( retval == 0)
		{
			execvp(cmd[0],cmd);
			perror("Error in Execvp");
		}
		else if( retval > 0)
		{
			//cleaning up
			//scanf("\n");

			waitpid(retval,&status,0);
			current->status = status;
			
  			if (WIFSIGNALED (status))
      				printf ("abnormal termination, signal number = %d \n", WTERMSIG (status));
			//fflush(stdin);
			printf("%s",prompt);
		}
	}

	return 0;
}
