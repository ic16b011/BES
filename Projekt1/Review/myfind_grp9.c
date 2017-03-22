/**
 * @file myfind.c
 * Betriebssysteme myfind File.
 * Beispiel 1
 *
 * @author Dominik Ballek <dominik.ballek@technikum-wien.at>
 * @author Emanuel Vig <emanuel.vig@technikum-wien.at>
 * @author Mathias Roll <mathias.roll@technikum-wien.at>
 * @date 2017/02/25
 *
 * @version 1
 *
 *
 */

/*
 * -------------------------------------------------------------- includes --
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <pwd.h>
#include <ctype.h>
#include <fnmatch.h>
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>



/*
 * ------------------------------------------------------------- functions --
 */

static void do_file(const char *file_name, char const * const * parms);
static void do_dir (const char *dir_name,  char const * const * parms);
static int checkArgSynt(int startIndex, char const * const * parms);
static void printHelp(const char * programName);

static char mode2char(const mode_t type);
static int comp_type(const char type,const struct stat* const file_stats);
static int comp_user(const char* const user , const struct stat* const file_stat);
static int uid_exists(uid_t uid);

static int print_ls(const char *path, struct stat* const attr);
static int do_name(const char *path, const char *pattern);
static int do_path(const char *path, const char *pattern);


/**
 *
 * \brief main of myfind
 *
 * read passed parameters and call do_file to start the search or print the help text
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return 0 on success, 1 on error
 *
 */
int main(int argc, const char *argv[])
{
//i=0;

/* ### FB: warum wird argc=argc gemacht? */
argc = argc;

	if (argc > 1)
	{
		if (*argv[1] != '-') //if first argument is no option, *argv[1] = first char of argument 1
		{
			if(checkArgSynt(2, argv) == 0) //start checking argument syntax from the second argument on
			{
				do_file(argv[1], argv);
				return 0;
			}
			return 1;
		}
		else if (strcmp(argv[1], "-help") == 0)
		{
			printHelp(argv[0]);
			return 0;
		}
		else //first passed argument is an option, use default starting location
			if(checkArgSynt(1, argv) == 1) //start checking argument syntax from the first argument on
				return 1;
	}
	
	do_file(".", argv);
	return 0;
}





/**
 *
 * \brief call do_dir if file_name is a directory, else print it if filters apply
 *
 * \param dir_name file to read
 * \param parms filter parameters if file is to print out
 *
 */
static void do_file (const char *file_name, char const * const * parms)
{
	/* ### FB: warum wird parms=parms gemacht? */
	parms = parms;
	struct stat buf;

	if( lstat(file_name, &buf) == -1)
		error(0, errno, "%s", file_name);

	else
	{
		int i = 0;
		int stillInRace = 1;
		int alreadyPrinted = 0;

		while(parms[i] != NULL)
		{

			if(strcmp(parms[i], "-nouser") == 0)
			{
				stillInRace &= uid_exists(buf.st_uid);
			}
			else if(strcmp(parms[i], "-path") == 0)
			{
				stillInRace &= do_path(file_name, parms[i+1]);
				i++; //-path has a follow-up string which is not to be parsed
			}
			else if(strcmp(parms[i], "-type") == 0)
			{
				stillInRace &= comp_type(*parms[i+1],&buf);
				i++;
			}
			else if(strcmp(parms[i], "-name") == 0)
			{
				stillInRace &= do_name(file_name, parms[i+1]);
				i++;
			}
			else if(strcmp(parms[i], "-user") == 0)
			{
				stillInRace &= comp_user(parms[i+1] , &buf);
				i++;
			}
	
			if(strcmp(parms[i], "-ls") == 0)
			{
				alreadyPrinted = 1;
				if ( stillInRace == 1)
					print_ls(file_name, &buf);
//				stillInRace = 1; //can be commented in to allow reset of filters with each -ls
			}
			else if(strcmp(parms[i], "-print") == 0 ||
							(parms[i+1] == NULL && alreadyPrinted == 0))
			{
				alreadyPrinted = 1;
				if ( stillInRace == 1)
					if ( printf("%s\n", file_name) < 0)
						error(0, errno, " ");
//				stillInRace = 1; //can be commented in to allow reset of filters with each -print
			}
			
			//has to be commented out to allow reset of filters with each printing option
			if(stillInRace == 0)
				break;
			
			i++;
		}
		
		if(S_ISDIR(buf.st_mode))	
			do_dir(file_name, parms);
	}
}


/**
 *
 * \brief calls do_file for each directory entry found in param dir_name
 *
 * \param dir_name directory to read
 * \param parms parameters to pass to do_file
 *
 */
static void do_dir (const char *dir_name,  char const * const * parms)
{
	/* ### FB: warum wird parms=parms gemacht? */
	parms = parms;
	
	DIR* dirp = opendir(dir_name);
	
	if(dirp == NULL)
		//error(0, errno, "%s", dir_name);
		error(0, errno, " ");
	
	else
	{
		const struct dirent * current_dirent;
		errno = 0;
		
		while ((current_dirent = readdir(dirp)) != NULL)
		{
			if ( strcmp(current_dirent->d_name, "..") != 0 && strcmp(current_dirent->d_name, ".") != 0)
			{
				char nextName [strlen(dir_name) + strlen(current_dirent->d_name)+2];
				//scope of VLA is block -> exactly one nextName string per depth of file tree
				//unknown number of function calls -> unknown number of mem needed, possible error source

				if( *(dir_name+strlen(dir_name)-1) == '/')
					/* ### FB: Kein Error Handling für sprintf */
					sprintf(nextName, "%s%s", dir_name, current_dirent->d_name);
				else
					/* ### FB: Kein Error Handling für sprintf */
					sprintf(nextName, "%s/%s", dir_name, current_dirent->d_name);

				do_file(nextName, parms);

				errno = 0;
			}
		}
	
		if (errno != 0)
			error(0, errno, " ");
	
		if( closedir(dirp) == -1)
			error(0, errno, " ");
	}
}


/**
 *
 * \brief check syntax of passed parameters
 *
 * check if passed parameters comply to syntax and call printHelp if not
 *
 * \param startIndex number on which parameter checking starts
 * \param parms parameters to check
 *
 * \return 0 if parameters fulfill syntax, 1 if parameters do not comply to syntax
 *
 */
static int checkArgSynt(int startIndex, char const * const * parms)
{
	/* ### FB: startIndex kann direkt verwendet werden */
	int i = startIndex;
	
	while(parms[i] != NULL)
	{
		// parms which need no further checking
		if(	strcmp(parms[i], "-nouser") == 0	|| strcmp(parms[i], "-ls") == 0 ||
				strcmp(parms[i], "-print")	 == 0	)
			{
				i++;
				continue;
			}
			
		// parms which need any string following them
		else if ( (strcmp(parms[i], "-name") == 0 || strcmp(parms[i], "-path") == 0 ||
							 strcmp(parms[i], "-user") == 0)
							&& (parms[i+1] != NULL) )
			{
				i += 2;
				continue;
			}
			
		// -type needs a following string but not any
		else if ( strcmp(parms[i], "-type") == 0 && parms[i+1] != NULL)
			if (strcmp(parms[i+1], "b") == 0 || strcmp(parms[i+1], "c") == 0 ||
          strcmp(parms[i+1], "d") == 0 || strcmp(parms[i+1], "p") == 0 ||
          strcmp(parms[i+1], "f") == 0 || strcmp(parms[i+1], "l") == 0 ||
          strcmp(parms[i+1], "s") == 0)
      	{
      		i += 2;
      		continue;
      	}

		fprintf(stderr, "%s: argv[%d] '%s' not valid - aborting\n\n", parms[0], i, parms[i]);
		printHelp(parms[0]);
		return 1;
	}
	return 0;
}


/**
 *
 * \brief print a help text on how to use the program
 *
 * \param programName name of this programs executable file
 *
 */
static void printHelp(const char * programName)
{
	if (printf("Usage:\n"
             "%s [-help] [path] [expression]\n\n"
             "default path is the current directory\n"
             "default expression is -print\n\n"
             "expression can be:\n"
             "-user <name>|<uid>  entries belonging to given user\n"
             "-nouser             entries not belonging to any user\n"
             "-name <pattern>     entries named <pattern>\n"
             "-path <pattern>     entries matching name and path\n"
             "-type [bcdpfls]     entries of given type\n"
             "-print              print matching entries\n"
             "-ls                 print matching entries in detail\n", programName) < 0) 
		error(0, errno, " ");
}



// =================================================================================== //

 
 
 /*
* Wandelt st_mode in ein Char
*/

static char mode2char(const mode_t type){
  
  char c=0;
  
  /* ### FB: else if könnte verwendet werden, um eine so große Verschachtelung zu vermeiden */
  if(S_ISBLK(type)) c='b';
  else
  if(S_ISCHR(type)) c='c';
  else
  if(S_ISDIR(type)) c='d';
  else
  if(S_ISREG(type)) c='f';
  else
  if(S_ISFIFO(type)) c='p';
  else
  if(S_ISLNK(type)) c='l';
  else
  if(S_ISSOCK(type)) c='s';
  else
	  /* ### FB: wo wird errno gesetzt? */
    error(1,errno,"´%c´ is not a valid type",type);
  
  return c;
   
 }
 




/**
 * @brief converts the entry attributes to a readable type
 *
 * @param attr the entry attributes from lstat
 *
 * @returns the entry type as a char
 */
static char do_get_type(struct stat* attr) {

  /* block special file */
  if (S_ISBLK(attr->st_mode)) {
    return 'b';
  }
  /* character special file */
  if (S_ISCHR(attr->st_mode)) {
    return 'c';
  }
  /* directory */
  if (S_ISDIR(attr->st_mode)) {
    return 'd';
  }
  /* fifo (named pipe) */
  if (S_ISFIFO(attr->st_mode)) {
    return 'p';
  }
  /* regular file */
  if (S_ISREG(attr->st_mode)) {
    return 'f';
  }
  /* symbolic link */
  if (S_ISLNK(attr->st_mode)) {
    return 'l';
  }
  /* socket */
  if (S_ISSOCK(attr->st_mode)) {
    return 's';
  }

  /* some other file type */
  return '?';
}

/**
 * @brief converts the entry attributes to readable permissions (static variable as cache)
 *
 * @param attr the entry attributes from lstat
 *
 * @returns the entry permissions as a string
 */
static char *do_get_perms(struct stat* attr) {
  static char perms[11];
  char type = do_get_type(attr);

  /*
   * cast is used to avoid the IDE warnings
   * about int possibly not fitting into char
   */
  perms[0] = (char)(type == 'f' ? '-' : type);
  perms[1] = (char)(attr->st_mode & S_IRUSR ? 'r' : '-');
  perms[2] = (char)(attr->st_mode & S_IWUSR ? 'w' : '-');
  perms[3] = (char)(attr->st_mode & S_ISUID ? (attr->st_mode & S_IXUSR ? 's' : 'S')
                                           : (attr->st_mode & S_IXUSR ? 'x' : '-'));
  perms[4] = (char)(attr->st_mode & S_IRGRP ? 'r' : '-');
  perms[5] = (char)(attr->st_mode & S_IWGRP ? 'w' : '-');
  perms[6] = (char)(attr->st_mode & S_ISGID ? (attr->st_mode & S_IXGRP ? 's' : 'S')
                                           : (attr->st_mode & S_IXGRP ? 'x' : '-'));
  perms[7] = (char)(attr->st_mode & S_IROTH ? 'r' : '-');
  perms[8] = (char)(attr->st_mode & S_IWOTH ? 'w' : '-');
  perms[9] = (char)(attr->st_mode & S_ISVTX ? (attr->st_mode & S_IXOTH ? 't' : 'T')
                                           : (attr->st_mode & S_IXOTH ? 'x' : '-'));
  perms[10] = '\0';

  return perms;
}

/**
 * @brief converts the entry attributes to username or, if not found, uid (static variable as cache)
 *
 * @param attr the entry attributes from lstat
 *
 * @returns the username if getpwuid() worked, otherwise uid, as a string
 */
static char *do_get_user(struct stat* attr) {
  struct passwd *pwd;

  static unsigned int cache_uid = UINT_MAX;
  static char *cache_pw_name = NULL;

  /* skip getgrgid if we have the record in cache */
  if (cache_pw_name!= NULL && cache_uid == attr->st_uid) {
    return cache_pw_name;
  }

  pwd = getpwuid(attr->st_uid);

  if (pwd== NULL) {
    /*
     * the user is not found or getpwuid failed,
     * return the uid as a string then;
     * an unsigned int needs 10 chars
     */

    static char user[11];
    if (snprintf(user, 11, "%u", attr->st_uid) <= 0) {     
      error(0,errno,"snprintf(): %s\n", strerror(errno));
      return "";
    }
    return user;
  }

  cache_uid = pwd->pw_uid;
  cache_pw_name = pwd->pw_name;

  /* getpwuid manual: do not pass the returned pointer to free() */

  return pwd->pw_name;
}

/**
 * @brief converts the entry attributes to groupname or, if not found, gid (static variable as cache)
 *
 * @param attr the entry attributes from lstat
 *
 * @returns the groupname if getgrgid() worked, otherwise gid, as a string
 */
static char *do_get_group(struct stat* attr) {
  struct group *grp;

  static unsigned int cache_gid = UINT_MAX;
  static char *cache_gr_name = NULL;

  /* skip getgrgid if we have the record in cache */
  if (cache_gr_name!=NULL && cache_gid == attr->st_gid) {
    return cache_gr_name;
  }

  grp = getgrgid(attr->st_gid);

  if (grp==NULL) {
    /*
     * the group is not found or getgrgid failed,
     * return the gid as a string then;
     * an unsigned int needs 10 chars
     */
    static char group[11];
    if (snprintf(group, 11, "%u", attr->st_gid) <= 0) {     
      error(0,errno,"snprintf(): %s\n", strerror(errno));
      return "";
    }
    return group;
  }

  cache_gid = grp->gr_gid;
  cache_gr_name = grp->gr_name;

  /* getgrgid manual: do not pass the returned pointer to free() */

  return grp->gr_name;
}

/**
 * @brief converts the entry attributes to a readable modification time (static variable as cache)
 *
 * @param attr the entry attributes from lstat
 *
 * @returns the entry modification time as a string
 */
static char *do_get_mtime(struct stat attr) {
  static char mtime[16]; /* 12 length + 3 special + null */
  char *format;

  //time_t now = time(NULL);
  //time_t six_months = 31556952 / 2; /* 365.2425 * 60 * 60 * 24 */
  struct tm *local_mtime = localtime(&attr.st_mtime);

  if (local_mtime==NULL) {   
    error(0,errno,"localtime(): %s\n", strerror(errno));
    return "";
  }
	format = "%b %e %H:%M"; /* recent */

  //if ((now - six_months) < attr.st_mtime) {
   // format = "%b %e %H:%M"; /* recent */
  //} else {
  //  format = "%b %e  %Y"; /* older than 6 months */
  //}
	/* ### FB: es ist besser, errno vor einem Funktionsaufruf = 0 zu setzen, da manche Funktionen bei erfolgreicher Ausführung errno nicht auf Success setzen */
  if (strftime(mtime, sizeof(mtime), format, local_mtime) == 0) {   
    error(0,errno,"strftime(): %s\n",strerror(errno));
    return "";
  }

  return mtime;
}

/**
 * @brief extracts the entry symlink
 *
 * @param path the entry path
 * @param attr the entry attributes from lstat
 *
 * @returns the entry symlink as a string
 */
static char *do_get_symlink(const char *path, struct stat* const attr) {

  if (S_ISLNK(attr->st_mode)) {
    /*
     * st_size appears to be an unreliable source of the link length
     * PATH_MAX is artificial and not used by the GNU C Library
     */
    ssize_t length;
    size_t buffer = 128;
    char *symlink = malloc(sizeof(char) * buffer);

    if (!symlink) {
         error(0,errno, "malloc(): %s\n", strerror(errno));
      return strdup("");
    }

    /*
     * if readlink() fills the buffer, double it and run again
     * even if it equals, because we need a character for the termination;
     * a check for > 0 is mandatory, because we are comparing signed and unsigned
     */
	 /* ### FB: es ist besser, errno vor einem Funktionsaufruf = 0 zu setzen, da manche Funktionen bei erfolgreicher Ausführung errno nicht auf Success setzen, in diesem Fall wäre es auch in der Schleife angebracht */
    while ((length = readlink(path, symlink, buffer)) > 0 && (size_t)length >= buffer) {
      buffer *= 2;
      char *new_symlink = realloc(symlink, sizeof(char) * buffer);

      if (!new_symlink) {        
        error(0,errno,"realloc(): %s\n", strerror(errno));
        free(symlink); /* realloc doesn't free the old object if it fails */
        return strdup("");
      }

      symlink = new_symlink;
    }

    if (length < 0) {      
      error(0,errno,"readlink(%s): %s\n", path, strerror(errno));
      free(symlink);
      return strdup("");
    }

    symlink[length] = '\0';
    return symlink;
  }

  /* the entry is not a symlink */
  return strdup("");
}


/**
 * @brief prints out the path with details (ls)
 *
 * @param path the path to be processed
 * @param attr the entry attributes from lstat
 *
 * @returns EXIT_SUCCESS, EXIT_FAILURE
 */
static int print_ls(const char *path, struct stat* const attr) {
  unsigned long inode = attr->st_ino;
  long long blocks = S_ISLNK(attr->st_mode) ? 0 : attr->st_blocks / 2;
  char *perms = do_get_perms(attr);
  unsigned long links = attr->st_nlink;
  char *user = do_get_user(attr);
  char *group = do_get_group(attr);
  long long size = attr->st_size;
  char *mtime = do_get_mtime(*attr);
  char *symlink = do_get_symlink(path, attr);
    char *arrow = symlink[0] ? " -> " : "";

  if (printf("%6lu %4lld %10s %3lu %-8s %-8s %8lld %12s %s%s%s\n", inode, blocks, perms, links,
             user, group, size, mtime, path, arrow, symlink) < 0) {
			/* ### FB: warum zweimal hintereinander error()? */
             error(0, errno, " ");    
    error(0,errno,"printf(): %s\n",strerror(errno));
    free(symlink);
    return EXIT_FAILURE;
  }

  free(symlink);

  return EXIT_SUCCESS;
}







 
 
/**
 * @brief checks if the filename matches the pattern
 *
 * @param path the entry path
 * @param pattern the pattern to match against
 *
 * @returns EXIT_SUCCESS, EXIT_FAILURE
 */
static int do_name(const char *path, const char *pattern) {
	char *filename = basename((char*)path);
	int flags = 0;

	if (fnmatch(pattern, filename, flags) == 0) {
		return 1;
	}

	/* basename manual: do not pass the returned pointer to free() */

	return 0;
}
 
/**
 * @brief checks if the path matches the pattern
 *
 * @param path the entry path
 * @param pattern the pattern to match against
 *
 * @returns EXIT_SUCCESS, EXIT_FAILURE
 */
static int do_path(const char *path, const char *pattern) {
	int flags = 0;

	if (fnmatch(pattern, path, flags) == 0) {
		return 1;
	}

	return 0;
}



/* Prüft  -type auf Korrektheit und vergleicht mit file type
 * ERROR sollte falsches argument mitgegeben worden sein...
*/
static int comp_type(const char type,const struct stat* const file_stats){
  
   char valid_types[] = {'b', 'c', 'd', 'f', 'p', 'l', 's', '\0'};
   int i=0;

  
  while(type != valid_types[i++])
  {
     if(valid_types=='\0')
		 /* ### FB: errno wird nirgends gesetzt */
        error(0,errno,"Unknown argument to -type: %c",type);
  }
     
  
  if(type == (mode2char(file_stats->st_mode)))
     return 1;
  else
     return 0;
 }


/*
* für nouser
*/
static int uid_exists(uid_t uid){
  errno=0;
  
  if( getpwuid(uid) == NULL){
    if(errno) error(0,errno,"getpwname");
    else return 1;
 }
  
  return 0;
}


/*
* vergleicht User mit FileOwner
* 1 .. gleich
* 0 .. ungleich
*/

static int comp_user(const char* const user , const struct stat* const file_stat){
 
  char* cp;
  uid_t uid;
  int exit_code=0;
  struct passwd* records;
  
  //reset
  errno=0;

  if( (records=getpwnam(user)) == NULL)
  {
      if(errno) error(0,errno,"getpwname");

	  /* ### FB: errno sollte = 0 gesetzt werden, wenn errno != 0 von voriger Funktion ist und strtoul errno bei erfolgreicher Ausführung nicht ändert, wird für strtoul trotzdem eine Fehlermeldung angezeigt */
      // Kein User => evt. uid? 
      uid=(uid_t)strtoul(user,&cp,10);
      
      if(errno) error(0,errno,"strtoul_l");
      
	  /* ### FB: errno sollte = 0 gesetzt werden, wenn errno != 0 von voriger Funktion ist und isspace errno bei erfolgreicher Ausführung nicht ändert, ist es möglich, dass für isspace trotzdem eine Fehlermeldung angezeigt wird */
      // Check ob valider integer string
      if( isspace(*cp) || *cp == 0)
      {
	if( uid == file_stat->st_uid)
	  exit_code=1;  
      }
      else
	error(1,errno,"´%s´ is not the name of a known user",user);

      
  
   
  }else{  
  /*User gefunden vergleiche mit File*/
    if( file_stat->st_uid == records->pw_uid) 
      exit_code=1;
    else 
      exit_code=0;
  }

  
  return exit_code;
}


/*
 * =================================================================== eof ==
 */
