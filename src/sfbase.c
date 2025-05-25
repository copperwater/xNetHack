/* NetHack 3.7	sfbase.c.template $NHDT-Date$  $NHDT-Branch$:$NHDT-Revision$ */
/* Copyright (c) Michael Allison, 2025. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "sfprocs.h"

/* #define DO_DEBUG */

struct sf_structlevel_procs sfoprocs[NUM_SAVEFORMATS], sfiprocs[NUM_SAVEFORMATS],
                zerosfoprocs = {0}, zerosfiprocs = {0};

void sf_log(NHFILE *, const char *, size_t, int);

#define SF_A(dtyp) \
void sfo_##dtyp(NHFILE *nhfp, dtyp *d_##dtyp, const char *myname)                   \
{                                                                                   \
    if (nhfp->fplog)                                                                \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1);                                  \
    if (nhfp->structlevel) {                                                        \
        (*sfoprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);              \
    }                                                                             \
}                                                                                   \
                                                                                    \
void sfi_##dtyp(NHFILE *nhfp, dtyp *d_##dtyp, const char *myname)                   \
{                                                                                   \
    if (nhfp->structlevel) {                                                        \
        (*sfiprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);              \
    }                                                                             \
    if (nhfp->fplog && !nhfp->eof)                                                  \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1);                                  \
    if (nhfp->eof)                                                                  \
        return;                                                                     \
    if (((nhfp->mode & CONVERTING) != 0) && nhfp->nhfpconvert)                      \
        sfo_##dtyp(nhfp->nhfpconvert, d_##dtyp, myname);                            \
}

#define SF_C(keyw, dtyp) \
void sfo_##dtyp(NHFILE *nhfp, keyw dtyp *d_##dtyp, const char *myname)              \
{                                                                                   \
                                                                                    \
    if (nhfp->structlevel) {                                                        \
        (*sfoprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);              \
    }                                                                            \
    if (nhfp->fplog && !nhfp->eof)                                                  \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1);                                  \
}                                                                                   \
                                                                                    \
void sfi_##dtyp(NHFILE *nhfp, keyw dtyp *d_##dtyp, const char *myname)              \
{                                                                                   \
    if (nhfp->structlevel) {                                                        \
        (*sfiprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);              \
    }                                                                              \
    if (nhfp->fplog && !nhfp->eof)                                                  \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1);                                  \
    if (nhfp->eof)                                                                  \
        return;                                                                     \
    if (((nhfp->mode & CONVERTING) != 0) && nhfp->nhfpconvert)                      \
        sfo_##dtyp(nhfp->nhfpconvert, d_##dtyp, myname);                            \
}

#define SF_X(xxx, dtyp) \
void sfo_##dtyp(NHFILE *nhfp, xxx *d_##dtyp, const char *myname, int bfsz)          \
{                                                                                   \
    if (nhfp->structlevel) {                                                        \
        (*sfoprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname, bfsz);        \
    }                                                                             \
    if (nhfp->fplog && !nhfp->eof)                                                  \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1);                                  \
}                                                                                   \
                                                                                    \
void sfi_##dtyp(NHFILE *nhfp, xxx *d_##dtyp, const char *myname, int bfsz)          \
{                                                                                   \
    if (nhfp->structlevel) {                                                        \
        (*sfiprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname, bfsz);        \
    }                                                                            \
    if (nhfp->fplog && !nhfp->eof)                                                  \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1);                                  \
    if (nhfp->eof)                                                                  \
        return;                                                                     \
    if (((nhfp->mode & CONVERTING) != 0) && nhfp->nhfpconvert)                      \
        sfo_##dtyp(nhfp->nhfpconvert, d_##dtyp, myname, bfsz);                      \
}

SF_C(struct, arti_info)
SF_C(struct, nhrect)
SF_C(struct, branch)
SF_C(struct, bubble)
SF_C(struct, cemetery)
SF_C(struct, context_info)
SF_C(struct, nhcoord)
SF_C(struct, damage)
SF_C(struct, dest_area)
SF_C(struct, dgn_topology)
SF_C(struct, dungeon)
SF_C(struct, d_level)
SF_C(struct, ebones)
SF_C(struct, edog)
SF_C(struct, egd)
SF_C(struct, emin)
SF_C(struct, engr)
SF_C(struct, epri)
SF_C(struct, eshk)
SF_C(struct, fe)
SF_C(struct, flag)
SF_C(struct, fruit)
SF_C(struct, gamelog_line)
SF_C(struct, kinfo)
SF_C(struct, levelflags)
SF_C(struct, ls_t)
SF_C(struct, linfo)
SF_C(struct, mapseen_feat)
SF_C(struct, mapseen_flags)
SF_C(struct, mapseen_rooms)
SF_C(struct, mkroom)
SF_C(struct, monst)
SF_C(struct, mvitals)
SF_C(struct, obj)
SF_C(struct, objclass)
SF_C(struct, q_score)
SF_C(struct, rm)
SF_C(struct, spell)
SF_C(struct, stairway)
SF_C(struct, s_level)
SF_C(struct, trap)
SF_C(struct, version_info)
SF_C(struct, you)
SF_C(union, any)

SF_A(aligntyp)
SF_A(boolean)
SF_A(coordxy)
SF_A(genericptr)
SF_A(int)
SF_A(int16)
SF_A(int32)
SF_A(int64)
SF_A(long)
SF_A(schar)
SF_A(short)
SF_A(size_t)
SF_A(time_t)
SF_A(uchar)
SF_A(uint16)
SF_A(uint32)
SF_A(uint64)
SF_A(ulong)
SF_A(unsigned)
SF_A(ushort)
SF_A(xint16)
SF_A(xint8)

void
sfo_char(NHFILE *nhfp, char *d_char, const char *myname, int cnt)
{
    if (nhfp->structlevel) {
        (*sfoprocs[nhfp->fnidx].fn.sf_char)(nhfp, d_char, myname, cnt);
    }
    if (nhfp->fplog && !nhfp->eof)
        sf_log(nhfp, myname, sizeof (char), cnt);
}

void
sfi_char(NHFILE *nhfp, char *d_char, const char *myname, int cnt)
{
    if (nhfp->structlevel) {
        (*sfiprocs[nhfp->fnidx].fn.sf_char)(nhfp, d_char, myname, cnt);
    }
    if (nhfp->fplog && !nhfp->eof)
        sf_log(nhfp, myname, sizeof (char), cnt);
    if (nhfp->eof)
        return;
    if (((nhfp->mode & CONVERTING) != 0) && nhfp->nhfpconvert)
        sfo_char(nhfp->nhfpconvert, d_char, myname, cnt);
}
SF_X(uint8_t, bitfield)

/* ---------------------------------------------------------------*/

void
sf_log(NHFILE *nhfp, const char *t1, size_t sz, int cnt)
{
    FILE *fp = nhfp->fplog;

    if (fp) {
        (void) fprintf(fp, "%ld %s sz=%zu cnt=%d\n",
                       nhfp->count++,
                       t1, sz, cnt);
        fflush(fp);
    }
}

/*
 *----------------------------------------------------------------------------
 * initialize the function pointers. These are called from initoptions_init().
 *----------------------------------------------------------------------------
 */

void
sf_init(void)
{
    sfoprocs[invalid] = zerosfoprocs;
    sfiprocs[invalid] = zerosfiprocs;
    sfoprocs[historical] = historical_sfo_procs;
    sfiprocs[historical] = historical_sfi_procs;
}



