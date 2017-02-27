#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>
#include <stdlib.h>
#include <pwd.h>

void do_dir(const char* dir_name, const char* parms[]);
void do_file(const char* file_name, const char* parms[]);

int main(int argc, const char* argv[])
{
	int i, j;

	const char** parms;

	if(argc == 1) printf("Please use some parameters\n");

	for(i = 0; i < argc; i++)
	{
		if(!strncmp(argv[i], "-", 1))
		{
			parms = &argv[i];
			break;
		}
	}

	/*int z=0;
	while(parms[z]!='\0')
	{
		printf("argv[%d]: %s\n", z, parms[z]);
		z++;
	}*/

	j=1;
	while(j<i)
	{
		do_dir(argv[j], parms);
		j++;
	}

	return 0;
}

void do_dir(const char* dir_name, const char* parms[])
{
	DIR *directory;
	struct dirent *entry;
	struct stat buffer;
	int status;
	char localname[100];
	localname[0] = '\0';
	int len;
	
	/* Öffne Directory */
	directory = opendir(dir_name);
	if(!directory)
	{
		printf("find: `%s': No such file or directory\n", dir_name);
		return;
	}

	printf("%s\n", dir_name);
	
	do
	{
		/* Lese Directory Einträge */
		entry = readdir(directory);

		if(entry && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
		{
			/* Erzeuge Fullpath */
			strcpy(localname, dir_name);
			len = strlen(localname);
			if(localname[len-1] != '/')
				strcat(localname, "/");
			
			strcat(localname, entry->d_name);

			/* Wenn Directory, rufe do_dir rekursiv auf */
			if(entry->d_type == DT_DIR)
			{
				do_dir(localname, parms);
			}
			
			/* Wenn Regular File, rufe do_file auf */
			else if(entry->d_type == DT_REG)
			{
				do_file(localname, parms);
			}
			
			/* Typ unbekannt, versuche Typ über stat() herauszufinden */
			else if(entry->d_type == DT_UNKNOWN)
			{
				status = stat(localname, &buffer);
				
				if(status != 0 && S_ISDIR(buffer.st_mode))
				{
					do_dir(localname, parms);
				}
				else if(status != 0 && S_ISREG(buffer.st_mode))
				{
					do_file(localname, parms);
				}
				else
				{
					printf("Cannot read Entry!\n");
					return;
				}
			}
		}
	}while(entry);

	/* Schließe Directory */
	closedir(directory);
}

void do_file(const char* file_name, const char* parms[])
{


	struct stat buffer;
	int status = stat(file_name, &buffer);
	/*printf("2: %s\n", file_name);
	printf("Inode2: %lu\n\n", buffer.st_ino);*/

	int count = 0;

	while(parms[count] != '\0')
	{

	if (!(strcmp(parms[count], "-print"))) printf("%s\n", file_name);

    else if (!(strcmp(parms[count], "-name")))
    {

        if (!(strcmp(basename(file_name), parms[count+1]))) printf("%s\n", file_name);
    }

    else if (!(strcmp(parms[count], "-user")))
    {


        if (buffer.st_uid == atoi(parms[count+1])) printf("%s\n", file_name);

        struct passwd *p;

        if((p = getpwnam(parms[count+1])) == NULL) perror("User nicht vorhanden");//hier sollte man eine einmalige Fehlermeldung anzeigen
        else
        {
        if(p->pw_uid == buffer.st_uid) printf("%s\n", file_name);
        }


    }



	count++;
	}
	return;


}
