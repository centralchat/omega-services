/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-2012 Omega Dev Team
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
 *    $Id: match.c 2252 2012-01-22 09:21:23Z twitch $
 */

#include "stdinc.h"
#include "server.h"

#include <regex.h>



int match_regx(char* str,char* pattern)
{
	regex_t r;
       regmatch_t match[3];
	int status;

       if (!str || !pattern)
		return -1;

       regcomp(&r, pattern, REG_EXTENDED|REG_ICASE);
       status = regexec(&r,str,2,match,0);

       regfree(&r);

       return status;
}


/******************************************************/
/**
 * do_match_wild:  Attempt to match a string to a pattern which might contain
 *              '*' or '?' wildcards.  Return 1 if the string matches the
 *              pattern, 0 if not.
 * @param pattern To be matched
 * @param str String in which the pattern is to be matched
 * @param docase Case In/Sensitive
 * @return 1 if the string matches the pattern, 0 if not.
 *
 *     match_wild borrowed from Anope1.7.21
 */
 
 
int match_wild(const char *pattern, const char *str, int docase)
{
    char c;
    const char *s;

    if (!str || !*str || !pattern || !*pattern) {
        return 0;
    }

    /* This WILL eventually terminate: either by *pattern == 0, or by a
     * trailing '*'. */

    for (;;) {
        switch (c = *pattern++) {
        case 0:
            if (!*str)
                return 1;
            return 0;
        case '?':
            if (!*str)
                return 0;
            str++;
            break;
        case '*':
            if (!*pattern)
                return 1;       /* trailing '*' matches everything else */
            s = str;
            while (*s) {
                if ((docase ? (*s == *pattern)
                     : (tolower(*s) == tolower(*pattern)))
                    && match_wild(pattern, s, docase))
                    return 1;
                s++;
            }
            break;
        default:
            if (docase ? (*str++ != c) : (tolower(*str++) != tolower(c)))
                return 0;
            break;
        }                       /* switch */
    }
}

/** Test whether an address matches the most significant bits of a mask.
 * @param[in] addr Address to test.
 * @param[in] mask Address to test against.
 * @param[in] bits Number of bits to test.
 * @return 0 on mismatch, 1 if bits <= 128 and all bits match; -1 if
 * bits > 128 and all bits match.
 * 
 * Given to us by jobe.
 */

int match_cidr(unsigned char addr[16], unsigned char mask[16], unsigned char bits)
{
  int k;

  for (k = 0; k < 16; k++) {
    if (bits < 8)
      return !((addr[k] ^ mask[k]) >> (8-bits));
    if (addr[k] != mask[k])
      return 0;
    if (!(bits -= 8))
      return 1;
  }
  return -1;
}




