/* $Id$
 * vis functions - visually encode characters
 * Originally from OpenBSD
 */

/*-
 * Copyright (c) 1989, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef HAVE_STRVIS
#include "vis.h"
/*
 * vis - visually encode characters
 */
char *
vis(dst, c, flag, nextc)
        register char *dst;
        int c, nextc;
        register int flag;
{
        if (isvisible(c)) {
                *dst++ = c;
                if (c == '\\' && (flag & VIS_NOSLASH) == 0)
                        *dst++ = '\\';
                *dst = '\0';
                return (dst);
        }

        if (flag & VIS_CSTYLE) {
                switch(c) {
                case '\n':
                        *dst++ = '\\';
                        *dst++ = 'n';
                        goto done;
                case '\r':
                        *dst++ = '\\';
                        *dst++ = 'r';
                        goto done;
                case '\b':
                        *dst++ = '\\';
                        *dst++ = 'b';
                        goto done;
#ifdef __STDC__
                case '\a':
#else
                case '\007':
#endif
                        *dst++ = '\\';
                        *dst++ = 'a';
                        goto done;
                case '\v':
                        *dst++ = '\\';
                        *dst++ = 'v';
                        goto done;
                case '\t':
                        *dst++ = '\\';
                        *dst++ = 't';
                        goto done;
                case '\f':
                        *dst++ = '\\';
                        *dst++ = 'f';
                        goto done;
                case ' ':
                        *dst++ = '\\';
                        *dst++ = 's';
                        goto done;
                case '\0':
                        *dst++ = '\\';
                        *dst++ = '0';
                        if (isoctal(nextc)) {
                                *dst++ = '0';
                                *dst++ = '0';
                        }
                        goto done;
                }
        }
        if (((c & 0177) == ' ') || (flag & VIS_OCTAL)) {        
                *dst++ = '\\';
                *dst++ = ((u_char)c >> 6 & 07) + '0';
                *dst++ = ((u_char)c >> 3 & 07) + '0';
                *dst++ = ((u_char)c & 07) + '0';
                goto done;
        }
        if ((flag & VIS_NOSLASH) == 0)
                *dst++ = '\\';
        if (c & 0200) {
                c &= 0177;
                *dst++ = 'M';
        }
        if (iscntrl(c)) {
                *dst++ = '^';
                if (c == 0177)
                        *dst++ = '?';
                else
                        *dst++ = c + '@';
        } else {
                *dst++ = '-';
                *dst++ = c;
        }
done:
        *dst = '\0';
        return (dst);
}

/*
 * strvis, strnvis, strvisx - visually encode characters from src into dst
 *      
 *      Dst must be 4 times the size of src to account for possible
 *      expansion.  The length of dst, not including the trailing NULL,
 *      is returned. 
 *
 *      Strnvis will write no more than siz-1 bytes (and will NULL terminate).
 *      The number of bytes needed to fully encode the string is returned.
 *
 *      Strvisx encodes exactly len bytes from src into dst.
 *      This is useful for encoding a block of data.
 */
int
strvis(dst, src, flag)
        register char *dst;
        register const char *src;
        int flag;
{
        register char c;
        char *start;

        for (start = dst; (c = *src);)
                dst = vis(dst, c, flag, *++src);
        *dst = '\0';
        return (dst - start);
}

int
strnvis(dst, src, siz, flag)
        register char *dst;
        register const char *src;
        size_t siz;
        int flag;
{
        register char c;
        char *start, *end;

        for (start = dst, end = start + siz - 1; (c = *src) && dst < end; ) {
                if (isvisible(c)) {
                        *dst++ = c;
                        if (c == '\\' && (flag & VIS_NOSLASH) == 0) {
                                /* need space for the extra '\\' */
                                if (dst < end)
                                        *dst++ = '\\';
                                else {
                                        dst--;
                                        break;
                                }
                        }
                        src++;
                } else {
                        /* vis(3) requires up to 4 chars */
                        if (dst + 3 < end)
                                dst = vis(dst, c, flag, *++src);
                        else
                                break;
                }
        }
        *dst = '\0';
        if (dst >= end) {
                char tbuf[5];

                /* adjust return value for truncation */
                while ((c = *src))
                        dst += vis(tbuf, c, flag, *++src) - tbuf;
        }
        return (dst - start);
}

int
strvisx(dst, src, len, flag)
        register char *dst;
        register const char *src;
        register size_t len;
        int flag;
{
        register char c;
        char *start;

        for (start = dst; len > 1; len--) {
                c = *src;
                dst = vis(dst, c, flag, *++src);
        }
        if (len)
                dst = vis(dst, *src, flag, '\0');
        *dst = '\0';
        return (dst - start);
}

/*
 * unvis - decode characters previously encoded by vis
 */
int
#ifdef __STDC__
unvis(char *cp, char c, int *astate, int flag)
#else
unvis(cp, c, astate, flag)
        char *cp;
        char c;
        int *astate, flag;
#endif
{

        if (flag & UNVIS_END) {
                if (*astate == S_OCTAL2 || *astate == S_OCTAL3) {
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                } 
                return (*astate == S_GROUND ? UNVIS_NOCHAR : UNVIS_SYNBAD);
        }

        switch (*astate) {

        case S_GROUND:
                *cp = 0;
                if (c == '\\') {
                        *astate = S_START;
                        return (0);
                } 
                *cp = c;
                return (UNVIS_VALID);

        case S_START:
                switch(c) {
                case '\\':
                        *cp = c;
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                        *cp = (c - '0');
                        *astate = S_OCTAL2;
                        return (0);
                case 'M':
                        *cp = (char) 0200;
                        *astate = S_META;
                        return (0);
                case '^':
                        *astate = S_CTRL;
                        return (0);
                case 'n':
                        *cp = '\n';
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case 'r':
                        *cp = '\r';
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case 'b':
                        *cp = '\b';
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case 'a':
                        *cp = '\007';
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case 'v':
                        *cp = '\v';
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case 't':
                        *cp = '\t';
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case 'f':
                        *cp = '\f';
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case 's':
                        *cp = ' ';
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case 'E':
                        *cp = '\033';
                        *astate = S_GROUND;
                        return (UNVIS_VALID);
                case '\n':
                        /*
                         * hidden newline
                         */
                        *astate = S_GROUND;
                        return (UNVIS_NOCHAR);
                case '$':
                        /*
                         * hidden marker
                         */
                        *astate = S_GROUND;
                        return (UNVIS_NOCHAR);
                }
                *astate = S_GROUND;
                return (UNVIS_SYNBAD);
                 
        case S_META:
                if (c == '-')
                        *astate = S_META1;
                else if (c == '^')
                        *astate = S_CTRL;
                else {
                        *astate = S_GROUND;
                        return (UNVIS_SYNBAD);
                }
                return (0);
                 
        case S_META1:
                *astate = S_GROUND;
                *cp |= c;
                return (UNVIS_VALID);
                 
        case S_CTRL:
                if (c == '?')
                        *cp |= 0177;
                else
                        *cp |= c & 037;
                *astate = S_GROUND;
                return (UNVIS_VALID);

        case S_OCTAL2:        /* second possible octal digit */
                if (isoctal(c)) {
                        /* 
                         * yes - and maybe a third 
                         */
                        *cp = (*cp << 3) + (c - '0');
                        *astate = S_OCTAL3;        
                        return (0);
                } 
                /* 
                 * no - done with current sequence, push back passed char 
                 */
                *astate = S_GROUND;
                return (UNVIS_VALIDPUSH);

        case S_OCTAL3:        /* third possible octal digit */
                *astate = S_GROUND;
                if (isoctal(c)) {
                        *cp = (*cp << 3) + (c - '0');
                        return (UNVIS_VALID);
                }
                /*
                 * we were done, push back passed char
                 */
                return (UNVIS_VALIDPUSH);
                        
        default:        
                /* 
                 * decoder in unknown state - (probably uninitialized) 
                 */
                *astate = S_GROUND;
                return (UNVIS_SYNBAD);
        }
}

/*
 * strunvis - decode src into dst 
 *
 *      Number of chars decoded into dst is returned, -1 on error.
 *      Dst is null terminated.
 */

int
strunvis(dst, src)
        register char *dst;
        register const char *src;
{
        register char c;
        char *start = dst;
        int state = 0;

        while ((c = *src++)) {
        again:
                switch (unvis(dst, c, &state, 0)) {
                case UNVIS_VALID:
                        dst++;
                        break;
                case UNVIS_VALIDPUSH:
                        dst++;
                        goto again;
                case 0:
                case UNVIS_NOCHAR:
                        break;
                default:
                        return (-1);
                }
        }
        if (unvis(dst, c, &state, UNVIS_END) == UNVIS_VALID)
                dst++;
        *dst = '\0';
        return (dst - start);
}

#endif /* HAVE_STRVIS */
