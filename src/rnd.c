/* NetHack 3.7	rnd.c	$NHDT-Date: 1596498205 2020/08/03 23:43:25 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.30 $ */
/*      Copyright (c) 2004 by Robert Patrick Rankin               */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef USE_ISAAC64
#include "isaac64.h"

staticfn int whichrng(int (*fn)(int));
staticfn int RND(int);
staticfn void set_random(unsigned long, int (*)(int));

#if 0
static isaac64_ctx rng_state;
#endif

struct rnglist_t {
    int (*fn)(int);
    boolean init;
    isaac64_ctx rng_state;
};

enum { CORE = 0, DISP = 1 };

static struct rnglist_t rnglist[] = {
    { rn2, FALSE, { 0 } },                      /* CORE */
    { rn2_on_display_rng, FALSE, { 0 } },       /* DISP */
};

staticfn int
whichrng(int (*fn)(int))
{
    int i;

    for (i = 0; i < SIZE(rnglist); ++i)
        if (rnglist[i].fn == fn)
            return i;
    return -1;
}

void
init_isaac64(unsigned long seed, int (*fn)(int))
{
    unsigned char new_rng_state[sizeof seed];
    unsigned i;
    int rngindx = whichrng(fn);

    if (rngindx < 0)
        panic("Bad rng function passed to init_isaac64().");

    for (i = 0; i < sizeof seed; i++) {
        new_rng_state[i] = (unsigned char) (seed & 0xFF);
        seed >>= 8;
    }
    isaac64_init(&rnglist[rngindx].rng_state, new_rng_state,
                 (int) sizeof seed);
}

staticfn int
RND(int x)
{
    return (isaac64_next_uint64(&rnglist[CORE].rng_state) % x);
}

/* 0 <= rn2(x) < x, but on a different sequence from the "main" rn2;
   used in cases where the answer doesn't affect gameplay and we don't
   want to give users easy control over the main RNG sequence. */
int
rn2_on_display_rng(int x)
{
    return (isaac64_next_uint64(&rnglist[DISP].rng_state) % x);
}

#else   /* USE_ISAAC64 */

/* "Rand()"s definition is determined by [OS]conf.h */
#if defined(UNIX) || defined(RANDOM)
#define RND(x) ((int) (Rand() % (long) (x)))
#else
/* Good luck: the bottom order bits are cyclic. */
#define RND(x) ((int) ((Rand() >> 3) % (x)))
#endif
int
rn2_on_display_rng(int x)
{
    static unsigned seed = 1;
    seed *= 2739110765;
    return (int) ((seed >> 16) % (unsigned) x);
}
#endif  /* USE_ISAAC64 */

/* 0 <= rn2(x) < x */
int
rn2(int x)
{
    if (x <= 0) {
        impossible("rn2(%d) attempted, returning 0", x);
        return 0;
    }
    x = RND(x);
    return x;
}

/* 0 <= rnl(x) < x; sometimes subtracting Luck;
   good luck approaches 0, bad luck approaches (x-1) */
int
rnl(int x)
{
    int i, adjustment;

    if (x <= 0) {
        impossible("rnl(%d) attempted, returning 0", x);
        return 0;
    }

    adjustment = Luck;
    if (x <= 15) {
        /* for small ranges, use Luck/3 (rounded away from 0);
           also guard against architecture-specific differences
           of integer division involving negative values */
        adjustment = (abs(adjustment) + 1) / 3 * sgn(adjustment);
        /*
         *       11..13 ->  4
         *        8..10 ->  3
         *        5.. 7 ->  2
         *        2.. 4 ->  1
         *       -1,0,1 ->  0 (no adjustment)
         *       -4..-2 -> -1
         *       -7..-5 -> -2
         *      -10..-8 -> -3
         *      -13..-11-> -4
         */
    }

    i = RND(x);
    if (adjustment && rn2(37 + abs(adjustment))) {
        i -= adjustment;
        if (i < 0)
            i = 0;
        else if (i >= x)
            i = x - 1;
    }
    return i;
}

/* 1 <= rnd(x) <= x */
int
rnd(int x)
{
    if (x <= 0) {
        impossible("rnd(%d) attempted, returning 1", x);
        return 1;
    }
    x = RND(x) + 1;
    return x;
}

int
rnd_on_display_rng(int x)
{
    return rn2_on_display_rng(x) + 1;
}

/* d(N,X) == NdX == dX+dX+...+dX N times; n <= d(n,x) <= (n*x) */
int
d(int n, int x)
{
    int tmp = n;

    if (x < 0 || n < 0 || (x == 0 && n != 0)) {
        impossible("d(%d,%d) attempted, returning 1", n, x);
        return 1;
    }
    while (n--)
        tmp += RND(x);
    return tmp; /* Alea iacta est. -- J.C. */
}

/* 1 <= rne(x) <= 10, with exponential distribution:
 * chance of 1: (x-1)/x
 * chance of 2: (x-1)/x^2
 * chance of 3: (x-1)/x^3
 * etc.
 */
int
rne(int x)
{
    int tmp;

    tmp = 1;
    while (tmp < 10 && !rn2(x))
        tmp++;
    return tmp;
}

/* rnz: everyone's favorite! */
int
rnz(int i)
{
    long x = (long) i;
    long tmp = 1000L;

    tmp += rn2(1000);
    tmp *= rne(4);
    if (rn2(2)) {
        x *= tmp;
        x /= 1000;
    } else {
        x *= 1000;
        x /= tmp;
    }
    return (int) x;
}

/* percent(x) = x% chance it returns true */
boolean
percent(int x)
{
    if (x < 0) {
        impossible("percent(%d) attempted, returning false", x);
        return FALSE;
    }
    else if (x > 100) {
        impossible("percent(%d) attempted, returning true", x);
        return TRUE;
    }
    return x > rn2(100);
}

/* rnf(x, y) = x in y (or x/y) chance it returns true */
boolean
rnf(int numerator, int denominator) /**< @returns (rnf(n,d) < n/d) */
{
    if (denominator <= 0) {
        impossible("rnf(%d, %d) attempted, returning false",
                   numerator, denominator);
        return FALSE;
    }
    else if (numerator > denominator) {
        impossible("rnf(%d, %d) attempted, returning true",
                   numerator, denominator);
        return TRUE;
    }
    return rn2(denominator) < numerator;
}

/* Sets the seed for the random number generator */
#ifdef USE_ISAAC64

staticfn void
set_random(unsigned long seed,
           int (*fn)(int))
{
    init_isaac64(seed, fn);
}

#else /* USE_ISAAC64 */

/*ARGSUSED*/
staticfn void
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

/*rnd.c*/
