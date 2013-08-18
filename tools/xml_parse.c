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

typedef struct xmlparams_ {
	char *name;
	char *value;
	struct xmlparams_ *next, *prev;
} XMLParams;

typedef struct xmlnode_ {
	char *name;
	char *innerData;
	int  child_count;
	struct xmlnode_ *parent;
	struct xmlnode_ *children[1024];
	XMLParams *params;
	struct xmlnode_ *next, *prev;
} XMLNode;



XMLNode *xmldom;
XMLNode *new_node(XMLNode *, char *name);

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
	char *ptr;
	
	int nested = 0;
	int tag_count = 0;
	int in_tag = 0;
	int do_inner = 0;
	int skip = 0;

	char stack[255][1024];
	
	char directive[1024];
	char param[1024];
	char value[1024];
	char innerData[40000];
	char *start, *pos;
	
	char file[255];
	
	 
	
	strncpy(file,current_file,255); 
	for (ptr = buffer; *ptr; ptr++)
	{
		switch (*ptr)
		{
			case '\n':
				break;
			case '<':
				ptr++;
				start = ptr;
				if (*ptr == '/') {
					nested--;
					printf("End tag: %s\n", directive);
					do_inner = 0;
					break;
				}

				for (;*ptr && *ptr != '>';ptr++) {		
					if ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\n') || (*ptr == '>'))
						break;
					if (!(*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <='Z') || (*ptr >= '0' && *ptr <= '9') || *ptr == '_')
						skip = 1;
				}
				if (skip) { skip = 0; break; }
				strlcpy(directive, start, (ptr+1) - start);
				tag_count++;
				printf(" [%d] Tag: %s \n", nested, directive);
				nested++;
				in_tag = 1;
				if (*ptr == '>')
					ptr--;
				break;
			case '>':
				if (in_tag) {
				 	in_tag = 0;
					do_inner = 1;
				}
				break;
			case '\t':
			case ' ':
			case '=':
			case '\r':
				break;
			default:
				if (in_tag) {
					memset(param, '\0', sizeof(param));
					start = ptr;
					for (; *ptr != '>' && *ptr; *ptr++) {
							pos = ptr;
							while ((*pos != '=') && (*pos) && (*pos != '>'))
									pos++;
							if (*pos == '=') {
								ptr++;		
								strncpy(param, start, pos - start);
								printf("         [%s=", param);
								if (*pos == ' ')
								pos--;
								ptr = pos++;
								
							}	
							//just incase
							if (*ptr == '=')
								ptr++; 
							if ((*ptr == '\'') || (*ptr == '\"')) {
								ptr++;
								start = ptr;
								//check for both ' and " in to allow users to use both (tho they should only be using " imo
								if (!(pos = strchr(ptr, '\''))) {
									if (!(pos = strchr(ptr, '\"'))) 
											return 0;
									
								}
								//incase they want multiline directives we should probably nuke these out.
								while ((*start == '\t') || (*start == '\n') || (*start == ' '))
										start++;
								memset(value, 0, sizeof(value));																
								strncpy(value, start, pos - start);
								
								printf("%s] \n", value);
								//clear out shit here so we dont go junking up our memory.
								memset(value,'\0',sizeof(value));
								
								ptr = pos;
								break;
							}
						}
				   if (*ptr == '>')
					ptr--;
				   break;				
				} 
				if (do_inner) {
					start = ptr;
					for (; *ptr != '<' && *ptr; *ptr++)
						/* Do nothing */ ;
					// innerData = (char *) malloc(sizeof(char) * (pos - start));
					 strncpy(innerData, start, ptr - start);
					 printf("Inner Data: %s\n", innerData);
					 //clear out shit here so we dont go junking up our memory.
					 memset(innerData,'\0',sizeof(innerData));
								
					if (*ptr == '<')
						ptr--;
									
				}				
				break;
		}
	}				
	return 1;
}





