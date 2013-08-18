/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-2012 Omega Dev Team
 *
 *    Previous Copyrights:
 *              Unreal Development Team 3.2 2007
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
 *    $Id: base64.c 2252 2012-01-22 09:21:23Z twitch $
 */


/*
   This is borrowed from Unreal
*/

#include "stdinc.h"
#include "server.h"

#define NUMNICKLOG 6
#define NUMNICKBASE (1 << NUMNICKLOG)
#define NUMNICKMASK (NUMNICKBASE - 1)

/* Yes, P10's encoding here is almost-but-not-quite MIME Base64.  Yay
 * for gratuitous incompatibilities. */
static const char convert2y[256] = {
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
  'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
  'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
  'w','x','y','z','0','1','2','3','4','5','6','7','8','9','[',']'
};

static const unsigned char convert2n[256] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  52,53,54,55,56,57,58,59,60,61, 0, 0, 0, 0, 0, 0,
   0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
  15,16,17,18,19,20,21,22,23,24,25,62, 0,63, 0, 0,
   0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
  41,42,43,44,45,46,47,48,49,50,51, 0, 0, 0, 0, 0
};


static char *int_to_base64(long);
static long base64_to_int(char *);

char *base64enc(long i)
{
    if (i < 0)
        return ("0");
    return int_to_base64(i);
}

long base64dec(char *b64)
{
    if (b64)
        return base64_to_int(b64);
    else
        return 0;
}


static const char Base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char Pad64 = '=';

/**

   (From RFC1521 and draft-ietf-dnssec-secext-03.txt)
   The following encoding technique is taken from RFC 1521 by Borenstein
   and Freed.  It is reproduced here in a slightly edited form for
   convenience.

   A 65-character subset of US-ASCII is used, enabling 6 bits to be
   represented per printable character. (The extra 65th character, "=",
   is used to signify a special processing function.)

   The encoding process represents 24-bit groups of input bits as output
   strings of 4 encoded characters. Proceeding from left to right, a
   24-bit input group is formed by concatenating 3 8-bit input groups.
   These 24 bits are then treated as 4 concatenated 6-bit groups, each
   of which is translated into a single digit in the base64 alphabet.

   Each 6-bit group is used as an index into an array of 64 printable
   characters. The character referenced by the index is placed in the
   output string.

                         Table 1: The Base64 Alphabet

      Value Encoding  Value Encoding  Value Encoding  Value Encoding
          0 A            17 R            34 i            51 z
          1 B            18 S            35 j            52 0
          2 C            19 T            36 k            53 1
          3 D            20 U            37 l            54 2
          4 E            21 V            38 m            55 3
          5 F            22 W            39 n            56 4
          6 G            23 X            40 o            57 5
          7 H            24 Y            41 p            58 6
          8 I            25 Z            42 q            59 7
          9 J            26 a            43 r            60 8
         10 K            27 b            44 s            61 9
         11 L            28 c            45 t            62 +
         12 M            29 d            46 u            63 /
         13 N            30 e            47 v
         14 O            31 f            48 w         (pad) =
         15 P            32 g            49 x
         16 Q            33 h            50 y

   Special processing is performed if fewer than 24 bits are available
   at the end of the data being encoded.  A full encoding quantum is
   always completed at the end of a quantity.  When fewer than 24 input
   bits are available in an input group, zero bits are added (on the
   right) to form an integral number of 6-bit groups.  Padding at the
   end of the data is performed using the '=' character.


   ':' and '#' and '&' and '+' and '@' must never be in this table.
    these tables must NEVER CHANGE!
*/

char int6_to_base64_map[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
    'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '{', '}'
};

char base64_to_int6_map[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1,
    -1, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, -1, 63, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};


/**
 *
 *  The following encoding technique is taken from RFC 1521 by Borenstein
 *  and Freed.  It is reproduced here in a slightly edited form for
 *  convenience. This function takes a char* and returns a base64
 *  encoded string.
 *
 * @param src A pointer to the string we are encoding.
 * @param srclength The length of the string we are to encode
 * @param target Destination of our encoded/end result
 * @param targsize Size of the end result.
 *
 *  @note
 *       Since all base64 input is an integral number of octets, only the
 *        -------------------------------------------------
 *
 *      (1) the final quantum of encoding input is an integral
 *          multiple of 24 bits; here, the final unit of encoded
 *	        output will be an integral multiple of 4 characters
 *	        with no "=" padding,
 *      (2) the final quantum of encoding input is exactly 8 bits;
 *          here, the final unit of encoded output will be two
 *	        characters followed by two "=" padding characters, or
 *      (3) the final quantum of encoding input is exactly 16 bits;
 *          here, the final unit of encoded output will be three
 *	        characters followed by one "=" padding character.
 *
 * @return the number of bytes encoded or -1 on fail.
 *
 */

int b64_encode(char *src, size_t srclength, char *target, size_t targsize)
{
    size_t datalength = 0;
    unsigned char input[3];
    unsigned char output[4];
    size_t i;

    while (2 < srclength) {
        input[0] = *src++;
        input[1] = *src++;
        input[2] = *src++;
        srclength -= 3;

        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);
        output[3] = input[2] & 0x3f;

        if (datalength + 4 > targsize)
            return (-1);
        target[datalength++] = Base64[output[0]];
        target[datalength++] = Base64[output[1]];
        target[datalength++] = Base64[output[2]];
        target[datalength++] = Base64[output[3]];
    }

    /* Now we worry about padding. */
    if (0 != srclength) {
        /* Get what's left. */
        input[0] = input[1] = input[2] = '\0';
        for (i = 0; i < srclength; i++)
            input[i] = *src++;

        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);

        if (datalength + 4 > targsize)
            return (-1);
        target[datalength++] = Base64[output[0]];
        target[datalength++] = Base64[output[1]];
        if (srclength == 1)
            target[datalength++] = Pad64;
        else
            target[datalength++] = Base64[output[2]];
        target[datalength++] = Pad64;
    }
    if (datalength >= targsize)
        return (-1);
    target[datalength] = '\0';  /* Returned value doesn't count \0. */
    return (datalength);
}


/**
 *  This function skips all whitespace anywhere. converts characters,
 *  four at a time, starting at (or after) src from base - 64 numbers into three
 *  8 bit bytes in the target area.
 *
 *  @param src String we are attempting to decode
 *  @param target Destination of the decoded string.
 *  @param targsize Size of target.
 *
 *  @return
 *    The number of data bytes stored at the target, or -1 on error.
 */

int b64_decode(char *src, char *target, size_t targsize)
{
    int tarindex, state, ch;
    char *pos;

    state = 0;
    tarindex = 0;

    while ((ch = *src++) != '\0') {
        if (isspace(ch))        /* Skip whitespace anywhere. */
            continue;

        if (ch == Pad64)
            break;

        pos = strchr(Base64, ch);
        if (pos == 0)           /* A non-base64 character. */
            return (-1);

        switch (state) {
        case 0:
            if (target) {
                if ((size_t) tarindex >= targsize)
                    return (-1);
                target[tarindex] = (pos - Base64) << 2;
            }
            state = 1;
            break;
        case 1:
            if (target) {
                if ((size_t) tarindex + 1 >= targsize)
                    return (-1);
                target[tarindex] |= (pos - Base64) >> 4;
                target[tarindex + 1] = ((pos - Base64) & 0x0f)
                    << 4;
            }
            tarindex++;
            state = 2;
            break;
        case 2:
            if (target) {
                if ((size_t) tarindex + 1 >= targsize)
                    return (-1);
                target[tarindex] |= (pos - Base64) >> 2;
                target[tarindex + 1] = ((pos - Base64) & 0x03)
                    << 6;
            }
            tarindex++;
            state = 3;
            break;
        case 3:
            if (target) {
                if ((size_t) tarindex >= targsize)
                    return (-1);
                target[tarindex] |= (pos - Base64);
            }
            tarindex++;
            state = 0;
            break;
        default:
            abort();
        }
    }

    /*
     * We are done decoding Base-64 chars.  Let's see if we ended
     * on a byte boundary, and/or with erroneous trailing characters.
     */

    if (ch == Pad64) {          /* We got a pad char. */
        ch = *src++;            /* Skip it, get next. */
        switch (state) {
        case 0:                /* Invalid = in first position */
        case 1:                /* Invalid = in second position */
            return (-1);

        case 2:                /* Valid, means one byte of info */
            /* Skip any number of spaces. */
            for ((void) NULL; ch != '\0'; ch = *src++)
                if (!isspace(ch))
                    break;
            /* Make sure there is another trailing = sign. */
            if (ch != Pad64)
                return (-1);
            ch = *src++;        /* Skip the = */
            /* Fall through to "single trailing =" case. */
            /* FALL THROUGH */

        case 3:                /* Valid, means two bytes of info */
            /*
             * We know this char is an =.  Is there anything but
             * whitespace after it?
             */
            for ((void) NULL; ch != '\0'; ch = *src++)
                if (!isspace(ch))
                    return (-1);

            /*
             * Now make sure for cases 2 and 3 that the "extra"
             * bits that slopped past the last full byte were
             * zeros.  If we don't check them, they become a
             * subliminal channel.
             */
            if (target && target[tarindex] != 0)
                return (-1);
        }
    } else {
        /*
         * We ended by seeing the end of the string.  Make sure we
         * have no partial bytes lying around.
         */
        if (state != 0)
            return (-1);
    }

    return (tarindex);
}


/**
 * Translate a standard IP (17bits) into base64 encrypted ip
 *
 *  @param ip unsigned char pointer to the IP we are encoding.
 *
 *  @return Return the end result or null if we fail for any reason.
 */


char *encode_ip(unsigned char *ip)
{
    static char buf[25];
    unsigned char *cp;
    struct in_addr ia;          /* For IPv4 */
    char *s_ip;                 /* Signed ip string */

    if (!ip)
        return "*";

    if (strchr((char *) ip, ':')) {
        return NULL;
    } else {
        s_ip = str_signed(ip);
        ia.s_addr = inet_addr(s_ip);
        cp = (unsigned char *) &ia.s_addr;
        b64_encode((char *) &cp, sizeof(struct in_addr), buf, 25);
    }
    return buf;
}

/**
 * Translate a standard IPv4 IP (17bits) into base64 encrypted ip
 *
 *  @param buf unsigned char pointer to the IP we are encoding.
 *
 *  @return Return s_addr of the corresponding buf/ip we are decoding
 *          or false if we fail or IP is IPv6
 */

int decode_ip(char *buf)
{
    int len = strlen(buf);
    char targ[25];
    struct in_addr ia;

    b64_decode(buf, targ, 25);
    ia = *(struct in_addr *) targ;
    if (len == 24) {            /* IPv6 */
        return 0;
    } else if (len == 8)        /* IPv4 */
        return ia.s_addr;
    else                        /* Error?? */
        return 0;
}


/**
 * Convert an int to base64
 * @param val the string we are converting
 * @return Always returns base64 version of val
 *
 * @note this doesn't return null under any circumstances
 *       but asserts to make sure we are within the boundaries
 *       of acceptable parameters.
 */


static char *int_to_base64(long val)
{
    /* 32/6 == max 6 bytes for representation,
     * +1 for the null, +1 for byte boundaries
     */
    static char base64buf[8];
    long i = 7;

    base64buf[i] = '\0';


	//instead of using an if assert for sanity :)
	assert(val < 2147483647L);

    do
    {
        base64buf[--i] = int6_to_base64_map[val & 63];

    }  while (val >>= 6);

    return base64buf + i;
}

/**
 * Convert a base64 expression to an integer
 *
 * @param b64 the string we are converting
 * @return return the base64 string in long form. If
 *        we don't get a good value for b64 return false.
 *
 *  @note There are no sanity checks imposed here, besides
 *        null value checking on b64, so v should always return
 *        something usable - at least that is what is assumed
 *        here.
 */


static long base64_to_int(char *b64)
{
    int v = base64_to_int6_map[(unsigned char) *b64++];

    if (!b64)
        return 0;

    while (*b64) {
        v <<= 6;
        v += base64_to_int6_map[(unsigned char) *b64++];
    }

    return v;
}

long base64dects(char *ts)
{
    char *token;
    char *params[10];

    long value;

    if (!ts) {
        return 0;
    }

    generictoken('!',10,ts,params);

    token = params[0];

    if (!token) {
        return strtoul(ts, NULL, 10);
    }
    value = base64dec(token);

    return value;
}

unsigned long int
base64toint(const char* s, int count)
{
    unsigned int i = 0;
    while (*s && count) {
        i = (i << NUMNICKLOG) + convert2n[(unsigned char)*s++];
        count--;
    }
    return i;
}

const char* inttobase64(char* buf, unsigned int v, unsigned int count)
{
  buf[count] = '\0';
  while (count > 0) {
      buf[--count] = convert2y[(unsigned char)(v & NUMNICKMASK)];
      v >>= NUMNICKLOG;
  }
  return buf;
}

