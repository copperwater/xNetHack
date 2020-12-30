/* NetHack 3.7	mapglyph.c	$NHDT-Date: 1596498176 2020/08/03 23:42:56 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.65 $ */
/* Copyright (c) David Cohrs, 1991                                */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#if defined(TTY_GRAPHICS)
#include "wintty.h" /* for prototype of has_color() only */
#endif
#include "color.h"
#define HI_DOMESTIC CLR_WHITE /* monst.c */

#if 0
#if !defined(TTY_GRAPHICS)
#define has_color(n) TRUE
#endif
#endif

#ifdef TEXTCOLOR
static const int explcolors[] = {
    CLR_BLACK,   /* dark    */
    CLR_GREEN,   /* noxious */
    CLR_BROWN,   /* muddy   */
    CLR_BLUE,    /* wet     */
    CLR_MAGENTA, /* magical */
    CLR_ORANGE,  /* fiery   */
    CLR_WHITE,   /* frosty  */
};

#define zap_color(n) color = iflags.use_color ? zapcolors[n] : NO_COLOR
#define cmap_color(n) color = iflags.use_color ? defsyms[n].color : NO_COLOR
#define obj_color(n) color = iflags.use_color ? objects[n].oc_color : NO_COLOR
#define mon_color(n) color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define invis_color(n) color = NO_COLOR
#define pet_color(n) color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define warn_color(n) \
    color = iflags.use_color ? def_warnsyms[n].color : NO_COLOR
#define explode_color(n) color = iflags.use_color ? explcolors[n] : NO_COLOR

#else /* no text color */

#define zap_color(n)
#define cmap_color(n)
#define obj_color(n)
#define mon_color(n)
#define invis_color(n)
#define pet_color(c)
#define warn_color(n)
#define explode_color(n)
#endif

#define is_objpile(x,y) (!Hallucination && g.level.objects[(x)][(y)] \
                         && g.level.objects[(x)][(y)]->nexthere)

#define GMAP_SET                 0x00000001
/* 0x00000002 used to be GMAP_ROGUELEVEL */
#define GMAP_ALTARCOLOR          0x00000004

/* Externify this array if it's ever needed anywhere else.
 * Colors are in the same order as materials are defined. */
static const int materialclr[] = {
    CLR_BLACK, HI_ORGANIC, CLR_WHITE, HI_ORGANIC, CLR_RED,
    CLR_WHITE, HI_CLOTH, HI_LEATHER, HI_WOOD, CLR_WHITE, CLR_BLACK,
    HI_METAL, HI_METAL, HI_COPPER, HI_SILVER, HI_GOLD, CLR_WHITE,
    HI_SILVER, CLR_WHITE, HI_GLASS, CLR_RED, CLR_GRAY
};

/*ARGSUSED*/
int
mapglyph(glyph, ochar, ocolor, ospecial, x, y, mgflags)
int glyph, *ocolor, x, y;
int *ochar;
unsigned *ospecial;
unsigned mgflags;
{
    register int offset, idx;
    int color = NO_COLOR;
    nhsym ch;
    unsigned special = 0;
    /* condense multiple tests in macro version down to single */
    boolean is_you = (x == u.ux && y == u.uy);
    struct engr* engr = engr_at(x, y);

    if (!g.glyphmap_perlevel_flags) {
        /*
         *    GMAP_SET                0x00000001
         *    GMAP_ALTARCOLOR         0x00000004
         */
        g.glyphmap_perlevel_flags |= GMAP_SET;

        if ((Is_astralevel(&u.uz) || Is_sanctum(&u.uz))) {
            g.glyphmap_perlevel_flags |= GMAP_ALTARCOLOR;
        }
    }

    /*
     *  Map the glyph back to a character and color.
     *
     *  Warning:  For speed, this makes an assumption on the order of
     *            offsets.  The order is set in display.h.
     */
    if ((offset = (glyph - GLYPH_NOTHING_OFF)) >= 0) {
        idx = SYM_NOTHING + SYM_OFF_X;
        color = NO_COLOR;
        special |= MG_NOTHING;
    } else if ((offset = (glyph - GLYPH_UNEXPLORED_OFF)) >= 0) {
        idx = SYM_UNEXPLORED + SYM_OFF_X;
        color = NO_COLOR;
        special |= MG_UNEXPL;
    } else if ((offset = (glyph - GLYPH_STATUE_OFF)) >= 0) { /* a statue */
        idx = mons[offset].mlet + SYM_OFF_M;
        struct obj* otmp = vobj_at(x, y);
        if (iflags.use_color && otmp && otmp->otyp == STATUE
            && otmp->material != MINERAL) {
            color = materialclr[otmp->material];
        }
        else
            obj_color(STATUE);
        special |= MG_STATUE;
        if (is_objpile(x,y))
            special |= MG_OBJPILE;
    } else if ((offset = (glyph - GLYPH_WARNING_OFF)) >= 0) { /* warn flash */
        idx = offset + SYM_OFF_W;
        warn_color(offset);
    } else if ((offset = (glyph - GLYPH_SWALLOW_OFF)) >= 0) { /* swallow */
        /* see swallow_to_glyph() in display.c */
        idx = (S_sw_tl + (offset & 0x7)) + SYM_OFF_P;
        mon_color(offset >> 3);
    } else if ((offset = (glyph - GLYPH_ZAP_OFF)) >= 0) { /* zap beam */
        /* see zapdir_to_glyph() in display.c */
        idx = (S_vbeam + (offset & 0x3)) + SYM_OFF_P;
        zap_color((offset >> 2));
    } else if ((offset = (glyph - GLYPH_EXPLODE_OFF)) >= 0) { /* explosion */
        idx = ((offset % MAXEXPCHARS) + S_explode1) + SYM_OFF_P;
        explode_color(offset / MAXEXPCHARS);
    } else if ((offset = (glyph - GLYPH_CMAP_OFF)) >= 0) { /* cmap */
        idx = offset + SYM_OFF_P;
        struct stairway *stairs = stairway_at(x, y);
#ifdef TEXTCOLOR
        if (is_cmap_door(offset) && door_is_iron(&levl[x][y])) {
            color = HI_METAL;
        /* provide a visible difference if normal and lit corridor
           use the same symbol */
        } else if (iflags.use_color && offset == S_litcorr
                   && g.showsyms[idx] == g.showsyms[S_corr + SYM_OFF_P]) {
            color = CLR_WHITE;
        /* show branch stairs in a different color */
        } else if (iflags.use_color
                   && (offset == S_upstair || offset == S_dnstair)
                   && (stairs && stairs->tolev.dnum != u.uz.dnum)
                   && (g.showsyms[idx] == g.showsyms[S_upstair + SYM_OFF_P]
                       || g.showsyms[idx] == g.showsyms[S_dnstair + SYM_OFF_P])) {
            color = CLR_YELLOW;
        }
        /* Colored Walls and Floors Patch */
        else if (iflags.use_color && offset >= S_vwall && offset <= S_trwall) {
            if (In_W_tower(x, y, &u.uz))
                color = CLR_MAGENTA;
            else if (In_sokoban(&u.uz))
                color = CLR_BLUE;
            else if (In_tower(&u.uz)) /* Vlad's */
                color = CLR_BLACK;
            else if (In_mines(&u.uz)) /* no in_rooms check */
                color = CLR_BROWN;
            else if (In_hell(&u.uz) && !Is_valley(&u.uz)) {
                if (getroomtype(x, y) == ABBATOIR)
                    color = CLR_RED;
                else if (getroomtype(x, y) == SEMINARY)
                    color = CLR_WHITE;
                else
                    color = CLR_ORANGE;
            }
            else if (Is_astralevel(&u.uz))
                color = CLR_WHITE;
            else if (getroomtype(x, y) == DELPHI) /* on a special level... */
                color = CLR_BRIGHT_BLUE;
            else if (!Is_special(&u.uz)) {
                /* Only in filler levels will these get colored walls for
                 * the moment; colored walls look weird when they're part of
                 * special levels that adjoin special room walls to other room
                 * walls directly. */
                if (getroomtype(x, y) == BEEHIVE)
                    color = CLR_YELLOW;
                else if (getroomtype(x, y) == COURT)
                    color = CLR_BRIGHT_MAGENTA;
                else if (getroomtype(x, y) == VAULT)
                    color = HI_METAL;
                else if (getroomtype(x, y) == TEMPLE)
                    color = CLR_WHITE;
                else if (getroomtype(x, y) == LEPREHALL)
                    color = CLR_BRIGHT_GREEN;
                else if (getroomtype(x, y) == ANTHOLE)
                    color = CLR_BROWN;
                else if (getroomtype(x, y) == SWAMP)
                    color = CLR_GREEN;
            }
            else
                cmap_color(offset);
        }
        /* Colored floors don't really work that well with dark_room.
         * The original patch was for 3.4.3, which didn't have that, so it might
         * have worked better there.
        else if (iflags.use_color
                 && (offset == S_room || offset == S_darkroom)) {
            if (Is_juiblex_level(&u.uz))
                color = CLR_GREEN;
            else
                cmap_color(offset);
        }
        */
        /* color altars */
        else if (iflags.use_color && offset == S_altar) {
            if (Is_astralevel(&u.uz) || Is_sanctum(&u.uz)) {
                color = CLR_BRIGHT_MAGENTA;
            }
            else {
                if (a_align(x, y) == A_LAWFUL) {
                    color = CLR_YELLOW;
                }
                else if (a_align(x, y) == A_NEUTRAL) {
                    color = CLR_GRAY;
                }
                else if (a_align(x, y) == A_CHAOTIC) {
                    color = CLR_RED;
                }
                else if (a_align(x, y) == A_NONE) {
                    color = CLR_BLACK;
                }
            }
        }
        /* visible engravings */
        else if (iflags.use_color && offset == S_engraving && engr
                 && engr->engr_type != HEADSTONE) {
            if (engr->engr_type == DUST) {
                color = CLR_BROWN;
            }
            else if (engr->engr_type == BURN) {
                color = CLR_BLACK;
            }
            else if (engr->engr_type == MARK) {
                int k = (coord_hash(x, y, ledger_no(&u.uz)) % 3);
                if (k == 0)
                    color = CLR_BRIGHT_GREEN;
                else if (k == 1)
                    color = CLR_BRIGHT_BLUE;
                else if (k == 2)
                    color = CLR_BRIGHT_MAGENTA;
            }
            else if (engr->engr_type == ENGR_BLOOD) {
                color = CLR_RED;
            }
            else { /* ENGRAVE */
                color = CLR_GRAY;
            }
#endif
        /* try to provide a visible difference between water and lava
           if they use the same symbol and color is disabled */
        } else if (!iflags.use_color && offset == S_lava
                   && (g.showsyms[idx] == g.showsyms[S_pool + SYM_OFF_P]
                       || g.showsyms[idx]
                          == g.showsyms[S_water + SYM_OFF_P])) {
            special |= MG_BW_LAVA;
        /* similar for floor [what about empty doorway?] and ice */
        } else if (!iflags.use_color && offset == S_ice
                   && (g.showsyms[idx] == g.showsyms[S_room + SYM_OFF_P]
                       || g.showsyms[idx]
                          == g.showsyms[S_darkroom + SYM_OFF_P])) {
            special |= MG_BW_ICE;
        } else if (offset == S_altar && iflags.use_color) {
            int amsk = altarmask_at(x, y); /* might be a mimic */

            if ((g.glyphmap_perlevel_flags & GMAP_ALTARCOLOR)
                && (amsk & AM_SHRINE) != 0) {
                /* high altar */
                color = CLR_BRIGHT_MAGENTA;
            } else {
                switch (amsk & AM_MASK) {
        /*
         * On OSX with TERM=xterm-color256 these render as
         *  white -> tty: gray, curses: ok
         *  gray  -> both tty and curses: black
         *  black -> both tty and curses: blue
         *  red   -> both tty and curses: ok.
         * Since the colors have specific associations (with the
         * unicorns matched with each alignment), we shouldn't use
         * scrambled colors and we don't have sufficient information
         * to handle platform-specific color variations.
         */
                case AM_LAWFUL:  /* 4 */
                    color = CLR_WHITE;
                    break;
                case AM_NEUTRAL: /* 2 */
                    color = CLR_GRAY;
                    break;
                case AM_CHAOTIC: /* 1 */
                    color = CLR_BLACK;
                    break;
#if 0
                case AM_LAWFUL:  /* 4 */
                case AM_NEUTRAL: /* 2 */
                case AM_CHAOTIC: /* 1 */
                    cmap_color(S_altar); /* gray */
                    break;
#endif /* 0 */
                case AM_NONE:    /* 0 */
                    color = CLR_RED;
                    break;
                default: /* 3, 5..7 -- shouldn't happen but 3 was possible
                          * prior to 3.6.3 (due to faulty sink polymorph) */
                    color = NO_COLOR;
                    break;
                }
            }
        } else {
            cmap_color(offset);
        }
    } else if ((offset = (glyph - GLYPH_OBJ_OFF)) >= 0) { /* object */
        struct obj* otmp = vobj_at(x, y);
        idx = objects[offset].oc_class + SYM_OFF_O;
        if (On_stairs(x,y) && levl[x][y].seenv) special |= MG_STAIRS;
        if (offset == BOULDER)
            idx = SYM_BOULDER + SYM_OFF_X;
#ifdef TEXTCOLOR
        if (iflags.use_color && otmp && otmp->otyp == offset
            && otmp->material != objects[offset].oc_material) {
            color = materialclr[otmp->material];
        } else
#endif
            obj_color(offset);
        if (offset != BOULDER && is_objpile(x,y))
            special |= MG_OBJPILE;
    } else if ((offset = (glyph - GLYPH_RIDDEN_OFF)) >= 0) { /* mon ridden */
        idx = mons[offset].mlet + SYM_OFF_M;
        mon_color(offset);
        special |= MG_RIDDEN;
    } else if ((offset = (glyph - GLYPH_BODY_OFF)) >= 0) { /* a corpse */
        if (On_stairs(x,y) && levl[x][y].seenv) special |= MG_STAIRS;
        idx = objects[CORPSE].oc_class + SYM_OFF_O;
        mon_color(offset);
        special |= MG_CORPSE;
        if (is_objpile(x,y))
            special |= MG_OBJPILE;
    } else if ((offset = (glyph - GLYPH_DETECT_OFF)) >= 0) { /* mon detect */
        idx = mons[offset].mlet + SYM_OFF_M;
        mon_color(offset);
        /* Disabled for now; anyone want to get reverse video to work? */
        /* is_reverse = TRUE; */
        special |= MG_DETECT;
    } else if ((offset = (glyph - GLYPH_INVIS_OFF)) >= 0) { /* invisible */
        idx = SYM_INVISIBLE + SYM_OFF_X;
        invis_color(offset);
        special |= MG_INVIS;
    } else if ((offset = (glyph - GLYPH_PEACEFUL_OFF)) >= 0) { /* peaceful */
        idx = mons[offset].mlet + SYM_OFF_M;
        mon_color(offset);
        special |= MG_PEACEFUL;
    } else if ((offset = (glyph - GLYPH_PET_OFF)) >= 0) { /* a pet */
        idx = mons[offset].mlet + SYM_OFF_M;
        pet_color(offset);
        special |= MG_PET;
    } else { /* a monster */
        idx = mons[glyph].mlet + SYM_OFF_M;
        mon_color(glyph);
#ifdef TEXTCOLOR
        /* special case the hero for `showrace' option */
        if (iflags.use_color && is_you && flags.showrace && !Upolyd)
            color = HI_DOMESTIC;
#endif
    }

    /* Colored Walls/Floors Patch: The Valley of the Dead is monochrome, turning
     * everything to shades of gray. */
    if (iflags.use_color && Is_valley(&u.uz) && color != CLR_BLACK) {
        color = (color < NO_COLOR) ? CLR_GRAY : CLR_WHITE;
    }

    /* These were requested by a blind player to enhance screen reader use */
    if (sysopt.accessibility == 1 && !(mgflags & MG_FLAG_NOOVERRIDE)) {
        int ovidx;

        if ((special & MG_PET) != 0) {
            ovidx = SYM_PET_OVERRIDE + SYM_OFF_X;
            if (g.ov_primary_syms[ovidx])
                idx = ovidx;
        }
        if (is_you) {
            ovidx = SYM_HERO_OVERRIDE + SYM_OFF_X;
            if (g.ov_primary_syms[ovidx])
                idx = ovidx;
        }
    }

    ch = g.showsyms[idx];
#ifdef TEXTCOLOR
    /* Turn off color if no color defined. */
    if (!has_color(color))
#endif
        color = NO_COLOR;
    *ochar = (int) ch;
    *ospecial = special;
    *ocolor = color;
    return idx;
}

char *
encglyph(glyph)
int glyph;
{
    static char encbuf[20]; /* 10+1 would suffice */

    Sprintf(encbuf, "\\G%04X%04X", g.context.rndencode, glyph);
    return encbuf;
}

char *
decode_mixed(buf, str)
char *buf;
const char *str;
{
    static const char hex[] = "00112233445566778899aAbBcCdDeEfF";
    char *put = buf;

    if (!str)
        return strcpy(buf, "");

    while (*str) {
        if (*str == '\\') {
            int rndchk, dcount, so, gv, ch = 0, oc = 0;
            unsigned os = 0;
            const char *dp, *save_str;

            save_str = str++;
            switch (*str) {
            case 'G': /* glyph value \GXXXXNNNN*/
                rndchk = dcount = 0;
                for (++str; *str && ++dcount <= 4; ++str)
                    if ((dp = index(hex, *str)) != 0)
                        rndchk = (rndchk * 16) + ((int) (dp - hex) / 2);
                    else
                        break;
                if (rndchk == g.context.rndencode) {
                    gv = dcount = 0;
                    for (; *str && ++dcount <= 4; ++str)
                        if ((dp = index(hex, *str)) != 0)
                            gv = (gv * 16) + ((int) (dp - hex) / 2);
                        else
                            break;
                    so = mapglyph(gv, &ch, &oc, &os, 0, 0, 0);
                    *put++ = g.showsyms[so];
                    /* 'str' is ready for the next loop iteration and '*str'
                       should not be copied at the end of this iteration */
                    continue;
                } else {
                    /* possible forgery - leave it the way it is */
                    str = save_str;
                }
                break;
#if 0
            case 'S': /* symbol offset */
                so = rndchk = dcount = 0;
                for (++str; *str && ++dcount <= 4; ++str)
                    if ((dp = index(hex, *str)) != 0)
                        rndchk = (rndchk * 16) + ((int) (dp - hex) / 2);
                    else
                        break;
                if (rndchk == g.context.rndencode) {
                    dcount = 0;
                    for (; *str && ++dcount <= 2; ++str)
                        if ((dp = index(hex, *str)) != 0)
                            so = (so * 16) + ((int) (dp - hex) / 2);
                        else
                            break;
                }
                *put++ = g.showsyms[so];
                break;
#endif
            case '\\':
                break;
            case '\0':
                /* String ended with '\\'.  This can happen when someone
                   names an object with a name ending with '\\', drops the
                   named object on the floor nearby and does a look at all
                   nearby objects. */
                /* brh - should we perhaps not allow things to have names
                   that contain '\\' */
                str = save_str;
                break;
            }
        }
        *put++ = *str++;
    }
    *put = '\0';
    return buf;
}

/*
 * This differs from putstr() because the str parameter can
 * contain a sequence of characters representing:
 *        \GXXXXNNNN    a glyph value, encoded by encglyph().
 *
 * For window ports that haven't yet written their own
 * XXX_putmixed() routine, this general one can be used.
 * It replaces the encoded glyph sequence with a single
 * showsyms[] char, then just passes that string onto
 * putstr().
 */

void
genl_putmixed(window, attr, str)
winid window;
int attr;
const char *str;
{
    char buf[BUFSZ];

    /* now send it to the normal putstr */
    putstr(window, attr, decode_mixed(buf, str));
}

/*
 * Window port helper function for menu invert routines to move the decision
 * logic into one place instead of 7 different window-port routines.
 */
boolean
menuitem_invert_test(mode, itemflags, is_selected)
int mode;
unsigned itemflags;     /* The itemflags for the item               */
boolean is_selected;    /* The current selection status of the item */
{
    boolean skipinvert = (itemflags & MENU_ITEMFLAGS_SKIPINVERT) != 0;

    if ((iflags.menuinvertmode == 1 || iflags.menuinvertmode == 2)
        && !mode && skipinvert && !is_selected)
        return FALSE;
    else if (iflags.menuinvertmode == 2
        && !mode && skipinvert && is_selected)
        return TRUE;
    else
        return TRUE;
}

/*mapglyph.c*/
