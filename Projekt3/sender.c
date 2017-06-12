#include <stdio.h>
#include <stdlib.h>
#include "sem.h"

int main(int argc, const char* argv[])
{
	argc = argc;
	int semid, semid2, shmid, opt;
	int ch, count = 0;
	int *shmptr;
	long long buffer = 0;
	unsigned long long tmpbuffer = 0;
	char* endptr;
	

	//wegen "-m30" und "-m 30"?
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
				fprintf(stderr, "%s: ring buffer size must be > 0 and not %lli\n", argv[0], tmpbuffer);
				fprintf(stderr, "usage: sender -m <ring buffer size>\n");
				return EXIT_FAILURE;
			}
			else
			{
				if(*endptr != '\0' || argv[optind] != '\0')
				{
					fprintf(stderr, "usage: sender -m <ring buffer size>\n");
					return EXIT_FAILURE;
				}//ist das * sizeof(int) wirklich notwendig?
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
	
	if((semid = semgrab(KEY)) == -1)
	{
		semid = seminit(KEY, 0660, 0);
		if(semid == -1)
		{
			printf("Fehler seminit()\n");
			return EXIT_FAILURE;
		}
	}
	
	if((semid2 = semgrab(KEY2)) == -1)
	{
		semid2 = seminit(KEY2, 0660, buffer);
		if(semid2 == -1)
		{
			printf("Fehler seminit2()\n");
			semrm(semid);
			return EXIT_FAILURE;
		}
	}

	errno = 0;
	//hier habe ich die Fehlermeldung (IPC_EXCL) absichtlich hinaus genommen, damit es egal ist ob empfänger oder sender zuerst aufgerufen wird.
	if((shmid = shmget(KEY, (buffer * sizeof(int)), 0660|IPC_CREAT)) == -1)
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
		if((ch = fgetc(stdin)) == EOF) break;
		//critical region
		errno = 0;
		while((P(semid2) == -1 ) && (errno == EINTR))
		{
			errno = 0;			
		}
		
		shmptr[count] = ch;
		count = (count + 1)%buffer;
		V(semid);
	}
	//senden von EOF
	P(semid2);
	shmptr[count] = ch;
	V(semid);
		
	printf("\n");
	return EXIT_SUCCESS;
}
