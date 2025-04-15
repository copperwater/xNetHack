/* NetHack 3.7	version.c	$NHDT-Date: 1737622664 2025/01/23 00:57:44 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.105 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2018. */
/* NetHack may be freely redistributed.  See license for details. */


#include "hack.h"
#include "dlb.h"

#ifndef MINIMAL_FOR_RECOVER

#ifndef OPTIONS_AT_RUNTIME
#define OPTIONS_AT_RUNTIME
#endif

staticfn void insert_rtoption(char *) NONNULLARG1;

/* fill buffer with short version (so caller can avoid including date.h)
 * buf cannot be NULL */
char *
version_string(char *buf, size_t bufsz)
{
    Snprintf(buf, bufsz, "%s",
             ((nomakedefs.version_string && nomakedefs.version_string[0])
              ? nomakedefs.version_string
              /* in case we try to write a paniclog entry after releasing
                 the 'nomakedefs' data */
              : mdlib_version_string(buf, ".")));
    return buf;
}

/* fill and return the given buffer with the long nethack version string */
char *
getversionstring(char *buf, size_t bufsz)
{
    Strcpy(buf, nomakedefs.version_id);

    {
        int c = 0;
#if defined(RUNTIME_PORT_ID)
        char tmpbuf[BUFSZ], *tmp;
#endif
        char *p = eos(buf);
        boolean dotoff = (p > buf && p[-1] == '.');

        if (dotoff)
            --p;
        Strcpy(p, " (");
#if defined(RUNTIME_PORT_ID)
        tmp = get_port_id(tmpbuf);
        if (tmp)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%s%s", c++ ? "," : "", tmp);
#endif
        if (nomakedefs.git_sha)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%s%s", c++ ? "," : "", nomakedefs.git_sha);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
        if (nomakedefs.git_branch)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%sbranch:%s",
                     c++ ? "," : "", nomakedefs.git_branch);
#endif
        if (nomakedefs.git_prefix)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%sprefix:%s",
                     c++ ? "," : "", nomakedefs.git_prefix);
        if (c)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%s", ")");
        else /* if nothing has been added, strip " (" back off */
            *p = '\0';
        if (dotoff)
            Snprintf(eos(buf), (bufsz - strlen(buf)) - 1,
                     "%s", ".");
    }
    return buf;
}

/* version info that could be displayed on status lines;
     "<game name> <git branch name> <x.y.z version number>";
   if game name is a prefix of--or same as--branch name, it is omitted
     "<git branch name> <x.y.z version number>";
   after release--or if branch info is unavailable--it will be
     "<game name> <x.y.z version number>";
   game name or branch name or both can be requested via flags */
char *
status_version(char *buf, size_t bufsz, boolean indent)
{
    const char *name = NULL, *altname = NULL, *indentation;
    unsigned vflags = flags.versinfo;
    boolean shownum = ((vflags & VI_NUMBER) != 0),
            showname = ((vflags & VI_NAME) != 0),
            showbranch = ((vflags & VI_BRANCH) != 0);

    /* game's name {variants should use own name, not "NetHack"} */
    if (showname) {
#ifdef VERS_GAME_NAME /* can be set to override default (base of filename) */
        name = VERS_GAME_NAME;
#else
        name = nh_basename(gh.hname, FALSE); /* hname is from xxxmain.c */
#endif
        if (!name || !*name) /* shouldn't happen */
            showname = FALSE;
    }
    /* git branch name, if available */
    if (showbranch) {
#if 1   /*#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)*/
        altname = nomakedefs.git_branch;
#endif
        if (!altname || !*altname)
            showbranch = FALSE;
    }
    if (showname && showbranch) {
        if (!strncmpi(name, altname, strlen(name)))
            showname = FALSE;
#if 0
        /* note: it's possible for branch name to be a prefix of game name
           but that's unlikely enough that we won't bother with it; having
           branch "nethack-3.7" be a superset of game "nethack" seems like
           including both is redundant, but having branch "net" be a subset
           of game "nethack" doesn't feel that way; optimizing "net" out
           seems like it would be a mistake */
        else if (!strncmpi(altname, name, strlen(altname)))
            showbranch = FALSE;
#endif
    } else if (!showname && !showbranch) {
        /* flags.versinfo could be set to only 'branch' but it might not
           be available */
        shownum = TRUE;
    }

    *buf = '\0';
    indentation = indent ? " " : "";
    if (showname) {
        Snprintf(eos(buf), bufsz - strlen(buf), "%s%s", indentation, name);
        indentation = " "; /* forced separator rather than optional indent */
    }
    if (showbranch) {
        Snprintf(eos(buf), bufsz - strlen(buf), "%s%s", indentation, altname);
        indentation = " ";
    }
    if (shownum) {
        /* x.y.z version number */
        Snprintf(eos(buf), bufsz - strlen(buf), "%s%s", indentation,
                 (nomakedefs.version_string && nomakedefs.version_string[0])
                     ? nomakedefs.version_string
                     : mdlib_version_string(buf, "."));
    }
    return buf;
}

/* the #versionshort command */
int
doversion(void)
{
    char buf[BUFSZ];

    pline("%s", getversionstring(buf, sizeof buf));
    return ECMD_OK;
}

/* the '#version' command; also a choice for '?' */
int
doextversion(void)
{
    int rtcontext = 0;
    const char *rtbuf;
    dlb *f = (dlb *) 0;
    char buf[BUFSZ], *p = 0;
    winid win = create_nhwindow(NHW_TEXT);
    boolean use_dlb = TRUE,
            done_rt = FALSE,
            done_dlb = FALSE,
            prolog;
    /* lua_info[] moved to util/mdlib.c and rendered via do_runtime_info() */

#if defined(OPTIONS_AT_RUNTIME)
    use_dlb = FALSE;
#else
    done_rt = TRUE;
#endif

    /* instead of using ``display_file(OPTIONS_USED,TRUE)'' we handle
       the file manually so we can include dynamic version info */

    (void) getversionstring(buf, sizeof buf);
    /* if extra text (git info) is present, put it on separate line
       but don't wrap on (x86) */
    if (strlen(buf) >= COLNO)
        p = strrchr(buf, '(');
    if (p && p > buf && p[-1] == ' ' && p[1] != 'x')
        p[-1] = '\0';
    else
        p = 0;
    putstr(win, 0, buf);
    if (p) {
        *--p = ' ';
        putstr(win, 0, p);
    }

    if (use_dlb) {
        f = dlb_fopen(OPTIONS_USED, "r");
        if (!f) {
            putstr(win, 0, "");
            Sprintf(buf, "[Configuration '%s' not available?]", OPTIONS_USED);
            putstr(win, 0, buf);
            done_dlb = TRUE;
        }
    }
    /*
     * already inserted above:
     * + outdented program name and version plus build date and time
     * dat/options; display contents with lines prefixed by '-' deleted:
     * - blank-line
     * -     indented program name and version
     *   blank-line
     *   outdented feature header
     * - blank-line
     *       indented feature list
     *       spread over multiple lines
     *   blank-line
     *   outdented windowing header
     * - blank-line
     *       indented windowing choices with
     *       optional second line for default
     * - blank-line
     * - EOF
     */

    prolog = TRUE; /* to skip indented program name */
    for (;;) {
        if (use_dlb && !done_dlb) {
            if (!dlb_fgets(buf, BUFSZ, f)) {
                done_dlb = TRUE;
                continue;
            }
        } else if (!done_rt) {
            if (!(rtbuf = do_runtime_info(&rtcontext))) {
                done_rt = TRUE;
                continue;
            }
            (void) strncpy(buf, rtbuf, BUFSZ - 1);
            buf[BUFSZ - 1] = '\0';
        } else {
            break;
        }
        (void) strip_newline(buf);
        if (strchr(buf, '\t') != 0)
            (void) tabexpand(buf);

        if (*buf && *buf != ' ') {
            /* found outdented header; insert a separator since we'll
               have skipped corresponding blank line inside the file */
            putstr(win, 0, "");
            prolog = FALSE;
        }
        /* skip blank lines and prolog (progame name plus version) */
        if (prolog || !*buf)
            continue;

        if (strchr(buf, ':'))
            insert_rtoption(buf);

        if (*buf)
            putstr(win, 0, buf);
    }
    if (use_dlb)
        (void) dlb_fclose(f);
    display_nhwindow(win, FALSE);
    destroy_nhwindow(win);
    return ECMD_OK;
}

void
early_version_info(boolean pastebuf)
{
    char buf1[BUFSZ], buf2[BUFSZ];
    char *buf, *tmp;

    Snprintf(buf1, sizeof buf1, "test");
    /* this is early enough that we have to do our own line-splitting */
    getversionstring(buf1, sizeof buf1);
    tmp = strstri(buf1, " ("); /* split at start of version info */
    if (tmp) {
        /* retain one buffer so that it all goes into the paste buffer */
        *tmp++ = '\0';
        Snprintf(buf2, sizeof (buf2),"%s\n%s", buf1, tmp);
        buf = buf2;
    } else {
        buf = buf1;
    }

    raw_printf("%s", buf);

    if (pastebuf) {
#if defined(RUNTIME_PASTEBUF_SUPPORT) && !defined(LIBNH)
        /*
         * Call a platform/port-specific routine to insert the
         * version information into a paste buffer. Useful for
         * easy inclusion in bug reports.
         */
        port_insert_pastebuf(buf);
#else
        raw_printf("%s", "Paste buffer copy is not available.\n");
#endif
    }
}

extern const char regex_id[];

/*
 * makedefs should put the first token into dat/options; we'll substitute
 * the second value for it.  The token must contain at least one colon
 * so that we can spot it, and should not contain spaces so that makedefs
 * won't split it across lines.  Ideally the length should be close to
 * that of the substituted value since we don't do phrase-splitting/line-
 * wrapping when displaying it.
 */
static struct rt_opt {
    const char *token, *value;
} rt_opts[] = {
    { ":PATMATCH:", regex_id },
    { ":LUAVERSION:", (const char *) gl.lua_ver },
    { ":LUACOPYRIGHT:", (const char *) gl.lua_copyright },
};

/*
 * 3.6.0
 * Some optional stuff is no longer available to makedefs because
 * it depends which of several object files got linked into the
 * game image, so we insert those options here.
 */
staticfn void
insert_rtoption(char *buf)
{
    int i;

    if (!gl.lua_ver[0])
        get_lua_version();

    for (i = 0; i < SIZE(rt_opts); ++i) {
        if (strstri(buf, rt_opts[i].token) && *rt_opts[i].value) {
            (void) strsubst(buf, rt_opts[i].token, rt_opts[i].value);
        }
        /* we don't break out of the loop after a match; there might be
           other matches on the same line */
    }
}

#ifdef MICRO
boolean
comp_times(long filetime)
{
    /* BUILD_TIME is constant but might have L suffix rather than UL;
       'filetime' is historically signed but ought to have been unsigned */
    return ((unsigned long) filetime < (unsigned long) nomakedefs.build_time);
}
#endif

boolean
check_version(
    struct version_info *version_data,
    const char *filename,
    boolean complain,
    unsigned long utdflags)
{
    if (!filename) {
#ifdef EXTRA_SANITY_CHECKS
        if (complain)
            impossible("check_version() called with"
                       " 'complain'=True but 'filename'=Null");
#endif
        complain = FALSE; /* 'complain' requires 'filename' for pline("%s") */
    }
    if (
#ifdef VERSION_COMPATIBILITY /* patchlevel.h */
        version_data->incarnation < VERSION_COMPATIBILITY
        || version_data->incarnation > nomakedefs.version_number
#else
        version_data->incarnation != nomakedefs.version_number
#endif
        ) {
        if (complain) {
            pline("Version mismatch for file \"%s\".", filename);
            if (WIN_MESSAGE != WIN_ERR)
                 display_nhwindow(WIN_MESSAGE, TRUE);
        }
        return FALSE;
    } else if (
        (version_data->feature_set & ~nomakedefs.ignored_features)
            != (nomakedefs.version_features & ~nomakedefs.ignored_features)
        || ((utdflags & UTD_SKIP_SANITY1) == 0
             && version_data->entity_count != nomakedefs.version_sanity1)
        ) {
        if (complain) {
            pline("Configuration incompatibility for file \"%s\".", filename);
            display_nhwindow(WIN_MESSAGE, TRUE);
        }
        return FALSE;
    }
    return TRUE;
}

void
store_version(NHFILE *nhfp)
{
    struct version_info version_data = {
        0UL, 0UL, 0UL,
    };

    /* actual version number */
    version_data.incarnation = nomakedefs.version_number;
    /* bitmask of config settings */
    version_data.feature_set = nomakedefs.version_features;
    /* # of monsters and objects */
    version_data.entity_count  = nomakedefs.version_sanity1;

    /* bwrite() before bufon() uses plain write() */
    if (nhfp->structlevel)
        bufoff(nhfp->fd);

    store_critical_bytes(nhfp);
    if (nhfp->structlevel) {
        bwrite(nhfp->fd, (genericptr_t) &version_data,
               (unsigned) (sizeof version_data));
    }

    if (nhfp->structlevel)
         bufon(nhfp->fd);
    return;
}

#ifdef AMIGA
const char amiga_version_string[] = AMIGA_VERSION_STRING;
#endif

unsigned long
get_feature_notice_ver(char *str)
{
    char buf[BUFSZ];
    int ver_maj, ver_min, patch;
    char *istr[3];
    int j = 0;

    if (!str)
        return 0L;
    str = strcpy(buf, str);
    istr[j] = str;
    while (*str) {
        if (*str == '.') {
            *str++ = '\0';
            j++;
            istr[j] = str;
            if (j == 2)
                break;
        } else if (strchr("0123456789", *str) != 0) {
            str++;
        } else
            return 0L;
    }
    if (j != 2)
        return 0L;
    ver_maj = atoi(istr[0]);
    ver_min = atoi(istr[1]);
    patch = atoi(istr[2]);
    return FEATURE_NOTICE_VER(ver_maj, ver_min, patch);
    /* macro from hack.h */
}

unsigned long
get_current_feature_ver(void)
{
    return FEATURE_NOTICE_VER(VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
}

/*ARGUSED*/
const char *
copyright_banner_line(int indx)
{
#ifdef COPYRIGHT_BANNER_A
    if (indx == 1)
        return COPYRIGHT_BANNER_A;
#endif
#ifdef COPYRIGHT_BANNER_B
    if (indx == 2)
        return COPYRIGHT_BANNER_B;
#endif

    if (indx == 3)
        return nomakedefs.copyright_banner_c;

#ifdef COPYRIGHT_BANNER_D
    if (indx == 4)
        return COPYRIGHT_BANNER_D;
#endif
    return "";
}

/* called by argcheck(allmain.c) from early_options(sys/xxx/xxxmain.c) */
void
dump_version_info(void)
{
    char buf[BUFSZ];
    const char *hname = gh.hname ? gh.hname : "nethack";

    if (strlen(hname) > 33)
        hname = eos(nhStr(hname)) - 33; /* discard const for eos() */
    runtime_info_init();
    Snprintf(buf, sizeof buf, "%-12.33s %08lx %08lx %08lx",
             hname,
             nomakedefs.version_number,
             (nomakedefs.version_features & ~nomakedefs.ignored_features),
             nomakedefs.version_sanity1);
    raw_print(buf);
    release_runtime_info();
    return;
}
#endif /* MINIMAL_FOR_RECOVER */

struct critical_sizes_with_names {
    uchar ucsize;
    const char *nm;
};

struct critical_sizes_with_names critical_sizes[] = {
    { 0, "unused" },
    /* simple types, that don't have subfields */
    { (uchar) sizeof(short), "short" },
    { (uchar) sizeof(int), "int" },
    { (uchar) sizeof(long), "long" },
    { (uchar) sizeof(long long), "long long" },
    { (uchar) sizeof(genericptr_t), "genericptr_t" },
    { (uchar) sizeof(aligntyp), "aligntyp" },
    { (uchar) sizeof(boolean), "boolean" },
    { (uchar) sizeof(coordxy), "coordxy" },
    { (uchar) sizeof(int16), "int16" },
    { (uchar) sizeof(int32), "int32" },
    { (uchar) sizeof(int64), "int64" },
    { (uchar) sizeof(schar), "schar" },
    { (uchar) sizeof(size_t), "size_t" },
    { (uchar) sizeof(uchar), "uchar" },
    { (uchar) sizeof(uint16), "uint16" },
    { (uchar) sizeof(uint32), "uint32" },
    { (uchar) sizeof(uint64), "uint64" },
    { (uchar) sizeof(ulong), "ulong" },
    { (uchar) sizeof(unsigned), "unsigned" },
    { (uchar) sizeof(ushort), "ushort" },
    { (uchar) sizeof(xint16), "xint16" },
    { (uchar) sizeof(xint8), "xint8" },
    /* complex - they break down into one or more simple types */
    { (uchar) sizeof(struct arti_info), "struct arti_info" },
    { (uchar) sizeof(struct nhrect), "struct nhrect" },
    { (uchar) sizeof(struct branch), "struct branch" },
    { (uchar) sizeof(struct bubble), "struct bubble" },
    { (uchar) sizeof(struct cemetery), "struct cemetery" },
    { (uchar) sizeof(struct context_info), "struct context_info" },
    { (uchar) sizeof(struct nhcoord), "struct nhcoord" },
    { (uchar) sizeof(struct damage), "struct damage" },
    { (uchar) sizeof(struct dest_area), "struct dest_area" },
    { (uchar) sizeof(struct dgn_topology), "struct dgn_topology" },
    { (uchar) sizeof(struct dungeon), "struct dungeon" },
    { (uchar) sizeof(struct d_level), "struct d_level" },
    { (uchar) sizeof(struct ebones), "struct ebones" },
    { (uchar) sizeof(struct edog), "struct edog" },
    { (uchar) sizeof(struct egd), "struct egd" },
    { (uchar) sizeof(struct emin), "struct emin" },
    { (uchar) sizeof(struct engr), "struct engr" },
    { (uchar) sizeof(struct epri), "struct epri" },
    { (uchar) sizeof(struct eshk), "struct eshk" },
    { (uchar) sizeof(struct fe), "struct fe" },
    { (uchar) sizeof(struct flag), "struct flag" },
    { (uchar) sizeof(struct fruit), "struct fruit" },
    { (uchar) sizeof(struct gamelog_line), "struct gamelog_line" },
    { (uchar) sizeof(struct kinfo), "struct kinfo" },
    { (uchar) sizeof(struct levelflags), "struct levelflags" },
    { (uchar) sizeof(struct ls_t), "struct ls_t" },
    { (uchar) sizeof(struct linfo), "struct linfo" },
    { (uchar) sizeof(struct mapseen_feat), "struct mapseen_feat" },
    { (uchar) sizeof(struct mapseen_flags), "struct mapseen_flags" },
    { (uchar) sizeof(struct mapseen_rooms), "struct mapseen_rooms" },
    { (uchar) sizeof(struct mextra), "struct mextra" },
    { (uchar) sizeof(struct mkroom), "struct mkroom" },
    { (uchar) sizeof(struct monst), "struct monst" },
    { (uchar) sizeof(struct mvitals), "struct mvitals" },
    { (uchar) sizeof(struct obj), "struct obj" },
    { (uchar) sizeof(struct objclass), "struct objclass" },
    { (uchar) sizeof(struct oextra), "struct oextra" },
    { (uchar) sizeof(struct q_score), "struct q_score" },
    { (uchar) sizeof(struct rm), "struct rm" },
    { (uchar) sizeof(struct spell), "struct spell" },
    { (uchar) sizeof(struct stairway), "struct stairway" },
    { (uchar) sizeof(struct s_level), "struct s_level" },
    { (uchar) sizeof(struct trap), "struct trap" },
    { (uchar) sizeof(struct version_info), "struct version_info" },
    { (uchar) sizeof(anything), "anything" },
    /* struct you requires 2 bytes */
    { (uchar) ((sizeof(struct you) & 0x00FF)), "you_LO" },
    { (uchar) ((sizeof(struct you) & 0xFF00) >> 8), "you_HI" },
#ifdef SF_INCLUDE_SUBSTRUCTS
    /*
     * the ones below are substructures of the ones
     * above, so there is no need to check these directly.
     */
    { (uchar) sizeof(struct attribs), "struct attribs" },
    { (uchar) sizeof(struct dig_info), "struct dig_info" },
    { (uchar) sizeof(struct tin_info), "struct tin_info" },
    { (uchar) sizeof(struct book_info), "struct book_info" },
    { (uchar) sizeof(struct takeoff_info), "struct takeoff_info" },
    { (uchar) sizeof(struct victual_info), "struct victual_info" },
    { (uchar) sizeof(struct engrave_info), "struct engrave_info" },
    { (uchar) sizeof(struct warntype_info), "struct warntype_info" },
    { (uchar) sizeof(struct polearm_info), "struct polearm_info" },
    { (uchar) sizeof(struct obj_split), "struct obj_split" },
    { (uchar) sizeof(struct tribute_info), "struct tribute_info" },
    { (uchar) sizeof(struct novel_tracking), "struct novel_tracking" },
    { (uchar) sizeof(struct achievement_tracking),
      "struct achievement_tracking" },
    { (uchar) sizeof(struct d_flags), "struct d_flags" },
    { (uchar) sizeof(struct mapseen), "struct mapseen" },
    { (uchar) sizeof(struct fakecorridor), "struct fakecorridor" },
    { (uchar) sizeof(struct bill_x), "struct bill_x" },
    { (uchar) sizeof(union vptrs), "union vptrs" },
    { (uchar) sizeof(struct prop), "struct prop" },
    { (uchar) sizeof(struct skills), "struct skills" },
    { (uchar) sizeof(union vlaunchinfo), "union vlaunchinfo" },
    { (uchar) sizeof(struct u_have), "struct u_have" },
    { (uchar) sizeof(struct u_event), "struct u_event" },
    { (uchar) sizeof(struct u_realtime), "struct u_realtime" },
    { (uchar) sizeof(struct u_conduct), "struct u_conduct" },
    { (uchar) sizeof(struct u_roleplay), "struct u_roleplay" },
#endif /* SF_INCLUDE_SUBSTRUCTS */
    /* 10 for future expansion without changing array size */
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
    { 0, "" },
};

uchar cscbuf[SIZE(critical_sizes)];

int
get_critical_size_count(void)
{
    return SIZE(critical_sizes);
}

#ifndef MINIMAL_FOR_RECOVER
void
store_critical_bytes(NHFILE *nhfp)
{
    int i, cnt;
    char indicate = 'u', csc_count = (char) SIZE(critical_sizes);
    /* int cmc = 0; */

    if (nhfp->mode & WRITING) {
        indicate = (nhfp->structlevel)      ? 'h'
                   : (nhfp->fnidx == ascii) ? 'a'
                                            : 'l';
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &indicate,
               (unsigned) sizeof indicate);
        }
        if (nhfp->structlevel) {
            bwrite(nhfp->fd, (genericptr_t) &csc_count,
               (unsigned) sizeof csc_count);
        }
        cnt = (int) csc_count;
        for (i = 0; i < cnt; ++i) {
            if (nhfp->structlevel) {
                bwrite(nhfp->fd, (genericptr_t) &critical_sizes[i].ucsize,
                       (unsigned) sizeof (uchar));
            }
        }
    }
}

/* this used to be based on file date and somewhat OS-dependent,
   but now examines the initial part of the file's contents */
boolean
uptodate(NHFILE *nhfp, const char *name, unsigned long utdflags)
{
    struct version_info vers_info;
    char indicator;
    boolean verbose = name ? TRUE : FALSE;
    int ccbresult = 0;
    /* int you_size = (int) sizeof (struct you); */

    if (nhfp->structlevel) {
        mread(nhfp->fd, (genericptr_t) &indicator, sizeof indicator);
    }
    if ((ccbresult = compare_critical_bytes(nhfp)) != 0) {
        if (ccbresult > 0) {
            raw_printf("compare of critical bytes failed at %d (%s).",
                       critical_sizes[ccbresult].ucsize,
                       critical_sizes[ccbresult].nm);
        }
        return FALSE;
    }

    if (nhfp->structlevel) {
        mread(nhfp->fd, (genericptr_t) &vers_info, sizeof vers_info);
    }
    if (!check_version(&vers_info, name, verbose, utdflags)) {
        if (verbose) {
            if ((utdflags & UTD_WITHOUT_WAITSYNCH_PERFILE) == 0)
                wait_synch();
        }
        return FALSE;
    }
    return TRUE;
}

int
compare_critical_bytes(NHFILE *nhfp)
{
    char active_csc_count = (char) SIZE(critical_sizes),
         file_csc_count = 0;
    int i, cnt = (int) active_csc_count;

    if (nhfp->structlevel) {
        mread(nhfp->fd, (genericptr_t) &file_csc_count,
              sizeof file_csc_count);
    }
    if (file_csc_count > cnt) {
        raw_printf("critical byte counts do not match"
                   ", file:%d, critical_sizes:%d.",
                   file_csc_count, SIZE(critical_sizes));
        return -1;
    }
    for (i = 0; i < (int) file_csc_count; ++i) {
        if (nhfp->structlevel) {
            mread(nhfp->fd, (genericptr_t) &cscbuf[i],
                  sizeof (uchar));
        }
    }
    for (i = 1; i < cnt; ++i) {
        if (cscbuf[i] != critical_sizes[i].ucsize)
            return i;
    }
    return 0; /* everything matched */
}
#endif /* MINIMAL_FOR_RECOVER */

/*version.c*/
