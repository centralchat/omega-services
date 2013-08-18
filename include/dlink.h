/* Sentinel - IRC Statistical and Operator Services
** dlink.h - Hybrid/Squid derived doubly linked list library
**
** Copyright W. Campbell and others.  See README for more details
** Some code Copyright: Jonathan George, Kai Seidler, ircd-hybrid Team,
**                      IRCnet IRCD developers.
**
** $Id: dlink.h 1545 2009-12-05 08:15:05Z twitch $
*/

/* Original Header: */

/* #Id: tools.h,v 1.5 2002/09/11 17:55:38 db Exp # */
#ifndef __TOOLS_H
#define __TOOLS_H

typedef struct _dlink_node dlink_node;
typedef struct _dlink_list dlink_list;

struct _dlink_node
{
  void *data;
  dlink_node *next;
  dlink_node *prev;
};

struct _dlink_list
{
  dlink_node *head;
  dlink_node *tail;
  int count;
};

extern size_t dlink_alloc;
extern int dlink_count;

dlink_node *dlink_create(void);
void dlink_free(dlink_node *m);

/* These macros are basically swiped from the linux kernel
 * they are simple yet effective
 */

/*
 * Walks forward of a list.  
 * pos is your node
 * head is your list head
 */
#define DLINK_FOREACH(pos, head) for (pos = (head); pos != NULL; pos = pos->next)

/*
 * Walks forward of a list safely while removing nodes 
 * pos is your node
 * n is another list head for temporary storage
 * head is your list head
 */
#define DLINK_FOREACH_SAFE(pos, n, head) for (pos = (head), n = pos ? pos->next : NULL; pos != NULL; pos = n, n = pos ? pos->next : NULL)

#define DLINK_FOREACH_PREV(pos, head) for (pos = (head); pos != NULL; pos = pos->prev)



void dlink_add(void *data, dlink_node *m, dlink_list *list);
void dlink_add_before(dlink_node *b, void *data, dlink_node *m, dlink_list *list);
void dlink_add_tail(void *data, dlink_node *m, dlink_list *list);
void dlink_delete(dlink_node *m, dlink_list *list);
dlink_node *dlink_find(void *data, dlink_list *list);
dlink_node *dlink_find_delete(void *data, dlink_list *list);
int dlink_length(dlink_list *list);

#endif

