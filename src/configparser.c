#include "stdinclude.h"
#include "appinclude.h"
#include "configparser.h"

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
	//make sure we keep a stack of what files we are including because we don't want to
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
	int  line = 0; 
	int in_comment = 0;
	int index = 0;
	int skip = 0;
        int open_tag = 0; 	
	int start_line;
	char *start, *pos;
	char tmp[2446];
	char list[2446];
	int to_include = 0;
	
	char file[255];
	
	ConfBase *cb;
	
	dlink_node *dl;
	
	line = 1;
	
	strncpy(file,current_file,255); 
	for (ptr = buffer; *ptr; ptr++)
	{
	
		switch (*ptr)
		{
			case '\n':
					line++;
					break;
			case '<':
					//directive opening
					open_tag = 1;
					start_line = line;
					memset(tmp, 0, sizeof(tmp));
					ptr++;
					start = ptr;
					//nuke out shit we don't need and advance token to the nearest space - if you own it you can purchase it
					//also if its a > pay owner 10x's the amount shown on the card.
					for (;*ptr && *ptr != '>';ptr++) {		
						if (*ptr == '?')  
							continue;
						if ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\n') || (*ptr == '>'))
							break;
						if (!(*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <='Z') || (*ptr >= '0' && *ptr <= '9') || *ptr == '_')
						{
							printf("Invalid characters in directive starting in [%s:%d]\n", file, start_line);
							skip = 1; break;
						}
					}
					//invalid character - skip this tag its junk
					if (skip) { skip = 0; break; }
						
					//we hit newline 
					if (*ptr == '\n') { line++; ptr++; } //new line skip, nuke, and continue passed it
					if (*ptr == '>') { ptr--; break; } //this tag is fucked bail
					
					strncpy(tmp, start, ptr - start);
					
					if (strcasecmp(tmp,"include")==0) 
							to_include = 1;
					if (strcasecmp(tmp, "die")==0) {
						printf("Reached die tag in config file, please edit your config files.");
						exit(0);
					} 
					index = (int) HASH(tmp, HASH_BY);
					cb = (ConfBase*) malloc(sizeof(ConfBase));
					memset(cb->map,0,sizeof(cb->map));
					break;
			case '>':
					if (to_include) {
					    open_tag = 0;
						to_include = 0;
						if (cb)
							free(cb);
						break;
					}
					if (!open_tag) {
						printf("Ignoring extra '>' on [%s:%d]\n", file, line);
						break;
					}
					open_tag = 0;
					dl = dlink_create();
					//this is so we don't have to loop through EVERYTHING.
					config_tree_entries++;
					config_tree_index[config_tree_entries] = index;
					dlink_add_tail(cb,dl,&config_tree[index]); //add our base into the tree
					break;
			case '#':
					//jump to the end of the line since # is a 1 line comment
					while (*(ptr + 1) != '\n')
							ptr++;
			case '/':
					//nook out any and all information between the comments as we really honestly don't need them.
					start_line = line;
					if (*(ptr + 1) == '/')
						for (;*ptr && *(ptr+1) != '\n';ptr++) 
							/** Do nothing **/ ;
					
                    if ((*(ptr-1)=='*') && (in_comment)) { 
                         in_comment = 0; 
                         break;                    
                    }	
                    
					if (*(ptr + 1) == '*')
					{
						for (;*ptr;ptr++) {
							in_comment = 1;
							
							if (*ptr == '\n')
								line++;
								
							if ((*ptr == '*') && (*(ptr + 1) == '/')) {
									in_comment = 0;
									ptr++; //nuke out the trailing /
									break;
							}
						}
					} 
					break;
			//skip these symbols we don't use them
			case '\t':
			case ' ':
			case '=':
			case '\r':
				break;
			default:
                    if (in_comment)
                         break;
					//we aren't in a tag - nook out garbage information and spit out an error
				    if (!open_tag) {
							printf("Found stray information not inside a directive tag in [%s:%d]\n", file, line);
							while (((*(ptr + 1) != '<') || (*(ptr + 1) != '/')) && (*ptr))
							{
									if (*ptr == '\n')
										line++;
								ptr++;
							}
							break;
					}
					//we are in side a tag lets parse this information that we need	
					for (;*ptr;ptr++)
					{
					
						if ((*ptr == '\t') || (*ptr == '\n') || (*ptr == ' '))
							ptr++;
							
						if (*ptr == '\n') { line++; ptr++; }
						if (*ptr == '>') { ptr--; break; }	
						
						start = ptr;
						/*
						 * get information starting at first constant after a space
						 * to the first appearance of a = this specifies end of our key
						 * if there is no = we will assume its all values and the VALUES
						 * will never be obtainable MUAHAHAHAH
						 */
						if ((pos = strchr(ptr, '='))) {
                            if ((pos - start) > sizeof(list))
                            {
                               printf("Buffer overflow detected: %s:%d\n", file, line);
                               printf("   Buffer: \n %s", start);
                               core_exit(0);
                            }
							strncpy(list, start, pos - start);
							
							if (*pos == ' ')
								pos--;
							ptr = pos++;
						}		
						/* we probably don't need this loop since we do alot w/o it 
						 * but in case we don't account for something user-side lets
						 * keep it
						 */
						for (;*ptr; ptr++)
						{
							if (*ptr == '=')
								ptr++;
								
							if ((*ptr == '\'') || (*ptr == '"')) {
								ptr++;
								start = ptr;
								//check for both ' and " in to allow users to use both (tho they should only be using " imo
								if (!(pos = strchr(ptr, '"'))) {
									if (!(pos = strchr(ptr, '\''))) {
										 printf("Unterminated string starting at [%s:%d]\n",file, line);
										 return 0;
									}
								}
								//in case they want multi line directives we should probably nuke these out.
								while ((*start == '\t') || (*start == '\n') || (*start == ' '))
										start++;
																								
								strncpy(cb->map[HASH(list,HASH_B)], start, pos - start);
								//we are in an include directive - so include the file
								//don't worry we wont save this in our tree (deleted at end of tag)
								if (to_include)
								{	
									if (strcasecmp(list,"file")==0)
										config_load (cb->map[HASH(list,HASH_B)]);
									ptr = pos;
									break;
								}
								//clear out shit here so we don't go junking up our memory.
								memset(list,0,sizeof(list));
								cb->entry_cnt++;
								ptr = pos;
								break;
							}
						}
					}
					break;
		}
		
	}
	if (in_comment) {
			printf("Unterminated comment starting at [%s:%d]", file, start_line);
			return 0;
	}
	if (open_tag) {
			printf("No matching > for directive starting at [%s:%d]",file, start_line);
			return 0;
	}
	return 1;
}

/**
 ** Test configuration
 ** @return (int) 1 on success 
 **/
int config_test (char * config_file)
{
    int ret;

    ret = config_load (config_file);

    destroy_config_tree();

    if (ret != 1)
    {
        return 0;
    } else {
        return 1;
    }
}


dlink_list * get_config_base(char *index)
{
		int key = 0;
		key = HASH(index,HASH_BY);
		return &config_tree[key];
}

int get_config_list(char *index, char *sub, char delim, const char **list)
{
	char *ce = NULL, *p = NULL;
	int cnt = 0;
	char *buf;

	if (!delim)
		delim = ',';

	//printf("Delimiter is set to: %c\n", delim);
	if ((ce = (char *)get_config_entry(index, sub)))
	{
		buf = strdup(ce);
		//alog(LOG_DEBUG2, "get_config_list: ce => %s", ce);
		while (1) {
			
			if((p = strchr(buf, delim)) != NULL)
			{
				list[cnt++] = buf;
				list[cnt] = NULL;
				*p++ = '\0';
				//printf("list[%d]: %s\n", cnt-1,(char*) list[cnt-1]);
				buf = p;
			} else {
				list[cnt] = buf;
				//printf("list[%d]: %s\n", cnt,(char*) list[cnt]);
				break;
			}
		}
		return (cnt)? cnt+1 : 0;
	}
	return cnt;
}

char * get_config_entry(char *index, char *sub)
{
	int key = 0;
	int sub_key = 0;
	
	dlink_node *dl,*sdl;
	ConfBase *cb;
	ConfEntry *ce;	
	if (!index)
			return NULL; 
	
	key = HASH(index,HASH_BY); //ugly but it works
	sub_key = HASH(sub,HASH_B); //ugly but it works 
	DLINK_FOREACH(dl,config_tree[key].head)
	{
			cb = dl->data;
			if (!cb->map[sub_key][0])
				continue;
			return cb->map[sub_key];
	}
	return NULL;
}


char * get_config_entry_by_cb(ConfBase *cb, char *sub)
{
	dlink_node *dl,*sdl;
	ConfEntry *ce;	
	if (!sub || !cb)
		return NULL; 
	int key = HASH(sub,HASH_B); //ugly but it works 
	return cb->map[key];
}


int get_config_bool(char *index, char *sub, int def) {
	int key = 0;
	int sub_hash = 0;
	dlink_node *dl;
	ConfBase *cb;
	char *ce;
	key = HASH(index,HASH_BY);
	sub_hash = HASH(sub,HASH_B);
	
	DLINK_FOREACH(dl,config_tree[key].head)
	{
			cb = dl->data;
			ce = cb->map[sub_hash];
			if (*ce)
			{
				if ((*ce == 'y') || (*ce == 'Y') || (*ce == '1'))
					return 1;
				else 
					return 0;
			} else
				return def;
	}
	return 0;
}

int get_config_int(char *index, char *sub, int def) {
	char * ce; 
	if ((ce = get_config_entry(index, sub)))
		return atoi(ce);
	return def;
}



int destroy_config_tree()
{
	int cnt;
	int index;
	dlink_node *dl,*tdl;
	dlink_node *ce_dl,*ce_tdl;
	
	ConfBase *cb;
	ConfEntry *ce;
		
	for (cnt = 0; cnt <= HASH_BY; cnt++)
	{
		DLINK_FOREACH_SAFE(dl,tdl,config_tree[cnt].head)
		{
				cb = dl->data;
				dlink_delete(dl,&config_tree[cnt]);
				free(cb);		
		}
		config_tree_index[cnt] = 0;
	}
	config_tree_entries = 0;
	memset(include_stack,0,sizeof(include_stack));
	return 1;
}


