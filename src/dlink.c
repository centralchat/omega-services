/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-2010 Omega Dev Team
 * 
 *   Previous Copyrights:
 * 	Copyright (C) 2003 W. Campbell and others.  See README for more details
 *	Copyright (C) 2002 Hybrid Development Team.

 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 *
 *    $Id: dlink.c 2283 2012-01-28 01:57:29Z twitch $
 */


#include <stdlib.h>
#include "dlink.h"

size_t dlink_alloc = 0;
int dlink_count = 0;

dlink_node *
dlink_create(void)
{
  dlink_node *m;

  m = (dlink_node *) malloc(sizeof(dlink_node));
  m->data = NULL;
  m->next = NULL;
  m->prev = NULL;

  dlink_alloc += sizeof(dlink_node);
  dlink_count++;

  return m;
}

/* XXX - macro? */
void
dlink_free(dlink_node *m)
{
  dlink_alloc -= sizeof(dlink_node);
  dlink_count--;
  free(m);
}

void
dlink_add(void *data, dlink_node *m, dlink_list *list)
{
  m->data = data;
  m->next = list->head;
  m->prev = NULL;

  if (list->head != NULL)
    list->head->prev = m;
  else if (list->tail == NULL)
    list->tail = m;

  list->head = m;
  list->count++;
}

void
dlink_add_before(dlink_node * b, void *data, dlink_node * m, dlink_list * list)
{
  /* Shortcut - if its the first one, call dlinkAdd only */
  if (b == list->head)
  {
    dlink_add(data, m, list);
  }
  else
  {
    m->data = data;
    b->prev->next = m;
    m->prev = b->prev;
    b->prev = m;
    m->next = b;
    list->count++;
  }
}

void
dlink_add_tail(void *data, dlink_node *m, dlink_list *list)
{
  if (!m)
	return;

  m->data = data;
  m->next = NULL;
  m->prev = list->tail;
  
  if (list->head == NULL)
    list->head = m;
  else if (list->tail != NULL)
    list->tail->next = m;

  list->tail = m;
  list->count++;
}

void
dlink_delete(dlink_node *m, dlink_list *list)
{
  /* item is at head */
  if (m->prev == NULL)
    list->head = m->next;
  else
    m->prev->next = m->next;

  /* item is at tail */
  if (m->next == NULL)
    list->tail = m->prev;
  else
    m->next->prev = m->prev;

  list->count--;
}

dlink_node *
dlink_find(void *data, dlink_list *list)
{
  dlink_node *ptr;

  DLINK_FOREACH(ptr, list->head)
  {
    if (ptr->data == data)
      return ptr;
  }

  return NULL;
}

dlink_node *
dlink_find_delete(void *data, dlink_list *list)
{
  dlink_node *ptr;

  DLINK_FOREACH(ptr, list->head)
  {
    if (ptr->data != data)
      continue;

    if (ptr->next != NULL)
      ptr->next->prev = ptr->prev;
    else
      list->tail = ptr->prev;

    if (ptr->prev != NULL)
      ptr->prev->next = ptr->next;
    else
      list->head = ptr->next;

    ptr->next = ptr->prev = NULL;

    list->count--;

    return ptr;
  }

  return NULL;
}

int
dlink_length(dlink_list *list)
{
  return list->count;
}

