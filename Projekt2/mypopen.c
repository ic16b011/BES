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
	{
		return NULL;
	}

	/* Child Process */
	else if (cpid == 0)
	{
		if (*type == 'w')
		{
			/* close read fd, because it is not used */
			if (close(pipefd[0]) == -1)
			{
				exit(EXIT_FAILURE);
			}

			/* associate fd with stdout */
			if (dup2(STDOUT_FILENO, pipefd[1]) == -1)
			{
				exit(EXIT_FAILURE);
			}

			/* execute command */
			if (execv("/bin/sh", (char* const*)command) == -1)
			{
				exit(EXIT_FAILURE);
			}

			if (close(pipefd[1]) == -1)
			{
				exit(EXIT_FAILURE);
			}
			/* return filestream */
			_exit(EXIT_SUCCESS);
		}

		else
		{
			/* close write fd, because it is not used */
			if (close(pipefd[1]) == -1)
			{
				exit(EXIT_FAILURE);
			}

			/* associate fd with stdout */
			if (dup2(STDIN_FILENO, pipefd[0]) == -1)
			{
				exit(EXIT_FAILURE);
			}

			/* execute command */
			if (execv("/bin/sh", (char* const*)command) == -1)
			{
				exit(EXIT_FAILURE);
			}

			if (close(pipefd[0]) == -1)
			{
				exit(EXIT_FAILURE);
			}
			/* return filestream */
			_exit(EXIT_SUCCESS);
		}
	}
	else
	{
		/* Elternprozess liest von Pipe */
		if (*type == 'w')
		{
			/* close write fd, because it is not used */
			if (close(pipefd[1]) == -1)
			{
				return NULL;
			}
			/* return filestream */
			return fdopen(pipefd[0], "r");
		}

		/* Elternprozess schreibt auf Pipe */
		else
		{
			/* close read fd, because it is not used */
			if (close(pipefd[0]) == -1)
			{
				return NULL;
			}
			/* return filestream */
			return fdopen(pipefd[1], "w");
		}
	}
}

int mypclose(FILE *stream)
{
	int status;

	/* check if child-process exists */
	if (waitpid(0, &status, WNOHANG) == -1)
	{
		errno = ECHILD;
		return -1;
	}
	
	/* ungültiger File Stream */
	if (stream == NULL || fgetc(stream) == '\0')
	{
		errno = EINVAL;
		return -1;
	}

	/* wait for child-process with same group id to terminate */
	do
	{
		if (waitpid(0, &status, WNOHANG) == -1)
		{
			return -1;
		}
	} while (WIFEXITED(status) == 0);

	/* close filestream */
	if (fclose(stream) != 0)
	{
		return NULL;
	}

	return status;
}