/**
 * @file mypopen.c
 * Betriebssysteme mypopen File.
 * Beispiel 2
 *
 * @author Dominik Himmler <ic16b071@technikum-wien.at>
 * @author Alexander Österreicher <ic16b011@technikum-wien.at>
 * @author David Sattler <ic16b036@technikum-wien.at>
 * @date 2017/05/01
 *
 * @version 1
 *
 *
 */
 
/*
 * -------------------------------------------------------------- includes --
 */

#include "mypopen.h"

#define WRITE 1
#define READ 0
/*
 * ------------------------------------------------------------- functions --
 */
 int close_pipe(int* pipefd, int type);
 void child_proc(int* pipefd, int type, const char* command);
 FILE *mypopen(const char *command, const char *type);
 int mypclose(FILE *stream);
 
 /**
 *
 * \brief close_pipe of mypopen
 *
 * closes the correct pipe end
 *
 * \param pipefd the pipe with both ends
 * \param type describes the ends of the pipe
 *
 * \return 0 on success, 1 on error
 *
 */

int close_pipe(int* pipefd, int type)
{
	/** close write fd, because the parent process needs to write something and the child needs to read it */
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
/**
 *
 * \brief child_proc of mypopen
 *
 * associate fd with stdin or stout and execute command in child process
 *
 * \param pipefd the pipe with both ends
 * \param type describes either the read or the write end of the pipe
 *	\param command the command which should be executed by the child process
 *
 * \return void
 *
 */

void child_proc(int* pipefd, int type, const char* command)
{
	/** associate fd with stdin, stdout (STDIN_FILENO = 0, STDOUT_FILENO = 1) */
	if (dup2(pipefd[type], type) == -1)
	{
		/** close kann ohne Fehlerbehandlung aufgerufen werden, im Fehlerfall wird ebenfalls exit(EXIT_FAILURE) aufgerufen */
		close(pipefd[type]);
		exit(EXIT_FAILURE);
	}

	close_pipe(pipefd, type);
	
	/** execute command */
	if (execl("/bin/sh", "sh", "-c", (const char *)command, (char *) NULL) == -1)
	{
		/** close kann ohne Fehlerbehandlung aufgerufen werden, im Fehlerfall wird ebenfalls exit(EXIT_FAILURE) aufgerufen */
		close(pipefd[type]);
		exit(EXIT_FAILURE);
	}
}

/**
 *
 * \brief mypopen of mypopen
 *
 * check if child-process already exists, forks child process and reads or write with parent process
 *
 * \param command is the command which should be executed by the child process
 * \param type describes one end (r or w) of the pipe
 * \var fd_result is the original FILE stream of mypopen and a global variable first initialized on NULL
 * \return FILE Stream on success, NULL on error
 *
 */
FILE * mypopen(const char *command, const char *type)
{	
	int pipefd[2];
	
	/** check if child-process already exists */
	if (fd_result != NULL)
	{
		errno = EAGAIN;
		return NULL;
	}

	/** check correct use of type */
	if (strcmp(type, "w") != 0 && strcmp(type, "r") != 0)
	{
		errno = EINVAL;
		return NULL;
	}

	/** create pipe */
	if (pipe(pipefd) == -1)
	{
		return NULL;
	}

	/** fork child process */
	child_pid = fork();
	if (child_pid == -1)
	{	
		close_pipe(pipefd, READ);
		close_pipe(pipefd, WRITE);
		return NULL;
	}

	/** Child Process */
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
		/** Elternprozess schreibt auf die Pipe */
		if (*type == 'w')
		{
			/** close read fd, because it is not used */
			if (close(pipefd[READ]) == -1)
			{
				return NULL;	
			}
			
			/** return filestream */
			if((fd_result = fdopen(pipefd[1], type)) == NULL)
			{
				close(pipefd[1]);
			}
			
			return fd_result;
		}

		/** Elternprozess liest von der Pipe */
		else
		{
			/** close write fd, because it is not used */
			if (close(pipefd[WRITE]) == -1)
			{
				return NULL;	
			}
			
			/** return filestream */
			if((fd_result = fdopen(pipefd[0], type)) == NULL)
			{
				close(pipefd[0]);
			}
			
			return fd_result;
		}
	}
}

/**
 *
 * \brief mypclose of mypopen
 *
 * close the FILE Stream, wait for the child process to terminate
 *
 * \param stream the FIlE Stream that should be terminated
 * \var fd_result is the original FILE stream of mypopen and a global variable
 *
 * \return status on success, -1 on error
 *
 */
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
			
			/** in case of error reset the child_pid & fd_result and return -1 for error */
			child_pid = -1;
			fd_result = NULL;
			
			return -1;	
        }
	}
	
	/** reset the global variables before exiting */
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