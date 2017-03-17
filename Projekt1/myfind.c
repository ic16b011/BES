#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <error.h>
#include <fnmatch.h>

void do_dir(const char* dir_name, const char* parms[]);
void do_file(const char* file_name, const char* parms[]);

int main(int argc, const char* argv[])
{
	int i, j;

	const char** parms = NULL;
	const char* punkt[] = { "." };

	if (argc == 1) printf("Please use some parameters\n");

	for (i = 1; i < argc; i++)
	{
		if (!strncmp(argv[i], "-", 1))
		{
			parms = &argv[i];
			break;
		}
	}

	j = 1;

	if (parms != NULL && i == 1)
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
	int status, len;
	char localname[100];

	localname[0] = '\0';

	/* Prüfe, ob es einen Eintrag dir_name gibt */
	status = stat(dir_name, &buffer);

	/* Eintrag i.O. und ein Directory */
	if ((status == 0 && S_ISDIR(buffer.st_mode)))
	{
		/* Actions vorhanden */
		if (parms != NULL)
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
					status = lstat(localname, &buffer);

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
	else if (status == 0)
	{
		do_file(dir_name, parms);
	}

	return;
}

void do_file(const char* file_name, const char* parms[])
{


	struct stat buffer;
	struct passwd *p;
	int status = lstat(file_name, &buffer);
	int uid;

	if (status == 0)
	{
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

					if (!(strcmp(basename((char*)file_name), parms[count+1]))) printf("%s\n", file_name);
					count += 2;
				}

				else if (!(strcmp(parms[count], "-user")))
				{
					errno = 0;
					p = getpwnam(parms[count + 1]);
					if (p == NULL)
					{
						uid = atoi(parms[count + 1]);
						if(uid != 0) 
						{
							errno = 0;
							p = getpwuid(uid);
						}
						if (p == NULL) error(EXIT_FAILURE, errno, "`%s' is not the name of a known user", parms[count+1]);
						else
						{
							if (p->pw_uid == buffer.st_uid) printf("%s\n", file_name);
						}
					}
					else
					{
						if (p->pw_uid == buffer.st_uid) printf("%s\n", file_name);
					}

					count += 2;

				}

				else if (!(strcmp(parms[count], "-nouser")))
				{
					if ((p = getpwuid(buffer.st_uid)) == NULL)
					{
						printf("%s\n", file_name);
					}
					
					count++;
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

					printf("    %u", buffer.st_nlink);

					if ((p = getpwuid(buffer.st_uid)) != NULL)
					{
						printf("  %s", p->pw_name);
					}
					if ((p = getpwuid(buffer.st_gid)) != NULL)
					{
						printf(" %s", p->pw_name);
					}

					printf(" %ld ", buffer.st_size);

					struct tm *info;
					char tm_buffer[200];
					info = localtime(&(buffer.st_mtime));

					strftime(tm_buffer, 200, "%b %d %H:%M", info);
					printf("  %s", tm_buffer);

					printf("   %s  ", file_name);

					if (S_ISLNK(buffer.st_mode) == 1)

					{


						char *linkname;
						ssize_t r, bufsiz;

						bufsiz = buffer.st_size + 1;

						linkname = malloc(bufsiz);
						if (linkname == NULL)
						{
							perror("malloc");
							exit(EXIT_FAILURE);
						}
						r = readlink(file_name, linkname, bufsiz);
						linkname[r] = '\0';

						printf("->  %s\n", linkname);

					}
					else printf("\n");
					
					count++;

				}
				else if (!(strcmp(parms[count], "-path")))
				{
					if (fnmatch(parms[count + 1], file_name, FNM_NOESCAPE) == 0) 
					{
						printf("%s\n", file_name);
					}
					
					count += 2;
				}
				
				else if (!(strcmp(parms[count], "-type"))) 
				{
					if (strlen(parms[++count]) != 1) 

					{
						error(EXIT_FAILURE, 0, "Arguments to -type should contain only one letter");
						return;
					}

					if ((S_ISDIR(buffer.st_mode)) && (strcmp(parms[count], "d") == 0)) printf("%s\n", file_name);

					else if ((S_ISREG(buffer.st_mode)) && (strcmp(parms[count], "f") == 0)) printf("%s\n", file_name);

					else if ((S_ISCHR(buffer.st_mode)) && (strcmp(parms[count], "c") == 0)) printf("%s\n", file_name); 

					else if ((S_ISBLK(buffer.st_mode)) && (strcmp(parms[count], "b") == 0)) printf("%s\n", file_name);

					else if ((S_ISFIFO(buffer.st_mode))&& (strcmp(parms[count], "p") == 0)) printf("%s\n", file_name);

					else if ((S_ISLNK(buffer.st_mode)) && (strcmp(parms[count], "l") == 0)) printf("%s\n", file_name);

					else if ((S_ISSOCK(buffer.st_mode)) && (strcmp(parms[count], "s") == 0))printf("%s\n", file_name); 
					
					count ++;
				}
				
				else
				{
					printf("find: unknown predicate %s\n", parms[count]);
					return;
				}
				
			}			
		}
		else
			printf("%s\n", file_name);
	}

	return;
}