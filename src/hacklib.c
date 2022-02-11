/* NetHack 3.7	hacklib.c	$NHDT-Date: 1596498172 2020/08/03 23:42:52 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.85 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2007. */
/* Copyright (c) Robert Patrick Rankin, 1991                      */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h" /* for config.h+extern.h */
/*=
    Assorted 'small' utility routines.  They're virtually independent of
    NetHack, except that rounddiv may call panic().  setrandom calls one
    of srandom(), srand48(), or srand() depending upon configuration.

      return type     routine name    argument type(s)
        boolean         digit           (char)
        boolean         letter          (char)
        char            highc           (char)
        char            lowc            (char)
        char *          lcase           (char *)
        char *          ucase           (char *)
        char *          upstart         (char *)
        char *          mungspaces      (char *)
        char *          trimspaces      (char *)
        char *          strip_newline   (char *)
        char *          stripchars      (char *, const char *, const char *)
        char *          stripdigits     (char *)
        char *          eos             (char *)
        boolean         str_end_is      (const char *, const char *)
        int             str_lines_maxlen (const char *)
        char *          strkitten       (char *,char)
        void            copynchars      (char *,const char *,int)
        char            chrcasecpy      (int,int)
        char *          strcasecpy      (char *,const char *)
        char *          s_suffix        (const char *)
        char *          ing_suffix      (const char *)
        char *          xcrypt          (const char *, char *)
        boolean         onlyspace       (const char *)
        char *          tabexpand       (char *)
        char *          visctrl         (char)
        char *          strsubst        (char *, const char *, const char *)
        int             strNsubst       (char *,const char *,const char *,int)
        const char *    ordin           (int)
        char *          sitoa           (int)
        int             sgn             (int)
        int             rounddiv        (long, int)
        int             distmin         (int, int, int, int)
        int             dist2           (int, int, int, int)
        boolean         online2         (int, int)
        unsigned int    coord_hash      (int, int, int)
        unsigned int    hash1           (int)
        int             int_hash1       (int)
        boolean         pmatch          (const char *, const char *)
        boolean         pmatchi         (const char *, const char *)
        boolean         pmatchz         (const char *, const char *)
        int             strncmpi        (const char *, const char *, int)
        char *          strstri         (const char *, const char *)
        boolean         fuzzymatch      (const char *, const char *,
                                         const char *, boolean)
        void            setrandom       (void)
        void            init_random     (fn)
        void            reseed_random   (fn)
        time_t          getnow          (void)
        int             getyear         (void)
        char *          yymmdd          (time_t)
        long            yyyymmdd        (time_t)
        long            hhmmss          (time_t)
        char *          yyyymmddhhmmss  (time_t)
        time_t          time_from_yyyymmddhhmmss (char *)
        int             phase_of_the_moon (void)
        boolean         friday_13th     (void)
        int             night           (void)
        int             midnight        (void)
        int             weekday         (void)
        int             days_since_epoch (int)
        boolean         current_holidays (void)
        void            strbuf_init     (strbuf *, const char *)
        void            strbuf_append   (strbuf *, const char *)
        void            strbuf_reserve  (strbuf *, int)
        void            strbuf_empty    (strbuf *)
        void            strbuf_nl_to_crlf (strbuf_t *)
        char *          nonconst        (const char *, char *)
        int             swapbits        (int, int, int)
        void            shuffle_int_array (int *, int)
        void            nh_snprintf     (const char *, int, char *, size_t,
                                         const char *, ...)
=*/
#ifdef LINT
#define Static /* pacify lint */
#else
#define Static static
#endif

static boolean pmatch_internal(const char *, const char *, boolean,
                               const char *);
static int weekday(void);
static int days_since_epoch(int);

/* is 'c' a digit? */
boolean
digit(char c)
{
    return (boolean) ('0' <= c && c <= '9');
}

/* is 'c' a letter?  note: '@' classed as letter */
boolean
letter(char c)
{
    return (boolean) ('@' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

/* force 'c' into uppercase */
char
highc(char c)
{
    return (char) (('a' <= c && c <= 'z') ? (c & ~040) : c);
}

/* force 'c' into lowercase */
char
lowc(char c)
{
    return (char) (('A' <= c && c <= 'Z') ? (c | 040) : c);
}

/* convert a string into all lowercase */
char *
lcase(char *s)
{
    register char *p;

    for (p = s; *p; p++)
        if ('A' <= *p && *p <= 'Z')
            *p |= 040;
    return s;
}

/* convert a string into all uppercase */
char *
ucase(char *s)
{
    register char *p;

    for (p = s; *p; p++)
        if ('a' <= *p && *p <= 'z')
            *p &= ~040;
    return s;
}

/* convert first character of a string to uppercase */
char *
upstart(char *s)
{
    if (s)
        *s = highc(*s);
    return s;
}

/* capitalize first letters of every word in a string */
char *
up_all_words(char * s)
{
    char * p;
    boolean space = TRUE;
    for (p = s; *p; p++) {
        if (*p == ' ') {
            space = TRUE;
        }
        else if (space && letter(*p)) {
            *p = highc(*p);
            space = FALSE;
        }
        else {
            space = FALSE;
        }
    }
    return s;
}

/* remove excess whitespace from a string buffer (in place) */
char *
mungspaces(char *bp)
{
    register char c, *p, *p2;
    boolean was_space = TRUE;

    for (p = p2 = bp; (c = *p) != '\0'; p++) {
        if (c == '\n')
            break; /* treat newline the same as end-of-string */
        if (c == '\t')
            c = ' ';
        if (c != ' ' || !was_space)
            *p2++ = c;
        was_space = (c == ' ');
    }
    if (was_space && p2 > bp)
        p2--;
    *p2 = '\0';
    return bp;
}

/* skip leading whitespace; remove trailing whitespace, in place */
char *
trimspaces(char *txt)
{
    char *end;

    /* leading whitespace will remain in the buffer */
    while (*txt == ' ' || *txt == '\t')
        txt++;
    end = eos(txt);
    while (--end >= txt && (*end == ' ' || *end == '\t'))
        *end = '\0';

    return txt;
}

/* remove \n from end of line; remove \r too if one is there */
char *
strip_newline(char *str)
{
    char *p = rindex(str, '\n');

    if (p) {
        if (p > str && *(p - 1) == '\r')
            --p;
        *p = '\0';
    }
    return str;
}

/* return the end of a string (pointing at '\0') */
char *
eos(register char *s)
{
    while (*s)
        s++; /* s += strlen(s); */
    return s;
}

/* determine whether 'str' ends in 'chkstr' */
boolean
str_end_is(const char *str, const char *chkstr)
{
    int clen = (int) strlen(chkstr);

    if ((int) strlen(str) >= clen)
        return (boolean) (!strncmp(eos((char *) str) - clen, chkstr, clen));
    return FALSE;
}

/* return max line length from buffer comprising newline-separated strings */
int
str_lines_maxlen(const char *str)
{
    const char *s1, *s2;
    int len, max_len = 0;

    s1 = str;
    while (s1 && *s1) {
        s2 = index(s1, '\n');
        if (s2) {
            len = (int) (s2 - s1);
            s1 = s2 + 1;
        } else {
            len = (int) strlen(s1);
            s1 = (char *) 0;
        }
        if (len > max_len)
            max_len = len;
    }

    return max_len;
}

/* append a character to a string (in place): strcat(s, {c,'\0'}); */
char *
strkitten(char *s, char c)
{
    char *p = eos(s);

    *p++ = c;
    *p = '\0';
    return s;
}

/* truncating string copy */
void
copynchars(char *dst, const char *src, int n)
{
    /* copies at most n characters, stopping sooner if terminator reached;
       treats newline as input terminator; unlike strncpy, always supplies
       '\0' terminator so dst must be able to hold at least n+1 characters */
    while (n > 0 && *src != '\0' && *src != '\n') {
        *dst++ = *src++;
        --n;
    }
    *dst = '\0';
}

/* convert char nc into oc's case; mostly used by strcasecpy */
char
chrcasecpy(int oc, int nc)
{
#if 0 /* this will be necessary if we switch to <ctype.h> */
    oc = (int) (unsigned char) oc;
    nc = (int) (unsigned char) nc;
#endif
    if ('a' <= oc && oc <= 'z') {
        /* old char is lower case; if new char is upper case, downcase it */
        if ('A' <= nc && nc <= 'Z')
            nc += 'a' - 'A'; /* lowc(nc) */
    } else if ('A' <= oc && oc <= 'Z') {
        /* old char is upper case; if new char is lower case, upcase it */
        if ('a' <= nc && nc <= 'z')
            nc += 'A' - 'a'; /* highc(nc) */
    }
    return (char) nc;
}

/* overwrite string, preserving old chars' case;
   for case-insensitive editions of makeplural() and makesingular();
   src might be shorter, same length, or longer than dst */
char *
strcasecpy(char *dst, const char *src)
{
    char *result = dst;
    int ic, oc, dst_exhausted = 0;

    /* while dst has characters, replace each one with corresponding
       character from src, converting case in the process if they differ;
       once dst runs out, propagate the case of its last character to any
       remaining src; if dst starts empty, it must be a pointer to the
       tail of some other string because we examine the char at dst[-1] */
    while ((ic = (int) *src++) != '\0') {
        if (!dst_exhausted && !*dst)
            dst_exhausted = 1;
        oc = (int) *(dst - dst_exhausted);
        *dst++ = chrcasecpy(oc, ic);
    }
    *dst = '\0';
    return result;
}

/* return a name converted to possessive */
char *
s_suffix(const char *s)
{
    Static char buf[BUFSZ];

    Strcpy(buf, s);
    if (!strcmpi(buf, "it")) /* it -> its */
        Strcat(buf, "s");
    else if (!strcmpi(buf, "you")) /* you -> your */
        Strcat(buf, "r");
    else if (*(eos(buf) - 1) == 's') /* Xs -> Xs' */
        Strcat(buf, "'");
    else /* X -> X's */
        Strcat(buf, "'s");
    return buf;
}

/* construct a gerund (a verb formed by appending "ing" to a noun) */
char *
ing_suffix(const char *s)
{
    static const char vowel[] = "aeiouwy";
    static char buf[BUFSZ];
    char onoff[10];
    char *p;

    Strcpy(buf, s);
    p = eos(buf);
    onoff[0] = *p = *(p + 1) = '\0';
    if ((p >= &buf[3] && !strcmpi(p - 3, " on"))
        || (p >= &buf[4] && !strcmpi(p - 4, " off"))
        || (p >= &buf[5] && !strcmpi(p - 5, " with"))) {
        p = rindex(buf, ' ');
        Strcpy(onoff, p);
        *p = '\0';
    }
    if (p >= &buf[3] && !index(vowel, *(p - 1))
        && index(vowel, *(p - 2)) && !index(vowel, *(p - 3))) {
        /* tip -> tipp + ing */
        *p = *(p - 1);
        *(p + 1) = '\0';
    } else if (p >= &buf[2] && !strcmpi(p - 2, "ie")) { /* vie -> vy + ing */
        *(p - 2) = 'y';
        *(p - 1) = '\0';
    } else if (p >= &buf[1] && *(p - 1) == 'e') /* grease -> greas + ing */
        *(p - 1) = '\0';
    Strcat(buf, "ing");
    if (onoff[0])
        Strcat(buf, onoff);
    return buf;
}

/* trivial text encryption routine (see makedefs) */
char *
xcrypt(const char *str, char *buf)
{
    register const char *p;
    register char *q;
    register int bitmask;

    for (bitmask = 1, p = str, q = buf; *p; q++) {
        *q = *p++;
        if (*q & (32 | 64))
            *q ^= bitmask;
        if ((bitmask <<= 1) >= 32)
            bitmask = 1;
    }
    *q = '\0';
    return buf;
}

/* is a string entirely whitespace? */
boolean
onlyspace(const char *s)
{
    for (; *s; s++)
        if (*s != ' ' && *s != '\t')
            return FALSE;
    return TRUE;
}

/* expand tabs into proper number of spaces (in place) */
char *
tabexpand(
    char *sbuf) /* assumed to be [BUFSZ] but can be smaller provided that
                 * expanded string fits; expansion bigger than BUFSZ-1
                 * will be truncated */
{
    char buf[BUFSZ + 10];
    register char *bp, *s = sbuf;
    register int idx;

    if (!*s)
        return sbuf;
    for (bp = buf, idx = 0; *s; s++) {
        if (*s == '\t') {
            /*
             * clang-8's optimizer at -Os has been observed to mis-compile
             * this code.  Symptom is nethack getting stuck in an apparent
             * infinite loop (or perhaps just an extremely long one) when
             * examining data.base entries.
             * clang-9 doesn't exhibit this problem.  [Was the incorrect
             * optimization fixed or just disabled?]
             */
            do
                *bp++ = ' ';
            while (++idx % 8);
        } else {
            *bp++ = *s;
            ++idx;
        }
        if (idx >= BUFSZ) {
            bp = &buf[BUFSZ - 1];
            break;
        }
    }
    *bp = 0;
    return strcpy(sbuf, buf);
}

#define VISCTRL_NBUF 5
/* make a displayable string from a character */
char *
visctrl(char c)
{
    Static char visctrl_bufs[VISCTRL_NBUF][5];
    static int nbuf = 0;
    register int i = 0;
    char *ccc = visctrl_bufs[nbuf];
    nbuf = (nbuf + 1) % VISCTRL_NBUF;

    if ((uchar) c & 0200) {
        ccc[i++] = 'M';
        ccc[i++] = '-';
    }
    c &= 0177;
    if (c < 040) {
        ccc[i++] = '^';
        ccc[i++] = c | 0100; /* letter */
    } else if (c == 0177) {
        ccc[i++] = '^';
        ccc[i++] = c & ~0100; /* '?' */
    } else {
        ccc[i++] = c; /* printable character */
    }
    ccc[i] = '\0';
    return ccc;
}

/* strip all the chars in stuff_to_strip from orig */
/* caller is responsible for ensuring that bp is a
   valid pointer to a BUFSZ buffer */
char *
stripchars(char *bp, const char *stuff_to_strip, const char *orig)
{
    int i = 0;
    char *s = bp;

    if (s) {
        while (*orig && i < (BUFSZ - 1)) {
            if (!index(stuff_to_strip, *orig)) {
                *s++ = *orig;
                i++;
            }
            orig++;
        }
        *s = '\0';
    } else
        impossible("no output buf in stripchars");
    return bp;
}

/* remove digits from string */
char *
stripdigits(char *s)
{
    char *s1, *s2;

    for (s1 = s2 = s; *s1; s1++)
        if (*s1 < '0' || *s1 > '9')
            *s2++ = *s1;
    *s2 = '\0';

    return s;
}

/* substitute a word or phrase in a string (in place) */
/* caller is responsible for ensuring that bp points to big enough buffer */
char *
strsubst(char *bp, const char *orig, const char *replacement)
{
    char *found, buf[BUFSZ];

    if (bp) {
        /* [this could be replaced by strNsubst(bp, orig, replacement, 1)] */
        found = strstr(bp, orig);
        if (found) {
            Strcpy(buf, found + strlen(orig));
            Strcpy(found, replacement);
            Strcat(bp, buf);
        }
    }
    return bp;
}

/* substitute the Nth occurrence of a substring within a string (in place);
   if N is 0, substitute all occurrences; returns the number of subsitutions;
   maximum output length is BUFSZ (BUFSZ-1 chars + terminating '\0') */
int
strNsubst(
    char *inoutbuf,   /* current string, and result buffer */
    const char *orig, /* old substring; if "", insert in front of Nth char */
    const char *replacement, /* new substring; if "", delete old substring */
    int n) /* which occurrence to replace; 0 => all */
{
    char *bp, *op, workbuf[BUFSZ];
    const char *rp;
    unsigned len = (unsigned) strlen(orig);
    int ocount = 0, /* number of times 'orig' has been matched */
        rcount = 0; /* number of subsitutions made */

    for (bp = inoutbuf, op = workbuf; *bp && op < &workbuf[BUFSZ - 1]; ) {
        if ((!len || !strncmp(bp, orig, len)) && (++ocount == n || n == 0)) {
            /* Nth match found */
            for (rp = replacement; *rp && op < &workbuf[BUFSZ - 1]; )
                *op++ = *rp++;
            ++rcount;
            if (len) {
                bp += len; /* skip 'orig' */
                continue;
            }
        }
        /* no match (or len==0) so retain current character */
        *op++ = *bp++;
    }
    if (!len && n == ocount + 1) {
        /* special case: orig=="" (!len) and n==strlen(inoutbuf)+1,
           insert in front of terminator (in other words, append);
           [when orig=="", ocount will have been incremented once for
           each input char] */
        for (rp = replacement; *rp && op < &workbuf[BUFSZ - 1]; )
            *op++ = *rp++;
        ++rcount;
    }
    if (rcount) {
        *op = '\0';
        Strcpy(inoutbuf, workbuf);
    }
    return rcount;
}

/* return the ordinal suffix of a number */
const char *
ordin(int n)               /* note: should be non-negative */
{
    register int dd = n % 10;

    return (dd == 0 || dd > 3 || (n % 100) / 10 == 1) ? "th"
               : (dd == 1) ? "st" : (dd == 2) ? "nd" : "rd";
}

DISABLE_WARNING_FORMAT_NONLITERAL  /* one compiler complains about
                                      result of ?: for format string */

/* make a signed digit string from a number */
char *
sitoa(int n)
{
    Static char buf[13];

    Sprintf(buf, (n < 0) ? "%d" : "+%d", n);
    return buf;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* return the sign of a number: -1, 0, or 1 */
int
sgn(int n)
{
    return (n < 0) ? -1 : (n != 0);
}

/* calculate x/y, rounding as appropriate */
int
rounddiv(long x, int y)
{
    int r, m;
    int divsgn = 1;

    if (y == 0)
        panic("division by zero in rounddiv");
    else if (y < 0) {
        divsgn = -divsgn;
        y = -y;
    }
    if (x < 0) {
        divsgn = -divsgn;
        x = -x;
    }
    r = x / y;
    m = x % y;
    if (2 * m >= y)
        r++;

    return divsgn * r;
}

/* distance between two points, in moves */
int
distmin(int x0, int y0, int x1, int y1)
{
    register int dx = x0 - x1, dy = y0 - y1;

    if (dx < 0)
        dx = -dx;
    if (dy < 0)
        dy = -dy;
    /*  The minimum number of moves to get from (x0,y0) to (x1,y1) is the
     *  larger of the [absolute value of the] two deltas.
     */
    return (dx < dy) ? dy : dx;
}

/* square of euclidean distance between pair of pts */
int
dist2(int x0, int y0, int x1, int y1)
{
    register int dx = x0 - x1, dy = y0 - y1;

    return dx * dx + dy * dy;
}

/* integer square root function without using floating point */
int
isqrt(int val)
{
    int rt = 0;
    int odd = 1;
    /*
     * This could be replaced by a faster algorithm, but has not been because:
     * + the simple algorithm is easy to read;
     * + this algorithm does not require 64-bit support;
     * + in current usage, the values passed to isqrt() are not really that
     *   large, so the performance difference is negligible;
     * + isqrt() is used in only few places, which are not bottle-necks.
     */
    while (val >= odd) {
        val = val - odd;
        odd = odd + 2;
        rt = rt + 1;
    }
    return rt;
}

/* are two points lined up (on a straight line)? */
boolean
online2(int x0, int y0, int x1, int y1)
{
    int dx = x0 - x1, dy = y0 - y1;
    /*  If either delta is zero then they're on an orthogonal line,
     *  else if the deltas are equal (signs ignored) they're on a diagonal.
     */
    return (boolean) (!dy || !dx || dy == dx || dy == -dx);
}

/* Deterministic hash of three coordinates (intended to be x, y, and z, but
 * they don't actually have to be). In a lot of cases, z should probably also
 * be ledger_no(&u.uz) so that the "z" is actually unique among levels; mere
 * depth is not unique due to having levels in multiple branches at the same
 * depth.
 * Throws ubirthday and sysopt.serverseed into the hash so that the hash should
 * be (practically) unique among the same coordinates in different games, so the
 * player shouldn't be able to get the result out of the visible game state.
 * (Note that sysopt.serverseed is the value of SERVERSEED plus a random number
 * generated at game start).
 */
unsigned int
coord_hash(int x, int y, int z)
{
    const int magic_number = 0x45d9f3b;
    /* use Cantor pairing to reduce (x,y) to a unique number */
    unsigned int a = ((x+y) * (x+y+1) / 2) + x + z + ubirthday
                                           + sysopt.serverseed;
    a = a * magic_number;
    a = ((a >> 16) ^ a) * magic_number;
    a = ((a >> 16) ^ a);
    return a;
}

/* Deterministic hash of a single number. Useful for hashes of non-coordinate
 * numbers such as object or monster ids. */
unsigned int
hash1(int x)
{
    /* wrap around coord_hash; ignore Cantor coordinate pairing */
    return coord_hash(0, 0, x);
}

/* hash1(), but returns a positive int, for various use cases that convert it to
 * int and which it would be unsafe to just use hash1 and possibly have that
 * value converted to negative. */
int
int_hash1(int x)
{
    unsigned int hash = hash1(x);
    while (hash > INT_MAX)
        hash /= 2;
    return (int) hash;
}

/* guts of pmatch(), pmatchi(), and pmatchz();
   match a string against a pattern */
static boolean
pmatch_internal(const char *patrn, const char *strng,
                boolean ci,     /* True => case-insensitive,
                                   False => case-sensitive */
                const char *sk) /* set of characters to skip */
{
    char s, p;
    /*
     *  Simple pattern matcher:  '*' matches 0 or more characters, '?' matches
     *  any single character.  Returns TRUE if 'strng' matches 'patrn'.
     */
 pmatch_top:
    if (!sk) {
        s = *strng++;
        p = *patrn++; /* get next chars and pre-advance */
    } else {
        /* fuzzy match variant of pmatch; particular characters are ignored */
        do {
            s = *strng++;
        } while (index(sk, s));
        do {
            p = *patrn++;
        } while (index(sk, p));
    }
    if (!p)                           /* end of pattern */
        return (boolean) (s == '\0'); /* matches iff end of string too */
    else if (p == '*')                /* wildcard reached */
        return (boolean) ((!*patrn
                           || pmatch_internal(patrn, strng - 1, ci, sk))
                          ? TRUE
                          : s ? pmatch_internal(patrn - 1, strng, ci, sk)
                              : FALSE);
    else if ((ci ? lowc(p) != lowc(s) : p != s) /* check single character */
             && (p != '?' || !s))               /* & single-char wildcard */
        return FALSE;                           /* doesn't match */
    else                 /* return pmatch_internal(patrn, strng, ci, sk); */
        goto pmatch_top; /* optimize tail recursion */
}

/* case-sensitive wildcard match */
boolean
pmatch(const char *patrn, const char *strng)
{
    return pmatch_internal(patrn, strng, FALSE, (const char *) 0);
}

/* case-insensitive wildcard match */
boolean
pmatchi(const char *patrn, const char *strng)
{
    return pmatch_internal(patrn, strng, TRUE, (const char *) 0);
}

#if 0
/* case-insensitive wildcard fuzzymatch;
   NEVER WORKED AS INTENDED but fortunately isn't needed */
boolean
pmatchz(const char *patrn, const char *strng)
{
    /* ignore spaces, tabs (just in case), dashes, and underscores */
    static const char fuzzychars[] = " \t-_";

    return pmatch_internal(patrn, strng, TRUE, fuzzychars);
}
#endif

#ifndef STRNCMPI
/* case insensitive counted string comparison */
/*{ aka strncasecmp }*/
int
strncmpi(register const char *s1, register const char *s2,
         register int n) /*(should probably be size_t, which is unsigned)*/
{
    register char t1, t2;

    while (n--) {
        if (!*s2)
            return (*s1 != 0); /* s1 >= s2 */
        else if (!*s1)
            return -1; /* s1  < s2 */
        t1 = lowc(*s1++);
        t2 = lowc(*s2++);
        if (t1 != t2)
            return (t1 > t2) ? 1 : -1;
    }
    return 0; /* s1 == s2 */
}
#endif /* STRNCMPI */

#ifndef STRSTRI
/* case insensitive substring search */
char *
strstri(const char *str, const char *sub)
{
    register const char *s1, *s2;
    register int i, k;
#define TABSIZ 0x20                  /* 0x40 would be case-sensitive */
    char tstr[TABSIZ], tsub[TABSIZ]; /* nibble count tables */
#if 0
    assert( (TABSIZ & ~(TABSIZ-1)) == TABSIZ ); /* must be exact power of 2 */
    assert( &lowc != 0 );                       /* can't be unsafe macro */
#endif

    /* special case: empty substring */
    if (!*sub)
        return (char *) str;

    /* do some useful work while determining relative lengths */
    for (i = 0; i < TABSIZ; i++)
        tstr[i] = tsub[i] = 0; /* init */
    for (k = 0, s1 = str; *s1; k++)
        tstr[*s1++ & (TABSIZ - 1)]++;
    for (s2 = sub; *s2; --k)
        tsub[*s2++ & (TABSIZ - 1)]++;

    /* evaluate the info we've collected */
    if (k < 0)
        return (char *) 0;       /* sub longer than str, so can't match */
    for (i = 0; i < TABSIZ; i++) /* does sub have more 'x's than str? */
        if (tsub[i] > tstr[i])
            return (char *) 0; /* match not possible */

    /* now actually compare the substring repeatedly to parts of the string */
    for (i = 0; i <= k; i++) {
        s1 = &str[i];
        s2 = sub;
        while (lowc(*s1++) == lowc(*s2++))
            if (!*s2)
                return (char *) &str[i]; /* full match */
    }
    return (char *) 0; /* not found */
}
#endif /* STRSTRI */

/* compare two strings for equality, ignoring the presence of specified
   characters (typically whitespace) and possibly ignoring case */
boolean
fuzzymatch(const char *s1, const char *s2, const char *ignore_chars,
           boolean caseblind)
{
    register char c1, c2;

    do {
        while ((c1 = *s1++) != '\0' && index(ignore_chars, c1) != 0)
            continue;
        while ((c2 = *s2++) != '\0' && index(ignore_chars, c2) != 0)
            continue;
        if (!c1 || !c2)
            break; /* stop when end of either string is reached */

        if (caseblind) {
            c1 = lowc(c1);
            c2 = lowc(c2);
        }
    } while (c1 == c2);

    /* match occurs only when the end of both strings has been reached */
    return (boolean) (!c1 && !c2);
}

/*
 * Time routines
 *
 * The time is used for:
 *  - seed for rand()
 *  - year on tombstone and yyyymmdd in record file
 *  - phase of the moon (various monsters react to NEW_MOON or FULL_MOON)
 *  - night and midnight (the undead are dangerous at midnight)
 *  - determination of what files are "very old"
 */

/* TIME_type: type of the argument to time(); we actually use &(time_t) */
#if defined(BSD) && !defined(POSIX_TYPES)
#define TIME_type long *
#else
#define TIME_type time_t *
#endif
/* LOCALTIME_type: type of the argument to localtime() */
#if (defined(ULTRIX) && !(defined(ULTRIX_PROTO) || defined(NHSTDC))) \
    || (defined(BSD) && !defined(POSIX_TYPES))
#define LOCALTIME_type long *
#else
#define LOCALTIME_type time_t *
#endif

#if defined(AMIGA) && !defined(AZTEC_C) && !defined(__SASC_60) \
    && !defined(_DCC) && !defined(__GNUC__)
extern struct tm *localtime(time_t *);
#endif
static struct tm *getlt(void);

/* Sets the seed for the random number generator */
#ifdef USE_ISAAC64

static void
set_random(unsigned long seed,
           int (*fn)(int))
{
    init_isaac64(seed, fn);
}

#else /* USE_ISAAC64 */

/*ARGSUSED*/
static void
set_random(unsigned long seed,
           int (*fn)(int) UNUSED)
{
    /*
     * The types are different enough here that sweeping the different
     * routine names into one via #defines is even more confusing.
     */
# ifdef RANDOM /* srandom() from sys/share/random.c */
    srandom((unsigned int) seed);
# else
#  if defined(__APPLE__) || defined(BSD) || defined(LINUX) \
    || defined(ULTRIX) || defined(CYGWIN32) /* system srandom() */
#   if defined(BSD) && !defined(POSIX_TYPES) && defined(SUNOS4)
    (void)
#   endif
        srandom((int) seed);
#  else
#   ifdef UNIX /* system srand48() */
    srand48((long) seed);
#   else       /* poor quality system routine */
    srand((int) seed);
#   endif
#  endif
# endif
}

#endif /* USE_ISAAC64 */

/* An appropriate version of this must always be provided in
   port-specific code somewhere. It returns a number suitable
   as seed for the random number generator */
extern unsigned long sys_random_seed(void);

/*
 * Initializes the random number generator.
 * Only call once.
 */
void
init_random(int (*fn)(int))
{
    set_random(sys_random_seed(), fn);
}

/* Reshuffles the random number generator. */
void
reseed_random(int (*fn)(int))
{
   /* only reseed if we are certain that the seed generation is unguessable
    * by the players. */
    if (has_strong_rngseed)
        init_random(fn);
}

time_t
getnow(void)
{
    time_t datetime = 0;

    (void) time((TIME_type) &datetime);
    return datetime;
}

static struct tm *
getlt(void)
{
    time_t date = getnow();

    return localtime((LOCALTIME_type) &date);
}

int
getyear(void)
{
    return (1900 + getlt()->tm_year);
}

#if 0
/* This routine is no longer used since in 20YY it yields "1YYmmdd". */
char *
yymmdd(time_t date)
{
    Static char datestr[10];
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
        lt = localtime((LOCALTIME_type) &date);

    Sprintf(datestr, "%02d%02d%02d",
            lt->tm_year, lt->tm_mon + 1, lt->tm_mday);
    return datestr;
}
#endif

long
yyyymmdd(time_t date)
{
    long datenum;
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
        lt = localtime((LOCALTIME_type) &date);

    /* just in case somebody's localtime supplies (year % 100)
       rather than the expected (year - 1900) */
    if (lt->tm_year < 70)
        datenum = (long) lt->tm_year + 2000L;
    else
        datenum = (long) lt->tm_year + 1900L;
    /* yyyy --> yyyymm */
    datenum = datenum * 100L + (long) (lt->tm_mon + 1);
    /* yyyymm --> yyyymmdd */
    datenum = datenum * 100L + (long) lt->tm_mday;
    return datenum;
}

long
hhmmss(time_t date)
{
    long timenum;
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
        lt = localtime((LOCALTIME_type) &date);

    timenum = lt->tm_hour * 10000L + lt->tm_min * 100L + lt->tm_sec;
    return timenum;
}

char *
yyyymmddhhmmss(time_t date)
{
    long datenum;
    static char datestr[15];
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
#if (defined(ULTRIX) && !(defined(ULTRIX_PROTO) || defined(NHSTDC))) \
    || defined(BSD)
        lt = localtime((long *) (&date));
#else
        lt = localtime(&date);
#endif
    /* just in case somebody's localtime supplies (year % 100)
       rather than the expected (year - 1900) */
    if (lt->tm_year < 70)
        datenum = (long) lt->tm_year + 2000L;
    else
        datenum = (long) lt->tm_year + 1900L;
    Snprintf(datestr, sizeof datestr, "%04ld%02d%02d%02d%02d%02d",
             datenum, lt->tm_mon + 1,
            lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    debugpline1("yyyymmddhhmmss() produced date string %s", datestr);
    return datestr;
}

time_t
time_from_yyyymmddhhmmss(char *buf)
{
    int k;
    time_t timeresult = (time_t) 0;
    struct tm t, *lt;
    char *d, *p, y[5], mo[3], md[3], h[3], mi[3], s[3];

    if (buf && strlen(buf) == 14) {
        d = buf;
        p = y; /* year */
        for (k = 0; k < 4; ++k)
            *p++ = *d++;
        *p = '\0';
        p = mo; /* month */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = md; /* day */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = h; /* hour */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = mi; /* minutes */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = s; /* seconds */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        lt = getlt();
        if (lt) {
            t = *lt;
            t.tm_year = atoi(y) - 1900;
            t.tm_mon = atoi(mo) - 1;
            t.tm_mday = atoi(md);
            t.tm_hour = atoi(h);
            t.tm_min = atoi(mi);
            t.tm_sec = atoi(s);
            timeresult = mktime(&t);
        }
        if ((int) timeresult == -1)
            debugpline1("time_from_yyyymmddhhmmss(%s) would have returned -1",
                        buf ? buf : "");
        else
            return timeresult;
    }
    return (time_t) 0;
}

/*
 * moon period = 29.53058 days ~= 30, year = 365.2422 days
 * days moon phase advances on first day of year compared to preceding year
 *      = 365.2422 - 12*29.53058 ~= 11
 * years in Metonic cycle (time until same phases fall on the same days of
 *      the month) = 18.6 ~= 19
 * moon phase on first day of year (epact) ~= (11*(year%19) + 29) % 30
 *      (29 as initial condition)
 * current phase in days = first day phase + days elapsed in year
 * 6 moons ~= 177 days
 * 177 ~= 8 reported phases * 22
 * + 11/22 for rounding
 */
int
phase_of_the_moon(void) /* 0-7, with 0: new, 4: full */
{
    register struct tm *lt = getlt();
    register int epact, diy, goldn;

    diy = lt->tm_yday;
    goldn = (lt->tm_year % 19) + 1;
    epact = (11 * goldn + 18) % 30;
    if ((epact == 25 && goldn > 11) || epact == 24)
        epact++;

    return ((((((diy + epact) * 6) + 11) % 177) / 22) & 7);
}

boolean
friday_13th(void)
{
    register struct tm *lt = getlt();

    /* tm_wday (day of week; 0==Sunday) == 5 => Friday */
    return (boolean) (lt->tm_wday == 5 && lt->tm_mday == 13);
}

int
night(void)
{
    register int hour = getlt()->tm_hour;

    return (hour < 6 || hour > 21);
}

int
midnight(void)
{
    return (getlt()->tm_hour == 0);
}

/* Returns current day of week (0==Sunday through 6==Saturday) */
static int
weekday(void)
{
    struct tm *lt = getlt();
    return lt->tm_wday;
}

/* Return the number of days since 01/01/0000 on the Gregorian calendar,
 * inclusive of the start date but not the current date.
 * Argument should be a yyyymmdd int. */
static int
days_since_epoch(int ymd)
{
    const int year = ymd / 10000;
    const int month = (ymd % 10000) / 100;
    const int date = ymd % 100;
    int monthlen[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    int i;

    /* insert Feb 29 if this is a leap year */
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        monthlen[1] += 1;
    }

    /* baseline: 365 days per year */
    int days = 365 * year;

    /* Gregorian leap years.
     * These will ignore that year 0 was a leap year, but that shouldn't matter
     * if all you're doing with this function is subtracting two epoch dates
     * from each other.
     * This does year - 1 so as NOT to count the current year, which the end of
     * February may or may not have happened in yet. */
    /* 1 per year divisible by 4. */
    days += (year - 1) / 4;
    /* Minus a leap year for each year divisible by 100. */
    days -= (year - 1) / 100;
    /* Plus a leap year for each year divisible by 400. */
    days += (year - 1) / 400;
    /* Plus the partial amount of days this year. */
    for (i = 0; i < (month - 1); ++i) {
        days += monthlen[i];
    }
    /* Plus the amount of days so far this month. */
    days += date;
    return days;
}

/* Return a bitmask of holidays it is today.
 * This uses a bitmask rather than an enum so that it can extend to "seasons"
 * which extend over multiple days and possibly intersect with different
 * holidays (for instance, Hanukkah can overlap with Christmas). */
int
current_holidays(void)
{
    const int ymd = yyyymmdd((time_t) 0);
    const int year = ymd / 10000;
    const int month = (ymd % 10000) / 100; /* 1..12 */
    const int date = ymd % 100;            /* 1..31 */
    const int today_epoch = days_since_epoch(ymd);
    const int hour = getlt()->tm_hour;
    int i;
    int retmask = 0;

    /* These cache the value of the last time this function did a full holiday
     * computation, to avoid recomputing it all over again unless needed. */
    static int cached_ymd = 0, cached_hour = 0, cached_retmask = 0;
    boolean recompute = FALSE;
    /* recompute if:
     * 1. first ever call (no need to check cached_hour == -1 because cached_ymd
     *    of 0 is always less than ymd in that case)
     * 2. date has changed
     * 3. this was last called before 6 pm and it is now after 6 pm (this
     *    changes the Jewish day which can begin or end a holiday, see below)
     */
    if (cached_ymd < ymd || (cached_hour < 18 && hour >= 18)) {
        recompute = TRUE;
    }
    cached_ymd = ymd;
    cached_hour = hour;
    if (!recompute) {
        return cached_retmask;
    }

    /* Simple holidays observed yearly on the Gregorian calendar. */
    if (month == 1 && date == 1) {
        retmask |= HOLIDAY_NEW_YEARS;
    }
    if (month == 2 && date == 2) {
        retmask |= HOLIDAY_GROUNDHOG_DAY;
    }
    if (month == 2 && date == 14) {
        retmask |= HOLIDAY_VALENTINES_DAY;
    }
    if (month == 3 && date == 14) {
        retmask |= HOLIDAY_PI_DAY;
    }
    if (month == 4 && date == 1) {
        retmask |= HOLIDAY_APRIL_FOOLS;
    }
    if (month == 7 && date == 1) {
        retmask |= HOLIDAY_CANADA_DAY;
    }
    if (month == 10 && date == 31) {
        retmask |= HOLIDAY_HALLOWEEN;
    }
    if (month == 11 && date >= 1 && date <= 2) {
        retmask |= HOLIDAY_LOS_MUERTOS;
    }
    if (month == 11 && date >= 22 && date <= 28 && weekday() == 4) {
        retmask |= HOLIDAY_THANKSGIVING;
    }
    if (month == 12 && date >= 24 && date <= 25) {
        /* counts Christmas Eve too */
        retmask |= HOLIDAY_CHRISTMAS;
    }

    /* Now for the tough stuff. */
    {
        /* Modified form of Gauss's algorithm to compute the date of Easter, also
         * known as the "Computus" algorithm. Relies on a not-inconsiderable amount
         * of voodoo magic. */
        int a = year % 19;
        int b = year / 100;
        int c = year % 100;
        int d = b / 4;
        int e = b % 4;
        int f = (b + 8) / 25;
        int gg = (b - f + 1) / 3; /* note: 'g' is a global variable */
        int h = ((19 * a) + b - d - gg + 15) % 30;
        int ii = c / 4; /* note: 'i' declared already in this function */
        int k = c % 4;
        int L = (32 + (2 * e) + (2 * ii) - h - k) % 7;
        int m = (a + (11 * h) + (22 * L)) / 451;
        int eastermonth = (h + L - (7 * m) + 114) / 31;
        int easterday = (h + L - (7 * m) + 114) % 31 + 1;
        if (month == eastermonth && date == easterday) {
            retmask |= HOLIDAY_EASTER;
        }
        /* Mardi Gras is based on Easter, 47 days before it */
        {
            int easterymd = year * 10000 + eastermonth * 100 + easterday;
            if (days_since_epoch(easterymd) - today_epoch == 47) {
                retmask |= HOLIDAY_MARDI_GRAS;
            }
        }
    }

    {
        /* The Islamic calendar begins on 16 Jul 622 in the Julian calendar (19
         * July in the Gregorian calendar). This is only one of many rule-based
         * reckonings for it.
         * The (lunar) year contains 12 months, in a 30 29 30 29... pattern,
         * except in leap years where the final month is 30 days. Leap years
         * repeat every 30-year cycle, on years 2, 5, 7, 10, 13, 16, 18, 21, 24,
         * 26, 29. This closely but imperfectly approximates the lunar period.
         * TODO: in year 3000 or so, check if it's drifted off by a day or two,
         * and fix it if it has.
         * Begin by calculating a delta number of days since the start of the
         * Islamic calendar.
         * Every 30-year period contains 10631 days, so we can immediately take
         * that delta modulo 10631 to ignore however many 30-year periods that
         * is.
         * This algorithm is not going to pinpoint Ramadan or other holidays
         * exactly, particularly since they depend on manual observation. But it
         * should get pretty close.
         */
        int lunar_leap[30] = { 0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,
                               1,0,1,0,0,1,0,0,1,0,1,0,0,1,0 };

        /* There are 166 days in the partial year 622. Start with that. */
        int date_delta = today_epoch - days_since_epoch(6220719);

        /* Now cut off as many 30-year periods as possible. */
        date_delta = date_delta % 10631;
        /* Then cut off year by year until we reach the current lunar year. */
        for (i = 0; i < 30; ++i) {
            int this_year_len = 354 + lunar_leap[i];
            if (date_delta < this_year_len) {
                break;
            }
            date_delta -= this_year_len;
        }
        if (date_delta < 0 || i == 30) {
            impossible("holiday: bad math finding lunar year");
            date_delta = 0;
        }
        /* Then using whatever is remaining, find the month and date of the
         * current day. */
        int islam_month = 0, islam_date = 0;
        for (i = 0; i < 12; ++i) {
            int month_len = (i % 2 == 1) ? 29 : 30;
            if (date_delta < month_len) {
                islam_month = i + 1;        /* convert back to human-readable */
                islam_date = date_delta + 1;
                break;
            }
            date_delta -= month_len;
        }
        if (date_delta < 0 || i == 12) {
            impossible("holiday: bad math finding lunar month/date");
        }
        if (islam_month == 9) {
            retmask |= HOLIDAY_RAMADAN;
        }
        if (islam_month == 10 && islam_date == 1) {
            retmask |= HOLIDAY_EID_AL_FITR;
        }
    }

    {
        /* The Hebrew calendar is even more complicated than the Islamic
         * calendar, but the real problem with it is that it depends on
         * obtaining the precise instant of the new moon. This is difficult to
         * do because the lunar synodic period fluctuates, and getting a new
         * moon wrong by even an hour can throw off the date of the new year by
         * up to two days. In lieu of piling a bunch of orbital mechanics
         * calculations on top of the Hebrew calendar to make it work for
         * arbitrary dates, this will just use dates for the Hebrew new year
         * obtained from hebcal.com.
         */
        const int heb_new_year[30] = {
            20200919, 20210907, 20220926, 20230916, 20241003, /* 2020-2024 */
            20250923, 20260912, 20271002, 20280921, 20290910, /* 2025-2029 */
            20300928, 20310918, 20320906, 20330924, 20340914, /* 2030-2034 */
            20351004, 20360922, 20370910, 20380930, 20390919, /* 2035-2039 */
            20400908, 20410926, 20420915, 20431005, 20440922, /* 2040-2044 */
            20450912, 20461001, 20470921, 20480908, 20490927, /* 2045-2049 */
        };
        if (year > 2048) {
            pline("This game is still being played after 2048?  Cool.");
            impossible("no data for Hebrew calendar in year %d", year);
        }
        else if (year < 2020) {
            pline("Time travel to the past?  Or fix your system clock.");
            impossible("no data for Hebrew calendar in year %d", year);
        }
        else {
            int tmp_epoch_today = today_epoch;
            /* The Gregorian day begins at midnight, but the Hebrew day begins
             * at sunset. Assume sunset is at 6 PM; if it's after that, advance
             * the day by 1. */
            if (hour >= 18) {
                tmp_epoch_today += 1;
            }
            /* ymd is no longer safe to use in this computation */

            /* advance index in heb_new_year until it points at the current
             * Hebrew year */
            int index = 0;
            int epoch_last_newyear, epoch_next_newyear;
            do {
                epoch_last_newyear = days_since_epoch(heb_new_year[index]);
                epoch_next_newyear = days_since_epoch(heb_new_year[index + 1]);
                index++;
            } while (epoch_next_newyear <= tmp_epoch_today);

            int heb_year_length = epoch_next_newyear - epoch_last_newyear;

            /* The leap year inserts a 30-day month in the middle of the year,
             * represented by a 0 here. */
            int heb_month_len[13] = { 30,29,30,29,30,0,29,30,29,30,29,30,29 };
            if (heb_year_length >= 383 && heb_year_length <= 385) {
                heb_month_len[5] = 30; /* insert Adar I */
            }
            else if (heb_year_length < 353 || heb_year_length > 355) {
                /* if not a leap year, year must be 353 to 355 days */
                impossible("illegal Hebrew year length %d", heb_year_length);
                heb_year_length = 354; /* try for graceful fallback */
            }
            if (heb_year_length % 10 == 3) { /* short year */
                heb_month_len[2] -= 1; /* deduct 1 day from Kislev */
            }
            else if (heb_year_length % 10 == 5) { /* full year */
                heb_month_len[1] += 1; /* add 1 day to Cheshvan */
            }
            int hebrew_month = 0, hebrew_date = 0;
            int date_delta = tmp_epoch_today - epoch_last_newyear;
            for (i = 0; i < 13; ++i) {
                if (date_delta < heb_month_len[i]) {
                    hebrew_month = i + 1;
                    hebrew_date = date_delta + 1;
                    break;
                }
                date_delta -= heb_month_len[i];
            }
            if (date_delta < 0 || i == 13) {
                impossible("holiday: bad math finding hebrew month/date");
            }
            if (hebrew_month == 1 && hebrew_date >= 1 && hebrew_date <= 2) {
                retmask |= HOLIDAY_ROSH_HASHANAH;
            }
            if (hebrew_month == 1 && hebrew_date == 10) {
                retmask |= HOLIDAY_YOM_KIPPUR;
            }
            /* There are a number of different ways to observe Passover; this
             * tracks the first two days but not the rest of its week. */
            if (hebrew_month == 8 && hebrew_date >= 15 && hebrew_date <= 16) {
                retmask |= HOLIDAY_PASSOVER;
            }
            /* Judging the end date of Hanukkah depends on whether Kislev was
             * reduced in length or not. */
            if ((hebrew_month == 3 && hebrew_date >= 25)
                || (hebrew_month == 4
                    && (hebrew_date <= (heb_month_len[2] == 30 ? 2 : 3)))) {
                retmask |= HOLIDAY_HANUKKAH;
            }
        }
    }
    cached_retmask = retmask; /* for next time */
    return retmask;
}

/* strbuf_init() initializes strbuf state for use */
void
strbuf_init(strbuf_t *strbuf)
{
    strbuf->str = NULL;
    strbuf->len = 0;
}

/* strbuf_append() appends given str to strbuf->str */
void
strbuf_append(strbuf_t *strbuf, const char *str)
{
    int len = (int) strlen(str) + 1;

    strbuf_reserve(strbuf,
                   len + (strbuf->str ? (int) strlen(strbuf->str) : 0));
    Strcat(strbuf->str, str);
}

/* strbuf_reserve() ensure strbuf->str has storage for len characters */
void
strbuf_reserve(strbuf_t *strbuf, int len)
{
    if (strbuf->str == NULL) {
        strbuf->str = strbuf->buf;
        strbuf->str[0] = '\0';
        strbuf->len = (int) sizeof strbuf->buf;
    }

    if (len > strbuf->len) {
        char *oldbuf = strbuf->str;

        strbuf->len = len + (int) sizeof strbuf->buf;
        strbuf->str = (char *) alloc(strbuf->len);
        Strcpy(strbuf->str, oldbuf);
        if (oldbuf != strbuf->buf)
            free((genericptr_t) oldbuf);
    }
}

/* strbuf_empty() frees allocated memory and set strbuf to initial state */
void
strbuf_empty(strbuf_t *strbuf)
{
    if (strbuf->str != NULL && strbuf->str != strbuf->buf)
        free((genericptr_t) strbuf->str);
    strbuf_init(strbuf);
}

/* strbuf_nl_to_crlf() converts all occurences of \n to \r\n */
void
strbuf_nl_to_crlf(strbuf_t *strbuf)
{
    if (strbuf->str) {
        int len = (int) strlen(strbuf->str);
        int count = 0;
        char *cp = strbuf->str;

        while (*cp)
            if (*cp++ == '\n')
                count++;
        if (count) {
            strbuf_reserve(strbuf, len + count + 1);
            for (cp = strbuf->str + len + count; count; --cp)
                if ((*cp = cp[-count]) == '\n') {
                    *--cp = '\r';
                    --count;
                }
        }
    }
}

char *
nonconst(const char *str, char *buf, size_t bufsz)
{
    char *retval = emptystr;

    if (str && buf)
        if (strlen(str) <= (bufsz - 1)) {
            Strcpy(buf, str);
            retval = buf;
        }
    return retval;
}

/* swapbits(val, bita, bitb) swaps bit a with bit b in val */
int
swapbits(int val, int bita, int bitb)
{
    int tmp = ((val >> bita) & 1) ^ ((val >> bitb) & 1);

    return (val ^ ((tmp << bita) | (tmp << bitb)));
}

/* randomize the given list of numbers  0 <= i < count */
void
shuffle_int_array(int *indices, int count)
{
    int i, iswap, temp;

    for (i = count - 1; i > 0; i--) {
        if ((iswap = rn2(i + 1)) == i)
            continue;
        temp = indices[i];
        indices[i] = indices[iswap];
        indices[iswap] = temp;
    }
}

DISABLE_WARNING_FORMAT_NONLITERAL

/*
 * Wrap snprintf for use in the main code.
 *
 * Wrap reasons:
 *   1. If there are any platform issues, we have one spot to fix them -
 *      snprintf is a routine with a troubling history of bad implementations.
 *   2. Add combersome error checking in one spot.  Problems with text
 *      wrangling do not have to be fatal.
 *   3. Gcc 9+ will issue a warning unless the return value is used.
 *      Annoyingly, explicitly casting to void does not remove the error.
 *      So, use the result - see reason #2.
 */
void
nh_snprintf(
    const char *func, int line,
    char *str, size_t size,
    const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
#ifdef NO_VSNPRINTF
    n = vsprintf(str, fmt, ap);
#else
    n = vsnprintf(str, size, fmt, ap);
#endif
    va_end(ap);
    if (n < 0 || (size_t) n >= size) { /* is there a problem? */
        impossible("snprintf %s: func %s, file line %d",
                   (n < 0) ? "format error" : "overflow",
                   func, line);
        str[size - 1] = '\0'; /* make sure it is nul terminated */
    }
}

RESTORE_WARNING_FORMAT_NONLITERAL

/*hacklib.c*/
