#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mypopen.h"

static int nrOpenPipe = 0;

FILE *mypopen(const char *command, const char *type)
{
	int pipefd[2];
	//int status;
	pid_t cpid;//, waitret;
	
	/*waitret = waitpid(0, &status, WNOHANG);
	if(waitret == -1 || errno == ECHILD)
	{
		return NULL;
	}
	else if(waitret != -1)
	{
		errno = EAGAIN;
		return NULL;
	}*/
		
	/* check correct use of type */
	if(*type != 'w' && *type != 'r')
	{
		errno = EINVAL;
		return NULL;
	}
	
	/* create pipe */
	if(pipe(pipefd) == -1)
	{
		return NULL;
	}
	nrOpenPipe = 1;
	
	/* fork child process */
	cpid = fork();
	if(cpid == -1)
	{
		return NULL;
	}
	
	/* Child Process */
	else if(cpid == 0)
	{
		if(*type == 'w')
		{
			/* close read fd, because it is not used */
			if(close(pipefd[0]) == -1)
			{
				return NULL;
			}
			
			/* associate fd with stdout */
			if(dup2(STDOUT_FILENO, pipefd[1]) == -1)
			{
				return NULL;
			}
			
			/* execute command */
			if (execl("/bin/sh", &command[0], (char*)NULL) == -1)
			{
				return NULL;
			}
			
			/* return filestream */
			return fdopen(pipefd[1], type);
		}

		else
		{
			/* close write fd, because it is not used */
			if(close(pipefd[1]) == -1)
			{
				return NULL;
			}
			
			/* associate fd with stdout */
			if(dup2(STDIN_FILENO, pipefd[0]) == -1)
			{
				return NULL;
			}
			
			/* execute command */
			if (execl("/bin/sh", &command[0], (char*)NULL) == -1)
			{
				return NULL;
			}
			
			/* return filestream */
			return fdopen(pipefd[0], type);
		}
	}
	else
	{
		return NULL;
	}
}

int mypclose(FILE *stream)
{
	int status;
	
	/* wait for child-process with same group id to terminate */
	do
	{
		if(waitpid(0, &status, WNOHANG) == -1)
		{
			return -1;
		}
		
	}while(WIFEXITED(status) == 0 && WIFSIGNALED(status) == 0);
	
	nrOpenPipe = 0;
	
	/* close filestream */
	if(fclose(stream) != 0)
	{
		return NULL;
	}
	
	return status;
}