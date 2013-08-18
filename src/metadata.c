/*
 *                OMEGA IRC SECURITY SERVICES
 *                  (C) 2008-2012 Omega Dev Team
 *
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
 *    $Id: metadata.c 2366 2012-02-27 05:38:02Z twitch $
 */


#include "stdinc.h"
#include "server.h"


int add_metadata_channel(Channel *c, char *key, char *value)  {
        Metadata *md;
        dlink_node *dl; 
         if (!*key)
                return 0;

        DLINK_FOREACH(dl, c->metadata.head) {
                md = dl->data;
                if (strcasecmp(md->key, key)==0)
                {
                
                        if (!*value) {
                                dlink_delete(dl, &c->metadata);
                                dlink_free(dl);
                                free(md);
                                return 1;
                        }       
                        strncpy(md->value, value, sizeof(md->value));
                        return 0;
                }
        }
        if (!(md = (Metadata*) malloc(sizeof(Metadata))))
                        return 0;

        strncpy(md->key, key, sizeof(md->key));
        strncpy(md->value, value, sizeof(md->value));

        dl = dlink_create();

        dlink_add_tail(md, dl, &c->metadata);

	return 0;        
} 

Metadata *get_metadata_user(User *usr, char *key) {
        Metadata *md;
        dlink_node *dl;

        DLINK_FOREACH(dl, usr->metadata.head) {
                md = dl->data;
                if (strcasecmp(md->key, key)==0)
                        return md; 
        }
        return NULL;
}

int add_metadata_user(User *usr, char *key, char *value)  {
        Metadata *md;
        dlink_node *dl,*tdl; 
         if (!*key)
                return 0;

        DLINK_FOREACH_SAFE(dl,tdl, usr->metadata.head) {
                md = dl->data;
                if (strcasecmp(md->key, key)==0)
                {
                        if (!*value) {
                                dlink_delete(dl, &usr->metadata);
                                dlink_free(dl);
                                free(md);
                                return 1;
                        }       
                        strncpy(md->value, value, sizeof(md->value));
                        return 0;
                }
        }

        if (!(md = (Metadata*) malloc(sizeof(Metadata))))
                        return 0;

        strncpy(md->key, key, sizeof(md->key));
        strncpy(md->value, value, sizeof(md->value));

        dl = dlink_create();

        dlink_add_tail(md, dl, &usr->metadata);

	return 0; 
} 
  
