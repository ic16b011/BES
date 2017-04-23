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

//global var
static pid_t child_pid = -1;
static FILE *fd_result = NULL;

extern FILE *mypopen(const char *command, const char *type);
extern int mypclose(FILE *stream);
#endif