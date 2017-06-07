#include <errno.h>
#include <error.h>
#include <limits.h>
#include <sem182.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

int main(int argc, const char* argv[])
{
	argc = argc;
	key_t key, key2;
	int semid, semid2, shmid, opt;
	int count = 0;
	int *shmptr;
	long long buffer = 0;
	unsigned long long tmpbuffer = 0;
	char *endptr;
	
	key = 1000 * getuid();
	key2 = 1000 * getuid() + 1;

	if(argc < 2 || argc > 3)
	{
		fprintf(stderr, "usage: sender -m <ring buffer size>\n");
		return EXIT_FAILURE;
	}
	while((opt = getopt(argc, (char* const*)argv, "m:")) != -1)
	{
		if(opt == 'm')
		{
			tmpbuffer = strtoull(optarg, &endptr, 10);
			if(strncmp(optarg, "-", 1) == 0 || tmpbuffer == 0)
			{
				fprintf(stderr, "%s: ring buffer size must be >0 and not %lli\n", argv[0], tmpbuffer);
				fprintf(stderr, "usage: sender -m <ring buffer size>\n");
				return EXIT_FAILURE;
			}
			else
			{
				if(*endptr != '\0' || argv[optind] != '\0')
				{
					fprintf(stderr, "usage: sender -m <ring buffer size>\n");
					return EXIT_FAILURE;
				}
				if(tmpbuffer > LLONG_MAX || (tmpbuffer * sizeof(int)) > SIZE_MAX)
				{
					fprintf(stderr, "%s: ring buffer size too big\n", argv[0]);
					fprintf(stderr, "usage: sender -m <ring buffer size>\n");
					return EXIT_FAILURE;
				}
				else
				{
					buffer = tmpbuffer;
				}
			}			
		}
		else
		{
			fprintf(stderr, "usage: sender [-h] -m <ring buffer size>\n");
			return EXIT_FAILURE;
		}
	}
	
	if((semid = semgrab(key)) == -1)
	{
		semid = seminit(key, 0660, 0);
		if(semid == -1)
		{
			printf("Fehler seminit()\n");
			return EXIT_FAILURE;
		}
	}
	
	if((semid2 = semgrab(key2)) == -1)
	{
		semid2 = seminit(key2, 0660, buffer);
		if(semid2 == -1)
		{
			printf("Fehler seminit2()\n");
			semrm(semid);
			return EXIT_FAILURE;
		}
	}

	errno = 0;
	if((shmid = shmget(key, (buffer * sizeof(int)), 0660|IPC_CREAT)) == -1)
	{
		if(errno != EEXIST)
		{
			printf("Fehler shmget()\n");
			semrm(semid);
			semrm(semid2);
			return EXIT_FAILURE;
		}
	}
	
	shmptr = shmat(shmid, NULL, 0);
	if(shmptr == (int *) -1)
	{
		printf("Fehler shmat()\n");
		semrm(semid);
		semrm(semid2);
		shmctl(shmid, IPC_RMID, NULL);
		return EXIT_FAILURE;
	}
	
	while(1)
	{
		
		errno = 0;
		while((P(semid) == -1 ) && (errno == EINTR))
		{
			errno = 0;			
		}
		if(shmptr[count] == EOF) break;
		fputc(shmptr[count], stdout);
		
		count = (count+1)%buffer;
		V(semid2);
	}
	//nur der empfaenger entfernt den Memory und die Semaphoren.
	semrm(semid);
	semrm(semid2);
	shmctl(shmid, IPC_RMID, NULL);
	
	return EXIT_SUCCESS;
}