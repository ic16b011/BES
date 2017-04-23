#include "mypopen.h"

FILE *mypopen(const char *command, const char *type)
{	
	int pipefd[2];
	
	/* check if child-process already exists */
	if (fd_result != NULL)
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
	child_pid = fork();
	if (child_pid == -1)
	{	//notwendig für den Testfall 11
		close(pipefd[0]);
		close(pipefd[1]);
		return NULL;
	}

	/* Child Process */
	else if (child_pid == 0)
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
				close(pipefd[0]);
				exit(EXIT_FAILURE);
			}

			if (close(pipefd[0]) == -1)
			{
				exit(EXIT_FAILURE);
			}
			
			/* execute command */
			if (execl("/bin/sh", "sh", "-c", (const char *)command, (char *) NULL) == -1)
			{
				close(pipefd[0]);
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
				close(pipefd[1]);
				exit(EXIT_FAILURE);
			}
			
			if (dup2(pipefd[1], STDERR_FILENO) == -1)
			{
				close(pipefd[1]);
				exit(EXIT_FAILURE);
			}

			if (close(pipefd[1]) == -1)
			{
				exit(EXIT_FAILURE);
			}
			
			/* execute command */
			if (execl("/bin/sh", "sh", "-c",(const char *)command, (char *) NULL) == -1)
			{
				close(pipefd[1]);
				exit(EXIT_FAILURE);
			}

			/* return filestream */
			exit(EXIT_SUCCESS);
		}
	}
	else
	{
		/* Elternprozess schreibt auf die Pipe */
		if (*type == 'w')
		{
			/* close read fd, because it is not used */
			if (close(pipefd[0]) == -1)
			{
				return NULL;
			}
			/* return filestream */
			if((fd_result = fdopen(pipefd[1], type)) == NULL)
			{
				close(pipefd[1]);
				return NULL;
			}
			else return fd_result;
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
			if((fd_result = fdopen(pipefd[0], type)) == NULL)
			{
				close(pipefd[0]);
				return NULL;
			}
			else return fd_result;
		}
	}
}

int mypclose(FILE *stream)
{
	int status;
	pid_t w = 0;
	
	
	if(fd_result == NULL) 
	{
		errno = ECHILD;		
		return -1;
	}
		
		
	if (fd_result != stream)
	{
		errno = EINVAL;
		return -1;
	}
	
	if (fclose(stream) == EOF)
	{
		child_pid = -1;
		fd_result = NULL;
		return -1;
	}
	
	w = waitpid(child_pid, &status, 0);
	
	while(w != child_pid)
	{
		if (w == -1) 
		{
			if(errno == EINTR)
			{
				continue;
			}
			
			/* in case of error reset the child_pid & fd_result and return -1 for error */
			child_pid = -1;
			fd_result = NULL;
			
			return -1;	
        }
	}
	
	/* reset the global variables before exiting */
	child_pid = -1;
	fd_result = NULL;

	
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