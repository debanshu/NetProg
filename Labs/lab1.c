/* IS C462
 * NETWORK PROGRAMMING
 * Lab 1 - Assignment
 

/* Note: Program has been tested in cygwin */

#include <unistd.h>
#include <stdio.h>

int main()
{
	printf("Text\tpid\tppid\n");
	printf("----\t---\t----\n");
	printf("%s\t%d\t%d\n","This",getpid(),getppid());

	//starting child process creation
	if(!fork())
	{
		printf("%s\t%d\t%d\n","is",getpid(),getppid());

		if(!fork())
		{  
			printf("%s\t%d\t%d\n","1st",getpid(),getppid());

			if(!fork())
				 printf("%s\t%d\t%d\n","of",getpid(),getppid());
			else if(!fork())
				 printf("%s\t%d\t%d\n","net",getpid(),getppid());
			else if(!fork())
				 printf("%s\t%d\t%d\n","prog",getpid(),getppid());
		}
		else if(!fork())
		{
			 printf("%s\t%d\t%d\n","Lab",getpid(),getppid());
		}
	}
	return 0;
}
