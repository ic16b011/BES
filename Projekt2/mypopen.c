#include "mypopen.h"

#define WRITE 1
#define READ 0

int close_pipe(int* pipefd, int type)
{
	/* close write fd, because the parent process needs to write something and the child needs to read it */
	if (close(pipefd[type]) == -1)
	{
		exit(EXIT_FAILURE);
	}
	
	if(type == 1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void child_proc(int* pipefd, int type, const char* command)
{
	/* associate fd with stdin, stdout (STDIN_FILENO = 0, STDOUT_FILENO = 1) */
	if (dup2(pipefd[type], type) == -1)
	{
		/* close kann ohne Fehlerbehandlung aufgerufen werden, im Fehlerfall wird ebenfalls exit(EXIT_FAILURE) aufgerufen */
		close(pipefd[type]);
		exit(EXIT_FAILURE);
	}

	close_pipe(pipefd, type);
	
	/* execute command */
	if (execl("/bin/sh", "sh", "-c", (const char *)command, (char *) NULL) == -1)
	{
		/* close kann ohne Fehlerbehandlung aufgerufen werden, im Fehlerfall wird ebenfalls exit(EXIT_FAILURE) aufgerufen */
		close(pipefd[type]);
		exit(EXIT_FAILURE);
	}
}

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
	{	
		close_pipe(pipefd, READ);
		close_pipe(pipefd, WRITE);
		return NULL;
	}

	/* Child Process */
	else if (child_pid == 0)
	{
		if (*type == 'w')
		{
			close_pipe(pipefd, WRITE);
			child_proc(pipefd, READ, command);
		}
		else
		{
			close_pipe(pipefd, READ);
			child_proc(pipefd, WRITE, command);
		}
		
		exit(EXIT_SUCCESS);
	}
	else
	{
		/* Elternprozess schreibt auf die Pipe */
		if (*type == 'w')
		{
			/* close read fd, because it is not used */
			close_pipe(pipefd, READ);
			
			/* return filestream */
			if((fd_result = fdopen(pipefd[1], type)) == NULL)
			{
				close(pipefd[1]);
			}
			
			return fd_result;
		}

		/* Elternprozess liest von der Pipe */
		else
		{
			/* close write fd, because it is not used */
			close_pipe(pipefd, WRITE);
			
			/* return filestream */
			if((fd_result = fdopen(pipefd[0], type)) == NULL)
			{
				close(pipefd[0]);
			}
			
			return fd_result;
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