/*  Version 1:
		rudimentäre Funkion print().
		Einschränkungen: argv[1] muss Directory sein
						argv[2] muss Dateiname sein
						Gibt nur etwas aus, wenn Dateiname im Directory vorhanden ist
*/

#include <stdio.h>
#include <string.h>
#include <dirent.h>

void print(const char* argv[]);

int main(int argc, const char* argv[])
{
	int i;

	for(i = 0; i < argc; i++)
	{
		printf("eingabe: %s\n",argv[i]);
		if(!strcmp(argv[i], "-print"))
			print(argv);
	}

	return 0;
}

void print(const char* argv[])
{
	DIR *hdir;
	struct dirent *entry;

	hdir = opendir(argv[1]);

	do
	{
		entry = readdir(hdir);

		if(entry && !strcmp(entry->d_name, argv[2]))
		{
			printf("%s\n", entry->d_name);
		}
	}while(entry);

	closedir(hdir);
}
