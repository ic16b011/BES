/*  Version 1:
		rudimentäre Funkion print().
		Einschränkungen: argv[1] muss Directory sein
						argv[2] muss Dateiname sein
						Gibt nur etwas aus, wenn Dateiname im Directory vorhanden ist
*/

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

void print(const char* argv[]);
void do_dir(const char * dir_name, const char * const * parms);
void do_file(const char * file_name, const char* argv[]);

int main(int argc, const char* argv[])
{
	int i, j;

	const char* parms;
	for(i = 0; i < argc; i++)
	{
		if(!strncmp(argv[i], "-", 1))
		{
			parms = &argv[i];
			break;
		}
	}
	
	j=1;
	while(j<i)
	{
		do_dir(argv[j], parms);
		j++;
	}

	return 0;
}

void do_dir(const char * dir_name, const char * const * parms)
{
	DIR *directory;
	struct dirent *entry;
	
	directory = opendir(dir_name);
	//printf("%s\n", directory);
	printf("%s\n%s\n", *parms, *(parms+1));
	do
	{

		entry = readdir(directory);

		if(entry && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
		{
			//printf("%s/%s\n", dir_name, entry->d_name);
			struct stat buffer;
			stat(entry->d_name, &buffer);
			//printf("\nInode: %lu\n", buffer.st_ino);

			do_file(entry->d_name, parms);
			if(entry->d_type == DT_DIR)
			{
				//printf("Directory found!\n");
				do_dir(entry->d_name, parms);
			}
		}
	}while(entry);

	closedir(directory);
}

void do_file(const char * file_name, const char* argv[])
{
	struct stat buffer;
	int status = stat(file_name, &buffer);
	//printf("\nInode: %lu\n", buffer.st_ino);
}

void print(const char* argv[])
{
	
}
