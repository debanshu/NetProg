/* IS C462
 * NETWORK PROGRAMMING
 * Lab 4 - Assignment
 *
 * Debanshu Sinha
 */



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#define MSGQ_PATH "./lab4.c"    

typedef struct my_msgbuf
{
  long mtype;
  char mtext[200];
}message;

void sendmessage(message buf,char q);
message recvmessage(char q);

int main()
{
	//starting child process creation
	if(!fork())
	{
		//PROCESS C2  
        message buf;
        buf = recvmessage('2');        
        strcat(buf.mtext,"C2");
        
        //send message
        sendmessage(buf,'3');
        sendmessage(buf,'4');
         

		if(!fork())
		{  
			//PROCESS C3
            message buf;
            message buf2 = recvmessage('3'); 
            buf.mtype = buf2.mtype;
            buf.mtext[0] = '\0';
            strcat(buf.mtext,"C3");
            strcat(buf.mtext,buf2.mtext);
            
            //send message
            sendmessage(buf,'5');
            sendmessage(buf,'6');
            sendmessage(buf,'7');

			if(!fork())
            {
                //PROCESS C5
                message buf;
                buf = recvmessage('5');
                buf.mtext[4] = toupper(buf.mtext[4]);
            
                printf("Output from C5: %s\n",buf.mtext); 
            }
			else if(!fork())
			{
                //PROCESS C6
                message buf;
                buf = recvmessage('6');
                buf.mtext[5] = toupper(buf.mtext[5]);
            
                printf("Output from C6: %s\n",buf.mtext); 
            }
			else if(!fork())
			{
                //PROCESS C7
                message buf;
                buf = recvmessage('7');
                buf.mtext[6] = toupper(buf.mtext[6]);
            
                printf("Output from C7: %s\n",buf.mtext); 
            }
		}
		else if(!fork())
		{
            //PROCESS C4
            message buf;
            buf = recvmessage('4');
            buf.mtext[3] = toupper(buf.mtext[3]);
            
            printf("Output from C4: %s\n",buf.mtext);            
			
		}
	}
    else
    {
        //PROCESS C1
        message buf;
        
        printf("Enter Input String for Process C1: ");
        gets(buf.mtext);
        int l =strlen(buf.mtext);
        
        //convert all to lowercase
        int i;
        for(i=0;i<l;i++)
            buf.mtext[i] = tolower(buf.mtext[i]);
        
        sendmessage(buf,'2');
            
    }
    
	return 0;
}

void sendmessage(message buf,char q)
{
        int msqid;
        key_t key;
        
        //create message queue
        if ((key = ftok (MSGQ_PATH, q)) == -1)
        {
          perror ("ftok");
          exit (1);
        }

        if ((msqid = msgget (key, 0644 | IPC_CREAT)) == -1)
        {
          perror ("msgget");
          exit (1);
        }
        
        buf.mtype = 1;  
        if (msgsnd (msqid, &(buf.mtype), sizeof (buf), 0) == -1)
            perror ("msgsnd");
            
}

message recvmessage(char q)
{
        message buf;
        int msqid;
        key_t key;
        
        //create message queue
        if ((key = ftok (MSGQ_PATH, q)) == -1)
        {
          perror ("ftok");
          exit (1);
        }

        if ((msqid = msgget (key, 0644 | IPC_CREAT)) == -1)
        {
          perror ("msgget");
          exit (1);
        }
        
        if (msgrcv (msqid, &(buf.mtype), sizeof (buf), 0, 0) == -1)
        {
          perror ("msgrcv");
          exit (1);
        }
        
        //delete msg queue
        if (msgctl (msqid, IPC_RMID, NULL) == -1)
        {
          perror ("msgctl");
          exit (1);
        }
        
        return buf;
}
