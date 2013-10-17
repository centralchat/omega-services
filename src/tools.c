#include "stdinclude.h"
#include "appinclude.h"

uint32 hash(char *item)
{

  uint32  hashstr, i;
  hashstr = 0; i = 0;
  for (hashstr=0, i=0; i < strlen(item); i++)
  {
    hashstr += tolower(item[i]);
    hashstr += (hashstr << 10);
    hashstr ^= (hashstr >> 5);
  }

  hashstr += (hashstr << 3);
  hashstr ^= (hashstr >> 11);
  hashstr += (hashstr << 15);

  return hashstr;

}


//aim to protect against poor hash table logic.
//borrowed from java hash table source - Twitch
unsigned int hash_safe(char *item, ub4 len)
{
    unsigned int i = (unsigned int) hash(item);
    i += ~(i << 9);
    i ^=  ((i >> 14) | (i << 18)); /* >>> */
    i +=  (i << 4);
    i ^=  ((i >> 10) | (i << 22)); /* >>> */
    return (unsigned int) (i % len);

}

int generictoken(char delim, int size, char *message, char **parv)
{
  char *next;
  int count;

  if (!message)
  {
    /* Something is seriously wrong...bail */
    parv[0] = NULL;
    return 0;
  }

  /* Now we take the beginning of message and find all the spaces...
  ** set them to \0 and use 'next' to go through the string
  */
  next = message;
  parv[0] = next;
  count = 1;

  while (*next)
  {
    /* This is fine here, since we don't have a :delimited
    ** parameter like tokenize
    */
    if (count == size)
      return count;
   
 
    if (*next == delim)
    {
      *next = '\0';
      next++;
      /* Eat any additional delimiters */
      while (*next == delim)
        next++;
      while (*next == ' ')
        next++; //eat whitespaces  
      /* If it's the end of the string, it's simply
      ** an extra space at the end.  Here we break.
      */
      if (*next == '\0')
        break;
      parv[count] = next;
      count++;
    }
    else
    {
      next++;
    }
  }

  return count;
}

