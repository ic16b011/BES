#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mypopen.h"
//global var
static pid_t child_pid = -1;

FILE *mypopen(const char *command, const char *type)
{	
	int pipefd[2];
	pid_t cpid;
	int status;
	
	/* check if child-process already exists */
	if (waitpid(0, &status, WNOHANG) != -1)
	{
		errno = EAGAIN;
		return NULL;
	}

	/* check correct use of type */
	if (strcmp(type, "w") != 0 && strcmp(type, "r") != 0)
	{
		errno = EINVAL;
		return NULL;
	}

	/* create pipe */
	if (pipe(pipefd) == -1)
	{
		return NULL;
	}

	/* fork child process */
	cpid = fork();
	if (cpid == -1)
	{	//notwendig für den Testfall 11
		close(pipefd[0]);
		close(pipefd[1]);
		return NULL;
	}

	/* Child Process */
	else if (cpid == 0)
	{
		if (*type == 'w')
		{
			/* close write fd, because the parent process needs to write something and the child needs to read it */
			if (close(pipefd[1]) == -1)
			{
				exit(EXIT_FAILURE);
			}

			/* associate fd with stin */
			if (dup2(pipefd[0], STDIN_FILENO) == -1)
			{
				exit(EXIT_FAILURE);
			}

			/* execute command */
			if (execl("/bin/sh", "sh", "-c", (const char *)command, (char *) NULL) == -1)
			{
				exit(EXIT_FAILURE);
			}

			if (close(pipefd[0]) == -1)
			{
				exit(EXIT_FAILURE);
			}
			/* return filestream */
			exit(EXIT_SUCCESS);
		}

		else
		{
			/* close read fd, because parent process wants to read the things the child process writes */
			if (close(pipefd[0]) == -1)
			{
				exit(EXIT_FAILURE);
			}

			/* associate fd with stdout */
			if (dup2(pipefd[1], STDOUT_FILENO) == -1)
			{
				exit(EXIT_FAILURE);
			}

			/* execute command */
			if (execl("/bin/sh", "sh", "-c",(const char *)command, (char *) NULL) == -1)
			{
				exit(EXIT_FAILURE);
			}

			if (close(pipefd[1]) == -1)
			{
				exit(EXIT_FAILURE);
			}
			/* return filestream */
			exit(EXIT_SUCCESS);
		}
	}
	else
	{
		child_pid = cpid;
		/* Elternprozess schreibt auf die Pipe */
		if (*type == 'w')
		{
			/* close read fd, because it is not used */
			if (close(pipefd[0]) == -1)
			{
				return NULL;
			}
			/* return filestream */
			return fdopen(pipefd[1], "w");
		}

		/* Elternprozess liest von der Pipe */
		else
		{
			/* close write fd, because it is not used */
			if (close(pipefd[1]) == -1)
			{
				return NULL;
			}
			/* return filestream */
			return fdopen(pipefd[0], "r");
		}
	}
}

int mypclose(FILE *stream)
{
	int status;
	pid_t w = 0;
	if(child_pid == -1) 
		{
		errno = ECHILD;		
		return -1;
		}
	if (stream == NULL || fgetc(stream) == '\0')
	{
		errno = EINVAL;
		return -1;
	}
	
	if (fclose(stream) != 0)
	{
		return -1;
	}
	
	while(w != child_pid){
		w = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
    if (w == -1) {
		printf("fehler bei waitpid im mypclose\n");
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
	}
	
	if (WIFEXITED(status)) 
	{
		status = WEXITSTATUS(status);
	}
	else
	{
		errno = ECHILD;
		return -1;
	}
	
	return status;
}