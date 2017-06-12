#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>
#include <error.h>
#include <limits.h>
#include <sem182.h>
#include <stdint.h>

#define KEY (1000 * getuid())
#define KEY2 (1000 * getuid() + 1)
