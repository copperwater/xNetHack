/* NetHack 3.7	sfbase.c.template $NHDT-Date$  $NHDT-Branch$:$NHDT-Revision$ */
/* Copyright (c) Michael Allison, 2025. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "sfprocs.h"

#ifdef SFCTOOL
//#include "sfproto.h"
#endif

/* #define DO_DEBUG */

//#define TURN_OFF_LOGGING 0x20
#define TURN_OFF_LOGGING (UNCONVERTING << 1)

struct sf_structlevel_procs sfoprocs[NUM_SAVEFORMATS], sfiprocs[NUM_SAVEFORMATS],
                zerosfoprocs = {0}, zerosfiprocs = {0};
struct sf_fieldlevel_procs sfoflprocs[NUM_SAVEFORMATS], sfiflprocs[NUM_SAVEFORMATS],
                zerosfoflprocs = {0}, zerosfiflprocs = {0};

char *sfvalue_aligntyp(aligntyp *a);
char *sfvalue_any(anything *a);
char *sfvalue_genericptr(genericptr a);
char *sfvalue_int16(int16 *a);
char *sfvalue_int32(int32 *a);
char *sfvalue_int64(int64 *a);
char *sfvalue_uchar(uchar *a);
char *sfvalue_uint16(uint16 *a);
char *sfvalue_uint32(uint32 *a);
char *sfvalue_uint64(uint64 *a);
char *sfvalue_size_t(size_t *a);
char *sfvalue_time_t(time_t *a);
char *sfvalue_short(short *a);
char *sfvalue_ushort(ushort *a);
char *sfvalue_int(int *a);
char *sfvalue_unsigned(unsigned *a);
char *sfvalue_long(long *a);
char *sfvalue_ulong(ulong *a);
char *sfvalue_xint8(xint8 *a);
char *sfvalue_xint16(xint16 *a);
char *sfvalue_char(char *a, int n);
char *sfvalue_boolean(boolean *a);
char *sfvalue_schar(schar *a);
char *sfvalue_bitfield(uint8 *a);
char *complex_dump(uchar *a);
char *bitfield_dump(uint8 *a);

void sf_log(NHFILE *, const char *, size_t, int, char *);

#if NH_C < 202300L
#define Sfvalue_aligntyp(a) sfvalue_aligntyp(a)
#define Sfvalue_any(a) sfvalue_any(a)
#define Sfvalue_genericptr(a) sfvalue_genericptr(a)
#define Sfvalue_coordxy(a) sfvalue_int16(a)
#define Sfvalue_int16(a) sfvalue_int16(a)
#define Sfvalue_int32(a) sfvalue_int32(a)
#define Sfvalue_int64(a) sfvalue_int64(a)
#define Sfvalue_uchar(a) sfvalue_uchar(a)
#define Sfvalue_uint16(a) sfvalue_uint16(a)
#define Sfvalue_uint32(a) sfvalue_uint32(a)
#define Sfvalue_uint64(a) sfvalue_uint64(a)
#define Sfvalue_size_t(a) sfvalue_size_t(a)
#define Sfvalue_time_t(a) sfvalue_time_t(a)
#define Sfvalue_short(a) sfvalue_short(a)
#define Sfvalue_ushort(a) sfvalue_ushort(a)
#define Sfvalue_int(a) sfvalue_int(a)
#define Sfvalue_unsigned(a) sfvalue_unsigned(a)
#define Sfvalue_xint8(a) sfvalue_xint8(a)
#define Sfvalue_xint16(a) sfvalue_xint16(a)

#else

#define sfvalue(x)                       \
    _Generic( (x),                       \
        anything *: sfvalue_any,         \
        genericptr_t *: sfvalue_genericptr, \
        int16_t *: sfvalue_int16,        \
        int32_t *: sfvalue_int32,        \
        int64_t *: sfvalue_int64,        \
        uchar *: sfvalue_uchar,          \
        uint16_t *: sfvalue_uint16,      \
        uint32_t *: sfvalue_uint32,      \
        uint64_t *: sfvalue_uint64,      \
        xint8 *: sfvalue_xint8           \
    )(x)

#define Sfvalue_any(a) sfvalue(a)
#define Sfvalue_aligntyp(a) sfvalue(a)
#define Sfvalue_genericptr(a) sfvalue(a)
#define Sfvalue_coordxy(a) sfvalue(a)
#define Sfvalue_int16(a) sfvalue(a)
#define Sfvalue_int32(a) sfvalue(a)
#define Sfvalue_int64(a) sfvalue(a)
#define Sfvalue_uchar(a) sfvalue(a)
#define Sfvalue_unsigned(a) sfvalue(a)
#define Sfvalue_uchar(a) sfvalue(a)
#define Sfvalue_uint16(a) sfvalue(a)
#define Sfvalue_uint32(a) sfvalue(a)
#define Sfvalue_uint64(a) sfvalue(a)
#define Sfvalue_size_t(a) sfvalue(a)
#define Sfvalue_time_t(a) sfvalue(a)
#define Sfvalue_short(a) sfvalue(a)
#define Sfvalue_ushort(a) sfvalue(a)
#define Sfvalue_int(a) sfvalue(a)
#define Sfvalue_unsigned(a) sfvalue(a)
#define Sfvalue_xint8(a) sfvalue(a)
#define Sfvalue_xint16(a) sfvalue(a)
#endif

/* not in _Generic */ 
#define Sfvalue_long(a) sfvalue_long(a)
#define Sfvalue_ulong(a) sfvalue_ulong(a)
#define Sfvalue_char(a, d) sfvalue_char(a, d)
#define Sfvalue_boolean(a) sfvalue_boolean(a)
#define Sfvalue_schar(a) sfvalue_schar(a)
#define Sfvalue_bitfield(a) sfvalue_bitfield(a)

#define SF_A(dtyp) \
void sfo_##dtyp(NHFILE *nhfp, dtyp *d_##dtyp, const char *myname)               \
{                                                                               \
    if (nhfp->fplog)                                                            \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1, Sfvalue_##dtyp(d_##dtyp));    \
    if (nhfp->structlevel) {                                                    \
        (*sfoprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);          \
    } else {                                                                    \
        FILE *save_fplog = nhfp->fplog;                                         \
                                                                                \
        nhfp->fplog = 0;                                                        \
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname);      \
        nhfp->fplog = save_fplog;                                               \
    }                                                                           \
}                                                                               \
                                                                                \
void sfi_##dtyp(NHFILE *nhfp, dtyp *d_##dtyp, const char *myname)               \
{                                                                               \
    if (nhfp->structlevel) {                                                    \
        (*sfiprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);          \
    } else {                                                                    \
        int save_mode = nhfp->mode;                                             \
                                                                                \
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);                             \
        nhfp->mode |= TURN_OFF_LOGGING;                                         \
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname);      \
        nhfp->mode = save_mode;                                                 \
    }                                                                           \
    if (!nhfp->eof) {                                                           \
        if ((((nhfp->mode & CONVERTING) != 0)                                   \
            || ((nhfp->mode & UNCONVERTING) != 0)) && nhfp->nhfpconvert)  {     \
            sfo_##dtyp(nhfp->nhfpconvert, d_##dtyp, myname);                    \
        }                                                                       \
        if (nhfp->fplog)                                                        \
            sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                           \
                   Sfvalue_##dtyp(d_##dtyp));                                   \
    }                                                                           \
}

#define SF_C(keyw, dtyp) \
void sfo_##dtyp(NHFILE *nhfp, keyw dtyp *d_##dtyp, const char *myname)          \
{                                                                               \
    if (nhfp->fplog)                                                            \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                               \
               complex_dump((uchar *) d_##dtyp));                                \
    if (nhfp->structlevel) {                                                    \
        (*sfoprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);          \
    } else {                                                                    \
        FILE *save_fplog = nhfp->fplog;                                         \
                                                                                \
        nhfp->fplog = 0;                                                        \
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname);        \
        nhfp->fplog = save_fplog;                                               \
    }                                                                           \
}                                                                               \
                                                                                \
void sfi_##dtyp(NHFILE *nhfp, keyw dtyp *d_##dtyp, const char *myname)          \
{                                                                               \
    if (nhfp->structlevel) {                                                    \
        (*sfiprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname);          \
    } else {                                                                    \
        int save_mode = nhfp->mode;                                             \
                                                                                \
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);                             \
        nhfp->mode |= TURN_OFF_LOGGING;                                         \
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname);        \
        nhfp->mode = save_mode;                                                 \
    }                                                                           \
    if (!nhfp->eof) {                                                           \
        if ((((nhfp->mode & CONVERTING) != 0)                                   \
            || ((nhfp->mode & UNCONVERTING) != 0))                              \
            && nhfp->nhfpconvert) {                                             \
            sfo_##dtyp(nhfp->nhfpconvert, d_##dtyp, myname);                    \
        }                                                                       \
        if (nhfp->fplog)                                                        \
            sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                           \
                       complex_dump((uchar *) d_##dtyp));                        \
    }                                                                           \
}
  
#define SF_X(xxx, dtyp) \
void sfo_##dtyp(NHFILE *nhfp, xxx *d_##dtyp, const char *myname, int bfsz)      \
{                                                                               \
    if (nhfp->fplog)                                                            \
            sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                           \
                   Sfvalue_##dtyp(d_##dtyp));                                   \
    if (nhfp->structlevel) {                                                    \
        (*sfoprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname, bfsz);    \
    } else {                                                                    \
        FILE *save_fplog = nhfp->fplog;                                         \
                                                                                \
        nhfp->fplog = 0;                                                        \
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname, bfsz);  \
        nhfp->fplog = save_fplog;                                               \
    }                                                                           \
    if (nhfp->fplog && !nhfp->eof)                                              \
        sf_log(nhfp, myname, sizeof *d_##dtyp, 1, Sfvalue_##dtyp(d_##dtyp));    \
}                                                                               \
                                                                                \
void sfi_##dtyp(NHFILE *nhfp, xxx *d_##dtyp, const char *myname, int bfsz)      \
{                                                                               \
    if (nhfp->structlevel) {                                                    \
        (*sfiprocs[nhfp->fnidx].fn.sf_##dtyp)(nhfp, d_##dtyp, myname, bfsz);    \
    } else {                                                                    \
        int save_mode = nhfp->mode;                                             \
                                                                                \
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);                             \
        nhfp->mode |= TURN_OFF_LOGGING;                                         \
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_##dtyp)(nhfp, d_##dtyp, myname, bfsz);  \
        nhfp->mode = save_mode;                                                 \
    }                                                                           \
    if (!nhfp->eof) {                                                           \
        if ((((nhfp->mode & CONVERTING) != 0)                                   \
            || ((nhfp->mode & UNCONVERTING) != 0))                              \
                       && nhfp->nhfpconvert) {                                  \
            sfo_##dtyp(nhfp->nhfpconvert, d_##dtyp, myname, bfsz);              \
        }                                                                       \
        if (nhfp->fplog)                                                        \
            sf_log(nhfp, myname, sizeof *d_##dtyp, 1,                           \
                   dtyp##_dump(d_##dtyp));                                      \
    }                                                                           \
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
SF_C(struct, you)
SF_C(union, any)

SF_A(aligntyp)
SF_A(boolean)
SF_A(coordxy)
//SF_A(genericptr)
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
SF_X(uint8_t, bitfield)

void
sfo_char(NHFILE *nhfp, char *d_char, const char *myname, int cnt)
{
    if (nhfp->fplog)
        sf_log(nhfp, myname, sizeof(char), cnt, Sfvalue_char(d_char, cnt));
    if (nhfp->structlevel) {
        (*sfoprocs[nhfp->fnidx].fn.sf_char)(nhfp, d_char, myname, cnt);
    } else {
        FILE *save_fplog = nhfp->fplog;

        nhfp->fplog = 0;
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_char)(nhfp, d_char, myname, cnt);
        nhfp->fplog = save_fplog;
    }
}

void
sfi_char(NHFILE *nhfp, char *d_char, const char *myname, int cnt)
{
    if (nhfp->structlevel) {
        (*sfiprocs[nhfp->fnidx].fn.sf_char)(nhfp, d_char, myname, cnt);
    } else {
        int save_mode = nhfp->mode;

        nhfp->mode &= ~(CONVERTING | UNCONVERTING);
        nhfp->mode |= TURN_OFF_LOGGING;
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_char)(nhfp, d_char, myname, cnt);
        nhfp->mode = save_mode;
    }
    if (!nhfp->eof) {
        if ((((nhfp->mode & CONVERTING) != 0)
             || ((nhfp->mode & UNCONVERTING) != 0))
            && nhfp->nhfpconvert) {
            sfo_char(nhfp->nhfpconvert, d_char, myname, cnt);
        }
        if (nhfp->fplog)
            sf_log(nhfp, myname, sizeof(char), cnt,
                   Sfvalue_char(d_char, cnt));
    }
}

void
sfo_genericptr(NHFILE *nhfp, void **d_genericptr, const char *myname)
{
    if (nhfp->fplog)
        sf_log(nhfp, myname, sizeof *d_genericptr, 1,
               Sfvalue_genericptr(d_genericptr));
    if (nhfp->structlevel) {
        (*sfoprocs[nhfp->fnidx].fn.sf_genericptr)(nhfp, d_genericptr, myname);
    } else {
        FILE *save_fplog = nhfp->fplog;
        nhfp->fplog = 0;
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_genericptr)(nhfp, d_genericptr,
                                                    myname);
        nhfp->fplog = save_fplog;
    }
}
void
sfi_genericptr(NHFILE *nhfp, void **d_genericptr, const char *myname)
{
    if (nhfp->structlevel) {
        (*sfiprocs[nhfp->fnidx].fn.sf_genericptr)(nhfp, d_genericptr, myname);
    } else {
        int save_mode = nhfp->mode;
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);
        nhfp->mode |= TURN_OFF_LOGGING;
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_genericptr)(nhfp, d_genericptr,
                                                    myname);
        nhfp->mode = save_mode;
    }
    if (!nhfp->eof) {
        if ((((nhfp->mode & CONVERTING) != 0) || ((nhfp->mode & UNCONVERTING) != 0))
            && nhfp->nhfpconvert) {
            sfo_genericptr(nhfp->nhfpconvert, d_genericptr, myname);
        }
        if (nhfp->fplog)
            sf_log(nhfp, myname, sizeof *d_genericptr, 1,
                   Sfvalue_genericptr(d_genericptr));
    }
}

void
sfo_version_info(NHFILE *nhfp, struct version_info *d_version_info,
                 const char *myname)
{
    if (nhfp->fplog)
        sf_log(nhfp, myname, sizeof *d_version_info, 1,
               complex_dump((uchar *) d_version_info));
    if (nhfp->structlevel) {
        (*sfoprocs[nhfp->fnidx].fn.sf_version_info)(nhfp, d_version_info,
                                                    myname);
    } else {
        FILE *save_fplog = nhfp->fplog;
        nhfp->fplog = 0;
        (*sfoflprocs[nhfp->fnidx].fn_x.sf_version_info)(nhfp, d_version_info,
                                                      myname);
        nhfp->fplog = save_fplog;
    }
}
void
sfi_version_info(NHFILE *nhfp, struct version_info *d_version_info,
                 const char *myname)
{
    if (nhfp->structlevel) {
        (*sfiprocs[nhfp->fnidx].fn.sf_version_info)(nhfp, d_version_info,
                                                    myname);
    } else {
        int save_mode = nhfp->mode;
        nhfp->mode &= ~(CONVERTING | UNCONVERTING);
        nhfp->mode |= TURN_OFF_LOGGING;
        (*sfiflprocs[nhfp->fnidx].fn_x.sf_version_info)(nhfp, d_version_info,
                                                      myname);
        nhfp->mode = save_mode;
    }
    if (!nhfp->eof) {
        if ((((nhfp->mode & CONVERTING) != 0) || ((nhfp->mode & UNCONVERTING) != 0))
            && nhfp->nhfpconvert) {
            d_version_info->feature_set |= SFCTOOL_BIT;
            sfo_version_info(nhfp->nhfpconvert, d_version_info, myname);
        }
        if (nhfp->fplog)
            sf_log(nhfp, myname, sizeof *d_version_info, 1,
                   complex_dump((uchar *) d_version_info));
    }
}

/* ---------------------------------------------------------------*/

void
sf_log(NHFILE *nhfp, const char *t1, size_t sz, int cnt, char *txtvalue)
{
    FILE *fp = nhfp->fplog;
    long *iocount;
    boolean dolog = ((nhfp->mode & TURN_OFF_LOGGING) == 0);

    if (fp && dolog) {
        iocount = ((nhfp->mode & WRITING) == 0) ? &nhfp->rcount : &nhfp->wcount;
        (void) fprintf(fp, "%08ld %s sz=%zu cnt=%d |%s|\n",
                      *iocount,
                       t1, sz, cnt, txtvalue);
//        (*iocount)++;
//        if (*iocount == 87)
//            __debugbreak();
        fflush(fp);
    }
}

char *sfvalue_char(char *a, int n)
{
    int i;
    static char buf[120];
    char *cp;

    cp = &buf[0];
    if (n < (int) (sizeof buf - 1))
        buf[n] = '\0';
    else
        buf[(int) (sizeof buf - 1)] = '\0';
    for (i = 0; i < n; ++i, ++cp, ++a)
        *cp = *a;
    *cp = '\0';
    return buf;
}

char *sfvalue_boolean(boolean *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%s",
             (*a == 0) ? "false" : "true");
    return buf;
}

char *sfvalue_schar(schar *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

char * sfvalue_aligntyp(aligntyp *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

char *
sfvalue_any(anything *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf,
             "%" PRId64,
             a->a_int64);
    return buf;
}

char *
sfvalue_genericptr(genericptr a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%s", 
             (a == 0) ? "0" : "glorkum");
    return buf;
}

char * sfvalue_int16(int16 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", *a);
    return buf;
}

char * sfvalue_int32(int32 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%" PRId32, *a);
    return buf;
}

char * sfvalue_int64(int64 *a)
{
    static char buf[20];
    Snprintf(buf, sizeof buf, "%" PRId64, *a);
    return buf;
}

char * sfvalue_uchar(uchar *a)
{
    static char buf[20];
    unsigned x;

    x = *a;
    Snprintf(buf, sizeof buf, "%03u", x);
    return buf;
}

char * sfvalue_uint16(uint16 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", (uint) *a);
    return buf;
}

char * sfvalue_uint32(uint32 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%" PRIu32, *a);
    return buf;
}

char * sfvalue_uint64(uint64 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%" PRIu64, *a);
    return buf;
}

char * sfvalue_size_t(size_t *a UNUSED)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%s", (char *) "");
    return buf;
}

char * sfvalue_time_t(time_t *a UNUSED)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%s", (char *) "");
    return buf;
}

char * sfvalue_short(short *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

char * sfvalue_ushort(ushort *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", (unsigned) *a);
    return buf;
}

char * sfvalue_int(int *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", *a);
    return buf;
}

char * sfvalue_unsigned(unsigned *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", *a);
    return buf;
}

char * sfvalue_long(long *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%ld", *a);
    return buf;
}

char * sfvalue_ulong(ulong *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%lu", *a);
    return buf;
}

char * sfvalue_xint8(xint8 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

char * sfvalue_xint16(xint16 *a)
{
    static char buf[20];


    Snprintf(buf, sizeof buf, "%d", (int) *a);
    return buf;
}

char *
sfvalue_bitfield(uint8 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", (uint) *a);
    return buf;
}

char *
bitfield_dump(uint8 *a)
{
    static char buf[20];

    Snprintf(buf, sizeof buf, "%u", (uint) *a);
    return buf;
}
char *
complex_dump(uchar *a)
{
    int i;
    uchar *uc = a;
    static char buf[50];
    unsigned x[10];

    for (i = 0; i < SIZE(x); ++i) {
        x[i] = *uc++;
    }
    Snprintf(buf, sizeof buf, "%03x %03x %03x %03x %03x %03x %03x %03x %03x %03x", 
             x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8], x[9]);
    buf[40] = '\0';
    return buf;
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
    sfoflprocs[exportascii] = zerosfoflprocs;
    sfiflprocs[exportascii] = zerosfiflprocs;
}

void
sf_setprocs(int idx, struct sf_structlevel_procs *sfi, struct sf_structlevel_procs *sfo)
{
    sfoprocs[idx] = *sfo;
    sfiprocs[idx] = *sfi;
}
void
sf_setflprocs(int idx, struct sf_fieldlevel_procs *flsfi,
            struct sf_fieldlevel_procs *flsfo)
{
    sfoflprocs[idx] = *flsfo;
    sfiflprocs[idx] = *flsfi;
}

#ifndef SFCTOOL
void normalize_pointers_any(union any *d_any);
void normalize_pointers_align(struct align *d_align);
void normalize_pointers_arti_info(struct arti_info *d_arti_info);
void normalize_pointers_attribs(struct attribs *d_attribs);
void normalize_pointers_bill_x(struct bill_x *d_bill_x);
void normalize_pointers_branch(struct branch *d_branch);
void normalize_pointers_bubble(struct bubble *d_bubble);
void normalize_pointers_cemetery(struct cemetery *d_cemetery);
void normalize_pointers_context_info(struct context_info *d_context_info);
void normalize_pointers_achievement_tracking(
    struct achievement_tracking *d_achievement_tracking);
void normalize_pointers_book_info(struct book_info *d_book_info);
void normalize_pointers_dig_info(struct dig_info *d_dig_info);
void normalize_pointers_engrave_info(struct engrave_info *d_engrave_info);
void normalize_pointers_obj_split(struct obj_split *d_obj_split);
void normalize_pointers_polearm_info(struct polearm_info *d_polearm_info);
void normalize_pointers_takeoff_info(struct takeoff_info *d_takeoff_info);
void normalize_pointers_tin_info(struct tin_info *d_tin_info);
void normalize_pointers_tribute_info(struct tribute_info *d_tribute_info);
void normalize_pointers_victual_info(struct victual_info *d_victual_info);
void normalize_pointers_warntype_info(struct warntype_info *d_warntype_info);
void normalize_pointers_d_flags(struct d_flags *d_d_flags);
void normalize_pointers_d_level(struct d_level *d_d_level);
void normalize_pointers_damage(struct damage *d_damage);
void normalize_pointers_dest_area(struct dest_area *d_dest_area);
void normalize_pointers_dgn_topology(struct dgn_topology *d_dgn_topology);
void normalize_pointers_dungeon(struct dungeon *d_dungeon);
void normalize_pointers_ebones(struct ebones *d_ebones);
void normalize_pointers_edog(struct edog *d_edog);
void normalize_pointers_egd(struct egd *d_egd);
void normalize_pointers_emin(struct emin *d_emin);
void normalize_pointers_engr(struct engr *d_engr);
void normalize_pointers_epri(struct epri *d_epri);
void normalize_pointers_eshk(struct eshk *d_eshk);
void normalize_pointers_fakecorridor(struct fakecorridor *d_fakecorridor);
void normalize_pointers_fe(struct fe *d_fe);
void normalize_pointers_flag(struct flag *d_flag);
void normalize_pointers_fruit(struct fruit *d_fruit);
void normalize_pointers_gamelog_line(struct gamelog_line *d_gamelog_line);
void normalize_pointers_kinfo(struct kinfo *d_kinfo);
void normalize_pointers_levelflags(struct levelflags *d_levelflags);
void normalize_pointers_linfo(struct linfo *d_linfo);
void normalize_pointers_ls_t(struct ls_t *d_ls_t);
void normalize_pointers_mapseen_feat(struct mapseen_feat *d_mapseen_feat);
void normalize_pointers_mapseen_flags(struct mapseen_flags *d_mapseen_flags);
void normalize_pointers_mapseen_rooms(struct mapseen_rooms *d_mapseen_rooms);
void normalize_pointers_mapseen(struct mapseen *d_mapseen);
void normalize_pointers_mextra(struct mextra *d_mextra);
void normalize_pointers_mkroom(struct mkroom *d_mkroom);
void normalize_pointers_monst(struct monst *d_monst);
void normalize_pointers_mvitals(struct mvitals *d_mvitals);
void normalize_pointers_nhcoord(struct nhcoord *d_nhcoord);
void normalize_pointers_nhrect(struct nhrect *d_nhrect);
void normalize_pointers_novel_tracking(struct novel_tracking *d_novel_tracking);
void normalize_pointers_obj(struct obj *d_obj);
void normalize_pointers_objclass(struct objclass *d_objclass);
void normalize_pointers_oextra(struct oextra *d_oextra);
void normalize_pointers_prop(struct prop *d_prop);
void normalize_pointers_q_score(struct q_score *d_q_score);
void normalize_pointers_rm(struct rm *d_rm);
void normalize_pointers_s_level(struct s_level *d_s_level);
void normalize_pointers_skills(struct skills *d_skills);
void normalize_pointers_spell(struct spell *d_spell);
void normalize_pointers_stairway(struct stairway *d_stairway);
void normalize_pointers_trap(struct trap *d_trap);
void normalize_pointers_u_conduct(struct u_conduct *d_u_conduct);
void normalize_pointers_u_event(struct u_event *d_u_event);
void normalize_pointers_u_have(struct u_have *d_u_have);
void normalize_pointers_u_realtime(struct u_realtime *d_u_realtime);
void normalize_pointers_u_roleplay(struct u_roleplay *d_u_roleplay);
void normalize_pointers_version_info(struct version_info *d_version_info);
void normalize_pointers_vlaunchinfo(union vlaunchinfo *d_vlaunchinfo);
void normalize_pointers_vptrs(union vptrs *d_vptrs);
void normalize_pointers_you(struct you *d_you);

void
normalize_pointers_any(union any *d_any UNUSED)
{
}
void
normalize_pointers_align(struct align *d_align UNUSED)
{
}

void
normalize_pointers_arti_info(struct arti_info *d_arti_info UNUSED)
{
}

void
normalize_pointers_attribs(struct attribs *d_attribs UNUSED)
{
}

void
normalize_pointers_bill_x(struct bill_x *d_bill_x UNUSED)
{
}

void
normalize_pointers_branch(struct branch *d_branch UNUSED)
{
}

void
normalize_pointers_bubble(struct bubble *d_bubble UNUSED)
{
}

void
normalize_pointers_cemetery(struct cemetery *d_cemetery UNUSED)
{
}

void
normalize_pointers_context_info(struct context_info *d_context_info UNUSED)
{
}

void
normalize_pointers_achievement_tracking(struct achievement_tracking *d_achievement_tracking UNUSED)
{
}

void
normalize_pointers_book_info(struct book_info *d_book_info UNUSED)
{
}

void
normalize_pointers_dig_info(struct dig_info *d_dig_info UNUSED)
{
}

void
normalize_pointers_engrave_info(struct engrave_info *d_engrave_info UNUSED)
{
}

void
normalize_pointers_obj_split(struct obj_split *d_obj_split UNUSED)
{
}

void
normalize_pointers_polearm_info(struct polearm_info *d_polearm_info UNUSED)
{
}

void
normalize_pointers_takeoff_info(struct takeoff_info *d_takeoff_info UNUSED)
{
}

void
normalize_pointers_tin_info(struct tin_info *d_tin_info UNUSED)
{
}

void
normalize_pointers_tribute_info(struct tribute_info *d_tribute_info UNUSED)
{
}

void
normalize_pointers_victual_info(struct victual_info *d_victual_info UNUSED)
{
}

void
normalize_pointers_warntype_info(struct warntype_info *d_warntype_info UNUSED)
{
}

void
normalize_pointers_d_flags(struct d_flags *d_d_flags UNUSED)
{
}

void
normalize_pointers_d_level(struct d_level *d_d_level UNUSED)
{
}

void
normalize_pointers_damage(struct damage *d_damage UNUSED)
{
}

void
normalize_pointers_dest_area(struct dest_area *d_dest_area UNUSED)
{
}

void
normalize_pointers_dgn_topology(struct dgn_topology *d_dgn_topology UNUSED)
{
}

void
normalize_pointers_dungeon(struct dungeon *d_dungeon UNUSED)
{
}

void
normalize_pointers_ebones(struct ebones *d_ebones UNUSED)
{
}

void
normalize_pointers_edog(struct edog *d_edog UNUSED)
{
}

void
normalize_pointers_egd(struct egd *d_egd UNUSED)
{
}

void
normalize_pointers_emin(struct emin *d_emin UNUSED)
{
}

void
normalize_pointers_engr(struct engr *d_engr UNUSED)
{
}

void
normalize_pointers_epri(struct epri *d_epri UNUSED)
{
}

void
normalize_pointers_eshk(struct eshk *d_eshk UNUSED)
{
}

void
normalize_pointers_fakecorridor(struct fakecorridor *d_fakecorridor UNUSED)
{
}

void
normalize_pointers_fe(struct fe *d_fe UNUSED)
{
}

void
normalize_pointers_flag(struct flag *d_flag UNUSED)
{
}

void
normalize_pointers_fruit(struct fruit *d_fruit UNUSED)
{
}

void
normalize_pointers_gamelog_line(struct gamelog_line *d_gamelog_line UNUSED)
{
}

void
normalize_pointers_kinfo(struct kinfo *d_kinfo UNUSED)
{
}

void
normalize_pointers_levelflags(struct levelflags *d_levelflags UNUSED)
{
}

void
normalize_pointers_linfo(struct linfo *d_linfo UNUSED)
{
}

void
normalize_pointers_ls_t(struct ls_t *d_ls_t UNUSED)
{
}

void
normalize_pointers_mapseen_feat(struct mapseen_feat *d_mapseen_feat UNUSED)
{
}

void
normalize_pointers_mapseen_flags(struct mapseen_flags *d_mapseen_flags UNUSED)
{
}

void
normalize_pointers_mapseen_rooms(struct mapseen_rooms *d_mapseen_rooms UNUSED)
{
}

void
normalize_pointers_mapseen(struct mapseen *d_mapseen UNUSED)
{
}

void
normalize_pointers_mextra(struct mextra *d_mextra UNUSED)
{
}

void
normalize_pointers_mkroom(struct mkroom *d_mkroom UNUSED)
{
}

void
normalize_pointers_monst(struct monst *d_monst UNUSED)
{
}

void
normalize_pointers_mvitals(struct mvitals *d_mvitals UNUSED)
{
}

void
normalize_pointers_nhcoord(struct nhcoord *d_nhcoord UNUSED)
{
}

void
normalize_pointers_nhrect(struct nhrect *d_nhrect UNUSED)
{
}

void
normalize_pointers_novel_tracking(struct novel_tracking *d_novel_tracking UNUSED)
{
}

void
normalize_pointers_obj(struct obj *d_obj UNUSED)
{
}

void
normalize_pointers_objclass(struct objclass *d_objclass UNUSED)
{
}

void
normalize_pointers_oextra(struct oextra *d_oextra UNUSED)
{
}

void
normalize_pointers_prop(struct prop *d_prop UNUSED)
{
}

void
normalize_pointers_q_score(struct q_score *d_q_score UNUSED)
{
}

void
normalize_pointers_rm(struct rm *d_rm UNUSED)
{
}

void
normalize_pointers_s_level(struct s_level *d_s_level UNUSED)
{
}

void
normalize_pointers_skills(struct skills *d_skills UNUSED)
{
}

void
normalize_pointers_spell(struct spell *d_spell UNUSED)
{
}

void
normalize_pointers_stairway(struct stairway *d_stairway UNUSED)
{
}

void
normalize_pointers_trap(struct trap *d_trap UNUSED)
{
}

void
normalize_pointers_u_conduct(struct u_conduct *d_u_conduct UNUSED)
{
}

void
normalize_pointers_u_event(struct u_event *d_u_event UNUSED)
{
}

void
normalize_pointers_u_have(struct u_have *d_u_have UNUSED)
{
}

void
normalize_pointers_u_realtime(struct u_realtime *d_u_realtime UNUSED)
{
}

void
normalize_pointers_u_roleplay(struct u_roleplay *d_u_roleplay UNUSED)
{
}

void
normalize_pointers_version_info(struct version_info *d_version_info UNUSED)
{
}

void
normalize_pointers_vlaunchinfo(union vlaunchinfo *d_vlaunchinfo UNUSED)
{
}

void
normalize_pointers_vptrs(union vptrs *d_vptrs UNUSED)
{
}

void
normalize_pointers_you(struct you *d_you UNUSED)
{
}
#endif  /* SFCTOOL */
