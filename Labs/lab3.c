/* IS C462
 * NETWORK PROGRAMMING
 * Lab 3 - Assignment
 *
 * Debanshu Sinha
 */

/* NOTE: bsd banner is implemented as printerbanner in linux.
 * 	This file has printerbanner exec command commented out, with just banner command executing.
 * 	Please change to printerbanner if banner command unavailable
 *	
 *	SystemV banner clone package, sysvbanner does not work like bsd banner since it does take stdin input
*/

#include <sys/types.h>
#include <unistd.h>

int main()
{
	int i;
	int p[2];
	pid_t ret;
	pipe (p); 
	ret = fork ();
	if (ret == 0)
	{
		int p2[2];
		pipe(p2);
		ret = fork();

		if(ret == 0)
		{
			int p3[2];
			pipe(p3);
			ret = fork();

			if(ret == 0)
			{
				close(1);
				dup(p3[1]);
				close(p3[0]);
				execlp ("ps", "ps", (char *) 0);
			}
			else
			{
				close(0);
				dup(p3[0]);
				close(p3[1]);
				wait(NULL);
		
				close (1);
				dup (p2[1]);
				close (p2[0]);
				
				execlp ("wc", "wc","-l", (char *) 0);
			}

		}
		else
		{
			close(0);
			dup(p2[0]);
			close(p2[1]);
			wait(NULL);
		
			close (1);
			dup (p[1]);
			close (p[0]);

			// bsd implementation of banner in debian is via printerbanner command in the bsdmainutils package
			// system V banner clone package, called sysvbanner does not take stdin input
			// execlp ("printerbanner", "printerbanner", (char *) 0);
			execlp ("banner", "banner", (char *) 0);
		}
	}
	else
	{
		close(0);
		dup(p[0]);
		close(p[1]);
		wait(NULL);
		
		execlp ("wc", "wc", (char *) 0);
	}
	return 0;
} 
