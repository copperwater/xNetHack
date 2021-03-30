/* NetHack 3.7	mkroom.c	$NHDT-Date: 1613086701 2021/02/11 23:38:21 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.52 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Entry points:
 *      do_mkroom() -- make and stock a room of a given type
 *      nexttodoor() -- return TRUE if adjacent to a door
 *      has_dnstairs() -- return TRUE if given room has a down staircase
 *      has_upstairs() -- return TRUE if given room has an up staircase
 *      courtmon() -- generate a court monster
 *      save_rooms() -- save rooms into file fd
 *      rest_rooms() -- restore rooms from file fd
 *      cmap_to_type() -- convert S_xxx symbol to XXX topology code
 */

#include "hack.h"

static boolean isbig(struct mkroom *);
static struct mkroom *pick_room(boolean);
static void mkshop(void), mkzoo(int), mkswamp(void);
static void mk_zoo_thronemon(int, int);
static struct mkroom *mktemple(void);
static void mkseminary(void);
static void mksubmerged(void);
static void mkstatuary(void);
static coord *shrine_pos(int);
static struct permonst *morguemon(void);
static struct permonst *squadmon(void);
static struct permonst *zoomon(void);
static struct permonst *demondenmon(void);
static struct permonst *abattoirmon(void);
static void save_room(NHFILE *, struct mkroom *);
static void rest_room(NHFILE *, struct mkroom *);

#define sq(x) ((x) * (x))

extern const struct shclass shtypes[]; /* defined in shknam.c */

/* Return TRUE if a room's rectangular floor area is larger than 20 */
static boolean
isbig(struct mkroom* sroom)
{
    register int area = (sroom->hx - sroom->lx + 1)
                        * (sroom->hy - sroom->ly + 1);

    return (boolean) (area > 20);
}

/* Create, in one of the rooms on the level, a special room of a given type.
 * Assumes that rooms have already been created and placed on the level.
 * Which room is selected is generally random, but this function doesn't
 * determine that; it depends on the specific behavior of mkzoo/mkswamp/etc.
 */
void
do_mkroom(int roomtype)
{
    if (roomtype == OROOM)
        return; /* not actually trying to make a special room */
    if (roomtype >= SHOPBASE)
        mkshop(); /* someday, we should be able to specify shop type */
    else {
        switch (roomtype) {
        case COURT:
        case ZOO:
        case BEEHIVE:
        case MORGUE:
        case BARRACKS:
        case LEPREHALL:
        case COCKNEST:
        case ANTHOLE:
        case DEMONDEN:
        case LAVAROOM:
        case ABATTOIR:
            /* all of these just call mkzoo directly */
            mkzoo(roomtype);
            break;
        case SWAMP:
            mkswamp();
            break;
        case TEMPLE:
            mktemple();
            break;
        case SUBMERGED:
            mksubmerged();
            break;
        case SEMINARY:
            mkseminary();
            break;
        case STATUARY:
            mkstatuary();
            break;
        default:
            impossible("Tried to make a room of type %d.", roomtype);
        }
    }
}

/* Create and stock a shop on a random room.
 *
 * In wizard mode, the SHOPTYPE environment variable can be used to force
 * creation of a certain special room type (not limited to shops; see below)
 * where a shop would have generated (in which case it will do that instead and
 * abort creating the shop). SHOPTYPE can also be used to specify a certain
 * sort of shop.
 *
 * Shops cannot be created in non-ordinary-type rooms, rooms containing
 * staircases, or rooms that don't have exactly one door. If no appropriate
 * room is found, this function will fail without further effect.
 *
 * Shops are always made lit. If a wand or a book shop whose area is larger than
 * 20 would have been created, it is instead replaced with a general store.
 */
static void
mkshop(void)
{
    register struct mkroom *sroom;
    int i = -1;
    char *ep = (char *) 0; /* (init == lint suppression) */

    /* first determine shoptype */
    if (wizard) {
        ep = nh_getenv("SHOPTYPE");
        if (ep) {
            if (*ep == 'z' || *ep == 'Z') {
                mkzoo(ZOO);
                return;
            }
            if (*ep == 'm' || *ep == 'M') {
                mkzoo(MORGUE);
                return;
            }
            if (*ep == 'b') {
                mkzoo(BEEHIVE);
                return;
            }
            if (*ep == 't' || *ep == 'T' || *ep == '\\') {
                mkzoo(COURT);
                return;
            }
            if (*ep == 'B') {
                mkzoo(BARRACKS);
                return;
            }
            if (*ep == 'a') {
                mkzoo(ANTHOLE);
                return;
            }
            if (*ep == 'c' || *ep == 'C') {
                mkzoo(COCKNEST);
                return;
            }
            if (*ep == 'l') {
                mkzoo(LEPREHALL);
                return;
            }
            if (*ep == '_') {
                mktemple();
                return;
            }
            if (*ep == '}') {
                mkswamp();
                return;
            }
            if (*ep == 'A') {
                mkzoo(ABATTOIR);
                return;
            }
            if (*ep == 'd' || *ep == 'D') {
                mkzoo(DEMONDEN);
                return;
            }
            if (*ep == 'L') {
                mkzoo(LAVAROOM);
                return;
            }
            if (*ep == 'S') {
                mkseminary();
                return;
            }
            if (*ep == 'u' || *ep == 'U') { /* underwater */
                mksubmerged();
                return;
            }
            if (*ep == 's') {
                mkstatuary();
                return;
            }
            for (i = 0; shtypes[i].name; i++)
                if (*ep == def_oc_syms[(int) shtypes[i].symb].sym)
                    goto gottype;
            if (*ep == 'g' || *ep == 'G')
                i = 0;
            else if (*ep == 'v' || *ep == 'V')
                i = FODDERSHOP - SHOPBASE; /* veggy food */
            else
                i = -1;
        }
    }

 gottype:
    for (sroom = &g.rooms[0];; sroom++) {
        if (sroom->hx < 0)
            /* could not find any suitable rooms */
            return;
        if (sroom - g.rooms >= g.nroom) {
            impossible("rooms[] not closed by -1?");
            return;
        }
        if (sroom->rtype != OROOM)
            continue;
        if (has_dnstairs(sroom) || has_upstairs(sroom))
            continue;
        if (sroom->doorct == 1 || (wizard && ep && sroom->doorct != 0))
            break;
    }
    if (!sroom->rlit) {
        int x, y;

        for (x = sroom->lx - 1; x <= sroom->hx + 1; x++)
            for (y = sroom->ly - 1; y <= sroom->hy + 1; y++)
                levl[x][y].lit = 1;
        sroom->rlit = 1;
    }

    if (i < 0) { /* shoptype not yet determined */
        register int j;

        /* pick a shop type at random */
        for (j = rnd(100), i = 0; (j -= shtypes[i].prob) > 0; i++)
            continue;

        /* big rooms cannot be wand or book shops,
         * - so make them general stores
         */
        if (isbig(sroom) && (shtypes[i].symb == WAND_CLASS
                             || shtypes[i].symb == SPBOOK_CLASS))
            i = 0;
    }
    sroom->rtype = SHOPBASE + i;

    /* set room bits before stocking the shop */
#ifdef SPECIALIZATION
    topologize(sroom, FALSE); /* doesn't matter - this is a special room */
#else
    topologize(sroom);
#endif

    /* The shop used to be stocked here, but this no longer happens--all we do
       is set its rtype, and it gets stocked at the end of makelevel() along
       with other special rooms. */
    sroom->needfill = FILL_NORMAL;
}

/* Select a room on the level that is suitable to place a special room in.
 * The room must be of ordinary type, and cannot contain the upstairs.
 * It cannot contain the downstairs either, unless strict is FALSE, in which
 * case it may allow it with 1/3 chance.
 * Rooms with exactly one door are heavily preferred; there is only a 1/5
 * chance of selecting a room with more doors than that. */
static struct mkroom *
pick_room(boolean strict)
{
    register struct mkroom *sroom;
    register int i = g.nroom;

    for (sroom = &g.rooms[rn2(g.nroom)]; i--; sroom++) {
        if (sroom == &g.rooms[g.nroom])
            sroom = &g.rooms[0];
        if (sroom->hx < 0)
            return (struct mkroom *) 0;
        if (sroom->rtype != OROOM)
            continue;
        if (!strict) {
            if (has_upstairs(sroom) || (has_dnstairs(sroom) && rn2(3)))
                continue;
        } else if (has_upstairs(sroom) || has_dnstairs(sroom))
            continue;
        if (sroom->doorct == 1 || !rn2(5) || wizard)
            return sroom;
    }
    return (struct mkroom *) 0;
}

/* Try to find a suitable room for a zoo of the given type and, if one can be
 * found, set its room type and call fill_zoo to stock it. */
static void
mkzoo(int type)
{
    register struct mkroom *sroom;

    if ((sroom = pick_room(FALSE)) != 0) {
        sroom->rtype = type;
        /* room does not get stocked at this time - it will get stocked at the
         * end of makelevel() */
        sroom->needfill = FILL_NORMAL;
    }
}

/* Create an appropriate "king" monster at the given location (assumed to be on
 * a throne). */
static void
mk_zoo_thronemon(int x,int y)
{
    int i = rnd(level_difficulty());
    int pm = (i > 9) ? PM_OGRE_TYRANT
        : (i > 5) ? PM_ELVEN_MONARCH
        : (i > 2) ? PM_DWARF_RULER
        : PM_GNOME_RULER;
    struct monst *mon = makemon(&mons[pm], x, y, NO_MM_FLAGS);

    if (mon) {
        mon->msleeping = 1;
        mon->mpeaceful = 0;
        set_malign(mon);
        /* Give him a sceptre to pound in judgment */
        struct obj* mace = mksobj(MACE, TRUE, FALSE);
        if (!rn2(15))
            set_material(mace, PLATINUM);
        else if (!rn2(14))
            set_material(mace, MITHRIL);
        else if (rnd(13) <= 3)
            set_material(mace, SILVER);
        else
            set_material(mace, GOLD);
        mpickobj(mon, mace);
    }
}

/* Populate one of the zoo-type rooms with monsters, objects, and possibly
 * dungeon features. Also set any appropriate level flags for level sound
 * purposes.
 * Currently, all of these involve placing a monster on every square of the
 * room, whereas objects may or may not be. */
void
fill_zoo(struct mkroom* sroom)
{
    struct monst *mon = NULL;
    register int sx, sy, i;
    int sh, tx = 0, ty = 0, goldlim = 0, type = sroom->rtype;
    int rmno = (int) ((sroom - g.rooms) + ROOMOFFSET);
    coord mm;

    /* Note: This doesn't check needfill; it assumes the caller has already
       done that. */
    sh = sroom->fdoor;
    switch (type) {
    case COURT:
        /* Did a special level hardcode the throne in a given spot? */
        if (g.level.flags.is_maze_lev) {
            for (tx = sroom->lx; tx <= sroom->hx; tx++)
                for (ty = sroom->ly; ty <= sroom->hy; ty++)
                    if (IS_THRONE(levl[tx][ty].typ))
                        goto throne_placed;
        }
        /* If not, pick a random spot. */
        i = 100;
        do { /* don't place throne on top of stairs */
            (void) somexy(sroom, &mm);
            tx = mm.x;
            ty = mm.y;
        } while (occupied((xchar) tx, (xchar) ty) && --i > 0);
    throne_placed:
        mk_zoo_thronemon(tx, ty);
        break;
    case BEEHIVE:
        tx = sroom->lx + (sroom->hx - sroom->lx + 1) / 2;
        ty = sroom->ly + (sroom->hy - sroom->ly + 1) / 2;
        if (sroom->irregular) {
            /* center might not be valid, so put queen elsewhere */
            if ((int) levl[tx][ty].roomno != rmno || levl[tx][ty].edge) {
                (void) somexy(sroom, &mm);
                tx = mm.x;
                ty = mm.y;
            }
        }
        break;
    case ZOO:
    case LEPREHALL:
        goldlim = 500 * level_difficulty();
        break;
    }

    /* fill room with monsters */
    for (sx = sroom->lx; sx <= sroom->hx; sx++)
        for (sy = sroom->ly; sy <= sroom->hy; sy++) {
            /* Don't fill the square right next to the door, or any of the ones
             * along the same wall as the door if the room is rectangular. */
            if (sroom->irregular) {
                if ((int) levl[sx][sy].roomno != rmno || levl[sx][sy].edge
                    || (sroom->doorct
                        && distmin(sx, sy, g.doors[sh].x, g.doors[sh].y) <= 1))
                    continue;
            } else if (!SPACE_POS(levl[sx][sy].typ)
                       || (sroom->doorct
                           && ((sx == sroom->lx && g.doors[sh].x == sx - 1)
                               || (sx == sroom->hx && g.doors[sh].x == sx + 1)
                               || (sy == sroom->ly && g.doors[sh].y == sy - 1)
                               || (sy == sroom->hy
                                   && g.doors[sh].y == sy + 1))))
                continue;
            /* don't place monster on explicitly placed throne */
            if (type == COURT && IS_THRONE(levl[sx][sy].typ))
                continue;

            /* create the appropriate room filler monster */
            struct permonst * fillermon = NULL;
            if (type == COURT) {
                fillermon = courtmon();
            }
            else if (type == BARRACKS) {
                fillermon = squadmon();
            }
            else if (type == MORGUE) {
                fillermon = morguemon();
            }
            else if (type == BEEHIVE) {
                if (sx == tx && sy == ty)
                    fillermon = &mons[PM_QUEEN_BEE];
                else
                    fillermon = &mons[PM_KILLER_BEE];
            }
            else if (type == LEPREHALL) {
                fillermon = &mons[PM_LEPRECHAUN];
            }
            else if (type == COCKNEST) {
                fillermon = &mons[PM_COCKATRICE];
            }
            else if (type == ANTHOLE) {
                fillermon = antholemon();
            }
            else if (type == ZOO) {
                fillermon = zoomon();
            }
            else if (type == DEMONDEN) {
                fillermon = demondenmon();
            }
            else if (type == LAVAROOM) {
                fillermon = rn2(5) ? NULL : &mons[PM_SALAMANDER];
            }
            else if (type == ABATTOIR) {
                fillermon = abattoirmon();
            }

            mon = NULL;
            if (fillermon) {
                /* All special rooms currently generate all their monsters asleep. */
                mon = makemon(fillermon, sx, sy, MM_ASLEEP);

                if (mon) {
                    if (type == COURT && mon->mpeaceful) {
                        /* Courts are also always hostile. */
                        mon->mpeaceful = 0;
                        set_malign(mon);
                    }
                    if (type == ABATTOIR || (type == DEMONDEN && !rn2(3))) {
                        /* undo its sleep */
                        mon->msleeping = 0;
                    }
                }
            }
            switch (type) {
            case ZOO:
            case LEPREHALL:
                /* place floor gold */
                if (sroom->doorct) {
                    int distval = dist2(sx, sy, g.doors[sh].x, g.doors[sh].y);
                    i = sq(distval);
                } else
                    i = goldlim;
                if (i >= goldlim)
                    i = 5 * level_difficulty();
                goldlim -= i;
                (void) mkgold((long) rn1(i, 10), sx, sy);
                break;
            case MORGUE:
                /* corpses and chests and headstones */
                if (!rn2(5))
                    (void) mk_tt_object(CORPSE, sx, sy);
                if (!rn2(10)) /* lots of treasure buried with dead */
                    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST, sx, sy,
                                     TRUE, FALSE);
                if (!rn2(5))
                    make_grave(sx, sy, (char *) 0);
                else if (!rn2(200)) {
                    maketrap(sx, sy, PIT);
                }
                break;
            case BEEHIVE:
                if (!rn2(3))
                    (void) mksobj_at(LUMP_OF_ROYAL_JELLY, sx, sy, TRUE,
                                     FALSE);
                break;
            case BARRACKS:
                if (!rn2(20)) /* the payroll and some loot */
                    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST, sx, sy,
                                     TRUE, FALSE);
                break;
            case COCKNEST:
                if (!rn2(3)) {
                    struct obj *sobj = mk_tt_object(STATUE, sx, sy);

                    if (sobj) {
                        set_material(sobj, MINERAL);
                        for (i = rn2(5); i; i--)
                            (void) add_to_container(
                                sobj, mkobj(RANDOM_CLASS, FALSE));
                        sobj->owt = weight(sobj);
                    }
                }
                break;
            case ANTHOLE:
                if (!rn2(3))
                    (void) mkobj_at(FOOD_CLASS, sx, sy, FALSE);
                break;
            case DEMONDEN:
                if (mon) {
                    /* treasure */
                    (void) mkgold((long) rn1(200, 10), sx, sy);
                    for (i = rn2(3) + 1; i > 0; i--) {
                        mkobj_at((rn2(2) ? RANDOM_CLASS : GEM_CLASS),
                                 sx, sy, TRUE);
                    }
                }
                break;
            case LAVAROOM:
                if (rn2(2) && levl[sx][sy].typ == ROOM) {
                    /* test for ROOM mainly so we don't overwrite the stairs;
                     * but also cover anything else weird */
                    levl[sx][sy].typ = LAVAPOOL;
                }
                break;
            case ABATTOIR:
                /* scatter some corpses, leashes, knives, blood */
                if (!rn2(7)) {
                    struct permonst *zptr = zoomon();
                    if (zptr) {
                        struct obj* sobj = mksobj_at(CORPSE, sx, sy, TRUE, FALSE);
                        sobj->corpsenm = monsndx(zptr);
                    }
                }
                if (!rn2(10)) {
                    mksobj_at(LEASH, sx, sy, FALSE, FALSE);
                }
                if (!rn2(6)) {
                    mksobj_at(KNIFE, sx, sy, TRUE, FALSE);
                }
                if (!rn2(2)) {
                    const char* bloodstains[] =
                        { "/", "-", "\\", ".", "," ":" };
                    make_engr_at(sx, sy, bloodstains[rn2(SIZE(bloodstains))],
                                 0, ENGR_BLOOD);
                }
                break;
            }
        }
    switch (type) {
    case COURT: {
        struct obj *chest, *gold;
        levl[tx][ty].typ = THRONE;
        (void) somexy(sroom, &mm);
        gold = mksobj(GOLD_PIECE, TRUE, FALSE);
        gold->quan = (long) rn1(50 * level_difficulty(), 10);
        gold->owt = weight(gold);
        /* the royal coffers */
        chest = mksobj_at(CHEST, mm.x, mm.y, TRUE, FALSE);
        add_to_container(chest, gold);
        chest->owt = weight(chest);
        chest->spe = 2; /* so it can be found later */
        g.level.flags.has_court = 1;
        break;
    }
    case BARRACKS:
        g.level.flags.has_barracks = 1;
        break;
    case ZOO:
        g.level.flags.has_zoo = 1;
        break;
    case MORGUE:
        g.level.flags.has_morgue = 1;
        break;
    case SWAMP:
        g.level.flags.has_swamp = 1;
        break;
    case BEEHIVE:
        g.level.flags.has_beehive = 1;
        break;
    }
}

/* Make a swarm of undead around mm.
 * Why is this in mkroom.c? It's only used when using cursed invocation items.
 */
void
mkundead(
    coord *mm,
    boolean revive_corpses,
    int mm_flags)
{
    int cnt = (level_difficulty() + 1) / 10 + rnd(5);
    struct permonst *mdat;
    struct obj *otmp;
    coord cc;

    while (cnt--) {
        mdat = morguemon();
        if (mdat && enexto(&cc, mm->x, mm->y, mdat)
            && (!revive_corpses
                || !(otmp = sobj_at(CORPSE, cc.x, cc.y))
                || !revive(otmp, FALSE)))
            (void) makemon(mdat, cc.x, cc.y, mm_flags);
    }
    g.level.flags.graveyard = TRUE; /* reduced chance for undead corpse */
}

/* Return an appropriate undead monster type for generating in graveyards or
 * when raising the dead. */
static struct permonst *
morguemon(void)
{
    register int i = rn2(100), hd = rn2(level_difficulty());

    if (hd > 10 && i < 10) {
        if (Inhell || In_endgame(&u.uz)) {
            return mkclass(S_DEMON, 0);
        } else {
            int ndemon_res = ndemon(A_NONE);
            if (ndemon_res != NON_PM)
                return &mons[ndemon_res];
            /* else do what? As is, it will drop to ghost/wraith/zombie */
        }
    }

    if (hd > 8 && i > 85)
        return mkclass(S_VAMPIRE, 0);

    return ((i < 20) ? &mons[PM_GHOST]
                     : (i < 40) ? &mons[PM_WRAITH]
                                : mkclass(S_ZOMBIE, 0));
}

/* Return an appropriate ant monster type for an anthole.
 * This is deterministic, so that all ants on the same level (practically
 * speaking, in a single anthole) are the same type of ant.
 */
struct permonst *
antholemon(void)
{
    int mtyp, trycnt = 0;
    unsigned int indx = hash1(ledger_no(&u.uz));

    /* Same monsters within a level, different ones between levels */
    do {
        switch ((indx + trycnt) % 3) {
        case 0:
            mtyp = PM_SOLDIER_ANT;
            break;
        case 1:
            mtyp = PM_FIRE_ANT;
            break;
        default:
            mtyp = PM_GIANT_ANT;
            break;
        }
        /* try again if chosen type has been genocided or used up */
    } while (++trycnt < 3 && (g.mvitals[mtyp].mvflags & G_GONE));

    return ((g.mvitals[mtyp].mvflags & G_GONE) ? (struct permonst *) 0
                                             : &mons[mtyp]);
}

/* Pick random zoo-like monsters. */
struct permonst *
zoomon(void)
{
    struct permonst * pm;
    int i;
    for(i = 0; i < 100; i++) {
        /* try to reroll until finding an animal */
        pm = rndmonst();
        if (pm && is_animal(pm)) {
            break;
        }
    }
    return pm;
}

/* Pick random demon den monsters. */
struct permonst *
demondenmon(void)
{
    if (!rn2(4)) {
        if (rn2(8)) {
            return mkclass(S_DEMON, 0);
        }
        else {
            return mkclass(S_IMP, 0);
        }
    }
    return NULL;
}

struct permonst *
abattoirmon(void)
{
    if (!rn2(10)) {
        if (!rn2(6)) {
            /* mon for slaughter */
            return zoomon();
        }
        else {
            /* butcher */
            return &mons[PM_MARILITH];
        }
    }
    return NULL;
}

/* Turn up to 5 ordinary rooms into swamp rooms.
 * Swamps contain a checkerboard pattern of pools (except next to doors),
 * F-class monsters, and possibly one sea monster, apiece.
 */
static void
mkswamp(void) /* Michiel Huisjes & Fred de Wilde */
{
    register struct mkroom *sroom;
    register int sx, sy, i, eelct = 0;

    for (i = 0; i < 5; i++) { /* turn up to 5 rooms swampy */
        sroom = &g.rooms[rn2(g.nroom)];
        if (sroom->hx < 0 || sroom->rtype != OROOM || has_upstairs(sroom)
            || has_dnstairs(sroom))
            continue;

        /* satisfied; make a swamp */
        sroom->rtype = SWAMP;
        for (sx = sroom->lx; sx <= sroom->hx; sx++)
            for (sy = sroom->ly; sy <= sroom->hy; sy++)
                if (!OBJ_AT(sx, sy) && !MON_AT(sx, sy) && !t_at(sx, sy)
                    && !nexttodoor(sx, sy)) {
                    if ((sx + sy) % 2) {
                        levl[sx][sy].typ = POOL;
                        if (!eelct || !rn2(4)) {
                            /* mkclass() won't do, as we might get kraken */
                            (void) makemon(rn2(5)
                                              ? &mons[PM_GIANT_EEL]
                                              : rn2(2)
                                                 ? &mons[PM_PIRANHA]
                                                 : &mons[PM_ELECTRIC_EEL],
                                           sx, sy, NO_MM_FLAGS);
                            eelct++;
                        }
                    } else if (!rn2(4)) { /* swamps tend to be moldy */
                        (void) makemon(mkclass(S_FUNGUS, 0), sx, sy,
                                       NO_MM_FLAGS);
                    } else if (!rn2(10))
                        levl[sx][sy].typ = TREE;
                }
        g.level.flags.has_swamp = 1;
    }
}

/* Return the position within a room at which its altar should be placed, if it
 * is to be a temple. It will be the exact center of the room, unless the center
 * isn't actually a square, in which case it'll be offset one space to the side.
 */
static coord *
shrine_pos(int roomno)
{
    static coord buf;
    int delta;
    struct mkroom *troom = &g.rooms[roomno - ROOMOFFSET];

    /* if width and height are odd, placement will be the exact center;
       if either or both are even, center point is a hypothetical spot
       between map locations and placement will be adjacent to that.
       Make deterministic so that if it is called multiple times on the same
       room it will return the same coordinate. */
    delta = troom->hx - troom->lx;
    buf.x = troom->lx + delta / 2;
    if ((delta % 2) && (roomno & 1))
        buf.x++;
    delta = troom->hy - troom->ly;
    buf.y = troom->ly + delta / 2;
    if ((delta % 2) && (roomno & 2))
        buf.y++;
    return &buf;
}

/* Try and find a suitable room for a temple and if successful, create the
 * temple with its altar and attendant priest.
 */
static struct mkroom *
mktemple(void)
{
    register struct mkroom *sroom;
    coord *shrine_spot;
    register struct rm *lev;

    if (!(sroom = pick_room(TRUE)))
        return NULL;

    /* set up Priest and shrine */
    sroom->rtype = TEMPLE;
    /*
     * In temples, shrines are blessed altars
     * located in the center of the room
     */
    shrine_spot = shrine_pos((int) ((sroom - g.rooms) + ROOMOFFSET));
    lev = &levl[shrine_spot->x][shrine_spot->y];
    lev->typ = ALTAR;
    if (!Inhell) {
        lev->altarmask = induced_align(80);
    }
    else {
        lev->altarmask = AM_NONE; /* Moloch */
    }
    priestini(&u.uz, sroom, shrine_spot->x, shrine_spot->y, FALSE);
    lev->altarmask |= AM_SHRINE;
    g.level.flags.has_temple = 1;
    return sroom;
}

/* Create a seminary - a temple containing the usual peaceful priest and some
 * roaming priests of the same god. */
void
mkseminary(void)
{
    struct mkroom * sroom = mktemple();
    int i;
    xchar x, y;
    coord *ss; /* shrine spot */

    if (!sroom) /* temple creation failed */
        return;

    /* get altar alignment, roaming priests should have the same */
    ss = shrine_pos((int) ((sroom - g.rooms) + ROOMOFFSET));
    if (levl[ss->x][ss->y].typ != ALTAR) {
        impossible("mkseminary: altar not present?");
        return;
    }
    sroom->rtype = SEMINARY;

    aligntyp altaralign = Amask2align(levl[ss->x][ss->y].altarmask & AM_MASK);

    for (i = rn1(4,1); i > 0; --i) {
        x = somex(sroom);
        y = somey(sroom);
        if (MON_AT(x, y)) {
            i++;
            continue;
        }
        /* peaceful if they're of your alignment */
        mk_roamer(&mons[PM_ALIGNED_CLERIC], altaralign, x, y,
                  (u.ualign.type == altaralign));
    }
}

/* Create a submerged room - filled entirely with water, populated with sea
 * monsters and kelp and hidden treasure. */
void
mksubmerged(void)
{
    struct mkroom *sroom;
    struct obj *chest, *obj;
    xchar x, y;

    if (!(sroom = pick_room(TRUE)))
        return;

    sroom->rtype = SUBMERGED;

    for (x = sroom->lx; x <= sroom->hx; x++) {
        for (y = sroom->ly; y <= sroom->hy; y++) {
            levl[x][y].typ = MOAT;
            if (!rn2(4)) {
                /* this actually turns *off* G_NOGEN disabling - we need that
                 * since all sea monsters are G_NOGEN */
                struct permonst* pm = mkclass(S_EEL, G_NOGEN);
                if (pm)
                    makemon(pm, x, y, NO_MM_FLAGS);
            }
            if (!rn2(20)) {
                mksobj_at(KELP_FROND, x, y, TRUE, FALSE);
            }
        }
    }
    x = somex(sroom);
    y = somey(sroom);
    chest = mksobj_at(CHEST, x, y, TRUE, FALSE);
    obj = mksobj(GOLD_PIECE, TRUE, FALSE);
    obj->quan = rn1(1000, 1000);
    add_to_container(chest, obj);
    for (x = rn2(10); x > 0; --x) {
        add_to_container(chest, mkobj(GEM_CLASS, FALSE));
    }
    if (!rn2(10)) {
        add_to_container(chest, mksobj(MAGIC_LAMP, TRUE, FALSE));
    }
}

/* Create a statuary room - eerily lined with empty statues of the player */
void
mkstatuary(void)
{
    struct mkroom *sroom;
    struct obj *statue;
    xchar x, y, width, height;

    if (!(sroom = pick_room(FALSE)))
        return;

    sroom->rtype = STATUARY;

#define MKSTATUE(x, y)                                       \
    if (!nexttodoor(x, y)) {                                 \
        statue = mkcorpstat(STATUE, NULL, &mons[u.umonster], \
                            x, y, CORPSTAT_NONE);            \
        oname(statue, g.plname);                             \
    }

    /* pick the longer dimension to place statues */
    width = sroom->hx - sroom->lx;
    height = sroom->hy - sroom->ly;
    if (width > height || (width == height && rn2(2))) {
        for (x = sroom->lx; x <= sroom->hx; x++) {
            MKSTATUE(x, sroom->ly);
            MKSTATUE(x, sroom->hy);
        }
    }
    else {
        for (y = sroom->ly; y <= sroom->hy; y++) {
            MKSTATUE(sroom->lx, y);
            MKSTATUE(sroom->hx, y);
        }
    }

#undef MKSTATUE
}

/* Return TRUE if the given location is next to a door or a secret door in any
 * direction. */
boolean
nexttodoor(int sx, int sy)
{
    register int dx, dy;
    register struct rm *lev;

    for (dx = -1; dx <= 1; dx++)
        for (dy = -1; dy <= 1; dy++) {
            if (!isok(sx + dx, sy + dy))
                continue;
            lev = &levl[sx + dx][sy + dy];
            if (IS_DOOR(lev->typ) || lev->typ == SDOOR)
                return TRUE;
        }
    return FALSE;
}

/* Return TRUE if the given room contains stairs (regular or branch), in the
 * specified direction. */
boolean
has_stairs(register struct mkroom *sroom, boolean up)
{
    stairway *stway = g.stairs;

    while (stway) {
        if (up == stway->up && inside_room(sroom, stway->sx, stway->sy))
            return TRUE;
        stway = stway->next;
    }
    return FALSE;
}

/* Return a random x coordinate within the x limits of a room. */
int
somex(struct mkroom* croom)
{
    return rn1(croom->hx - croom->lx + 1, croom->lx);
}

/* Return a random y coordinate within the y limits of a room. */
int
somey(struct mkroom* croom)
{
    return rn1(croom->hy - croom->ly + 1, croom->ly);
}

/* Return TRUE if the given position falls within both the x and y limits
 * of a room.
 */
boolean
inside_room(struct mkroom* croom, xchar x, xchar y)
{
    if (croom->irregular) {
        int i = (int) ((croom - g.rooms) + ROOMOFFSET);
        return (!levl[x][y].edge && (int) levl[x][y].roomno == i);
    }

    return (boolean) (x >= croom->lx - 1 && x <= croom->hx + 1
                      && y >= croom->ly - 1 && y <= croom->hy + 1);
}

/* Populate c.x and c.y with some random coordinate inside the given room.
 * Return TRUE if it was able to do this successfully, and FALSE if it failed
 * for some reason. */
boolean
somexy(struct mkroom* croom,coord * c)
{
    int try_cnt = 0;
    int i;

    if (croom->irregular) {
        i = (int) ((croom - g.rooms) + ROOMOFFSET);

        while (try_cnt++ < 100) {
            c->x = somex(croom);
            c->y = somey(croom);
            if (!levl[c->x][c->y].edge && (int) levl[c->x][c->y].roomno == i)
                return TRUE;
        }
        /* try harder; exhaustively search until one is found */
        for (c->x = croom->lx; c->x <= croom->hx; c->x++)
            for (c->y = croom->ly; c->y <= croom->hy; c->y++)
                if (!levl[c->x][c->y].edge
                    && (int) levl[c->x][c->y].roomno == i)
                    return TRUE;
        return FALSE;
    }

    if (!croom->nsubrooms) {
        c->x = somex(croom);
        c->y = somey(croom);
        return TRUE;
    }

    /* Check that coords doesn't fall into a subroom or into a wall */

    while (try_cnt++ < 100) {
        c->x = somex(croom);
        c->y = somey(croom);
        if (IS_WALL(levl[c->x][c->y].typ))
            continue;
        for (i = 0; i < croom->nsubrooms; i++)
            if (inside_room(croom->sbrooms[i], c->x, c->y))
                goto you_lose;
        break;
    you_lose:
        ;
    }
    if (try_cnt >= 100)
        return FALSE;
    return TRUE;
}

boolean
somexyspace(struct mkroom* croom, coord *c)
{
    int trycnt = 0;
    boolean okay;

    do {
        okay = somexy(croom, c) && isok(c->x, c->y) && !occupied(c->x, c->y)
            && (levl[c->x][c->y].typ == ROOM
                || levl[c->x][c->y].typ == CORR
                || levl[c->x][c->y].typ == ICE
                || levl[c->x][c->y].typ == GRASS);
    } while (trycnt++ < 100 && !okay);
    return okay;
}

/*
 * Search for a special room given its type (zoo, court, etc...)
 *      Special values :
 *              - ANY_SHOP
 *              - ANY_TYPE
 */
struct mkroom *
search_special(schar type)
{
    register struct mkroom *croom;

    for (croom = &g.rooms[0]; croom->hx >= 0; croom++)
        if ((type == ANY_TYPE && croom->rtype != OROOM)
            || (type == ANY_SHOP && croom->rtype >= SHOPBASE)
            || croom->rtype == type)
            return croom;
    for (croom = &g.subrooms[0]; croom->hx >= 0; croom++)
        if ((type == ANY_TYPE && croom->rtype != OROOM)
            || (type == ANY_SHOP && croom->rtype >= SHOPBASE)
            || croom->rtype == type)
            return croom;
    return (struct mkroom *) 0;
}

/* Return an appropriate monster type for generating in throne rooms. */
struct permonst *
courtmon(void)
{
    int i = rn2(60) + rn2(3 * level_difficulty());

    if (i > 100)
        return mkclass(S_DRAGON, 0);
    else if (i > 95)
        return mkclass(S_GIANT, 0);
    else if (i > 85)
        return mkclass(S_TROLL, 0);
    else if (i > 75)
        return mkclass(S_CENTAUR, 0);
    else if (i > 60)
        return mkclass(S_ORC, 0);
    else if (i > 45)
        return &mons[PM_BUGBEAR];
    else if (i > 30)
        return &mons[PM_HOBGOBLIN];
    else if (i > 15)
        return mkclass(S_GNOME, 0);
    else
        return mkclass(S_KOBOLD, 0);
}

static const struct {
    unsigned pm;
    unsigned prob;
} squadprob[] = { { PM_SOLDIER, 80 },
                  { PM_SERGEANT, 15 },
                  { PM_LIEUTENANT, 4 },
                  { PM_CAPTAIN, 1 } };

/* Return an appropriate Yendorian Army monster type for generating in
 * barracks. They will generate with the percentage odds given above. */
static struct permonst *
squadmon(void)
{
    int sel_prob, i, cpro, mndx;

    sel_prob = rnd(80 + level_difficulty());

    cpro = 0;
    for (i = 0; i < SIZE(squadprob); i++) {
        cpro += squadprob[i].prob;
        if (cpro > sel_prob) {
            mndx = squadprob[i].pm;
            goto gotone;
        }
    }
    mndx = squadprob[rn2(SIZE(squadprob))].pm;
gotone:
    if (!(g.mvitals[mndx].mvflags & G_GONE))
        return &mons[mndx];
    else
        return (struct permonst *) 0;
}

/*
 * save_room : A recursive function that saves a room and its subrooms
 * (if any).
 */
static void
save_room(NHFILE* nhfp, struct mkroom* r)
{
    short i;

    /*
     * Well, I really should write only useful information instead
     * of writing the whole structure. That is I should not write
     * the g.subrooms pointers, but who cares ?
     */
    if (nhfp->structlevel)
        bwrite(nhfp->fd, (genericptr_t) r, sizeof (struct mkroom));
    for (i = 0; i < r->nsubrooms; i++) {
        save_room(nhfp, r->sbrooms[i]);
    }
}

/*
 * save_rooms : Save all the rooms on disk!
 */
void
save_rooms(NHFILE* nhfp)
{
    short i;

    /* First, write the number of rooms */
    if (nhfp->structlevel)
        bwrite(nhfp->fd, (genericptr_t) &g.nroom, sizeof(g.nroom));
    for (i = 0; i < g.nroom; i++)
        save_room(nhfp, &g.rooms[i]);
}

static void
rest_room(NHFILE* nhfp, struct mkroom* r)
{
    short i;

    if (nhfp->structlevel)
        mread(nhfp->fd, (genericptr_t) r, sizeof(struct mkroom));

    for (i = 0; i < r->nsubrooms; i++) {
        r->sbrooms[i] = &g.subrooms[g.nsubroom];
        rest_room(nhfp, &g.subrooms[g.nsubroom]);
        g.subrooms[g.nsubroom++].resident = (struct monst *) 0;
    }
}

/*
 * rest_rooms : That's for restoring rooms. Read the rooms structure from
 * the disk.
 */
void
rest_rooms(NHFILE* nhfp)
{
    short i;

    if (nhfp->structlevel)
        mread(nhfp->fd, (genericptr_t) &g.nroom, sizeof(g.nroom));

    g.nsubroom = 0;
    for (i = 0; i < g.nroom; i++) {
        rest_room(nhfp, &g.rooms[i]);
        g.rooms[i].resident = (struct monst *) 0;
    }
    g.rooms[g.nroom].hx = -1; /* restore ending flags */
    g.subrooms[g.nsubroom].hx = -1;
}

/* convert a display symbol for terrain into topology type;
   used for remembered terrain when mimics pose as furniture */
int
cmap_to_type(int sym)
{
    int typ = STONE; /* catchall */

    switch (sym) {
    case S_stone:
        typ = STONE;
        break;
    case S_vwall:
        typ = VWALL;
        break;
    case S_hwall:
        typ = HWALL;
        break;
    case S_tlcorn:
        typ = TLCORNER;
        break;
    case S_trcorn:
        typ = TRCORNER;
        break;
    case S_blcorn:
        typ = BLCORNER;
        break;
    case S_brcorn:
        typ = BRCORNER;
        break;
    case S_crwall:
        typ = CROSSWALL;
        break;
    case S_tuwall:
        typ = TUWALL;
        break;
    case S_tdwall:
        typ = TDWALL;
        break;
    case S_tlwall:
        typ = TLWALL;
        break;
    case S_trwall:
        typ = TRWALL;
        break;
    case S_ndoor:  /* no door (empty doorway) */
    case S_vodoor: /* open door in vertical wall */
    case S_hodoor: /* open door in horizontal wall */
    case S_vcdoor: /* closed door in vertical wall */
    case S_hcdoor:
        typ = DOOR;
        break;
    case S_bars:
        typ = IRONBARS;
        break;
    case S_tree:
        typ = TREE;
        break;
    case S_room:
        typ = ROOM;
        break;
    case S_corr:
    case S_litcorr:
        typ = CORR;
        break;
    case S_upstair:
    case S_dnstair:
        typ = STAIRS;
        break;
    case S_upladder:
    case S_dnladder:
        typ = LADDER;
        break;
    case S_altar:
        typ = ALTAR;
        break;
    case S_grave:
        typ = GRAVE;
        break;
    case S_throne:
        typ = THRONE;
        break;
    case S_sink:
        typ = SINK;
        break;
    case S_fountain:
        typ = FOUNTAIN;
        break;
    case S_pool:
        typ = POOL;
        break;
    case S_ice:
        typ = ICE;
        break;
    case S_grass:
        typ = GRASS;
        break;
    case S_lava:
        typ = LAVAPOOL;
        break;
    case S_vodbridge: /* open drawbridge spanning north/south */
    case S_hodbridge:
        typ = DRAWBRIDGE_DOWN;
        break;        /* east/west */
    case S_vcdbridge: /* closed drawbridge in vertical wall */
    case S_hcdbridge:
        typ = DBWALL;
        break;
    case S_air:
        typ = AIR;
        break;
    case S_cloud:
        typ = CLOUD;
        break;
    case S_water:
        typ = WATER;
        break;
    default:
        break; /* not a cmap symbol? */
    }
    return typ;
}

/*mkroom.c*/
