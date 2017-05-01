/**
 * @file mypopen.h
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
 
 /**
 * -------------------------------------------------------------- includes --
 */

#ifndef MYPOPEN_H
#define MYPOPEN_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
* \var child_pid is the global child pid initialized on minus one
* \var fd_result is the original FILE stream of mypopen which will be returned on success
*/
static pid_t child_pid = -1;
static FILE *fd_result = NULL;

extern FILE *mypopen(const char *command, const char *type);
extern int mypclose(FILE *stream);
#endif