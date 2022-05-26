/* NetHack 3.7	mklev.c	$NHDT-Date: 1648066813 2022/03/23 20:20:13 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.121 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Alex Smith, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* for UNIX, Rand #def'd to (long)lrand48() or (long)random() */
/* croom->lx etc are schar (width <= int), so % arith ensures that */
/* conversion of result to int is reasonable */

static int mkmonst_in_room(struct mkroom *);
static boolean generate_stairs_room_good(struct mkroom *, int);
static struct mkroom *generate_stairs_find_room(void);
static void generate_stairs(void);
static boolean find_okay_roompos(struct mkroom *, coord *);
static void makevtele(void);
void clear_level_structures(void);
static void fill_ordinary_room(struct mkroom *);
static void makelevel(void);
static boolean bydoor(xchar, xchar);
static struct mkroom *find_branch_room(coord *);
static boolean place_niche(struct mkroom *, int *, int *, int *);
static void makeniche(int);
static void make_niches(void);
static int QSORTCALLBACK mkroom_cmp(const genericptr, const genericptr);
static void dosdoor(xchar, xchar, struct mkroom *, int);
static void join(int, int, boolean);
static void do_room_or_subroom(struct mkroom *, int, int, int, int, boolean,
                               schar, boolean, boolean);
static void makerooms(void);
static boolean door_into_nonjoined(xchar, xchar);
static boolean finddpos(coord *, xchar, xchar, xchar, xchar);
static void mkinvpos(xchar, xchar, int);
static void mk_knox_portal(xchar, xchar);

#define create_vault() create_room(-1, -1, 2, 2, -1, -1, VAULT, TRUE)
#define init_vault() g.vault_x = -1
#define do_vault() (g.vault_x != -1)

/* Compare two room pointers by their x-coordinate. Used as a callback to
 * qsort.
 * Args must be (const genericptr) so that qsort will always be happy. */

static int QSORTCALLBACK
mkroom_cmp(const genericptr vx, const genericptr vy)
{
#ifdef LINT
    /* lint complains about possible pointer alignment problems, but we know
       that vx and vy are always properly aligned. Hence, the following
       bogus definition:
    */
    return (vx == vy) ? 0 : -1;
#else
    register const struct mkroom *x, *y;

    x = (const struct mkroom *) vx;
    y = (const struct mkroom *) vy;
    if (x->lx < y->lx)
        return -1;
    return (x->lx > y->lx);
#endif /* LINT */
}

/* Return TRUE if a door placed at (x, y) which otherwise passes okdoor()
 * checks would be connecting into an area that was declared as joined = false.
 * Checking for this in finddpos() enables us to have rooms with sub-areas
 * (such as shops) that will never randomly generate unwanted doors in order
 * to connect them up to other areas.
 */
static boolean
door_into_nonjoined(xchar x, xchar y)
{
    xchar tx, ty, i;

    for (i = 0; i < 4; i++) {
        tx = x + xdir[dirs_ord[i]];
        ty = y + ydir[dirs_ord[i]];
        if (!isok(tx, ty) || IS_ROCK(levl[tx][ty].typ))
            continue;

        /* Is this connecting to a room that doesn't want joining? */
        if (levl[tx][ty].roomno >= ROOMOFFSET
            && !g.rooms[levl[tx][ty].roomno - ROOMOFFSET].needjoining) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Find a valid position to place a door within the rectangle bounded by
 * (xl, yl, xh, yh), as defined by okdoor(). First, try to pick a single random
 * spot, then iterate over the entire area.
 * If it can't find any valid places it'll just default to an
 * existing door.
 */
static boolean
finddpos(coord *cc, xchar xl, xchar yl, xchar xh, xchar yh)
{
    register xchar x, y;

    x = rn1(xh - xl + 1, xl);
    y = rn1(yh - yl + 1, yl);
    /* Avoid placing doors connecting to !needjoining areas. */
    if (okdoor(x, y) && !door_into_nonjoined(x, y))
        goto gotit;

    for (x = xl; x <= xh; x++)
        for (y = yl; y <= yh; y++)
            if (okdoor(x, y) && !door_into_nonjoined(x, y))
                goto gotit;

    for (x = xl; x <= xh; x++)
        for (y = yl; y <= yh; y++)
            if (IS_DOOR(levl[x][y].typ) || levl[x][y].typ == SDOOR)
                goto gotit;
    /* cannot find something reasonable -- strange. */
    impossible("couldn't find an okdoor pos within (%d, %d, %d, %d)!",
               xl, yl, xh, yh);
    x = xl;
    y = yh;
    return FALSE;
 gotit:
    cc->x = x;
    cc->y = y;
    return TRUE;
}

/* Sort rooms on the level so they're ordered from left to right on the map.
   makecorridors() by default links rooms N and N+1 */
void
sort_rooms(void)
{
    int x, y;
    unsigned i, ri[MAXNROFROOMS + 1], n = (unsigned) g.nroom;

    qsort((genericptr_t) g.rooms, n, sizeof (struct mkroom), mkroom_cmp);

    /* Update the roomnos on the map */
    for (i = 0; i < n; i++)
        ri[g.rooms[i].roomnoidx] = i;

    for (x = 1; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++) {
            unsigned rno = levl[x][y].roomno;

            if (rno >= ROOMOFFSET && rno < MAXNROFROOMS + 1)
                levl[x][y].roomno = ri[rno - ROOMOFFSET] + ROOMOFFSET;
        }
}

/* Initialize the croom struct and the portion of the level it sits on. This
 * must be a regular (rectangular) room.
 * lowx, lowy, hix, hiy: the bounding box of the room floor, NOT including its
 *   walls.
 * lit: Whether to light the whole room area.
 * rtype: The room type. This directly sets croom->rtype without calling mkroom
 *   even for special rooms. All randomly generated rooms currently specify
 *   OROOM, but special levels may want to specify a rtype and leave the room
 *   unfilled (e.g. an abandoned temple).
 * special: If FALSE, this function will initialize the room terrain to be a
 *   rectangle of floor surrounded by the appropriate walls. If TRUE, it will
 *   skip this step.
 * is_room: Whether this room is a full room. FALSE if it's a subroom.
 *   Only relevant to wallification and if special = FALSE. */
static void
do_room_or_subroom(register struct mkroom *croom,
                   int lowx, int lowy, register int hix, register int hiy,
                   boolean lit, schar rtype, boolean special, boolean is_room)
{
    register int x, y;
    struct rm *lev;

    /* locations might bump level edges in wall-less rooms */
    /* add/subtract 1 to allow for edge locations */
    if (!lowx)
        lowx++;
    if (!lowy)
        lowy++;
    if (hix >= COLNO - 1)
        hix = COLNO - 2;
    if (hiy >= ROWNO - 1)
        hiy = ROWNO - 2;

    if (lit) {
        for (x = lowx - 1; x <= hix + 1; x++) {
            lev = &levl[x][max(lowy - 1, 0)];
            for (y = lowy - 1; y <= hiy + 1; y++)
                lev++->lit = 1;
        }
        croom->rlit = 1;
    } else
        croom->rlit = 0;

    croom->roomnoidx = (croom - g.rooms);
    croom->lx = lowx;
    croom->hx = hix;
    croom->ly = lowy;
    croom->hy = hiy;
    croom->rtype = rtype;
    croom->doorct = 0;
    /* if we're not making a vault, g.doorindex will still be 0
     * if we are, we'll have problems adding niches to the previous room
     * unless fdoor is at least g.doorindex
     */
    croom->fdoor = g.doorindex;
    croom->irregular = FALSE;

    croom->nsubrooms = 0;
    croom->sbrooms[0] = (struct mkroom *) 0;
    if (!special) {
        croom->needjoining = TRUE;
        for (x = lowx - 1; x <= hix + 1; x++)
            for (y = lowy - 1; y <= hiy + 1; y += (hiy - lowy + 2)) {
                levl[x][y].typ = HWALL;
                levl[x][y].horizontal = 1; /* For open/secret doors. */
            }
        for (x = lowx - 1; x <= hix + 1; x += (hix - lowx + 2))
            for (y = lowy; y <= hiy; y++) {
                levl[x][y].typ = VWALL;
                levl[x][y].horizontal = 0; /* For open/secret doors. */
            }
        for (x = lowx; x <= hix; x++) {
            lev = &levl[x][lowy];
            for (y = lowy; y <= hiy; y++)
                lev++->typ = ROOM;
        }
        if (is_room) {
            levl[lowx - 1][lowy - 1].typ = TLCORNER;
            levl[hix + 1][lowy - 1].typ = TRCORNER;
            levl[lowx - 1][hiy + 1].typ = BLCORNER;
            levl[hix + 1][hiy + 1].typ = BRCORNER;
        } else { /* a subroom */
            wallification(lowx - 1, lowy - 1, hix + 1, hiy + 1);
        }
    }
}

/* Adds a new room to the map.
 * Arguments are the same as do_room_or_subroom(), except is_room is hardcoded
 * to TRUE.
 */
void
add_room(int lowx, int lowy, int hix, int hiy,
         boolean lit, schar rtype, boolean special)
{
    register struct mkroom *croom;

    croom = &g.rooms[g.nroom];
    do_room_or_subroom(croom, lowx, lowy, hix, hiy, lit, rtype, special,
                       (boolean) TRUE);
    croom++;
    croom->hx = -1;
    g.nroom++;
}

/* Adds a new subroom to the map as part of the given room.
 * Arguments are again the same as those passed to do_room_or_subroom() with
 * is_room hardcoded to FALSE.
 */
void
add_subroom(struct mkroom *proom, int lowx, int lowy, int hix, int hiy,
            boolean lit, schar rtype, boolean special)
{
    register struct mkroom *croom;

    croom = &g.subrooms[g.nsubroom];
    do_room_or_subroom(croom, lowx, lowy, hix, hiy, lit, rtype, special,
                       (boolean) FALSE);
    proom->sbrooms[proom->nsubrooms++] = croom;
    croom++;
    croom->hx = -1;
    g.nsubroom++;
}

void
free_luathemes(boolean keependgame) /* F: done, T: discarding main dungeon */
{
    int i;

    for (i = 0; i < g.n_dgns; ++i) {
        if (keependgame && i == astral_level.dnum)
            continue;
        if (g.luathemes[i]) {
            nhl_done((lua_State *) g.luathemes[i]);
            g.luathemes[i] = (lua_State *) 0;
        }
    }
}

/* Repeatedly create rooms and place them on the map until we can't create any
 * more. */
static void
makerooms(void)
{
    boolean tried_vault = FALSE;
    int themeroom_tries = 0;
    char *fname;
    nhl_sandbox_info sbi = {NHL_SB_SAFE, 0, 0, 0};
    lua_State *themes = (lua_State *) g.luathemes[u.uz.dnum];

    if (!themes && *(fname = g.dungeons[u.uz.dnum].themerms)) {
        if ((themes = nhl_init(&sbi)) != 0) {
            if (!nhl_loadlua(themes, fname)) {
                /* loading lua failed, don't use themed rooms */
                nhl_done(themes);
                themes = (lua_State *) 0;
            } else {
                /* success; save state for this dungeon branch */
                g.luathemes[u.uz.dnum] = (genericptr_t) themes;
                /* keep themes context, so not 'nhl_done(themes);' */
                iflags.in_lua = FALSE; /* can affect error messages */
            }
        }
        if (!themes) /* don't try again when making next level */
            *fname = '\0'; /* g.dungeons[u.uz.dnum].themerms */
    }

    if (themes) {
        create_des_coder();
    }

    /* rnd_rect() will return 0 if no more rects are available... */
    while (g.nroom < MAXNROFROOMS && rnd_rect()) {
        /* If a certain number of rooms have already been created, and we have
         * not yet tried to make a vault, with 50% probability, try to create
         * one. */
        if (g.nroom >= (MAXNROFROOMS / 6) && rn2(2) && !tried_vault) {
            tried_vault = TRUE;
            if (create_vault()) {
                /* This won't actually create the room and edit the terrain
                 * with add_room. It'll just set the lx and ly of rooms[nroom]
                 * to represent its location. */
                g.vault_x = g.rooms[g.nroom].lx;
                g.vault_y = g.rooms[g.nroom].ly;
                g.rooms[g.nroom].hx = -1;
            }
        } else {
            if (themes) {
                iflags.in_lua = g.in_mk_themerooms = TRUE;
                g.themeroom_failed = FALSE;
                lua_getglobal(themes, "themerooms_generate");
		if ( nhl_pcall(themes, 0, 0)){
		    impossible("Lua error: %s", lua_tostring(themes, -1));
		}
                iflags.in_lua = g.in_mk_themerooms = FALSE;
                if (g.themeroom_failed
                    && ((themeroom_tries++ > 10)
                        || (g.nroom >= (MAXNROFROOMS / 6))))
                    break;
            } else {
                /* Try to create another random room. If it can't find anywhere for
                 * one to go, stop making rooms.
                 * Use the parameters for a totally random ordinary room. */
                if (!create_room(-1, -1, -1, -1, -1, -1, OROOM, -1))
                    break;;
            }
        }
    }
    if (themes) {
        wallification(1, 0, COLNO - 1, ROWNO - 1);
        free(g.coder);
        g.coder = NULL;
        /* Exiting lua code now, so this needs to be unset, but nhl_done() can't
         * be used because that would free g.luathemes */
        iflags.in_lua = FALSE;
    }
}

/* Join rooms a and b together by drawing a corridor and placing doors.
 * If nxcor is TRUE, it will be pickier about whether to draw the corridor at
 * all, and will not create doors in !okdoor() locations.
 * The corridor will be made of CORR terrain unless this is an arboreal level
 * in which case it will use ROOM.
 * Afterwards, the smeq values of a and b will be set equal to each other.
 * Should this return boolean (success or failure)? */
static void
join(register int a, register int b, boolean nxcor)
{
    coord cc, tt, org, dest;
    register xchar tx, ty, xx, yy;
    register struct mkroom *croom, *troom;
    register int dx, dy;

    croom = &g.rooms[a];
    troom = &g.rooms[b];

    if (!croom->needjoining || !troom->needjoining)
        return;

    /* find positions cc and tt for doors in croom and troom
       and direction for a corridor between them */

    /* if either room is not an actual room (hx = -1), or if too many doors
     * exist already, abort */
    if (troom->hx < 0 || croom->hx < 0 || g.doorindex >= DOORMAX)
        return;

    /* Determine how croom and troom are positioned relative to each other,
     * then pick random positions on their walls that face each other where
     * doors will be created.
     * Note: This has a horizontal bias; if troom is, for instance, both to the
     * right of and below croom, the ordering of the if clauses here will
     * always place the doors on croom's right wall and troom's left wall.
     * This may be intentional, since the playing field is much wider than it
     * is tall. */
    if (troom->lx > croom->hx) {
        /* troom to the right of croom */
        dx = 1;
        dy = 0;
        xx = croom->hx + 1;
        tx = troom->lx - 1;
        if (!finddpos(&cc, xx, croom->ly, xx, croom->hy))
            return;
        if (!finddpos(&tt, tx, troom->ly, tx, troom->hy))
            return;
    } else if (troom->hy < croom->ly) {
        /* troom below croom */
        dy = -1;
        dx = 0;
        yy = croom->ly - 1;
        ty = troom->hy + 1;
        if (!finddpos(&cc, croom->lx, yy, croom->hx, yy))
            return;
        if (!finddpos(&tt, troom->lx, ty, troom->hx, ty))
            return;
    } else if (troom->hx < croom->lx) {
        /* troom to the left of croom */
        dx = -1;
        dy = 0;
        xx = croom->lx - 1;
        tx = troom->hx + 1;
        if (!finddpos(&cc, xx, croom->ly, xx, croom->hy))
            return;
        if (!finddpos(&tt, tx, troom->ly, tx, troom->hy))
            return;
    } else {
        /* otherwise troom must be below croom */
        dy = 1;
        dx = 0;
        yy = croom->hy + 1;
        ty = troom->ly - 1;
        if (!finddpos(&cc, croom->lx, yy, croom->hx, yy))
            return;
        if (!finddpos(&tt, troom->lx, ty, troom->hx, ty))
            return;
    }
    xx = cc.x;
    yy = cc.y;
    tx = tt.x - dx;
    ty = tt.y - dy;

    /* If nxcor is TRUE and the space outside croom's door isn't stone (maybe
     * some previous corridor has already been drawn here?), abort. */
    if (nxcor && levl[xx + dx][yy + dy].typ != STONE)
        return;

    /* If we can put a door in croom's wall or nxcor is FALSE, do so. */
    if (okdoor(xx, yy) || !nxcor)
        dodoor(xx, yy, croom);

    /* Attempt to dig the corridor. If it fails for some reason, abort. */
    org.x = xx + dx;
    org.y = yy + dy;
    dest.x = tx;
    dest.y = ty;

    if (!dig_corridor(&org, &dest, nxcor,
                      g.level.flags.arboreal ? ROOM : CORR, STONE))
        return;

    /* We succeeded in digging the corridor.
     * If we can put the door in troom's wall or nxcor is FALSE, do so. */
    if (okdoor(tt.x, tt.y) || !nxcor)
        dodoor(tt.x, tt.y, troom);

    /* Set the smeq values for these rooms to be equal to each other, denoting
     * that these two rooms are now part of the same reachable section of the
     * level.
     * Importantly, this does NOT propagate the smeq value to any other rooms
     * with the to-be-overwritten smeq value! */
    if (g.smeq[a] < g.smeq[b])
        g.smeq[b] = g.smeq[a];
    else
        g.smeq[a] = g.smeq[b];
}

/* Generate corridors connecting all the rooms on the level. */
void
makecorridors(void)
{
    int a, b, i;
    boolean any = TRUE;

    /* Connect each room to the next room in rooms.
     *
     * Since during normal random level generation, rooms is sorted by order of
     * x-coordinate prior to calling this function, this first step will,
     * unless it hits the !rn2(50), connect each room to the next room to its
     * right, which will set everyone's smeq value to the same number. This
     * will deny the next two loops in this function from getting to connect
     * anything. Occasionally a level will be created by this having a series
     * of up-and-down switchbacks, and no other corridors.
     *
     * It's rather easy to see all the rooms joined in order from left to right
     * across the level if you know what you're looking for. */
    for (a = 0; a < g.nroom - 1; a++) {
        join(a, a + 1, FALSE);
        if (!rn2(50))
            break; /* allow some randomness */
    }

    /* Connect each room to the room two rooms after it in rooms, if and only
     * if they do not have the same smeq already. */
    for (a = 0; a < g.nroom - 2; a++)
        if (g.smeq[a] != g.smeq[a + 2])
            join(a, a + 2, FALSE);

    /* Connect any remaining rooms with different smeqs.
     * The "any" variable is an optimization; if on a given loop no different
     * smeqs were found from the current room, there's nothing more to be done.
     * */
    for (a = 0; any && a < g.nroom; a++) {
        any = FALSE;
        for (b = 0; b < g.nroom; b++)
            if (g.smeq[a] != g.smeq[b]) {
                join(a, b, FALSE);
                any = TRUE;
            }
    }
    /* By now, all rooms should be guaranteed to be connected. */

    /* Attempt to draw a few more corridors between rooms, but don't draw the
     * corridor if it starts on an already carved out corridor space. Possibly
     * also don't create the doors.*/
    if (g.nroom > 2)
        for (i = rn2(g.nroom) + 4; i; i--) {
            a = rn2(g.nroom);
            b = rn2(g.nroom - 2);
            if (b >= a)
                b += 2;
            join(a, b, TRUE);
        }
}

/* Adds a door, not to the level itself, but to the doors array, and updates
 * other mkroom structs as necessary.
 * x and y are the coordinates of the door, and aroom is the room which is
 * getting the door. */
void
add_door(register int x, register int y, register struct mkroom *aroom)
{
    register struct mkroom *broom;
    register int tmp;
    int i;

    /* Do nothing if this door already exists in g.doors. */
    if (aroom->doorct) {
        for (i = 0; i < aroom->doorct; i++) {
            tmp = aroom->fdoor + i;
            if (g.doors[tmp].x == x && g.doors[tmp].y == y)
                return;
        }
    }

    /* If this room doesn't have any doors yet, it becomes the last room on the
     * doors array. */
    if (aroom->doorct == 0)
        aroom->fdoor = g.doorindex;

    aroom->doorct++;

    /* If this room did already have doors, move all the other doors up in
     * position by 1. */
    for (tmp = g.doorindex; tmp > aroom->fdoor; tmp--)
        g.doors[tmp] = g.doors[tmp - 1];

    /* If this room was not the last room on the doors array, increment fdoor
     * for any rooms after it (because aroom's will be eating up another index)
     */
    for (i = 0; i < g.nroom; i++) {
        broom = &g.rooms[i];
        if (broom != aroom && broom->doorct && broom->fdoor >= aroom->fdoor)
            broom->fdoor++;
    }
    /* ditto for subrooms */
    for (i = 0; i < g.nsubroom; i++) {
        broom = &g.subrooms[i];
        if (broom != aroom && broom->doorct && broom->fdoor >= aroom->fdoor)
            broom->fdoor++;
    }

    /* finally, increment doorindex because we have one more door now, and
     * aroom's first door becomes this one. */
    g.doorindex++;
    g.doors[aroom->fdoor].x = x;
    g.doors[aroom->fdoor].y = y;
}

/* Generate the door mask for a random door. Contains the random probabilities
 * that determine what doorstate the door gets, and whether it becomes trapped,
 * but does not actually set any level structures.
 * typ is either DOOR or SDOOR.
 */
xchar
random_door_mask(int typ, boolean shdoor)
{
    struct rm tmprm; /* so we can use various set_door_*() */

    tmprm.doormask = 0;

    if (!IS_DOOR(typ) && typ != SDOOR) {
        impossible("random_door_mask: bad typ %d", typ);
    }

    /* is it a locked door, closed, or a doorway? */
    if (!rn2(3) || typ == SDOOR || g.level.flags.is_maze_lev) {
        /* 1/3 of random doorways have a physical door, unless it's a maze level
         * in which case 100% of random doorways do, for some reason. */
        if (rn2(5) || typ == SDOOR) {
            /* 80% of doorways with a door have it closed. Secret doors must be
             * closed. */
            set_doorstate(&tmprm, D_CLOSED);
            if (!rn2(4)) {
                /* 25% of closed doors are locked. */
                set_door_lock(&tmprm, TRUE);
            }
        }
        else {
            set_doorstate(&tmprm, D_ISOPEN);
        }
    } else {
        /* 2/3 of random doorways lack a physical door... but shop doors need a
         * real one so force a real door to exist */
        set_doorstate(&tmprm, shdoor ? D_ISOPEN : D_NODOOR);
    }

    /* We don't have the x, y coordinates for the door, so we can't determine
     * whether the trap that would generate there would be suitable for the
     * initial door state. Set the trap flag based on probability only; the
     * caller must check to see if it's valid. */
    if (!rn2(25) && !shdoor) {
        set_door_trap(&tmprm, TRUE);
    }

    /* chance of an iron door on deeper levels */
    if (doorstate(&tmprm) != D_NODOOR && rn1(40, 10) < level_difficulty()) {
        set_door_iron(&tmprm, TRUE);
    }

    return tmprm.doormask;
}

/* Determine whether a door trap at a given x, y coordinate would not make sense
 * with the door's state, and if it's nonsensical, remove the trap.
 * Return TRUE if nonsense is found. */
void
clear_nonsense_doortraps(xchar x, xchar y)
{
    const int traptype = getdoortrap(x, y);
    struct rm *lev = &levl[x][y];

    if (!IS_DOOR(lev->typ) && lev->typ != SDOOR) {
        impossible("trying to check trap on non-door at %d %d?", x, y);
    }

    if (!door_is_trapped(lev)) {
        return; /* no trap = nothing to do */
    }

    switch (traptype) {
    case SELF_LOCK:
    case HINGE_SCREECH:
    case STATIC_SHOCK:
    case ROCKFALL:
    case HOT_KNOB:
        /* traps that require an actual unbroken door */
        if (doorstate(lev) != D_ISOPEN && doorstate(lev) != D_CLOSED) {
            set_door_trap(lev, FALSE);
        }
        break;
    case WATER_BUCKET:
    case HINGELESS_FORWARD:
    case HINGELESS_BACKWARD:
    case FIRE_BLAST:
        /* traps that require a closed door */
        if (doorstate(lev) != D_CLOSED)
            set_door_trap(lev, FALSE);
        break;
    default:
        impossible("clear_nonsense_doortrap: bad door trap type %d", traptype);
    }
}

/* Create a door or a secret door (using type) in aroom at location (x,y).
 * Sets the doormask randomly.
 *
 * Doors are never generated broken. Shop doors tend to be generated open, and
 * never generate trapped. (They can be locked, though, in which case the shop
 * becomes closed for inventory.) Secret doors always generate closed or locked.
 */
static void
dosdoor(register xchar x, register xchar y, struct mkroom *aroom, int type)
{
    struct rm* lev = &levl[x][y];

    if (!IS_WALL(lev->typ)) /* avoid SDOORs on already made doors */
        type = DOOR;
    lev->typ = type;

    lev->doormask = random_door_mask(type, *in_rooms(x, y, SHOPBASE));

    if (door_is_trapped(lev)) {
        struct monst *mtmp;

        if (level_difficulty() >= 9 && !rn2(7) && type != SDOOR
            && !((g.mvitals[PM_SMALL_MIMIC].mvflags & G_GONE)
                    && (g.mvitals[PM_LARGE_MIMIC].mvflags & G_GONE)
                    && (g.mvitals[PM_GIANT_MIMIC].mvflags & G_GONE))) {
            /* make a mimic instead */
            set_door_trap(lev, FALSE);
            set_doorstate(lev, D_NODOOR);
            mtmp = makemon(mkclass(S_MIMIC, 0), x, y, NO_MM_FLAGS);
            if (mtmp)
                set_mimic_sym(mtmp);
        }
    }

    clear_nonsense_doortraps(x, y);
    add_door(x, y, aroom);
}

/* Determine whether a niche (closet) can be placed on one edge of a room.
 * Contrary to the name, this does not actually place a niche; perhaps it
 * should be renamed to something more straightforward.
 * If the niche can be placed, xx and yy will then contain the coordinate
 * for the door, and dy will contain the direction it's supposed to go in (that
 * is, the actual niche square is (xx, yy+dy)).
 */
static boolean
place_niche(register struct mkroom *aroom, int *dy, int *xx, int *yy)
{
    coord dd;

    /* Niches only ever generate on the top and bottom walls of rooms, for some
     * reason. Probably because it looks better.
     * Horizontal "niches" might still appear from time to time as a result of
     * dig_corridor shenanigans, but they're failed corridors, not real niches.
     * Look for a suitable spot on one of these walls to place a niche. */
    if (rn2(2)) {
        *dy = 1;
        if (!finddpos(&dd, aroom->lx, aroom->hy + 1, aroom->hx, aroom->hy + 1))
            return FALSE;
    } else {
        *dy = -1;
        if (!finddpos(&dd, aroom->lx, aroom->ly - 1, aroom->hx, aroom->ly - 1))
            return FALSE;
    }
    *xx = dd.x;
    *yy = dd.y;
    /* Spot for the niche must be stone; other spot just inside the room must
     * not be water or another dungeon feature.
     * Note that there's no checking that the area surrounding the niche is
     * also stone; niches can generate touching one or more corridor spaces. */
    return (boolean) ((isok(*xx, *yy + *dy)
                       && levl[*xx][*yy + *dy].typ == STONE)
                      && (isok(*xx, *yy - *dy)
                          && !IS_POOL(levl[*xx][*yy - *dy].typ)
                          && !IS_FURNITURE(levl[*xx][*yy - *dy].typ)));
}

/* there should be one of these per trap, in the same order as trap.h */
static NEARDATA const char *trap_engravings[TRAPNUM] = {
    (char *) 0,      (char *) 0,    (char *) 0,    (char *) 0, (char *) 0,
    (char *) 0,      (char *) 0,    (char *) 0,    (char *) 0, (char *) 0,
    (char *) 0,      (char *) 0,    (char *) 0,    (char *) 0,
    /* 14..16: trap door, teleport, level-teleport */
    "Vlad was here", "ad aerarium", "ad aerarium", (char *) 0, (char *) 0,
    (char *) 0,      (char *) 0,    (char *) 0,    (char *) 0, (char *) 0,
    /* 24..25 */
    (char *) 0, (char *) 0,
};

/* Actually create a niche/closet, on a random room. Place a trap on it if
 * trap_type != NO_TRAP.
 */
static void
makeniche(int trap_type)
{
    register struct mkroom *aroom;
    struct rm *rm;
    int vct = 8; /* number of attempts */
    int dy, xx, yy;
    struct trap *ttmp;

    if (g.doorindex < DOORMAX) {
        while (vct--) {
            aroom = &g.rooms[rn2(g.nroom)];
            if (aroom->rtype != OROOM)
                /* don't place niches in special rooms */
                continue;
            if (aroom->doorct == 1 && rn2(5))
                /* usually don't place in rooms with 1 door */
                continue;
            if (!place_niche(aroom, &dy, &xx, &yy))
                /* didn't find a suitable spot */
                continue;

            rm = &levl[xx][yy + dy];
            if (trap_type || !rn2(4)) {
                /* all closets with traps and 25% of other closets require some
                 * searching */
                rm->typ = SCORR;
                if (trap_type) {
                    /* don't place fallthru traps on undiggable levels */
                    if (is_hole(trap_type) && !Can_fall_thru(&u.uz))
                        trap_type = ROCKTRAP;
                    ttmp = maketrap(xx, yy + dy, trap_type);
                    if (ttmp) {
                        if (trap_type != ROCKTRAP)
                            ttmp->once = 1;
                        /* make the specified engraving in front of the door */
                        if (trap_engravings[trap_type]) {
                            make_engr_at(xx, yy - dy,
                                         trap_engravings[trap_type], 0L,
                                         DUST);
                            wipe_engr_at(xx, yy - dy, 5,
                                         FALSE); /* age it a little */
                        }
                    }
                }
                /* place the door */
                dosdoor(xx, yy, aroom, SDOOR);
            } else {
                rm->typ = CORR;
                /* 1/7 of these niches are generated inaccessible - no actual
                 * connection to their corresponding room */
                if (rn2(7))
                    dosdoor(xx, yy, aroom, rn2(5) ? SDOOR : DOOR);
                else {
                    /* inaccessible niches occasionally have iron bars */
                    if (!rn2(5) && IS_WALL(levl[xx][yy].typ)) {
                        (void) set_levltyp(xx, yy, IRONBARS);
                        if (rn2(3))
                            /* For the love of God, Montresor! */
                            (void) mkcorpstat(CORPSE, (struct monst *) 0,
                                              mkclass(S_HUMAN, 0), xx,
                                              yy + dy, TRUE);
                    }
                    /* Place a teleport scroll here so the player can escape.
                     * If an inaccessible niche is generated on a no-tele
                     * level, the player shouldn't be able to get into it
                     * without some way of getting back out... */
                    if (!g.level.flags.noteleport)
                        (void) mksobj_at(SCR_TELEPORTATION, xx, yy + dy, TRUE,
                                         FALSE);
                    if (!rn2(3))
                        (void) mkobj_at(0, xx, yy + dy, TRUE);
                }
            }
            /* mark as niche */
            rm->is_niche = TRUE;
            return;
        }
    }
}

/* Try to create several random niches across an entire level.
 * Does NOT include the niche for a vault teleporter, if one exists. */
static void
make_niches(void)
{
    /* This should really be nroom / 2... */
    int ct = rnd((g.nroom >> 1) + 1), dep = depth(&u.uz);
    boolean ltptr = (!g.level.flags.noteleport && dep > 15),
            vamp = (dep > 5 && dep < 25);

    while (ct--) {
        if (ltptr && !rn2(6)) {
            /* occasional fake vault teleporter */
            ltptr = FALSE;
            makeniche(LEVEL_TELEP);
        } else if (vamp && !rn2(6)) {
            /* "Vlad was here" trapdoor */
            vamp = FALSE;
            makeniche(TRAPDOOR);
        } else
            /* regular untrapped niche */
            makeniche(NO_TRAP);
    }
}

/* Create a vault teleporter niche.
 * The code seems to assume that any teleport trap inside a niche should always
 * go to a vault; this may become problematic if the player ever gains the
 * ability to make teleport traps...
 */
static void
makevtele(void)
{
    makeniche(TELEP_TRAP);
}

/* Choose an appropriate special room type for the given level. */
int
rand_roomtype(void)
{
    int u_depth = depth(&u.uz);
    /* minimum number of rooms needed to allow a random special room */
    int room_threshold = Is_branchlev(&u.uz) ? 4 : 3;
    if (g.level.flags.has_vault)
        room_threshold++;

    if (!Inhell) {
        if (u_depth > 1 && u_depth < depth(&medusa_level)
            && g.nroom >= room_threshold && rn2(u_depth) < 3) {
            /* random shop */
            return SHOPBASE;
        }
        else if (u_depth > 4 && !rn2(6))
            return COURT;
        else if (u_depth > 5 && !rn2(8)
                    && !(g.mvitals[PM_LEPRECHAUN].mvflags & G_GONE))
            return LEPREHALL;
        else if (u_depth > 6 && !rn2(7))
            return ZOO;
        else if (u_depth > 8 && !rn2(5))
            return TEMPLE;
        else if (u_depth > 9 && !rn2(5)
                    && !(g.mvitals[PM_KILLER_BEE].mvflags & G_GONE))
            return BEEHIVE;
        else if (u_depth > 11 && !rn2(6))
            return MORGUE;
        else if (u_depth > 12 && !rn2(8) && antholemon())
            return ANTHOLE;
        else if (u_depth > 14 && !rn2(4)
                    && !(g.mvitals[PM_SOLDIER].mvflags & G_GONE))
            return BARRACKS;
        else if (u_depth > 15 && !rn2(6))
            return SWAMP;
        else if (u_depth > 16 && !rn2(8)
                    && !(g.mvitals[PM_COCKATRICE].mvflags & G_GONE))
            return COCKNEST;
        else
            return OROOM;
    }
    else { /* Gehennom random special rooms */
        /* provisionally: depth doesn't really matter too much since none of
         * these rooms have a wildly higher difficulty. */
        int chance = rn2(100);
        if (chance < 25)
            return MORGUE;
        else if (chance < 45)
            return DEMONDEN;
        else if (chance < 60)
            return SUBMERGED;
        else if (chance < 75)
            return LAVAROOM;
        else if (chance < 90)
            return ABATTOIR;
        else if (chance < 95)
            return SEMINARY;
        else if (chance < 99)
            return TEMPLE; /* Moloch temple */
        else
            return STATUARY;
    }
}

/* clear out various globals that keep information on the current level.
 * some of this is only necessary for some types of levels (maze, normal,
 * special) but it's easier to put it all in one place than make sure
 * each type initializes what it needs to separately.
 */
void
clear_level_structures(void)
{
    static struct rm zerorm = { GLYPH_UNEXPLORED,
                                0, 0, 0, 0, 0, 0, 0, 0, 0 };
    register int x, y;
    register struct rm *lev;

    /* note:  normally we'd start at x=1 because map column #0 isn't used
       (except for placing vault guard at <0,0> when removed from the map
       but not from the level); explicitly reset column #0 along with the
       rest so that we start the new level with a completely clean slate */
    for (x = 0; x < COLNO; x++) {
        lev = &levl[x][0];
        for (y = 0; y < ROWNO; y++) {
            *lev++ = zerorm;
            g.level.objects[x][y] = (struct obj *) 0;
            g.level.monsters[x][y] = (struct monst *) 0;
        }
    }
    g.level.objlist = (struct obj *) 0;
    g.level.buriedobjlist = (struct obj *) 0;
    g.level.monlist = (struct monst *) 0;
    g.level.damagelist = (struct damage *) 0;
    g.level.bonesinfo = (struct cemetery *) 0;

    g.level.flags.nfountains = 0;
    g.level.flags.nsinks = 0;
    g.level.flags.has_shop = 0;
    g.level.flags.has_vault = 0;
    g.level.flags.has_zoo = 0;
    g.level.flags.has_court = 0;
    g.level.flags.has_morgue = g.level.flags.graveyard = 0;
    g.level.flags.has_beehive = 0;
    g.level.flags.has_barracks = 0;
    g.level.flags.has_temple = 0;
    g.level.flags.has_swamp = 0;
    g.level.flags.noteleport = 0;
    g.level.flags.hardfloor = 0;
    g.level.flags.nommap = 0;
    g.level.flags.hero_memory = 1;
    g.level.flags.shortsighted = 0;
    g.level.flags.sokoban_rules = 0;
    g.level.flags.is_maze_lev = 0;
    g.level.flags.is_cavernous_lev = 0;
    g.level.flags.arboreal = 0;
    g.level.flags.wizard_bones = 0;
    g.level.flags.corrmaze = 0;
    g.level.flags.outdoors = 0;

    g.nroom = 0;
    g.rooms[0].hx = -1;
    g.nsubroom = 0;
    g.subrooms[0].hx = -1;
    g.doorindex = 0;
    init_rect();
    init_vault();
    stairway_free_all();
    g.made_branch = FALSE;
    clear_regions();
    g.xstart = 1;
    g.ystart = 0;
    g.xsize = COLNO - 1;
    g.ysize = ROWNO;
    if (g.lev_message) {
        free(g.lev_message);
        g.lev_message = (char *) 0;
    }
}

/* Fill a "random" room (i.e. a typical non-special room in the Dungeons of
 * Doom) with random monsters, objects, and dungeon features.
 */
static void
fill_ordinary_room(struct mkroom *croom)
{
    int trycnt = 0;
    coord pos;
    int x, y;

    if (croom->rtype != OROOM && croom->rtype != THEMEROOM)
        return;

    /* If there are subrooms, fill them now - we don't want an outer room
     * that's specified to be unfilled to block an inner subroom that's
     * specified to be filled. */
    for (x = 0; x < croom->nsubrooms; ++x) {
        fill_ordinary_room(croom->sbrooms[x]);
    }

    if (croom->needfill != FILL_NORMAL)
        return;

    /* maybe place some dungeon features inside
     * This should go first because it's capable of creating non-ACCESSIBLE
     * terrain types; we don't want to embed any monsters, objects, or traps
     * in a tree.
     * [3.7]: Most or all of the below functions use somexyspace, which
     * attempts to find a ROOM, CORR or ICE square, all of which are
     * ACCESSIBLE. */
    if (!rn2(10))
        mkfount(0, croom);
    if (!rn2(60))
        mksink(croom);
    if (!rn2(60))
        mkaltar(croom);
    if (!rn2(30 + (depth(&u.uz) * 5)))
        mktree(croom);
    x = 80 - (depth(&u.uz) * 2);
    if (x < 2)
        x = 2;
    if (!rn2(x))
        mkgrave(croom);

    /* put traps and mimics inside */
    x = 8 - (level_difficulty() / 6);
    if (x < 2)
        /* maxes out at level_difficulty() == 36 */
        x = 2;
    while (!rn2(x) && (++trycnt < 1000))
        mktrap(0, MKTRAP_NOFLAGS, croom, (coord *) 0);

    /* maybe put a monster inside */
    if (u.uhave.amulet || !rn2(2)) {
        mkmonst_in_room(croom);
    }

    /* maybe put some gold inside */
    if (!rn2(3) && somexyspace(croom, &pos)) {
        (void) mkgold(0L, pos.x, pos.y);
    }

    /* put statues inside */
    if (!rn2(20) && somexyspace(croom, &pos)) {
        (void) mkcorpstat(STATUE, (struct monst *) 0,
                          (struct permonst *) 0, pos.x,
                          pos.y, CORPSTAT_INIT);
    }
    /* put box/chest inside;
     *  40% chance for at least 1 box, regardless of number
     *  of rooms; about 5 - 7.5% for 2 boxes, least likely
     *  when few rooms; chance for 3 or more is negligible.
     */
    if (!rn2(g.nroom * 5 / 2) && somexyspace(croom, &pos)) {
        (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST,
                         pos.x, pos.y, TRUE, FALSE);
    }

    /* Maybe make some graffiti.
     * Chance decreases the lower you get in the dungeon.
     * On dlvl1, put a special graffiti in the starting room: this is always
     * a true rumor, never a false one or random engraving, and is never
     * damaged. */
    if (depth(&u.uz) == 1 && has_upstairs(croom)) {
        char buf[BUFSZ];
        getrumor(1, buf, TRUE);
        if (find_okay_roompos(croom, &pos)) {
            make_engr_at(pos.x, pos.y, buf, 0, MARK);
        }
    }
    else if (!rn2(27 + 3 * abs(depth(&u.uz)))) {
        char buf[BUFSZ];
        const char *mesg = random_engraving(buf);

        if (mesg) {
            do {
                somexyspace(croom, &pos);
                x = pos.x;
                y = pos.y;
            } while (levl[x][y].typ != ROOM && !rn2(40));
            if (!(IS_POOL(levl[x][y].typ)
                    || IS_FURNITURE(levl[x][y].typ)))
                make_engr_at(x, y, mesg, 0L, MARK);
        }
    }

    /* place a random object in the room (40% chance), with a recursive 20%
     * chance of placing another */
    x = 2;
    while (rnd(5) <= x && somexyspace(croom, &pos)) {
        (void) mkobj_at(0, pos.x, pos.y, TRUE);
        x = 1;
    }
}

/* Full initialization of all level structures, map, objects, etc.
 * Handles any level - special levels will load that special level, Gehennom
 * will create mazes, and so on.
 * Called only from mklev(). */
static void
makelevel(void)
{
    register struct mkroom *croom;
    branch *branchp;
    stairway *prevstairs;
    register s_level *slev = Is_special(&u.uz);
    int i;

    /* this is apparently used to denote that a lot of program state is
     * uninitialized */
    if (wiz1_level.dlevel == 0)
        init_dungeons();
    oinit(); /* assign level dependent obj probabilities */
    clear_level_structures(); /* full level reset */

    /* check for special levels */
    if (slev) {
        /* a special level */
        makemaz(slev->proto);
    } else if (g.dungeons[u.uz.dnum].proto[0]) {
        /* named prototype file */
        makemaz("");
    } else if (g.dungeons[u.uz.dnum].fill_lvl[0]) {
        /* various types of filler, e.g. "minefill" */
        makemaz(g.dungeons[u.uz.dnum].fill_lvl);
    } else if (In_quest(&u.uz)) {
        /* quest filler */
        char fillname[9];
        s_level *loc_lev;

        Sprintf(fillname, "%s-loca", g.urole.filecode);
        loc_lev = find_level(fillname);

        Sprintf(fillname, "%s-fil", g.urole.filecode);
        Strcat(fillname,
                (u.uz.dlevel < loc_lev->dlevel.dlevel) ? "a" : "b");
        makemaz(fillname);
    } else if (In_hell(&u.uz)
                || (rn2(5) && u.uz.dnum == medusa_level.dnum
                    && depth(&u.uz) > depth(&medusa_level))) {
        /* Gehennom, or 80% of levels below Medusa - maze filler */
        makemaz("");
    } else {
        /* otherwise, fall through - it's a "regular" level. */
        makerooms();

        /* order rooms[] by x-coordinate */
        sort_rooms();

        generate_stairs(); /* up and down stairs */

        branchp = Is_branchlev(&u.uz);    /* possible dungeon branch */
        makecorridors();
        make_niches();

        /* Did makerooms place a 2x2 unconnected room to be a vault? If so, fill
         * it.
         * Is there really a reason for do_vault() to be a macro? All it does is
         * test whether vault_x is a real coordinate. It's only used here. */
        if (do_vault()) {
            xchar w, h;

            debugpline0("trying to make a vault...");
            w = 1;
            h = 1;
            /* make sure vault can actually be placed */
            if (check_room(&g.vault_x, &w, &g.vault_y, &h, TRUE)) {
 fill_vault:
                add_room(g.vault_x, g.vault_y, g.vault_x + w, g.vault_y + h,
                        TRUE, VAULT, FALSE);
                g.level.flags.has_vault = 1;
                g.rooms[g.nroom - 1].needfill = FILL_NORMAL;
                fill_special_room(&g.rooms[g.nroom - 1]);
                mk_knox_portal(g.vault_x + w, g.vault_y + h);
                /* Only put a vault teleporter with 1/3 chance;
                 * a teleportation trap in a closet is a sure sign that a vault is
                 * on the level, but a vault is not a sure sign of a vault
                 * teleporter. */
                if (!g.level.flags.noteleport && !rn2(3))
                    makevtele();
            } else if (rnd_rect() && create_vault()) {
                /* If we didn't create a vault already, try once more. */
                g.vault_x = g.rooms[g.nroom].lx;
                g.vault_y = g.rooms[g.nroom].ly;
                if (check_room(&g.vault_x, &w, &g.vault_y, &h, TRUE))
                    goto fill_vault;
                else
                    g.rooms[g.nroom].hx = -1;
            }
        }

        /* Try to create one special room on the level.
         * The available special rooms depend on how deep you are.
         * If a special room is selected and fails to be created (e.g. it tried
         * to make a shop and failed because no room had exactly 1 door), it
         * won't try to create the other types of available special rooms.
         * Note that mkroom doesn't guarantee a room gets created, and that this
         * step only sets the room's rtype - it doesn't fill it yet. */
        if (wizard && nh_getenv("SHOPTYPE"))
            /* special case that overrides everything else for wizard mode */
            do_mkroom(SHOPBASE);
        else
            do_mkroom(rand_roomtype());

        prevstairs = g.stairs; /* used to test for place_branch() success */
        /* Place multi-dungeon branch. */
        place_branch(branchp, 0, 0);

        /* for main dungeon level 1, the stairs up where the hero starts
           are branch stairs; treat them as if hero had just come down
           them by marking them as having been traversed; most recently
           created stairway is held in 'g.stairs' */
        if (u.uz.dnum == 0 && u.uz.dlevel == 1 && g.stairs != prevstairs)
            g.stairs->u_traversed = TRUE;

        /* for each room: put things inside */
        for (croom = g.rooms; croom->hx > 0; croom++) {
            fill_ordinary_room(croom);
        }
    }
    /* Fill all special rooms now, regardless of whether this is a special
     * level, proto level, or ordinary level. */
    for (i = 0; i < g.nroom; ++i) {
        fill_special_room(&g.rooms[i]);
    }
}

/*
 * Place deposits of minerals (gold and misc gems) in the stone
 * surrounding the rooms on the map.
 * Also place kelp in water.
 * mineralize(-1, -1, -1, -1, FALSE); => "default" behaviour
 * The four probability arguments aren't percentages; assuming the spot to
 * place the item is suitable, kelp will be placed with 1/prob chance;
 * whereas gold and gems will be placed with prob/1000 chance.
 * skip_lvl_checks will ignore any checks that items don't get mineralized in
 * the wrong levels. This is currently only TRUE if a special level forces it
 * to be.
 */
void
mineralize(int kelp_pool, int kelp_moat, int goldprob, int gemprob,
           boolean skip_lvl_checks)
{
    s_level *sp;
    struct obj *otmp;
    int x, y, cnt;

    if (kelp_pool < 0)
        kelp_pool = 10;
    if (kelp_moat < 0)
        kelp_moat = 30;

    /* Place kelp, except on the plane of water */
    if (!skip_lvl_checks && In_endgame(&u.uz))
        return;
    for (x = 2; x < (COLNO - 2); x++)
        for (y = 1; y < (ROWNO - 1); y++)
            if ((kelp_pool && levl[x][y].typ == POOL && !rn2(kelp_pool))
                || (kelp_moat && levl[x][y].typ == MOAT && !rn2(kelp_moat)))
                (void) mksobj_at(KELP_FROND, x, y, TRUE, FALSE);

    /* determine if it is even allowed;
       almost all special levels are excluded */
    if (!skip_lvl_checks
        && (In_hell(&u.uz) || In_V_tower(&u.uz)
            || g.level.flags.arboreal
            || ((sp = Is_special(&u.uz)) != 0 && !Is_oracle_level(&u.uz)
                && (!In_mines(&u.uz) || sp->flags.town))))
        return;

    /* basic level-related probabilities */
    if (goldprob < 0)
        goldprob = 20 + depth(&u.uz) / 3;
    if (gemprob < 0)
        gemprob = goldprob / 4;

    /* mines have ***MORE*** goodies - otherwise why mine? */
    if (!skip_lvl_checks) {
        if (In_mines(&u.uz)) {
            goldprob *= 2;
            gemprob *= 3;
        } else if (In_quest(&u.uz)) {
            goldprob /= 4;
            gemprob /= 6;
        }
    }

    /*
     * Seed rock areas with gold and/or gems.
     * We use fairly low level object handling to avoid unnecessary
     * overhead from placing things in the floor chain prior to burial.
     */
    for (x = 2; x < (COLNO - 2); x++)
        for (y = 1; y < (ROWNO - 1); y++)
            if (levl[x][y + 1].typ != STONE) { /* <x,y> spot not eligible */
                y += 2; /* next two spots aren't eligible either */
            } else if (levl[x][y].typ != STONE) { /* this spot not eligible */
                y += 1; /* next spot isn't eligible either */
            } else if (!(levl[x][y].wall_info & W_NONDIGGABLE)
                       && levl[x][y - 1].typ == STONE
                       && levl[x + 1][y - 1].typ == STONE
                       && levl[x - 1][y - 1].typ == STONE
                       && levl[x + 1][y].typ == STONE
                       && levl[x - 1][y].typ == STONE
                       && levl[x + 1][y + 1].typ == STONE
                       && levl[x - 1][y + 1].typ == STONE) {
                if (rn2(1000) < goldprob) {
                    if ((otmp = mksobj(GOLD_PIECE, FALSE, FALSE)) != 0) {
                        otmp->ox = x, otmp->oy = y;
                        otmp->quan = 1L + rnd(goldprob * 3);
                        otmp->owt = weight(otmp);
                        if (!rn2(3))
                            add_to_buried(otmp);
                        else
                            place_object(otmp, x, y);
                    }
                }
                if (rn2(1000) < gemprob) {
                    for (cnt = rnd(2 + dunlev(&u.uz) / 3); cnt > 0; cnt--)
                        if ((otmp = mkobj(GEM_CLASS, FALSE)) != 0) {
                            if (otmp->otyp == ROCK) {
                                dealloc_obj(otmp); /* discard it */
                            } else {
                                otmp->ox = x, otmp->oy = y;
                                if (!rn2(3))
                                    add_to_buried(otmp);
                                else
                                    place_object(otmp, x, y);
                            }
                        }
                }
            }
}

/* Topmost level creation routine.
 * Mainly just wraps around makelevel(), but also handles loading bones files,
 * mineralizing after the level is created, blocking digging, setting roomnos
 * via topologize, and a couple other things.
 * Called from a few places: newgame() (to generate level 1), goto_level (any
 * other levels), and wiz_makemap (wizard mode regenerating the level).
 */
void
level_finalize_topology(void)
{
    struct mkroom *croom;
    int ridx;

    bound_digging();
    mineralize(-1, -1, -1, -1, FALSE);
    g.in_mklev = FALSE;
    /* avoid coordinates in future lua-loads for this level being thrown off
     * because xstart and ystart aren't saved with the level and will be 0 after
     * leaving and returning */
    g.xstart = g.ystart = 0;
    /* has_morgue gets cleared once morgue is entered; graveyard stays
       set (graveyard might already be set even when has_morgue is clear
       [see fixup_special()], so don't update it unconditionally) */
    if (g.level.flags.has_morgue)
        g.level.flags.graveyard = 1;
    if (!g.level.flags.is_maze_lev) {
        for (croom = &g.rooms[0]; croom != &g.rooms[g.nroom]; croom++)
#ifdef SPECIALIZATION
            topologize(croom, FALSE);
#else
            topologize(croom);
#endif
    }
    set_wall_state();
    /* for many room types, g.rooms[].rtype is zeroed once the room has been
       entered; g.rooms[].orig_rtype always retains original rtype value */
    for (ridx = 0; ridx < SIZE(g.rooms); ridx++)
        g.rooms[ridx].orig_rtype = g.rooms[ridx].rtype;
}

void
mklev(void)
{
    reseed_random(rn2);
    reseed_random(rn2_on_display_rng);

    init_mapseen(&u.uz);
    if (getbones())
        return;

    g.in_mklev = TRUE;
    makelevel();

    level_finalize_topology();

    reseed_random(rn2);
    reseed_random(rn2_on_display_rng);
}

/* Set the roomno correctly for all squares of the given room.
 * Mostly this sets them to the roomno from croom, but if there are any walls
 * that already have a roomno defined, it changes them to SHARED.
 * Then it recurses on subrooms.
 *
 * If SPECIALIZATION is defined and croom->rtype = OROOM, it will set the
 * roomno to NO_ROOM, but only if do_ordinary is TRUE.
 */
void
#ifdef SPECIALIZATION
topologize(struct mkroom *croom, boolean do_ordinary)
#else
topologize(struct mkroom *croom)
#endif
{
    register int x, y, roomno = (int) ((croom - g.rooms) + ROOMOFFSET);
    int lowx = croom->lx, lowy = croom->ly;
    int hix = croom->hx, hiy = croom->hy;
#ifdef SPECIALIZATION
    schar rtype = croom->rtype;
#endif
    int subindex, nsubrooms = croom->nsubrooms;

    /* skip the room if already done; i.e. a shop handled out of order */
    /* also skip if this is non-rectangular (it _must_ be done already) */
    if ((int) levl[lowx][lowy].roomno == roomno || croom->irregular)
        return;
#ifdef SPECIALIZATION
    if ((rtype != OROOM) || do_ordinary)
#endif
        {
        /* do innards first */
        for (x = lowx; x <= hix; x++)
            for (y = lowy; y <= hiy; y++)
#ifdef SPECIALIZATION
                if (rtype == OROOM)
                    levl[x][y].roomno = NO_ROOM;
                else
#endif
                    levl[x][y].roomno = roomno;
        /* top and bottom edges */
        for (x = lowx - 1; x <= hix + 1; x++)
            for (y = lowy - 1; y <= hiy + 1; y += (hiy - lowy + 2)) {
                levl[x][y].edge = 1;
                if (levl[x][y].roomno)
                    levl[x][y].roomno = SHARED;
                else
                    levl[x][y].roomno = roomno;
            }
        /* sides */
        for (x = lowx - 1; x <= hix + 1; x += (hix - lowx + 2))
            for (y = lowy; y <= hiy; y++) {
                levl[x][y].edge = 1;
                if (levl[x][y].roomno)
                    levl[x][y].roomno = SHARED;
                else
                    levl[x][y].roomno = roomno;
            }
    }
    /* g.subrooms */
    for (subindex = 0; subindex < nsubrooms; subindex++)
#ifdef SPECIALIZATION
        topologize(croom->sbrooms[subindex], (boolean) (rtype != OROOM));
#else
        topologize(croom->sbrooms[subindex]);
#endif
}

/* Find an unused room for a branch location. */
static struct mkroom *
find_branch_room(coord *mp)
{
    struct mkroom *croom = 0;

    if (g.nroom == 0) {
        mazexy(mp); /* already verifies location */
    } else {
        croom = generate_stairs_find_room();

        if (!somexyspace(croom, mp))
            impossible("Can't place branch!");
    }
    return croom;
}

/* Place a branch staircase or ladder for branch br at the coordinates (x,y).
 * If x is zero, pick the branch room and coordinates within it randomly.
 * If br is null, or the global made_branch is TRUE, do nothing. */
void
place_branch(
    branch *br,       /* branch to place */
    xchar x, xchar y) /* location */
{
    coord m = {0};
    d_level *dest;
    boolean make_stairs;

    /*
     * Return immediately if there is no branch to make or we have
     * already made one.  This routine can be called twice when
     * a special level is loaded that specifies an SSTAIR location
     * as a favored spot for a branch.
     */
    if (!br || g.made_branch)
        return;

    if (!x) { /* find random coordinates for branch */
        /* br_room = find_branch_room(&m); */
        (void) find_branch_room(&m);  /* sets m via mazexy() or somexy() */
        x = m.x;
        y = m.y;
    }

    if (on_level(&br->end1, &u.uz)) {
        /* we're on end1 */
        make_stairs = br->type != BR_NO_END1;
        dest = &br->end2;
    } else {
        /* we're on end2 */
        make_stairs = br->type != BR_NO_END2;
        dest = &br->end1;
    }

    if (br->type == BR_PORTAL) {
        mkportal(x, y, dest->dnum, dest->dlevel);
    } else if (make_stairs) {
        boolean goes_up = on_level(&br->end1, &u.uz) ? br->end1_up
                                                     : !br->end1_up;

        stairway_add(x, y, goes_up, FALSE, dest);
        (void) set_levltyp(x, y, STAIRS);
        levl[x][y].ladder = goes_up ? LA_UP : LA_DOWN;
    }
    /*
     * Set made_branch to TRUE even if we didn't make a stairwell (i.e.
     * make_stairs is false) since there is currently only one branch
     * per level, if we failed once, we're going to fail again on the
     * next call.
     */
    g.made_branch = TRUE;
}

/* Return TRUE if the given location is directly adjacent to a door or secret
 * door in any direction. */
static boolean
bydoor(register xchar x, register xchar y)
{
    register int typ;

    if (isok(x + 1, y)) {
        typ = levl[x + 1][y].typ;
        if (IS_DOOR(typ) || typ == SDOOR)
            return TRUE;
    }
    if (isok(x - 1, y)) {
        typ = levl[x - 1][y].typ;
        if (IS_DOOR(typ) || typ == SDOOR)
            return TRUE;
    }
    if (isok(x, y + 1)) {
        typ = levl[x][y + 1].typ;
        if (IS_DOOR(typ) || typ == SDOOR)
            return TRUE;
    }
    if (isok(x, y - 1)) {
        typ = levl[x][y - 1].typ;
        if (IS_DOOR(typ) || typ == SDOOR)
            return TRUE;
    }
    return FALSE;
}

/* Return TRUE if it is allowable to create a door at (x,y).
 * The given coordinate must be a wall and not be adjacent to a door, and we
 * can't be at the max number of doors.
 * FIXME: This should return boolean. */
int
okdoor(xchar x, xchar y)
{
    boolean near_door = bydoor(x, y);

    return ((levl[x][y].typ == HWALL || levl[x][y].typ == VWALL)
            && ((isok(x - 1, y) && !IS_ROCK(levl[x - 1][y].typ))
                || (isok(x + 1, y) && !IS_ROCK(levl[x + 1][y].typ))
                || (isok(x, y - 1) && !IS_ROCK(levl[x][y - 1].typ))
                || (isok(x, y + 1) && !IS_ROCK(levl[x][y + 1].typ)))
            && g.doorindex < DOORMAX && !near_door);
}

/* Wrapper for dosdoor. Create a door randomly at location (x,y) in aroom.
 * For some reason, the logic of whether or not to make the door secret is
 * here, while all the other logic of determining the door state is in dosdoor.
 */
void
dodoor(int x, int y, struct mkroom *aroom)
{
    if (g.doorindex >= DOORMAX) {
        impossible("DOORMAX exceeded?");
        return;
    }

    /* Probability of a random door being a secret door: sqrt(depth-3) / 35.
     * If depth <= 3, probability is 0.
     * Math here: random[0..1)      < sqrt(depth - 3) / 35
     *            random[0..35)     < sqrt(depth - 3)
     *            random[0..35) ^ 2 < depth - 3
     * It's important to compute one random number and square it, rather than
     * taking two rn2(35) and multiplying them, or going for a uniform
     * rn2(35*35) distribution. */
    xchar doortyp = DOOR;
    schar u_depth = depth(&u.uz);
    int r = rn2(35);
    if (u_depth > 3 && (r * r) < u_depth - 3) {
        doortyp = SDOOR;
    }
    dosdoor(x, y, aroom, doortyp);
}

/* Return TRUE if the given location contains a trap, dungeon furniture,
 * inaccessible terrain, or the vibrating square.
 * Generally used for determining if a space is unsuitable for placing
 * something.
 */
boolean
occupied(xchar x, xchar y)
{
    return (boolean) (t_at(x, y) || IS_FURNITURE(levl[x][y].typ)
                      || !ACCESSIBLE(levl[x][y].typ) /* covers lava and water */
                      || invocation_pos(x, y));
}

/* Create a trap.
 * If num is a valid trap index, create that specific trap.
 * If tm is non-NULL, create the trap at tm's coordinates. Otherwise, if
 * mazeflag is TRUE, choose a random maze position; if FALSE, assume that croom
 * is non-NULL and pick a random location inside croom.
 *
 * If num is invalid as a trap index, it will create a random trap. In
 * Gehennom, there is a 20% chance it will just pick fire trap. If various
 * factors mean that the trap is unsuitable (usually because of difficulty), it
 * will keep trying until it picks something valid.
 *
 * If a fallthru trap is created on a undiggable-floor level, it defaults to
 * PIT. If a WEB is created, a giant spider is created on top of it, unless the
 * MKTRAP_NOSPIDERONWEB flag is on in flags.
 * Finally, if it is very early in the dungeon, and the trap is potentially
 * lethal, create a minimal fake bones pile on the trap.
 */
void
mktrap(
    int num,
    int mktrapflags,
    struct mkroom *croom,
    coord *tm)
{
    register int kind;
    struct trap *t;
    unsigned lvl = level_difficulty();
    coord m;

    /* no traps in pools */
    if (tm && is_pool(tm->x, tm->y))
        return;

    if (num > NO_TRAP && num < TRAPNUM) {
        kind = num;
    } else if (Inhell && !rn2(5)) {
        /* bias the frequency of fire traps in Gehennom */
        kind = FIRE_TRAP;
    } else {
        do {
            kind = rnd(TRAPNUM - 1);
            /* reject "too hard" traps */
            switch (kind) {
            /* these are controlled by the feature or object they guard,
               not by the map so mustn't be created on it */
            case TRAPPED_DOOR:
            case TRAPPED_CHEST:
                kind = NO_TRAP;
                break;
            /* these can have a random location but can't be generated
               randomly */
            case MAGIC_PORTAL:
            case VIBRATING_SQUARE:
                kind = NO_TRAP;
                break;
            case ROLLING_BOULDER_TRAP:
            case SLP_GAS_TRAP:
                if (lvl < 2)
                    kind = NO_TRAP;
                break;
            case LEVEL_TELEP:
                if (lvl < 5 || g.level.flags.noteleport
                    || single_level_branch(&u.uz))
                    kind = NO_TRAP;
                break;
            case SPIKED_PIT:
                if (lvl < 5)
                    kind = NO_TRAP;
                break;
            case ANTI_MAGIC:
            case LANDMINE:
                if (lvl < 6)
                    kind = NO_TRAP;
                break;
            case WEB:
                if (lvl < 7 && !(mktrapflags & MKTRAP_NOSPIDERONWEB))
                    kind = NO_TRAP;
                break;
            case STATUE_TRAP:
            case POLY_TRAP:
                if (lvl < 8)
                    kind = NO_TRAP;
                break;
            case FIRE_TRAP:
                if (!Inhell)
                    kind = NO_TRAP;
                break;
            case TELEP_TRAP:
                if (g.level.flags.noteleport)
                    kind = NO_TRAP;
                break;
            case HOLE:
                /* make these much less often than other traps */
                if (rn2(7))
                    kind = NO_TRAP;
                break;
            case RUST_TRAP:
            case ROCKTRAP:
                /* certain traps that rely on a ceiling to make sense */
                if (!ceiling_exists())
                    kind = NO_TRAP;
            }
        } while (kind == NO_TRAP);
    }

    if (is_hole(kind) && !Can_fall_thru(&u.uz))
        kind = PIT;

    if (tm) {
        m = *tm;
    } else {
        register int tryct = 0;
        boolean avoid_boulder = (is_pit(kind) || is_hole(kind));

        /* Try up to 200 times to find a random coordinate for the trap. */
        do {
            if (++tryct > 200)
                return;
            if (mktrapflags & MKTRAP_MAZEFLAG)
                mazexy(&m);
            else if (!somexy(croom, &m))
                return;
        } while (occupied(m.x, m.y)
                 || (avoid_boulder && sobj_at(BOULDER, m.x, m.y)));
    }

    t = maketrap(m.x, m.y, kind);
    /* we should always get type of trap we're asking for (occupied() test
       should prevent cases where that might not happen) but be paranoid */
    kind = t ? t->ttyp : NO_TRAP;

    if (kind == WEB && !(mktrapflags & MKTRAP_NOSPIDERONWEB))
        (void) makemon(&mons[PM_GIANT_SPIDER], m.x, m.y, NO_MM_FLAGS);
    if (t && (mktrapflags & MKTRAP_SEEN))
        t->tseen = TRUE;

    /* The hero isn't the only person who's entered the dungeon in
       search of treasure. On the very shallowest levels, there's a
       chance that a created trap will have killed something already
       (and this is guaranteed on the first level).

       This isn't meant to give any meaningful treasure (in fact, any
       items we drop here are typically cursed, other than ammo fired
       by the trap). Rather, it's mostly just for flavour and to give
       players on very early levels a sufficient chance to avoid traps
       that may end up killing them before they have a fair chance to
       build max HP. Including cursed items gives the same fair chance
       to the starting pet, and fits the rule that possessions of the
       dead are normally cursed.

       Some types of traps are excluded because they're entirely
       nonlethal, even indirectly. We also exclude all of the
       later/fancier traps because they tend to have special
       considerations (e.g. webs, portals), often are indirectly
       lethal, and tend not to generate on shallower levels anyway.
       Finally, pits are excluded because it's weird to see an item
       in a pit and yet not be able to identify that the pit is there. */
    if (kind != NO_TRAP && lvl <= (unsigned) rnd(4)
        && kind != SQKY_BOARD && kind != RUST_TRAP
        /* rolling boulder trap might not have a boulder if there was no
           viable path (such as when placed in the corner of a room), in
           which case tx,ty==launch.x,y; no boulder => no dead predecessor */
        && !(kind == ROLLING_BOULDER_TRAP
             && t->launch.x == t->tx && t->launch.y == t->ty)
        && !is_pit(kind) && kind < HOLE) {
        /* Object generated by the trap; initially NULL, stays NULL if
           we fail to generate an object or if the trap doesn't
           generate objects. */
        struct obj *otmp = NULL;
        int victim_mnum; /* race of the victim */
        int quan = rnd(4); /* amount of ammo to dump */

        /* Not all trap types have special handling here; only the ones
           that kill in a specific way that's obvious after the fact. */
        switch (kind) {
        case ARROW_TRAP:
        case DART_TRAP:
        case ROCKTRAP:
            /* Make sure there is some ammo left in the trap after splitting
             * some out of it. */
            if (t->ammo) {
                if (t->ammo->quan <= quan) {
                    t->ammo->quan = quan + 1;
                }
                otmp = splitobj(t->ammo, quan); /* this handles weights */
                if (otmp) {
                    extract_nobj(otmp, &t->ammo);
                    place_object(otmp, m.x, m.y);
                }
            }
            else {
                impossible("fresh trap %d without ammo?", t->ttyp);
            }
        default:
            /* no item dropped by the trap */
            break;
        }

        /* now otmp is reused for other items we're placing */

        /* Place a random possession. This could be a weapon, tool,
           food, or gem, i.e. the item classes that are typically
           nonmagical and not worthless. */
        do {
            int poss_class = RANDOM_CLASS; /* init => lint suppression */

            switch (rn2(4)) {
            case 0:
                poss_class = WEAPON_CLASS;
                break;
            case 1:
                poss_class = TOOL_CLASS;
                break;
            case 2:
                poss_class = FOOD_CLASS;
                break;
            case 3:
                poss_class = GEM_CLASS;
                break;
            }

            otmp = mkobj(poss_class, FALSE);
            /* these items are always cursed, both for flavour (owned
               by a dead adventurer, bones-pile-style) and for balance
               (less useful to use, and encourage pets to avoid the trap) */
            if (otmp) {
                otmp->blessed = 0;
                otmp->cursed = 1;
                otmp->owt = weight(otmp);
                place_object(otmp, m.x, m.y);
            }

            /* 20% chance of placing an additional item, recursively */
        } while (!rn2(5));

        /* Place a corpse. */
        switch (rn2(15)) {
        case 0:
            /* elf corpses are the rarest as they're the most useful */
            victim_mnum = PM_ELF;
            /* elven adventurers get sleep resistance early; so don't
               generate elf corpses on sleeping gas traps unless a)
               we're on dlvl 2 (1 is impossible) and b) we pass a coin
               flip */
            if (kind == SLP_GAS_TRAP && !(lvl <= 2 && rn2(2)))
                victim_mnum = PM_HUMAN;
            break;
        case 1: case 2:
            victim_mnum = PM_DWARF;
            break;
        case 3: case 4: case 5:
            victim_mnum = PM_ORC;
            break;
        case 6: case 7: case 8: case 9:
            /* more common as they could have come from the Mines */
            victim_mnum = PM_GNOME;
            /* 10% chance of a candle too */
            if (!rn2(10)) {
                otmp = mksobj(rn2(4) ? TALLOW_CANDLE : WAX_CANDLE,
                              TRUE, FALSE);
                otmp->quan = 1;
                otmp->blessed = 0;
                otmp->cursed = 1;
                otmp->owt = weight(otmp);
                place_object(otmp, m.x, m.y);
            }
            break;
        default:
            /* the most common race */
            victim_mnum = PM_HUMAN;
            break;
        }
        otmp = mkcorpstat(CORPSE, NULL, &mons[victim_mnum], m.x, m.y,
                          CORPSTAT_INIT);
        if (otmp)
            otmp->age -= (TAINT_AGE + 1); /* died too long ago to eat */
    }
}

/* Create stairs up or down at x,y.
   If force is TRUE, change the terrain to ROOM first */
void
mkstairs(
    xchar x, xchar y,
    char up,       /* [why 'char' when usage is boolean?] */
    struct mkroom *croom UNUSED,
    boolean force)
{
    int ltyp;
    d_level dest;

    if (!x || !isok(x, y)) {
        impossible("mkstairs:  bogus stair attempt at <%d,%d>", x, y);
        return;
    }
    if (force)
        levl[x][y].typ = ROOM;
    ltyp = levl[x][y].typ; /* somexyspace() allows ice */
    if (ltyp != ROOM && ltyp != CORR && ltyp != ICE) {
        int glyph = back_to_glyph(x, y),
            sidx = glyph_to_cmap(glyph);

        impossible("mkstairs:  placing stairs %s on %s at <%d,%d>",
                   up ? "up" : "down", defsyms[sidx].explanation, x, y);
    }

    /*
     * We can't make a regular stair off an end of the dungeon.  This
     * attempt can happen when a special level is placed at an end and
     * has an up or down stair specified in its description file.
     */
    if (dunlev(&u.uz) == (up ? 1 : dunlevs_in_dungeon(&u.uz)))
        return;

    dest.dnum = u.uz.dnum;
    dest.dlevel = u.uz.dlevel + (up ? -1 : 1);
    stairway_add(x, y, up ? TRUE : FALSE, FALSE, &dest);

    (void) set_levltyp(x, y, STAIRS);
    levl[x][y].ladder = up ? LA_UP : LA_DOWN;
}

/* is room a good one to generate up or down stairs in? */
static boolean
generate_stairs_room_good(struct mkroom *croom, int phase)
{
    /*
     * phase values, smaller allows for more relaxed criteria:
     *  2 == no relaxed criteria;
     *  1 == allow a themed room;
     *  0 == allow same room as existing up/downstairs;
     * -1 == allow an unjoined room.
     */
    return (croom && (croom->needjoining || (phase < 0))
            && ((!has_dnstairs(croom) && !has_upstairs(croom))
                || phase < 1)
            && (croom->rtype == OROOM
                || ((phase < 2) && croom->rtype == THEMEROOM)));
}

/* find a good room to generate an up or down stairs in */
static struct mkroom *
generate_stairs_find_room(void)
{
    struct mkroom *croom;
    int i, phase, tryct = 0;

    if (!g.nroom)
        return (struct mkroom *) 0;

    for (phase = 2; phase > -1; phase--) {
        do {
            croom = &g.rooms[rn2(g.nroom)];
            if (generate_stairs_room_good(croom, phase))
                return croom;
        } while (tryct++ < 50);
    }

    for (phase = 2; phase > -2; phase--) {
        for (i = 0; i < g.nroom; i++) {
            croom = &g.rooms[i];
            if (generate_stairs_room_good(croom, phase))
                return croom;
        }
    }

    croom = &g.rooms[rn2(g.nroom)];
    return croom;
}

/* construct stairs up and down within the same branch,
   up and down in different rooms if possible */
static void
generate_stairs(void)
{
    struct mkroom *croom = generate_stairs_find_room();
    coord pos;

    if (!Is_botlevel(&u.uz)) {
        if (!somexyspace(croom, &pos)) {
            pos.x = somex(croom);
            pos.y = somey(croom);
        }
        mkstairs(pos.x, pos.y, 0, croom, FALSE); /* down */
    }

    if (g.nroom > 1)
        croom = generate_stairs_find_room();

    /* now do the upstairs */
    if (u.uz.dlevel != 1) {
        if (!somexyspace(croom, &pos)) {
            pos.x = somex(croom);
            pos.y = somey(croom);
        }
        mkstairs(pos.x, pos.y, 1, croom, FALSE); /* up */
    }
}

/* Return number of monsters created. */
int
mkmonst_in_room(struct mkroom *croom)
{
    int num_monst = 1;
    struct monst *tmonst; /* always put a web with a spider */
    coord pos;
    if (!somexyspace(croom, &pos)) {
        return 0; /* can't place any monsters */
    }
    tmonst = makemon((struct permonst *) 0, pos.x, pos.y, MM_NOGRP);
    if (tmonst && tmonst->data == &mons[PM_GIANT_SPIDER]
        && !occupied(pos.x, pos.y)) {
        (void) maketrap(pos.x, pos.y, WEB);
    }
    /* maybe place another monster in the same room */
    if(!rn2(3)) {
        num_monst += mkmonst_in_room(croom);
    }
    return num_monst;
}

void
mkfount(int mazeflag, struct mkroom *croom)
{
    coord m;
    register int tryct = 0;

    do {
        if (++tryct > 200)
            return;
        if (mazeflag)
            mazexy(&m);
        else if (!somexy(croom, &m))
            return;
    } while (occupied(m.x, m.y) || bydoor(m.x, m.y));

    /* Put a fountain at m.x, m.y */
    if (!set_levltyp(m.x, m.y, FOUNTAIN))
        return;
    /* Is it a "blessed" fountain? (affects drinking from fountain) */
    if (!rn2(7))
        levl[m.x][m.y].blessedftn = 1;

    g.level.flags.nfountains++;
}

static boolean
find_okay_roompos(struct mkroom *croom, coord *crd)
{
    int tryct = 0;

    do {
        if (++tryct > 200)
            return FALSE;
        if (!somexy(croom, crd))
            return FALSE;
    } while (occupied(crd->x, crd->y) || bydoor(crd->x, crd->y)
             || levl[crd->x][crd->y].typ != ROOM);
    return TRUE;
}

/* Add a ring under a sink. */
void
bury_sink_ring(xchar x, xchar y)
{
    if (levl[x][y].typ != SINK) {
        impossible("burying ring under non-sink %d at (%d,%d)?",
                   levl[x][y].typ, x, y);
    }
    struct obj* ring = mkobj(RING_CLASS, TRUE);
    ring->ox = x;
    ring->oy = y;
    add_to_buried(ring);
}

void
mksink(struct mkroom *croom)
{
    coord m;

    if (!find_okay_roompos(croom, &m))
        return;

    /* Put a sink at m.x, m.y */
    if (!set_levltyp(m.x, m.y, SINK))
        return;

    /* All sinks have a ring stuck in the pipes below */
    bury_sink_ring(m.x, m.y);

    g.level.flags.nsinks++;
}

void
mkaltar(struct mkroom *croom)
{
    coord m;
    aligntyp al;

    if (croom->rtype != OROOM)
        return;

    if (!find_okay_roompos(croom, &m))
        return;

    /* Put an altar at m.x, m.y */
    if (!set_levltyp(m.x, m.y, ALTAR))
        return;

    /* -1 - A_CHAOTIC, 0 - A_NEUTRAL, 1 - A_LAWFUL */
    al = rn2((int) A_LAWFUL + 2) - 1;
    levl[m.x][m.y].altarmask = Align2amask(al);
}

void
mkgrave(struct mkroom *croom)
{
    coord m;
    register int tryct = 0;
    register struct obj *otmp;
    const char * inscription = (const char *) 0;

    if (croom->rtype != OROOM)
        return;

    if (!find_okay_roompos(croom, &m))
        return;

    /* Put a grave at <m.x,m.y> */
    if (!rn2(10)) {
        /* Leave a bell, in case we accidentally buried someone alive */
        inscription = "Saved by the bell!";
        mksobj_at(BELL, m.x, m.y, TRUE, FALSE);
    }
    else if (!rn2(11)) {
        inscription = "Apres moi, le deluge.";
        mksobj_at(SCR_WATER, m.x, m.y, TRUE, FALSE);
    }

    /* Put a grave at <m.x,m.y> */
    make_grave(m.x, m.y, inscription);

    /* Possibly fill it with objects */
    if (!rn2(3)) {
        /* this used to use mkgold(), which puts a stack of gold on
           the ground (or merges it with an existing one there if
           present), and didn't bother burying it; now we create a
           loose, easily buriable, stack but we make no attempt to
           replicate mkgold()'s level-based formula for the amount */
        struct obj *gold = mksobj(GOLD_PIECE, TRUE, FALSE);

        gold->quan = (long) (rnd(20) + level_difficulty() * rnd(5));
        gold->owt = weight(gold);
        gold->ox = m.x, gold->oy = m.y;
        add_to_buried(gold);
    }

    for (tryct = rn2(5); tryct; tryct--) {
        otmp = mkobj(RANDOM_CLASS, TRUE);
        if (!otmp)
            break;
        curse(otmp);
        otmp->ox = m.x;
        otmp->oy = m.y;
        add_to_buried(otmp);
    }
}

void
mktree(struct mkroom *croom)
{
    coord m;

    if (croom->rtype != OROOM)
        return;

    if (!find_okay_roompos(croom, &m))
        return;

    set_levltyp(m.x, m.y, TREE);
}

/* maze levels have slightly different constraints from normal levels */
#define x_maze_min 2
#define y_maze_min 2

/*
 * Major level transmutation:  add a set of stairs (to the Sanctum) after
 * an earthquake that leaves behind a new topology, centered at inv_pos.
 * Assumes there are no rooms within the invocation area and that g.inv_pos
 * is not too close to the edge of the map.  Also assume the hero can see,
 * which is guaranteed for normal play due to the fact that sight is needed
 * to read the Book of the Dead.  [That assumption is not valid; it is
 * possible that "the Book reads the hero" rather than vice versa if
 * attempted while blind (in order to make blind-from-birth conduct viable).]
 */
void
mkinvokearea(void)
{
    int dist;
    xchar xmin = g.inv_pos.x, xmax = g.inv_pos.x,
          ymin = g.inv_pos.y, ymax = g.inv_pos.y;
    register xchar i;

    /* slightly odd if levitating, but not wrong */
    pline_The("floor shakes violently under you!");
    /*
     * TODO:
     *  Suppress this message if player has dug out all the walls
     *  that would otherwise be affected.
     */
    pline_The("walls around you begin to bend and crumble!");
    display_nhwindow(WIN_MESSAGE, TRUE);

    /* any trap hero is stuck in will be going away now */
    if (u.utrap) {
        if (u.utraptype == TT_BURIEDBALL)
            buried_ball_to_punishment();
        reset_utrap(FALSE);
    }
    mkinvpos(xmin, ymin, 0); /* middle, before placing stairs */

    for (dist = 1; dist < 7; dist++) {
        xmin--;
        xmax++;

        /* top and bottom */
        if (dist != 3) { /* the area is wider that it is high */
            ymin--;
            ymax++;
            for (i = xmin + 1; i < xmax; i++) {
                mkinvpos(i, ymin, dist);
                mkinvpos(i, ymax, dist);
            }
        }

        /* left and right */
        for (i = ymin; i <= ymax; i++) {
            mkinvpos(xmin, i, dist);
            mkinvpos(xmax, i, dist);
        }

        flush_screen(1); /* make sure the new glyphs shows up */
        delay_output();
    }

    You("are standing at the top of a stairwell leading down!");
    mkstairs(u.ux, u.uy, 0, (struct mkroom *) 0, FALSE); /* down */
    newsym(u.ux, u.uy);
    g.vision_full_recalc = 1; /* everything changed */
}

/* Change level topology.  Boulders in the vicinity are eliminated.
 * Temporarily overrides vision in the name of a nice effect.
 */
static void
mkinvpos(xchar x, xchar y, int dist)
{
    struct trap *ttmp;
    struct obj *otmp;
    boolean make_rocks;
    register struct rm *lev = &levl[x][y];
    struct monst *mon;

    /* clip at existing map borders if necessary */
    if (!within_bounded_area(x, y, x_maze_min + 1, y_maze_min + 1,
                             g.x_maze_max - 1, g.y_maze_max - 1)) {
        /* outermost 2 columns and/or rows may be truncated due to edge */
        if (dist < (7 - 2))
            panic("mkinvpos: <%d,%d> (%d) off map edge!", x, y, dist);
        return;
    }

    /* clear traps */
    if ((ttmp = t_at(x, y)) != 0) {
        deltrap_with_ammo(ttmp, DELTRAP_DESTROY_AMMO);
    }

    /* clear boulders; leave some rocks for non-{moat|trap} locations */
    make_rocks = (dist != 1 && dist != 4 && dist != 5) ? TRUE : FALSE;
    while ((otmp = sobj_at(BOULDER, x, y)) != 0) {
        if (make_rocks) {
            fracture_rock(otmp);
            make_rocks = FALSE; /* don't bother with more rocks */
        } else {
            obj_extract_self(otmp);
            obfree(otmp, (struct obj *) 0);
        }
    }

    /* fake out saved state */
    lev->seenv = 0;
    set_doorstate(lev, D_NODOOR);
    if (dist < 6)
        lev->lit = TRUE;
    lev->waslit = TRUE;
    lev->horizontal = FALSE;
    /* short-circuit vision recalc */
    g.viz_array[y][x] = (dist < 6) ? (IN_SIGHT | COULD_SEE) : COULD_SEE;

    switch (dist) {
    case 1: /* fire traps */
        if (is_pool(x, y))
            break;
        lev->typ = ROOM;
        ttmp = maketrap(x, y, FIRE_TRAP);
        if (ttmp)
            ttmp->tseen = TRUE;
        break;
    case 0: /* lit room locations */
    case 2:
    case 3:
    case 6: /* unlit room locations */
        lev->typ = ROOM;
        break;
    case 4: /* pools (aka a wide moat) */
    case 5:
        lev->typ = MOAT;
        /* No kelp! */
        break;
    default:
        impossible("mkinvpos called with dist %d", dist);
        break;
    }

    if ((mon = m_at(x, y)) != 0) {
        /* wake up mimics, don't want to deal with them blocking vision */
        if (mon->m_ap_type)
            seemimic(mon);

        if ((ttmp = t_at(x, y)) != 0)
            (void) mintrap(mon, NO_TRAP_FLAGS);
        else
            (void) minliquid(mon);
    }

    if (!does_block(x, y, lev))
        unblock_point(x, y); /* make sure vision knows this location is open */

    /* display new value of position; could have a monster/object on it */
    newsym(x, y);
}

/*
 * The portal to Ludios is special.  The entrance can only occur within a
 * vault in the main dungeon at a depth greater than 10.  The Ludios branch
 * structure reflects this by having a bogus "source" dungeon:  the value
 * of n_dgns (thus, Is_branchlev() will never find it).
 *
 * Ludios will remain isolated until the branch is corrected by this function.
 */
static void
mk_knox_portal(xchar x, xchar y)
{
    d_level *source;
    branch *br;
    schar u_depth;

    br = dungeon_branch("Fort Ludios");
    if (on_level(&knox_level, &br->end1)) {
        source = &br->end2;
    } else {
        /* disallow Knox branch on a level with one branch already */
        if (Is_branchlev(&u.uz))
            return;
        source = &br->end1;
    }

    /* Already set. */
    if (source->dnum < g.n_dgns)
        return;

    if (!(u.uz.dnum == oracle_level.dnum      /* in main dungeon */
          && !at_dgn_entrance("The Quest")    /* but not Quest's entry */
          && (u_depth = depth(&u.uz)) > 10    /* beneath 10 */
          && u_depth < depth(&medusa_level))) /* and above Medusa */
        return;

    /* Adjust source to be current level and re-insert branch. */
    *source = u.uz;
    insert_branch(br, TRUE);

    debugpline0("Made knox portal.");
    place_branch(br, x, y);
}

/*mklev.c*/
