#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <pwd.h>
#include <time.h>

void do_dir(const char* dir_name, const char* parms[]);
void do_file(const char* file_name, const char* parms[]);

int main(int argc, const char* argv[])
{
	int i, j;

	const char** parms = NULL;
	const char* punkt[] = {"."};

	/*system("mkdir test");
	system("touch test2");
	system("touch asdf.c");*/

	if (argc == 1) printf("Please use some parameters\n");

	for (i = 0; i < argc; i++)
	{
		if (!strncmp(argv[i], "-", 1))
		{
			parms = &argv[i];
			i++;
			break;
		}
	}

	j = 1;

	if (parms != NULL && i == 2)
	{
		do_dir(punkt[0], parms);
	}
	else
	{
		while (j < i)
		{
			do_dir(argv[j], parms);
			j++;
		}
	}
	return 0;
}

void do_dir(const char* dir_name, const char* parms[])
{
	DIR *directory;
	struct dirent *entry;
	struct stat buffer;
	int status, len, count;
	char localname[100];

	localname[0] = '\0';
	count = 0;

	/* Prüfe, ob es einen Eintrag dir_name gibt */
	status = stat(dir_name, &buffer);

	/* Eintrag i.O. und ein Directory */
	if ((status == 0 && S_ISDIR(buffer.st_mode)))
	{
		/* keine Actions vorhanden */
		if (parms == NULL)
		{
			printf("%s\n", dir_name);
		}
		/* Actions vorhanden */
		else
		{
			do_file(dir_name, parms);
		}

		/* Öffne Directory */
		directory = opendir(dir_name);
		if (!directory)
		{
			printf("find: `%s': No such file or directory\n", dir_name);
			return;
		}

		do
		{
			/* Lese Directory Einträge */
			entry = readdir(directory);

			if (entry && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
			{
				/* Erzeuge Fullpath */
				strcpy(localname, dir_name);
				len = strlen(localname);
				if (localname[len - 1] != '/')
					strcat(localname, "/");

				strcat(localname, entry->d_name);
					
				/* Wenn Directory, rufe do_dir rekursiv auf */
				if (entry->d_type == DT_DIR)
				{
					do_dir(localname, parms);
				}

				/* Wenn Regular File oder Symbolic Link, rufe do_file auf */
				else if (entry->d_type == DT_REG || entry->d_type == DT_LNK)
				{
					do_file(localname, parms);
				}

				/* Typ unbekannt, versuche Typ über stat() herauszufinden */
				else if (entry->d_type == DT_UNKNOWN)
				{
					status = stat(localname, &buffer);

					if (status == 0 && S_ISDIR(buffer.st_mode))
					{
						do_dir(localname, parms);
					}
					else if (status == 0 && (S_ISREG(buffer.st_mode) || S_ISLNK(buffer.st_mode)))
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
		} while (entry);

		/* Schließe Directory */
		closedir(directory);
	}
	/* Eintrag i.O., aber kein Directory */
	else if(status == 0)
	{
		do_file(dir_name, parms);
	}

	return;
}

void do_file(const char* file_name, const char* parms[])
{


	struct stat buffer;
	int status = stat(file_name, &buffer);
	if (status == 0)
	{
		/*printf("2: %s\n", file_name);
		printf("Inode2: %lu\n\n", buffer.st_ino);*/

		int count = 0;

		if (parms != NULL)
		{
			while (parms[count] != '\0')
			{

				if (!(strcmp(parms[count], "-print")))
				{
					printf("%s\n", file_name);
					break;
				}

				else if (!(strcmp(parms[count], "-name")))
				{

					if (!(strcmp(basename((char*)file_name), parms[count + 1]))) printf("%s\n", file_name);
					count += 2;
				}

				else if (!(strcmp(parms[count], "-user")))
				{
					if (buffer.st_uid == atoi(parms[count + 1])) printf("%s\n", file_name);

					struct passwd *p;

					//if((p = getpwnam(parms[count+1])) == NULL) perror("User nicht vorhanden");//hier sollte man eine einmalige Fehlermeldung anzeigen

					if (p->pw_uid == buffer.st_uid) printf("%s\n", file_name);

					count += 2;

				}

				else if (!(strcmp(parms[count], "-ls")))
				{
					printf("%9lu    %3lu ", buffer.st_ino, (buffer.st_blocks / 2));

					printf((S_ISDIR(buffer.st_mode)) ? "d" : "-");
					printf((buffer.st_mode & S_IRUSR) ? "r" : "-");
					printf((buffer.st_mode & S_IWUSR) ? "w" : "-");
					printf((buffer.st_mode & S_IXUSR) ? "x" : "-");
					printf((buffer.st_mode & S_IRGRP) ? "r" : "-");
					printf((buffer.st_mode & S_IWGRP) ? "w" : "-");
					printf((buffer.st_mode & S_IXGRP) ? "x" : "-");
					printf((buffer.st_mode & S_IROTH) ? "r" : "-");
					printf((buffer.st_mode & S_IWOTH) ? "w" : "-");
					printf((buffer.st_mode & S_IXOTH) ? "x" : "-");

					printf("    %lu", buffer.st_nlink);

					struct passwd *p;
					if ((p = getpwuid(buffer.st_uid)) != NULL)
					{
						printf("  %s", p->pw_name);
					}
					if ((p = getpwuid(buffer.st_gid)) != NULL)
					{
						printf(" %s", p->pw_name);
					}
					//time


					struct tm *info;
					char tm_buffer[200];
					info = localtime(&(buffer.st_mtime));

					strftime(tm_buffer, 200, "%B %d %H:%M", info);
					printf("  %s", tm_buffer);

					//printf("  %s", ctime(&(buffer.st_mtim)));

					printf("   %s\n", file_name);

					count++;
				}
			}
		}
		else
			printf("%s\n", file_name);
	}

	return;
}