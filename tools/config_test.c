#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>	/* for umask() on some systems */
#include <sys/types.h>
#include <sys/time.h>
#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h> /* for rlimit */
#endif
#include <fcntl.h>
#include <dlfcn.h>
#include <math.h>
#include <pthread.h>

int config_load(char *);
int file_exists (char *);
int config_parse(char *);

int dodirective(char *, char *);


/*************************************/

int main(int ac, char **av) { 
    printf("\nOmega Security Services\n");
	printf(" Configuration Tester\n\n");
	printf("    Developers: \033[1;32m \n        * Twitch\n        * Jer\n\033[0m\n");

	printf("Loading Config: \033[1;31m%s\033[0m\n\n", av[1]);
	printf("\n Result:  \033[1;32m%s\033[0m\n", (config_load(av[1]))? "Success" : "Error");
	return 0;
}

/*************************************/
/** 
 * file_exists - Check weather or
 * a file exists
 * @param fileName (string) path to file we are checking
 * @return (int) 1 on success
 */

int file_exists (char * fileName)
{
    FILE *fp;   
    if (fp = fopen(fileName,"r"))
	{	
		fclose(fp);
		return 1;
	}
		
   return 0;

}
/*************************************/
/**
 * Open a config file and begine reading it
 * @param file (string) path to the file we are opening
 * @return (int) 1 on success
 */
 

//set the limit of included files to 10 - else this could really bog us down
#define MAX_INCLUDE 10 


int  include_cnt = 0;
char include_stack[MAX_INCLUDE][255];
char *current_file;




/*************************************/
/**
 * Open a config file and begin reading it
 * @param file (string) path to the file we are opening
 * @return (int) 1 on success
 */
 
int config_load(char *file) 
{
	int config_fd;
	struct stat stat_buf;
	int ret = 0;
	int t = 0;
	char *config_buffer= NULL;
	
	if (include_cnt > MAX_INCLUDE) {
		printf("Error: Reached max include value - cannot include another file.");
		return -1;
	}
	
	for (t = 0; include_stack[t][0]; t++)
	{
			if (strcasecmp(include_stack[t],file)==0)
			{
				printf("Error: This file [%s] is included recursively... (loop inclusion)\n", file);
				return -1;
			}
	}
	if (!(config_fd = open(file, O_RDONLY)))
	{
		printf("Could not locate config file [%s]", file);
		return 0;
	}
	if (fstat(config_fd, &stat_buf) == -1)
	{
		printf("Couldn't fstat \"%s\": %s\n", file, strerror(errno));
		close(config_fd);
		return 0;
	}
	if (!stat_buf.st_size)
	{
		close(config_fd);
		return 0;
	}
	if (!(config_buffer = (char *)malloc(stat_buf.st_size + 1)))
	{
		printf("Unable to create a buffer for config file.\n");
		close(config_fd);
		return 0;
	}
	ret = read(config_fd, config_buffer, stat_buf.st_size);
	if (ret != stat_buf.st_size)
	{
		printf("Error while reading file\n");
		free(config_buffer);
		close(config_fd);
	}
	//make sure we keep a stack of what files we are including because we dont want to
	//be recursive... :/
	strncpy(include_stack[include_cnt],file,255);
	include_cnt++;
	
	config_buffer[ret] = '\0';
	close(config_fd);
	current_file = file;
	config_parse(config_buffer);
	free(config_buffer);
	return 1;
}


/*********************************************/
/**
 * Parse a config buffer
 * @param (String) Buffer we are to parse
 * @return (int) 1 on success 
 */
 
int config_parse(char *buffer)
{

	char *ptr = NULL;
	char *pos = NULL;
	char *buf = NULL;

	int error_cnt = 0;
	int line_cnt = 0;

	int in_directive = 0;

	int cnt = 0, i = 0 , x = 0;

	char directive[250];
	char param[250];
	char value[250];


	while (*buf) {
		switch (*buf) {
			case '\n':
				line_cnt++;
				break;
			case '\t':
				break;
			case '<':
				memset(directive, '\0', sizeof(directive));
				dodirective(buf, directive);
				printf("\n--------\nBuf:\n %s--------\n\n", buf);
				break;
			case '>':
				printf("Directive: %s\n", directive);
				in_directive = 0;
				break;
			case ' ':
				break;
			default:
				*ptr++ = *buf; 
				break;
		}
		buf++;
	}
	return line_cnt;
}


/**********************************************************/

int dodirective(char * pos, char * tag) { 
	char *ptr = tag;
	while ((*pos) && (*pos != '\n') || (*pos != ' ')) {
			*ptr++ = *pos;
			pos++;
	}
	if (strlen(tag)==0)
		return 0;
	return 1;
}



