/* NetHack 3.6	trap.c	$NHDT-Date: 1593768051 2020/07/03 09:20:51 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.361 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2013. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

extern const char *const destroy_strings[][3]; /* from zap.c */

static boolean FDECL(keep_saddle_with_steedcorpse, (unsigned, struct obj *,
                                                    struct obj *));
static boolean FDECL(mu_maybe_destroy_web, (struct monst *, BOOLEAN_P,
                                            struct trap *));
static char *FDECL(trapnote, (struct trap *, BOOLEAN_P));
static int FDECL(steedintrap, (struct trap *, struct obj *));
static void FDECL(launch_drop_spot, (struct obj *, XCHAR_P, XCHAR_P));
static int FDECL(mkroll_launch, (struct trap *, XCHAR_P, XCHAR_P,
                                 SHORT_P, long));
static boolean FDECL(isclearpath, (coord *, int, SCHAR_P, SCHAR_P));
static void FDECL(dofiretrap, (struct obj *));
static void NDECL(domagictrap);
static boolean FDECL(emergency_disrobe, (boolean *));
static int FDECL(untrap_prob, (struct trap *));
static void FDECL(move_into_trap, (struct trap *));
static int FDECL(try_disarm, (struct trap *, BOOLEAN_P));
static void FDECL(reward_untrap, (struct trap *, struct monst *));
static int FDECL(disarm_holdingtrap, (struct trap *));
static int FDECL(disarm_landmine, (struct trap *));
static int FDECL(disarm_squeaky_board, (struct trap *));
static int FDECL(disarm_shooting_trap, (struct trap *));
static void FDECL(clear_conjoined_pits, (struct trap *));
static boolean FDECL(adj_nonconjoined_pit, (struct trap *));
static int FDECL(try_lift, (struct monst *, struct trap *, int,
                            BOOLEAN_P));
static int FDECL(help_monster_out, (struct monst *, struct trap *));
#if 0
static void FDECL(join_adjacent_pits, (struct trap *));
#endif
static boolean FDECL(thitm, (int, struct monst *, struct obj *, int,
                             BOOLEAN_P));
static void NDECL(maybe_finish_sokoban);

static const char *const a_your[2] = { "a", "your" };
static const char *const A_Your[2] = { "A", "Your" };
static const char tower_of_flame[] = "tower of flame";
static const char *const A_gush_of_water_hits = "A gush of water hits";
static const char *const blindgas[6] = { "humid",   "odorless",
                                         "pungent", "chilling",
                                         "acrid",   "biting" };

/* called when you're hit by fire (dofiretrap,buzz,zapyourself,explode);
   returns TRUE if hit on torso */
boolean
burnarmor(victim)
struct monst *victim;
{
    struct obj *item;
    char buf[BUFSZ];
    int mat_idx, oldspe;
    boolean hitting_u;

    if (!victim)
        return 0;
    hitting_u = (victim == &g.youmonst);

    /* burning damage may dry wet towel */
    item = hitting_u ? carrying(TOWEL) : m_carrying(victim, TOWEL);
    while (item) {
        if (is_wet_towel(item)) {
            oldspe = item->spe;
            dry_a_towel(item, rn2(oldspe + 1), TRUE);
            if (item->spe != oldspe)
                break; /* stop once one towel has been affected */
        }
        item = item->nobj;
    }

#define burn_dmg(obj, descr) erode_obj(obj, descr, ERODE_BURN, EF_GREASE)
    while (1) {
        switch (rn2(5)) {
        case 0:
            item = hitting_u ? uarmh : which_armor(victim, W_ARMH);
            if (item) {
                mat_idx = item->material;
                Sprintf(buf, "%s %s", materialnm[mat_idx],
                        helm_simple_name(item));
            }
            if (!burn_dmg(item, item ? buf : "helmet"))
                continue;
            break;
        case 1:
            item = hitting_u ? uarmc : which_armor(victim, W_ARMC);
            if (item) {
                (void) burn_dmg(item, cloak_simple_name(item));
                return TRUE;
            }
            item = hitting_u ? uarm : which_armor(victim, W_ARM);
            if (item) {
                (void) burn_dmg(item, xname(item));
                return TRUE;
            }
            item = hitting_u ? uarmu : which_armor(victim, W_ARMU);
            if (item)
                (void) burn_dmg(item, "shirt");
            return TRUE;
        case 2:
            item = hitting_u ? uarms : which_armor(victim, W_ARMS);
            if (!burn_dmg(item, "shield"))
                continue;
            break;
        case 3:
            item = hitting_u ? uarmg : which_armor(victim, W_ARMG);
            if (!burn_dmg(item, "gloves"))
                continue;
            break;
        case 4:
            item = hitting_u ? uarmf : which_armor(victim, W_ARMF);
            if (!burn_dmg(item, "boots"))
                continue;
            break;
        }
        break; /* Out of while loop */
    }
#undef burn_dmg

    return FALSE;
}

/* Generic erode-item function.
 * "ostr", if non-null, is an alternate string to print instead of the
 *   object's name.
 * "type" is an ERODE_* value for the erosion type
 * "flags" is an or-ed list of EF_* flags
 *
 * Returns an erosion return value (ER_*)
 */
int
erode_obj(otmp, ostr, type, ef_flags)
register struct obj *otmp;
const char *ostr;
int type;
int ef_flags;
{
    static NEARDATA const char
        *const action[] = { "smoulder", "rust", "rot", "corrode" },
        *const msg[] = { "burnt", "rusted", "rotten", "corroded" },
        *const bythe[] = { "heat", "oxidation", "decay", "corrosion" };
    boolean vulnerable = FALSE, is_primary = TRUE,
            check_grease = (ef_flags & EF_GREASE) ? TRUE : FALSE,
            print = (ef_flags & EF_VERBOSE) ? TRUE : FALSE,
            uvictim, vismon, visobj;
    int erosion, cost_type;
    struct monst *victim;

    if (!otmp)
        return ER_NOTHING;

    victim = carried(otmp) ? &g.youmonst
             : mcarried(otmp) ? otmp->ocarry
               : (struct monst *) 0;
    uvictim = (victim == &g.youmonst);
    vismon = victim && (victim != &g.youmonst) && canseemon(victim);
    /* Is g.bhitpos correct here? Ugh. */
    visobj = !victim && cansee(g.bhitpos.x, g.bhitpos.y);

    switch (type) {
    case ERODE_BURN:
        vulnerable = is_flammable(otmp);
        check_grease = FALSE;
        cost_type = COST_BURN;
        break;
    case ERODE_RUST:
        vulnerable = is_rustprone(otmp);
        cost_type = COST_RUST;
        break;
    case ERODE_ROT:
        vulnerable = is_rottable(otmp);
        check_grease = FALSE;
        is_primary = FALSE;
        cost_type = COST_ROT;
        break;
    case ERODE_CORRODE:
        vulnerable = is_corrodeable(otmp);
        is_primary = FALSE;
        cost_type = COST_CORRODE;
        break;
    default:
        impossible("Invalid erosion type in erode_obj");
        return ER_NOTHING;
    }
    erosion = is_primary ? otmp->oeroded : otmp->oeroded2;

    if (!ostr)
        ostr = cxname(otmp);
    /* 'visobj' messages insert "the"; probably ought to switch to the() */
    if (visobj && !(uvictim || vismon) && !strncmpi(ostr, "the ", 4))
        ostr += 4;

    if (check_grease && otmp->greased) {
        grease_protect(otmp, ostr, victim);
        return ER_GREASED;
    } else if (!erosion_matters(otmp)) {
        return ER_NOTHING;
    } else if (!vulnerable || (otmp->oerodeproof && otmp->rknown)) {
        if (flags.verbose && print && (uvictim || vismon))
            pline("%s %s %s not affected by %s.",
                  uvictim ? "Your" : s_suffix(Monnam(victim)),
                  ostr, vtense(ostr, "are"), bythe[type]);
        return ER_NOTHING;
    } else if (otmp->oerodeproof || (otmp->blessed && !rnl(4))) {
        if (flags.verbose && (print || otmp->oerodeproof)
            && (uvictim || vismon || visobj))
            pline("Somehow, %s %s %s not affected by the %s.",
                  uvictim ? "your"
                          : !vismon ? "the" /* visobj */
                                    : s_suffix(mon_nam(victim)),
                  ostr, vtense(ostr, "are"), bythe[type]);
        /* We assume here that if the object is protected because it
         * is blessed, it still shows some minor signs of wear, and
         * the hero can distinguish this from an object that is
         * actually proof against damage.
         */
        if (otmp->oerodeproof) {
            otmp->rknown = TRUE;
            if (victim == &g.youmonst)
                update_inventory();
        }

        return ER_NOTHING;
    } else if (erosion < MAX_ERODE) {
        const char *adverb = (erosion + 1 == MAX_ERODE) ? " completely"
                             : erosion ? " further"
                               : "";

        if (uvictim || vismon || visobj)
            pline("%s %s %s%s!",
                  uvictim ? "Your"
                          : !vismon ? "The" /* visobj */
                                    : s_suffix(Monnam(victim)),
                  ostr, vtense(ostr, action[type]), adverb);

        if (ef_flags & EF_PAY)
            costly_alteration(otmp, cost_type);

        if (is_primary)
            otmp->oeroded++;
        else
            otmp->oeroded2++;

        if (victim == &g.youmonst)
            update_inventory();

        return ER_DAMAGED;
    } else if (ef_flags & EF_DESTROY) {
        if (uvictim || vismon || visobj)
            pline("%s %s %s away!",
                  uvictim ? "Your"
                          : !vismon ? "The" /* visobj */
                                    : s_suffix(Monnam(victim)),
                  ostr, vtense(ostr, action[type]));

        if (ef_flags & EF_PAY)
            costly_alteration(otmp, cost_type);

        setnotworn(otmp);
        delobj(otmp);
        return ER_DESTROYED;
    } else {
        if (flags.verbose && print) {
            if (uvictim)
                Your("%s %s completely %s.",
                     ostr, vtense(ostr, Blind ? "feel" : "look"), msg[type]);
            else if (vismon || visobj)
                pline("%s %s %s completely %s.",
                      !vismon ? "The" : s_suffix(Monnam(victim)),
                      ostr, vtense(ostr, "look"), msg[type]);
        }
        return ER_NOTHING;
    }
}

/* Protect an item from erosion with grease. Returns TRUE if the grease
 * wears off.
 */
boolean
grease_protect(otmp, ostr, victim)
register struct obj *otmp;
const char *ostr;
struct monst *victim;
{
    static const char txt[] = "protected by the layer of grease!";
    boolean vismon = victim && (victim != &g.youmonst) && canseemon(victim);

    if (ostr) {
        if (victim == &g.youmonst)
            Your("%s %s %s", ostr, vtense(ostr, "are"), txt);
        else if (vismon)
            pline("%s's %s %s %s", Monnam(victim),
                  ostr, vtense(ostr, "are"), txt);
    } else if (victim == &g.youmonst || vismon) {
        pline("%s %s", Yobjnam2(otmp, "are"), txt);
    }
    if (!rn2(2)) {
        otmp->greased = 0;
        if (carried(otmp)) {
            pline_The("grease dissolves.");
            update_inventory();
        }
        return TRUE;
    }
    return FALSE;
}

struct trap *
maketrap(x, y, typ)
int x, y, typ;
{
    static union vlaunchinfo zero_vl;
    boolean oldplace;
    struct trap *ttmp;
    struct rm *lev = &levl[x][y];
    struct obj *otmp;

    if ((ttmp = t_at(x, y)) != 0) {
        if (ttmp->ttyp == MAGIC_PORTAL || ttmp->ttyp == VIBRATING_SQUARE)
            return (struct trap *) 0;
        oldplace = TRUE;
        if (u.utrap && x == u.ux && y == u.uy
            && ((u.utraptype == TT_BEARTRAP && typ != BEAR_TRAP)
                || (u.utraptype == TT_WEB && typ != WEB)
                || (u.utraptype == TT_PIT && !is_pit(typ))
                || (u.utraptype == TT_LAVA && !is_lava(x, y))))
            reset_utrap(FALSE);
        /* old <tx,ty> remain valid */
    } else if (IS_FURNITURE(lev->typ)
               && (!IS_GRAVE(lev->typ) || (typ != PIT && typ != HOLE))) {
        /* no trap on top of furniture (caller usually screens the
           location to inhibit this, but wizard mode wishing doesn't) */
        return (struct trap *) 0;
    } else {
        oldplace = FALSE;
        ttmp = newtrap();
        (void) memset((genericptr_t)ttmp, 0, sizeof(struct trap));
        ttmp->ntrap = 0;
        ttmp->tx = x;
        ttmp->ty = y;
    }
    /* [re-]initialize all fields except ntrap (handled below) and <tx,ty> */
    ttmp->vl = zero_vl;
    ttmp->launch.x = ttmp->launch.y = -1; /* force error if used before set */
    ttmp->dst.dnum = ttmp->dst.dlevel = -1;
    ttmp->madeby_u = 0;
    ttmp->once = 0;
    ttmp->tseen = (typ == HOLE); /* hide non-holes */
    ttmp->ttyp = typ;
    set_trap_ammo(ttmp, NULL);

    switch (typ) {
    case SQKY_BOARD: {
        int tavail[12], tpick[12], tcnt = 0, k;
        struct trap *t;

        for (k = 0; k < 12; ++k)
            tavail[k] = tpick[k] = 0;
        for (t = g.ftrap; t; t = t->ntrap)
            if (t->ttyp == SQKY_BOARD && t != ttmp)
                tavail[t->tnote] = 1;
        /* now populate tpick[] with the available indices */
        for (k = 0; k < 12; ++k)
            if (tavail[k] == 0)
                tpick[tcnt++] = k;
        /* choose an unused note; if all are in use, pick a random one */
        ttmp->tnote = (short) ((tcnt > 0) ? tpick[rn2(tcnt)] : rn2(12));
        break;
    }
    case STATUE_TRAP: { /* create a "living" statue */
        struct monst *mtmp;
        struct obj *otmp, *statue;
        struct permonst *mptr;
        int trycount = 10;

        do { /* avoid ultimately hostile co-aligned unicorn */
            mptr = &mons[rndmonnum()];
        } while (--trycount > 0 && is_unicorn(mptr)
                 && sgn(u.ualign.type) == sgn(mptr->maligntyp));
        statue = mkcorpstat(STATUE, (struct monst *) 0, mptr, x, y,
                            CORPSTAT_NONE);
        /* we would like statues to use random materials, but not allow them to
         * get the random spellbook */
        init_obj_material(statue);
        mtmp = makemon(&mons[statue->corpsenm], 0, 0, MM_NOCOUNTBIRTH);
        if (!mtmp)
            break; /* should never happen */
        while (mtmp->minvent) {
            otmp = mtmp->minvent;
            otmp->owornmask = 0;
            obj_extract_self(otmp);
            (void) add_to_container(statue, otmp);
        }
        statue->owt = weight(statue);
        mongone(mtmp);
        break;
    }
    case ROLLING_BOULDER_TRAP: /* boulder will roll towards trigger */
        (void) mkroll_launch(ttmp, x, y, BOULDER, 1L);
        break;
    case PIT:
    case SPIKED_PIT:
        ttmp->conjoined = 0;
        /*FALLTHRU*/
    case HOLE:
    case TRAPDOOR:
        if (*in_rooms(x, y, SHOPBASE)
            && (is_hole(typ) || IS_DOOR(lev->typ) || IS_WALL(lev->typ)))
            add_damage(x, y, /* schedule repair */
                       ((IS_DOOR(lev->typ) || IS_WALL(lev->typ))
                        && !g.context.mon_moving)
                           ? SHOP_HOLE_COST
                           : 0L);
        lev->flags = 0;        /* subsumes altarmask, icedpool... */
        if (IS_ROOM(lev->typ)) /* && !IS_AIR(lev->typ) */
            lev->typ = ROOM;
        /*
         * some cases which can happen when digging
         * down while phazing thru solid areas
         */
        else if (lev->typ == STONE || lev->typ == SCORR)
            lev->typ = CORR;
        else if (IS_WALL(lev->typ) || lev->typ == SDOOR)
            lev->typ = g.level.flags.is_maze_lev ? ROOM
                       : g.level.flags.is_cavernous_lev ? CORR
                         : DOOR;

        unearth_objs(x, y);
        break;
    case ROCKTRAP:
        otmp = mksobj(ROCK, TRUE, FALSE);
        /* TODO: Scale this with depth. */
        otmp->quan = 5 + rnd(10);
        set_trap_ammo(ttmp, otmp);
        break;
    case DART_TRAP:
        otmp = mksobj(DART, TRUE, FALSE);
        otmp->quan = 15 + rnd(20);
        /* darts are poisoned 1/6 of the time on level 7+ */
        otmp->opoisoned = (level_difficulty() > 6 && !rn2(6));
        set_trap_ammo(ttmp, otmp);
        break;
    case ARROW_TRAP:
        otmp = mksobj(ARROW, TRUE, FALSE);
        otmp->quan = 15 + rnd(20);
        /* arrows are not poisoned */
        otmp->opoisoned = 0;
        set_trap_ammo(ttmp, otmp);
        break;
    case BEAR_TRAP:
        set_trap_ammo(ttmp, mksobj(BEARTRAP, TRUE, FALSE));
        break;
    case LANDMINE:
        set_trap_ammo(ttmp, mksobj(LAND_MINE, TRUE, FALSE));
        break;
    }

    if (!oldplace) {
        ttmp->ntrap = g.ftrap;
        g.ftrap = ttmp;
    } else {
        /* oldplace;
           it shouldn't be possible to override a sokoban pit or hole
           with some other trap, but we'll check just to be safe */
        if (Sokoban)
            maybe_finish_sokoban();
    }
    return ttmp;
}

/* Assign obj to be the ammo of trap. Deletes any ammo currently in the trap.
 * obj can be set to NULL to delete the ammo without putting in anything else.
 */
void
set_trap_ammo(trap, obj)
struct trap *trap;
struct obj *obj;
{
    if (!trap) {
        impossible("set_trap_ammo: null trap!");
        return;
    }
    while (trap->ammo) {
        struct obj* oldobj = trap->ammo;
        extract_nobj(oldobj, &trap->ammo);
        obfree(oldobj, (struct obj *) 0);
    }
    if (!obj) {
        trap->ammo = (struct obj *) 0;
        return;
    }
    if (obj->where != OBJ_FREE) {
        panic("putting non-free object into trap");
    }
    obj->where = OBJ_INTRAP;
    trap->ammo = obj;
}

void
fall_through(td, ftflags)
boolean td; /* td == TRUE : trap door or hole */
unsigned ftflags;
{
    d_level dtmp;
    char msgbuf[BUFSZ];
    const char *dont_fall = 0;
    int newlevel, bottom;
    struct trap *t = (struct trap *) 0;

    /* we'll fall even while levitating in Sokoban; otherwise, if we
       won't fall and won't be told that we aren't falling, give up now */
    if (Blind && Levitation && !Sokoban)
        return;

    bottom = dunlevs_in_dungeon(&u.uz);
    /* when in the upper half of the quest, don't fall past the
       middle "quest locate" level if hero hasn't been there yet */
    if (In_quest(&u.uz)) {
        int qlocate_depth = qlocate_level.dlevel;

        /* deepest reached < qlocate implies current < qlocate */
        if (dunlev_reached(&u.uz) < qlocate_depth)
            bottom = qlocate_depth; /* early cut-off */
    }
    newlevel = dunlev(&u.uz); /* current level */
    do {
        newlevel++;
    } while (!rn2(4) && newlevel < bottom);

    if (td) {
        t = t_at(u.ux, u.uy);
        feeltrap(t);
        if (!Sokoban && !(ftflags & TOOKPLUNGE)) {
            if (t->ttyp == TRAPDOOR)
                pline("A trap door opens up under you!");
            else
                pline("There's a gaping hole under you!");
        }
    } else
        pline_The("%s opens up under you!", surface(u.ux, u.uy));

    if (Sokoban && Can_fall_thru(&u.uz)) {
        ; /* KMH -- You can't escape the Sokoban level traps */
    } else if (Levitation || u.ustuck
             || (!Can_fall_thru(&u.uz) && !levl[u.ux][u.uy].candig)
             || ((Flying || !grounded(g.youmonst.data)
                  || (ceiling_hider(g.youmonst.data) && u.uundetected))
                 && !(ftflags & TOOKPLUNGE))
             || (Inhell && !u.uevent.invoked && newlevel == bottom)) {
        dont_fall = "don't fall in.";
    } else if (g.youmonst.data->msize >= MZ_HUGE) {
        dont_fall = "don't fit through.";
    } else if (!next_to_u()) {
        dont_fall = "are jerked back by your pet!";
    }
    if (dont_fall) {
        You1(dont_fall);
        /* hero didn't fall through, but any objects here might */
        impact_drop((struct obj *) 0, u.ux, u.uy, 0);
        if (!td) {
            display_nhwindow(WIN_MESSAGE, FALSE);
            pline_The("opening under you closes up.");
        }
        return;
    }
    if ((Flying || is_clinger(g.youmonst.data))
        && (ftflags & TOOKPLUNGE) && td && t)
        You("%s down %s!",
            Flying ? "swoop" : "deliberately drop",
            (t->ttyp == TRAPDOOR)
                ? "through the trap door"
                : "into the gaping hole");

    if (*u.ushops)
        shopdig(1);
    if (Is_stronghold(&u.uz)) {
        find_hell(&dtmp);
    } else {
        int dist = newlevel - dunlev(&u.uz);

        dtmp.dnum = u.uz.dnum;
        dtmp.dlevel = newlevel;
        if (dist > 1)
            You("fall down a %s%sshaft!", dist > 3 ? "very " : "",
                dist > 2 ? "deep " : "");
    }
    if (!td)
        Sprintf(msgbuf, "The hole in the %s above you closes up.",
                ceiling(u.ux, u.uy));

    schedule_goto(&dtmp, FALSE, TRUE, 0, (char *) 0,
                  !td ? msgbuf : (char *) 0);
}

/*
 * Animate the given statue.  May have been via shatter attempt, trap,
 * or stone to flesh spell.  Return a monster if successfully animated.
 * If the monster is animated, the object is deleted.  If fail_reason
 * is non-null, then fill in the reason for failure (or success).
 *
 * The cause of animation is:
 *
 *      ANIMATE_NORMAL  - hero "finds" the monster
 *      ANIMATE_SHATTER - hero tries to destroy the statue
 *      ANIMATE_SPELL   - stone to flesh spell hits the statue
 *
 * Perhaps x, y is not needed if we can use get_obj_location() to find
 * the statue's location... ???
 *
 * Sequencing matters:
 *      create monster; if it fails, give up with statue intact;
 *      give "statue comes to life" message;
 *      if statue belongs to shop, have shk give "you owe" message;
 *      transfer statue contents to monster (after stolen_value());
 *      delete statue.
 *      [This ordering means that if the statue ends up wearing a cloak of
 *       invisibility or a mummy wrapping, the visibility checks might be
 *       wrong, but to avoid that we'd have to clone the statue contents
 *       first in order to give them to the monster before checking their
 *       shop status--it's not worth the hassle.]
 */
struct monst *
animate_statue(statue, x, y, cause, fail_reason)
struct obj *statue;
xchar x, y;
int cause;
int *fail_reason;
{
    int mnum = statue->corpsenm;
    struct permonst *mptr = &mons[mnum];
    struct monst *mon = 0, *shkp;
    struct obj *item;
    coord cc;
    boolean historic = (Role_if(PM_ARCHEOLOGIST)
                        && (statue->spe & STATUE_HISTORIC) != 0),
            golem_xform = FALSE, use_saved_traits;
    const char *comes_to_life;
    char statuename[BUFSZ], tmpbuf[BUFSZ];
    static const char historic_statue_is_gone[] =
        "that the historic statue is now gone";

    if (cant_revive(&mnum, TRUE, statue)) {
        /* mnum has changed; we won't be animating this statue as itself */
        if (mnum != PM_DOPPELGANGER)
            mptr = &mons[mnum];
        use_saved_traits = FALSE;
    } else if (is_golem(mptr) && cause == ANIMATE_SPELL) {
        /* statue of any golem hit by stone-to-flesh becomes flesh golem */
        golem_xform = (mptr != &mons[PM_FLESH_GOLEM]);
        mnum = PM_FLESH_GOLEM;
        mptr = &mons[PM_FLESH_GOLEM];
        use_saved_traits = (has_omonst(statue) && !golem_xform);
    } else {
        use_saved_traits = has_omonst(statue);
    }

    if (use_saved_traits) {
        /* restore a petrified monster */
        cc.x = x, cc.y = y;
        mon = montraits(statue, &cc, (cause == ANIMATE_SPELL));
        if (mon && mon->mtame && !mon->isminion)
            wary_dog(mon, TRUE);
    } else {
        /* statues of unique monsters from bones or wishing end
           up here (cant_revive() sets mnum to be doppelganger;
           mptr reflects the original form for use by newcham()) */
        if ((mnum == PM_DOPPELGANGER && mptr != &mons[PM_DOPPELGANGER])
            /* block quest guards from other roles */
            || (mptr->msound == MS_GUARDIAN
                && quest_info(MS_GUARDIAN) != mnum)) {
            mon = makemon(&mons[PM_DOPPELGANGER], x, y,
                          NO_MINVENT | MM_NOCOUNTBIRTH | MM_ADJACENTOK);
            /* if hero has protection from shape changers, cham field will
               be NON_PM; otherwise, set form to match the statue */
            if (mon && mon->cham >= LOW_PM)
                (void) newcham(mon, mptr, FALSE, FALSE);
        } else
            mon = makemon(mptr, x, y, (cause == ANIMATE_SPELL)
                                          ? (NO_MINVENT | MM_ADJACENTOK)
                                          : NO_MINVENT);
    }

    if (!mon) {
        if (fail_reason)
            *fail_reason = unique_corpstat(&mons[statue->corpsenm])
                               ? AS_MON_IS_UNIQUE
                               : AS_NO_MON;
        return (struct monst *) 0;
    }

    /* a non-montraits() statue might specify gender */
    if (statue->spe & STATUE_MALE)
        mon->female = FALSE;
    else if (statue->spe & STATUE_FEMALE)
        mon->female = TRUE;
    /* if statue has been named, give same name to the monster */
    if (has_oname(statue) && !unique_corpstat(mon->data))
        mon = christen_monst(mon, ONAME(statue));
    /* mimic statue becomes seen mimic; other hiders won't be hidden */
    if (M_AP_TYPE(mon))
        seemimic(mon);
    else
        mon->mundetected = FALSE;
    mon->msleeping = 0;
    if (cause == ANIMATE_NORMAL || cause == ANIMATE_SHATTER) {
        /* trap always releases hostile monster */
        mon->mtame = 0; /* (might be petrified pet tossed onto trap) */
        mon->mpeaceful = 0;
        set_malign(mon);
    }

    comes_to_life = !canspotmon(mon) ? "disappears"
                    : golem_xform ? "turns into flesh"
                      : (nonliving(mon->data) || is_vampshifter(mon)) ? "moves"
                        : "comes to life";
    if ((x == u.ux && y == u.uy) || cause == ANIMATE_SPELL) {
        /* "the|your|Manlobbi's statue [of a wombat]" */
        shkp = shop_keeper(*in_rooms(mon->mx, mon->my, SHOPBASE));
        Sprintf(statuename, "%s%s", shk_your(tmpbuf, statue),
                (cause == ANIMATE_SPELL
                 /* avoid "of a shopkeeper" if it's Manlobbi himself
                    (if carried, it can't be unpaid--hence won't be
                    described as "Manlobbi's statue"--because there
                    wasn't any living shk when statue was picked up) */
                 && (mon != shkp || carried(statue)))
                   ? xname(statue)
                   : "statue");
        pline("%s %s!", upstart(statuename), comes_to_life);
    } else if (Hallucination) { /* They don't know it's a statue */
        pline_The("%s suddenly seems more animated.", rndmonnam((char *) 0));
    } else if (cause == ANIMATE_SHATTER) {
        if (cansee(x, y))
            Sprintf(statuename, "%s%s", shk_your(tmpbuf, statue),
                    xname(statue));
        else
            Strcpy(statuename, "a statue");
        pline("Instead of shattering, %s suddenly %s!", statuename,
              comes_to_life);
    } else { /* cause == ANIMATE_NORMAL */
        You("find %s posing as a statue.",
            canspotmon(mon) ? a_monnam(mon) : something);
        if (!canspotmon(mon) && Blind)
            map_invisible(x, y);
        stop_occupation();
    }

    /* if this isn't caused by a monster using a wand of striking,
       there might be consequences for the hero */
    if (!g.context.mon_moving) {
        /* if statue is owned by a shop, hero will have to pay for it;
           stolen_value gives a message (about debt or use of credit)
           which refers to "it" so needs to follow a message describing
           the object ("the statue comes to life" one above) */
        if (cause != ANIMATE_NORMAL && costly_spot(x, y)
            && (carried(statue) ? statue->unpaid : !statue->no_charge)
            && (shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) != 0
            /* avoid charging for Manlobbi's statue of Manlobbi
               if stone-to-flesh is used on petrified shopkeep */
            && mon != shkp)
            (void) stolen_value(statue, x, y, (boolean) shkp->mpeaceful,
                                FALSE);

        if (historic) {
            You_feel("guilty %s.", historic_statue_is_gone);
            adjalign(-1);
        }
    } else {
        if (historic && cansee(x, y))
            You_feel("regret %s.", historic_statue_is_gone);
        /* no alignment penalty */
    }

    /* transfer any statue contents to monster's inventory */
    while ((item = statue->cobj) != 0) {
        obj_extract_self(item);
        (void) mpickobj(mon, item);
    }
    m_dowear(mon, TRUE);
    /* in case statue is wielded and hero zaps stone-to-flesh at self */
    if (statue->owornmask)
        remove_worn_item(statue, TRUE);
    /* statue no longer exists */
    delobj(statue);

    /* avoid hiding under nothing */
    if (x == u.ux && y == u.uy && Upolyd && hides_under(g.youmonst.data)
        && !concealed_spot(x, y))
        u.uundetected = 0;

    if (fail_reason)
        *fail_reason = AS_OK;
    return mon;
}

/*
 * You've either stepped onto a statue trap's location or you've triggered a
 * statue trap by searching next to it or by trying to break it with a wand
 * or pick-axe.
 */
struct monst *
activate_statue_trap(trap, x, y, shatter)
struct trap *trap;
xchar x, y;
boolean shatter;
{
    struct monst *mtmp = (struct monst *) 0;
    struct obj *otmp = sobj_at(STATUE, x, y);
    int fail_reason;

    /*
     * Try to animate the first valid statue.  Stop the loop when we
     * actually create something or the failure cause is not because
     * the mon was unique.
     */
    deltrap(trap);
    while (otmp) {
        mtmp = animate_statue(otmp, x, y,
                              shatter ? ANIMATE_SHATTER : ANIMATE_NORMAL,
                              &fail_reason);
        if (mtmp || fail_reason != AS_MON_IS_UNIQUE)
            break;

        otmp = nxtobj(otmp, STATUE, TRUE);
    }

    feel_newsym(x, y);
    return mtmp;
}

static boolean
keep_saddle_with_steedcorpse(steed_mid, objchn, saddle)
unsigned steed_mid;
struct obj *objchn, *saddle;
{
    if (!saddle)
        return FALSE;
    while (objchn) {
        if (objchn->otyp == CORPSE && has_omonst(objchn)) {
            struct monst *mtmp = OMONST(objchn);

            if (mtmp->m_id == steed_mid) {
                /* move saddle */
                xchar x, y;
                if (get_obj_location(objchn, &x, &y, 0)) {
                    obj_extract_self(saddle);
                    place_object(saddle, x, y);
                    stackobj(saddle);
                }
                return TRUE;
            }
        }
        if (Has_contents(objchn)
            && keep_saddle_with_steedcorpse(steed_mid, objchn->cobj, saddle))
            return TRUE;
        objchn = objchn->nobj;
    }
    return FALSE;
}

/* monster or you go through and possibly destroy a web.
   return TRUE if could go through. */
static boolean
mu_maybe_destroy_web(mtmp, domsg, trap)
struct monst *mtmp;
boolean domsg;
struct trap *trap;
{
    boolean isyou = (mtmp == &g.youmonst);
    struct permonst *mptr = mtmp->data;

    if (amorphous(mptr) || is_whirly(mptr) || flaming(mptr)
        || unsolid(mptr) || mptr == &mons[PM_GELATINOUS_CUBE]) {
        xchar x = trap->tx;
        xchar y = trap->ty;

        if (flaming(mptr) || acidic(mptr)) {
            if (domsg) {
                if (isyou)
                    You("%s %s spider web!",
                        (flaming(mptr)) ? "burn" : "dissolve",
                        a_your[trap->madeby_u]);
                else
                    pline("%s %s %s spider web!", Monnam(mtmp),
                          (flaming(mptr)) ? "burns" : "dissolves",
                          a_your[trap->madeby_u]);
            }
            deltrap(trap);
            newsym(x, y);
            return TRUE;
        }
        if (domsg) {
            if (isyou) {
                You("flow through %s spider web.", a_your[trap->madeby_u]);
            } else {
                pline("%s flows through %s spider web.", Monnam(mtmp),
                      a_your[trap->madeby_u]);
                seetrap(trap);
            }
        }
        return TRUE;
    }
    return FALSE;
}

/* Will a monster suffer any adverse effects from a certain trap?
 * Note: this does NOT mean "will a monster trigger a trap in the first place",
 * though if it won't that does imply that they'll not suffer adverse effects.
 * For example, an elf is considered immune to sleeping gas traps even though
 * they'll set the trap off.
 * Return value:
 *  TRAP_NOT_IMMUNE = not immune at the moment
 *  TRAP_CLEARLY_IMMUNE = obviously immune (assuming player is spoiled on which
 *    monsters are immune to various traps)
 *  TRAP_HIDDEN_IMMUNE = immune but in a non-obvious way such as an unidentified
 *    item or hidden intrinsic providing a resistance; the player should still
 *    be warned of this trap, while monsters implicitly know they're immune. */
enum trap_immunities
immune_to_trap(mon, ttype)
struct monst* mon;
xchar ttype;
{
    if (!mon) {
        impossible("immune_to_trap: null monster");
        return TRAP_NOT_IMMUNE;
    }
    struct permonst * pm = mon->data;
    boolean you = (mon == &g.youmonst);

    switch(ttype) {
    case ARROW_TRAP:
    case DART_TRAP:
    case ROCKTRAP:
        /* can hit anything. Even noncorporeal monsters might get a blessed
         * projectile. */
        return TRAP_NOT_IMMUNE;
    case BEAR_TRAP:
        if (pm->msize <= MZ_SMALL || amorphous(pm) || is_whirly(pm)
            || unsolid(pm))
            return TRAP_CLEARLY_IMMUNE;
        /* FALLTHRU */
    case SQKY_BOARD:
    case LANDMINE:
    case ROLLING_BOULDER_TRAP:
    case HOLE:
    case TRAPDOOR:
    case PIT:
    case SPIKED_PIT:
        /* ground-based traps, which can be evaded by levitation, flying, or
         * hanging to the ceiling */
        if (Sokoban && (ttype == PIT || ttype == SPIKED_PIT || ttype == HOLE
                        || ttype == TRAPDOOR))
            return TRAP_NOT_IMMUNE;
        if (!grounded(pm))
            return TRAP_CLEARLY_IMMUNE;
        else if (you && (Levitation || Flying))
            return TRAP_CLEARLY_IMMUNE;
        return TRAP_NOT_IMMUNE;
    case SLP_GAS_TRAP:
        if (breathless(pm))
            return TRAP_CLEARLY_IMMUNE;
        else if (!you && resists_sleep(mon))
            return TRAP_CLEARLY_IMMUNE;
        else if (you && Sleep_resistance)
            return TRAP_HIDDEN_IMMUNE;
        return TRAP_NOT_IMMUNE;
    case LEVEL_TELEP:
    case TELEP_TRAP:
        /* Consider unintended teleporting to be an adverse effect. If in the
         * endgame or carrying the Amulet, the teleport trap won't work anyway,
         * so anything hitting it is immune. */
        if (In_endgame(&u.uz) || mon_has_amulet(mon))
            return TRAP_CLEARLY_IMMUNE;
        return TRAP_NOT_IMMUNE;
    case POLY_TRAP:
        if (resists_magm(mon))
            /* covers Antimagic for player */
            return (you ? TRAP_HIDDEN_IMMUNE : TRAP_CLEARLY_IMMUNE);
        return TRAP_NOT_IMMUNE;
    case STATUE_TRAP:
        /* no effect on monsters, only affects players; even so, a player should
         * never actually be able to see a statue trap without triggering it;
         * in case they ever can, do consider it an adverse effect */
        if (!you)
            return TRAP_CLEARLY_IMMUNE;
        return TRAP_NOT_IMMUNE;
    case WEB:
        /* most of this code is lifted from mu_maybe_destroy_web */
        if (webmaker(pm) || amorphous(pm) || is_whirly(pm) || flaming(pm)
            || unsolid(pm) || pm == &mons[PM_GELATINOUS_CUBE])
            return TRAP_CLEARLY_IMMUNE;
        return TRAP_NOT_IMMUNE;
    case ANTI_MAGIC:
        /* doesn't hurt any non-magic-resistant monster with no magic */
        if (you) {
            if (Antimagic)
                return TRAP_NOT_IMMUNE;
            else if (u.uenmax == 0)
                /* player won't lose HP and can't lose more Pw */
                return TRAP_HIDDEN_IMMUNE;
        }
        /* following conditional lifted from mintrap ANTI_MAGIC logic */
        else if (!resists_magm(mon)
                 && (mon->mcan || (!attacktype(pm, AT_MAGC)
                                   && !attacktype(pm, AT_BREA)))) {
            return TRAP_CLEARLY_IMMUNE;
        }
        return TRAP_NOT_IMMUNE;
    case RUST_TRAP:
        /* harmful if wearing anything rustable or if the monster is an iron
         * golem */
        if (pm == &mons[PM_IRON_GOLEM])
            return TRAP_NOT_IMMUNE;
        else {
            struct obj * obj = (you ? g.invent : mon->minvent);
            for (; obj; obj = obj->nobj) {
                /* rust traps can currently hit only worn armor and weapons */
                if (is_rustprone(obj) && obj->owornmask) {
                    return TRAP_NOT_IMMUNE;
                }
            }
        }
        return TRAP_CLEARLY_IMMUNE;
    case MAGIC_TRAP:
        /* for player, any number of bad effects.
         * for monsters, only replicates fire trap, so fall through */
        if (you)
            return TRAP_NOT_IMMUNE;
        /* else FALLTHRU */
    case FIRE_TRAP: /* can always destroy items being carried */
        /* harmful if wearing anything burnable or if the monster isn't
         * resistant */
        if (you && !Fire_resistance)
            return TRAP_NOT_IMMUNE;
        else if (!you && !resists_fire(mon))
            return TRAP_NOT_IMMUNE;
        else {
            struct obj * obj = (you ? g.invent : mon->minvent);
            for (; obj; obj = obj->nobj) {
                if (obj->oclass == SCROLL_CLASS || obj->oclass == POTION_CLASS
                    || obj->oclass == SPBOOK_CLASS
                    || (obj->owornmask && is_flammable(obj)))
                    return TRAP_NOT_IMMUNE;
            }
        }
        return (you ? TRAP_HIDDEN_IMMUNE : TRAP_CLEARLY_IMMUNE);
    case MAGIC_PORTAL:
        /* never hurts anything, but player is considered non-immune so they
         * can be asked about entering it */
        if (!you)
            return TRAP_CLEARLY_IMMUNE;
        return TRAP_NOT_IMMUNE;
    case VIBRATING_SQUARE:
        /* no adverse effects */
        return TRAP_CLEARLY_IMMUNE;
    default:
        impossible("immune_to_trap: bad ttype %d", ttype);
    }
    return TRAP_NOT_IMMUNE;
}

void
set_utrap(tim, typ)
unsigned tim, typ;
{
    /* if we get here through reset_utrap(), the caller of that might
       have already set u.utrap to 0 so this check won't be sufficient
       in that situation; caller will need to set context.botl itself */
    if (!u.utrap ^ !tim)
        g.context.botl = TRUE;

    u.utrap = tim;
    /* FIXME:
     * utraptype==0 is bear trap rather than 'none'; we probably ought
     * to change that but can't do so until save file compatability is
     * able to be broken.
     */
    u.utraptype = tim ? typ : 0;

    float_vs_flight(); /* maybe block Lev and/or Fly */
}

void
reset_utrap(msg)
boolean msg;
{
    boolean was_Lev = (Levitation != 0), was_Fly = (Flying != 0);

    set_utrap(0, 0);

    if (msg) {
        if (!was_Lev && Levitation)
            float_up();
        if (!was_Fly && Flying)
            You("can fly.");
    }
}

void
dotrap(trap, trflags)
register struct trap *trap;
unsigned trflags;
{
    register int ttype = trap->ttyp;
    struct obj *otmp;
    boolean already_seen = trap->tseen,
            forcetrap = ((trflags & FORCETRAP) != 0
                         || (trflags & FAILEDUNTRAP) != 0),
            webmsgok = (trflags & NOWEBMSG) == 0,
            forcebungle = (trflags & FORCEBUNGLE) != 0,
            plunged = (trflags & TOOKPLUNGE) != 0,
            viasitting = (trflags & VIASITTING) != 0,
            conj_pit = conjoined_pits(trap, t_at(u.ux0, u.uy0), TRUE),
            adj_pit = adj_nonconjoined_pit(trap);
    int oldumort;
    int steed_article = ARTICLE_THE;

    nomul(0);

    /* Correct conj_pit and adj_pit if the player isn't moving; this function
     * can also be called by a pit fiend hurling you into a pit on its turn,
     * which has nothing to do with moving between pits */
    if (!g.context.mon_moving) {
        conj_pit = adj_pit = FALSE;
    }

    /* KMH -- You can't escape the Sokoban level traps */
    if (Sokoban && (is_pit(ttype) || is_hole(ttype))) {
        /* The "air currents" message is still appropriate -- even when
         * the hero isn't flying or levitating -- because it conveys the
         * reason why the player cannot escape the trap with a dexterity
         * check, clinging to the ceiling, etc.
         */
        pline("Air currents pull you down into %s %s!",
              a_your[trap->madeby_u],
              trapname(ttype, TRUE)); /* do force "pit" while hallucinating */
        /* then proceed to normal trap effect */
    } else if (already_seen && !forcetrap) {
        if ((Levitation || (Flying && !plunged) || is_clinger(g.youmonst.data))
            && (is_pit(ttype) || ttype == HOLE
                || ttype == BEAR_TRAP || ttype == ROLLING_BOULDER_TRAP
                || ttype == SQKY_BOARD)) {
            You("%s over %s %s.", (Levitation ? "float" :
                                                (Flying ? "fly" : "hang")),
                a_your[trap->madeby_u],
                trapname(ttype, FALSE));
            return;
        }
        if (!Fumbling && ttype != MAGIC_PORTAL && ttype != VIBRATING_SQUARE
            && ttype != ANTI_MAGIC && !forcebungle && !plunged
            && !conj_pit && !adj_pit
            && (!rn2(5) || (is_pit(ttype)
                            && !grounded(g.youmonst.data)))) {
                You("escape %s %s.", (ttype == ARROW_TRAP && !trap->madeby_u)
                                     ? "an"
                                     : a_your[trap->madeby_u],
                trapname(ttype, FALSE));
            return;
        }
    }

    if (u.usteed) {
        u.usteed->mtrapseen |= (1 << (ttype - 1));
        /* suppress article in various steed messages when using its
           name (which won't occur when hallucinating) */
        if (has_mname(u.usteed) && !Hallucination)
            steed_article = ARTICLE_NONE;
    }

    /*
     * Note:
     *  Most references to trap types here don't use trapname() for
     *  hallucination.  This could be considered to be a bug but doing
     *  that would hide the actual trap situation from the player which
     *  would be somewhat harsh for what's usually a minor impairment.
     */

    switch (ttype) {
    case ARROW_TRAP:
    case DART_TRAP:
        if (!trap->ammo) {
            You_hear("a loud click!");
            deltrap(trap);
            newsym(u.ux, u.uy);
            break;
        }
        otmp = trap->ammo;
        if (trap->ammo->quan > 1) {
            otmp = splitobj(trap->ammo, 1);
        }
        extract_nobj(otmp, &trap->ammo);
        seetrap(trap);
        pline("%s shoots out at you!", An(xname(otmp)));

        oldumort = u.umortality;
        if (u.usteed && !rn2(2) && steedintrap(trap, otmp)) {
            ; /* nothing */
        } else if (thitu(8, dmgval(otmp, &g.youmonst), &otmp, (const char *) 0)) {
            if (otmp) {
                if (otmp->opoisoned)
                    /* Poison can either deal extra HP damage or attribute loss.
                     * We'd rather do the latter for a trap, so don't count this
                     * missile as a "thrown weapon". */
                    poisoned("dart", A_CON, OBJ_NAME(objects[otmp->otyp]),
                             10, FALSE);
                /* TODO: use hero-missile ammo breakage formula rather than
                 * unconditionally destroying otmp? */
                obfree(otmp, (struct obj *) 0);
            }
        } else {
            place_object(otmp, u.ux, u.uy);
            if (!Blind)
                otmp->dknown = 1;
            stackobj(otmp);
            newsym(u.ux, u.uy);
        }
        break;

    case ROCKTRAP:
        if (!trap->ammo) {
            pline("A trap door in %s opens, but nothing falls out!",
                  the(ceiling(u.ux, u.uy)));
            deltrap(trap);
            newsym(u.ux, u.uy);
        } else {
            int dmg = d(2, 6); /* should be std ROCK dmg? */

            otmp = trap->ammo;
            if (trap->ammo->quan > 1) {
                otmp = splitobj(trap->ammo, 1);
            }
            extract_nobj(otmp, &trap->ammo);
            feeltrap(trap);
            place_object(otmp, u.ux, u.uy);

            pline("A trap door in %s opens and %s falls on your %s!",
                  the(ceiling(u.ux, u.uy)), an(xname(otmp)), body_part(HEAD));
            if (uarmh) {
                if (is_hard(uarmh)) {
                    pline("Fortunately, you are wearing a hard helmet.");
                    dmg = 2;
                } else if (flags.verbose) {
                    pline("%s does not protect you.", Yname2(uarmh));
                }
            }
            if (!Blind)
                otmp->dknown = 1;
            stackobj(otmp);
            newsym(u.ux, u.uy); /* map the rock */

            losehp(Maybe_Half_Phys(dmg), "falling rock", KILLED_BY_AN);
            exercise(A_STR, FALSE);
        }
        break;

    case SQKY_BOARD: /* stepped on a squeaky board */
        if ((Levitation || Flying) && !forcetrap) {
            if (!Blind) {
                seetrap(trap);
                if (Hallucination)
                    You("notice a crease in the linoleum.");
                else
                    You("notice a loose board below you.");
            }
        } else {
            seetrap(trap);
            if (Hallucination)
                You("accidentally step on a squeaky toy!");
            else
                pline("A board beneath you %s%s%s.",
                    Deaf ? "vibrates" : "squeaks ",
                    Deaf ? "" : trapnote(trap, 0), Deaf ? "" : " loudly");
            wake_nearby();
        }
        break;

    case BEAR_TRAP: {
        int dmg = d(2, 4);

        if ((Levitation || Flying) && !forcetrap)
            break;
        feeltrap(trap);
        if (amorphous(g.youmonst.data) || is_whirly(g.youmonst.data)
            || unsolid(g.youmonst.data)) {
            pline("%s bear trap closes harmlessly through you.",
                  A_Your[trap->madeby_u]);
            break;
        }
        if (!u.usteed && g.youmonst.data->msize <= MZ_SMALL) {
            pline("%s bear trap closes harmlessly over you.",
                  A_Your[trap->madeby_u]);
            break;
        }
        set_utrap((unsigned) rn1(4, 4), TT_BEARTRAP);
        if (u.usteed) {
            pline("%s bear trap closes on %s %s!", A_Your[trap->madeby_u],
                  s_suffix(mon_nam(u.usteed)), mbodypart(u.usteed, FOOT));
            if (thitm(0, u.usteed, (struct obj *) 0, dmg, FALSE))
                reset_utrap(TRUE); /* steed died, hero not trapped */
        } else {
            pline("%s bear trap closes on your %s!", A_Your[trap->madeby_u],
                  body_part(FOOT));
            set_wounded_legs(rn2(2) ? RIGHT_SIDE : LEFT_SIDE, rn1(10, 10));
            if (u.umonnum == PM_OWLBEAR || u.umonnum == PM_BUGBEAR)
                You("howl in anger!");
            losehp(Maybe_Half_Phys(dmg), "bear trap", KILLED_BY_AN);
        }
        exercise(A_DEX, FALSE);
        break;
    }

    case SLP_GAS_TRAP:
        seetrap(trap);
        if (Sleep_resistance || breathless(g.youmonst.data)) {
            You("are enveloped in a cloud of gas!");
        } else {
            pline("A cloud of gas puts you to sleep!");
            fall_asleep(-rnd(25), TRUE);
        }
        (void) steedintrap(trap, (struct obj *) 0);
        break;

    case RUST_TRAP:
        seetrap(trap);

        /* Unlike monsters, traps cannot aim their rust attacks at
         * you, so instead of looping through and taking either the
         * first rustable one or the body, we take whatever we get,
         * even if it is not rustable.
         */
        switch (rn2(5)) {
        case 0:
            pline("%s you on the %s!", A_gush_of_water_hits, body_part(HEAD));
            (void) water_damage(uarmh, helm_simple_name(uarmh), TRUE);
            break;
        case 1:
            pline("%s your left %s!", A_gush_of_water_hits, body_part(ARM));
            if (water_damage(uarms, "shield", TRUE) != ER_NOTHING)
                break;
            if (u.twoweap || (uwep && bimanual(uwep)))
                (void) water_damage(u.twoweap ? uswapwep : uwep, 0, TRUE);
 glovecheck:
            (void) water_damage(uarmg, "gauntlets", TRUE);
            /* Not "metal gauntlets" since it gets called
             * even if it's leather for the message
             */
            break;
        case 2:
            pline("%s your right %s!", A_gush_of_water_hits, body_part(ARM));
            (void) water_damage(uwep, 0, TRUE);
            goto glovecheck;
        default:
            pline("%s you!", A_gush_of_water_hits);
            for (otmp = g.invent; otmp; otmp = otmp->nobj)
                if (otmp->lamplit && otmp != uwep
                    && (otmp != uswapwep || !u.twoweap))
                    (void) snuff_lit(otmp);
            if (uarmc)
                (void) water_damage(uarmc, cloak_simple_name(uarmc), TRUE);
            else if (uarm)
                (void) water_damage(uarm, suit_simple_name(uarm), TRUE);
            else if (uarmu)
                (void) water_damage(uarmu, "shirt", TRUE);
        }
        update_inventory();

        if (u.umonnum == PM_IRON_GOLEM) {
            int dam = u.mhmax;

            You("are covered with rust!");
            losehp(Maybe_Half_Phys(dam), "rusting away", KILLED_BY);
        } else if (u.umonnum == PM_GREMLIN && rn2(3)) {
            (void) split_mon(&g.youmonst, (struct monst *) 0);
        }

        break;

    case FIRE_TRAP:
        seetrap(trap);
        dofiretrap((struct obj *) 0);
        break;

    case PIT:
    case SPIKED_PIT:
        /* KMH -- You can't escape the Sokoban level traps */
        if (!Sokoban && (Levitation || (Flying && !plunged)))
            break;
        feeltrap(trap);
        if (!Sokoban && !grounded(g.youmonst.data) && !plunged) {
            if (trap->tseen) {
                You_see("%s %spit below you.", a_your[trap->madeby_u],
                        ttype == SPIKED_PIT ? "spiked " : "");
            } else {
                pline("%s pit %sopens up under you!", A_Your[trap->madeby_u],
                      ttype == SPIKED_PIT ? "full of spikes " : "");
                You("don't fall in!");
            }
            break;
        }
        /* is the pit an "open grave"? (if it's in a graveyard, yes) */
        boolean is_grave = (getroomtype(u.ux, u.uy) == MORGUE &&
                            ttype != SPIKED_PIT);
        if (!Sokoban) {
            char verbbuf[BUFSZ];

            *verbbuf = '\0';
            if (u.usteed) {
                if ((trflags & RECURSIVETRAP) != 0)
                    Sprintf(verbbuf, "and %s fall",
                            x_monnam(u.usteed, steed_article, (char *) 0,
                                     SUPPRESS_SADDLE, FALSE));
                else
                    Sprintf(verbbuf, "lead %s",
                            x_monnam(u.usteed, steed_article, "poor",
                                     SUPPRESS_SADDLE, FALSE));
            } else if (conj_pit) {
                You("move into an adjacent pit.");
            } else if (adj_pit) {
                You("stumble over debris%s.",
                    !rn2(5) ? " between the pits" : "");
            } else {
                Strcpy(verbbuf,
                       !plunged ? "fall" : (Flying ? "dive" : "plunge"));
            }
            if (*verbbuf)
                You("%s into %s %s!", verbbuf, a_your[trap->madeby_u],
                    (is_grave ? "open grave" : "pit"));
        }
        /* wumpus reference */
        if (Role_if(PM_RANGER) && !trap->madeby_u && !trap->once
            && In_quest(&u.uz) && Is_qlocate(&u.uz)) {
            pline("Fortunately it has a bottom after all...");
            trap->once = 1;
        } else if (u.umonnum == PM_PIT_VIPER || u.umonnum == PM_PIT_FIEND) {
            pline("How pitiful.  Isn't that the pits?");
        } else if (is_grave) {
            if(is_undead(g.youmonst.data)) {
                pline("It seems quite cozy down here.");
            }
            else {
                pline("It's a little early for that, isn't it?");
            }
        }
        if (ttype == SPIKED_PIT) {
            const char *predicament = "on a set of sharp iron spikes";

            if (u.usteed) {
                pline("%s %s %s!",
                      upstart(x_monnam(u.usteed, steed_article, "poor",
                                       SUPPRESS_SADDLE, FALSE)),
                      conj_pit ? "steps" : "lands", predicament);
            } else
                You("%s %s!", conj_pit ? "step" : "land", predicament);
        }
        /* FIXME:
         * if hero gets killed here, setting u.utrap in advance will
         * show "you were trapped in a pit" during disclosure's display
         * of enlightenment, but hero is dying *before* becoming trapped.
         */
        set_utrap((unsigned) rn1(6, 2), TT_PIT);
        if (!steedintrap(trap, (struct obj *) 0)) {
            if (ttype == SPIKED_PIT) {
                oldumort = u.umortality;
                losehp(Maybe_Half_Phys(rnd(conj_pit ? 4 : adj_pit ? 6 : 10)),
                       /* note: these don't need locomotion() handling;
                          if fatal while poly'd and Unchanging, the
                          death reason will be overridden with
                          "killed while stuck in creature form" */
                       plunged
                          ? "deliberately plunged into a pit of iron spikes"
                          : conj_pit
                             ? "stepped into a pit of iron spikes"
                             : adj_pit
                                ? "stumbled into a pit of iron spikes"
                                : "fell into a pit of iron spikes",
                       NO_KILLER_PREFIX);
                if (!rn2(6))
                    poisoned("spikes", A_STR,
                             (conj_pit || adj_pit)
                                ? "stepping on poison spikes"
                                : "fall onto poison spikes",
                             /* if damage triggered life-saving,
                                poison is limited to attrib loss */
                             (u.umortality > oldumort) ? 0 : 8, FALSE);
            } else {
                /* plunging flyers take spike damage but not pit damage */
                if (!conj_pit
                    && !(plunged && (Flying || is_clinger(g.youmonst.data))))
                    losehp(Maybe_Half_Phys(rnd(adj_pit ? 3 : 6)),
                           plunged ? "deliberately plunged into a pit"
                                   : "fell into a pit",
                           NO_KILLER_PREFIX);
            }
            if (Punished && !carried(uball)) {
                unplacebc();
                ballfall();
                placebc();
            }
            if (!conj_pit)
                selftouch("Falling, you");
            g.vision_full_recalc = 1; /* vision limits change */
            exercise(A_STR, FALSE);
            exercise(A_DEX, FALSE);
        }
        break;

    case HOLE:
    case TRAPDOOR:
        if (!Can_fall_thru(&u.uz)) {
            seetrap(trap); /* normally done in fall_through */
            impossible("dotrap: %ss cannot exist on this level.",
                       trapname(ttype, TRUE));
            break; /* don't activate it after all */
        }
        fall_through(TRUE, (trflags & TOOKPLUNGE));
        break;

    case TELEP_TRAP:
        seetrap(trap);
        tele_trap(trap);
        break;

    case LEVEL_TELEP:
        seetrap(trap);
        level_tele_trap(trap, trflags);
        break;

    case WEB: /* Our luckless player has stumbled into a web. */
        feeltrap(trap);
        if (mu_maybe_destroy_web(&g.youmonst, webmsgok, trap))
            break;
        if (webmaker(g.youmonst.data)) {
            if (webmsgok)
                pline(trap->madeby_u ? "You take a walk on your web."
                                     : "There is a spider web here.");
            break;
        }
        if (webmsgok) {
            char verbbuf[BUFSZ];

            if (forcetrap || viasitting || g.context.nopick) {
                Strcpy(verbbuf, "are caught by");
            } else if (u.usteed) {
                Sprintf(verbbuf, "lead %s into",
                        x_monnam(u.usteed, steed_article, "poor",
                                 SUPPRESS_SADDLE, FALSE));
            } else {
                Sprintf(verbbuf, "%s into",
                        Levitation ? (const char *) "float"
                                   : locomotion(g.youmonst.data, "stumble"));
            }
            You("%s %s spider web!", verbbuf, a_your[trap->madeby_u]);
        }

        /* time will be adjusted below */
        set_utrap(1, TT_WEB);

        /* Time stuck in the web depends on your/steed strength. */
        {
            int tim, str = ACURR(A_STR);

            /* If mounted, the steed gets trapped.  Use mintrap
             * to do all the work.  If mtrapped is set as a result,
             * unset it and set utrap instead.  In the case of a
             * strongmonst and mintrap said it's trapped, use a
             * short but non-zero trap time.  Otherwise, monsters
             * have no specific strength, so use player strength.
             * This gets skipped for webmsgok, which implies that
             * the steed isn't a factor.
             */
            if (u.usteed && webmsgok) {
                /* mtmp location might not be up to date */
                u.usteed->mx = u.ux;
                u.usteed->my = u.uy;

                /* mintrap currently does not return 2(died) for webs */
                if (mintrap(u.usteed)) {
                    u.usteed->mtrapped = 0;
                    if (strongmonst(u.usteed->data))
                        str = 17;
                } else {
                    reset_utrap(FALSE);
                    break;
                }

                webmsgok = FALSE; /* mintrap printed the messages */
            }
            if (str <= 3)
                tim = rn1(6, 6);
            else if (str < 6)
                tim = rn1(6, 4);
            else if (str < 9)
                tim = rn1(4, 4);
            else if (str < 12)
                tim = rn1(4, 2);
            else if (str < 15)
                tim = rn1(2, 2);
            else if (str < 18)
                tim = rnd(2);
            else if (str < 69)
                tim = 1;
            else {
                tim = 0;
                if (webmsgok)
                    You("tear through %s web!", a_your[trap->madeby_u]);
                deltrap(trap);
                newsym(u.ux, u.uy); /* get rid of trap symbol */
            }
            set_utrap((unsigned) tim, TT_WEB);
        }
        break;

    case STATUE_TRAP:
        (void) activate_statue_trap(trap, u.ux, u.uy, FALSE);
        break;

    case MAGIC_TRAP: /* A magic trap. */
        seetrap(trap);
        if (!rn2(30)) {
            deltrap(trap);
            newsym(u.ux, u.uy); /* update position */
            You("are caught in a magical explosion!");
            losehp(rnd(10), "magical explosion", KILLED_BY_AN);
            Your("body absorbs some of the magical energy!");
            u.uenmax += 2;
            u.uen = u.uenmax;
            break;
        } else {
            domagictrap();
        }
        (void) steedintrap(trap, (struct obj *) 0);
        break;

    case ANTI_MAGIC:
        seetrap(trap);
        /* hero without magic resistance loses spell energy,
           hero with magic resistance takes damage instead;
           possibly non-intuitive but useful for play balance */
        if (!Antimagic) {
            drain_en(rnd(u.ulevel) + 1);
        } else {
            int dmgval2 = rnd(4), hp = Upolyd ? u.mh : u.uhp;

            /* Half_XXX_damage has opposite its usual effect (approx)
               but isn't cumulative if hero has more than one */
            if (Half_physical_damage || Half_spell_damage)
                dmgval2 += rnd(4);
            /* give Magicbane wielder dose of own medicine */
            if (uwep && uwep->oartifact == ART_MAGICBANE)
                dmgval2 += rnd(4);
            /* having an artifact--other than own quest one--which
               confers magic resistance simply by being carried
               also increases the effect */
            for (otmp = g.invent; otmp; otmp = otmp->nobj)
                if (otmp->oartifact && !is_quest_artifact(otmp)
                    && defends_when_carried(AD_MAGM, otmp))
                    break;
            if (otmp)
                dmgval2 += rnd(4);
            if (Passes_walls)
                dmgval2 = (dmgval2 + 3) / 4;

            You_feel((dmgval2 >= hp) ? "unbearably torpid!"
                                     : (dmgval2 >= hp / 4) ? "very lethargic."
                                                           : "sluggish.");
            /* opposite of magical explosion */
            losehp(dmgval2, "anti-magic implosion", KILLED_BY_AN);
        }
        break;

    case POLY_TRAP: {
        char verbbuf[BUFSZ];

        seetrap(trap);
        if (viasitting)
            Strcpy(verbbuf, "trigger"); /* follows "You sit down." */
        else if (u.usteed)
            Sprintf(verbbuf, "lead %s onto",
                    x_monnam(u.usteed, steed_article, (char *) 0,
                             SUPPRESS_SADDLE, FALSE));
        else
            Sprintf(verbbuf, "%s onto",
                    Levitation ? (const char *) "float"
                               : locomotion(g.youmonst.data, "step"));
        You("%s a polymorph trap!", verbbuf);
        /* your Antimagic also protects your steed; however, your Unchanging
         * won't */
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            You_feel("momentarily different.");
            /* Trap did nothing; don't remove it --KAA */
        } else {
            (void) steedintrap(trap, (struct obj *) 0); /* may call deltrap() */
            if (!Unchanging) {
                if (t_at(u.ux, u.uy)) {
                    /* steed didn't call deltrap so must not have been affected */
                    deltrap(trap);      /* delete trap before polymorph */
                    newsym(u.ux, u.uy); /* get rid of trap symbol */
                }
                You_feel("a change coming over you.");
                polyself(0);
            }
        }
        break;
    }
    case LANDMINE: {
        unsigned steed_mid = 0;
        struct obj *saddle = 0;

        if ((Levitation || Flying) && !forcetrap) {
            if (!already_seen && rn2(3))
                break;
            feeltrap(trap);
            pline("%s %s in a pile of soil below you.",
                  already_seen ? "There is" : "You discover",
                  trap->madeby_u ? "the trigger of your mine" : "a trigger");
            if (already_seen && rn2(3))
                break;
            pline("KAABLAMM!!!  %s %s%s off!",
                  forcebungle ? "Your inept attempt sets"
                              : "The air currents set",
                  already_seen ? a_your[trap->madeby_u] : "",
                  already_seen ? " land mine" : "it");
        } else {
            /* prevent landmine from killing steed, throwing you to
             * the ground, and you being affected again by the same
             * mine because it hasn't been deleted yet
             */
            static boolean recursive_mine = FALSE;

            if (recursive_mine)
                break;
            feeltrap(trap);
            pline("KAABLAMM!!!  You triggered %s land mine!",
                  a_your[trap->madeby_u]);
            if (u.usteed)
                steed_mid = u.usteed->m_id;
            recursive_mine = TRUE;
            (void) steedintrap(trap, (struct obj *) 0);
            recursive_mine = FALSE;
            saddle = sobj_at(SADDLE, u.ux, u.uy);
            set_wounded_legs(LEFT_SIDE, rn1(35, 41));
            set_wounded_legs(RIGHT_SIDE, rn1(35, 41));
            exercise(A_DEX, FALSE);
        }
        blow_up_landmine(trap);
        if (steed_mid && saddle && !u.usteed)
            (void) keep_saddle_with_steedcorpse(steed_mid, fobj, saddle);
        newsym(u.ux, u.uy); /* update trap symbol */
        losehp(Maybe_Half_Phys(rnd(16)), "land mine", KILLED_BY_AN);
        /* fall recursively into the pit... */
        if ((trap = t_at(u.ux, u.uy)) != 0)
            dotrap(trap, RECURSIVETRAP);
        fill_pit(u.ux, u.uy);
        break;
    }

    case ROLLING_BOULDER_TRAP: {
        int style = ROLL | (trap->tseen ? LAUNCH_KNOWN : 0);
        if (Levitation || Flying || !grounded(g.youmonst.data))
            break;

        feeltrap(trap);
        pline("Click!  You trigger a rolling boulder trap!");
        if (!launch_obj(BOULDER, trap->launch.x, trap->launch.y,
                        trap->launch2.x, trap->launch2.y, style)) {
            deltrap(trap);
            newsym(u.ux, u.uy); /* get rid of trap symbol */
            pline("Fortunately for you, no boulder was released.");
        }
        break;
    }

    case MAGIC_PORTAL:
        feeltrap(trap);
        domagicportal(trap);
        break;

    case VIBRATING_SQUARE:
        feeltrap(trap);
        /* messages handled elsewhere; the trap symbol is merely to mark the
         * square for future reference */
        break;

    default:
        feeltrap(trap);
        impossible("You hit a trap of type %u", trap->ttyp);
    }
}

static char *
trapnote(trap, noprefix)
struct trap *trap;
boolean noprefix;
{
    static char tnbuf[12];
    const char *tn,
        *tnnames[12] = { "C note",  "D flat", "D note",  "E flat",
                         "E note",  "F note", "F sharp", "G note",
                         "G sharp", "A note", "B flat",  "B note" };

    tnbuf[0] = '\0';
    tn = tnnames[trap->tnote];
    if (!noprefix)
        Sprintf(tnbuf, "%s ",
                (*tn == 'A' || *tn == 'E' || *tn == 'F') ? "an" : "a");
    Sprintf(eos(tnbuf), "%s", tn);
    return tnbuf;
}

static int
steedintrap(trap, otmp)
struct trap *trap;
struct obj *otmp;
{
    struct monst *steed = u.usteed;
    int tt;
    boolean trapkilled, steedhit;

    if (!steed || !trap)
        return 0;
    tt = trap->ttyp;
    steed->mx = u.ux;
    steed->my = u.uy;
    trapkilled = steedhit = FALSE;

    switch (tt) {
    case ARROW_TRAP:
    case DART_TRAP:
        if (!otmp) {
            impossible("steed hit by non-existent arrow/dart?");
            return 0;
        }
        trapkilled = thitm(8, steed, otmp, 0, FALSE);
        steedhit = TRUE;
        break;
    case SLP_GAS_TRAP:
        if (!resists_sleep(steed) && !breathless(steed->data)
            && !steed->msleeping && steed->mcanmove) {
            if (sleep_monst(steed, rnd(25), -1))
                /* no in_sight check here; you can feel it even if blind */
                pline("%s suddenly falls asleep!", Monnam(steed));
        }
        steedhit = TRUE;
        break;
    case LANDMINE:
        trapkilled = thitm(0, steed, (struct obj *) 0, rnd(16), FALSE);
        steedhit = TRUE;
        break;
    case PIT:
    case SPIKED_PIT:
        trapkilled = (DEADMONSTER(steed)
                      || thitm(0, steed, (struct obj *) 0,
                               rnd((tt == PIT) ? 6 : 10), FALSE));
        steedhit = TRUE;
        break;
    case POLY_TRAP:
        if (!resists_magm(steed) && !resist(steed, WAND_CLASS, 0, NOTELL)) {
            struct permonst *mdat = steed->data;

            deltrap(trap);
            newsym(steed->mx, steed->my); /* get rid of trap symbol */
            (void) newcham(steed, (struct permonst *) 0, FALSE, TRUE);
            if (!can_saddle(steed) || !can_ride(steed)) {
                dismount_steed(DISMOUNT_POLY);
            } else {
                char buf[BUFSZ];

                Strcpy(buf, x_monnam(steed, ARTICLE_YOUR, (char *) 0,
                                            SUPPRESS_SADDLE, FALSE));
                if (mdat != steed->data)
                    (void) strsubst(buf, "your ", "your new ");
                You("adjust yourself in the saddle on %s.", buf);
            }
        }
        steedhit = TRUE;
        break;
    default:
        break;
    }

    if (trapkilled) {
        dismount_steed(DISMOUNT_POLY);
        return 2;
    }
    return steedhit ? 1 : 0;
}

/* some actions common to both player and monsters for triggered landmine */
void
blow_up_landmine(trap)
struct trap *trap;
{
    int x = trap->tx, y = trap->ty, dbx, dby;
    struct rm *lev = &levl[x][y];

    set_trap_ammo(trap, NULL); /* useup the land mine obj */
    (void) scatter(x, y, 4,
                   MAY_DESTROY | MAY_HIT | MAY_FRACTURE | VIS_EFFECTS,
                   (struct obj *) 0);
    del_engr_at(x, y);
    wake_nearto(x, y, 400);
    if (IS_DOOR(lev->typ))
        set_doorstate(lev, D_BROKEN);
    /* destroy drawbridge if present */
    if (lev->typ == DRAWBRIDGE_DOWN || is_drawbridge_wall(x, y) >= 0) {
        dbx = x, dby = y;
        /* if under the portcullis, the bridge is adjacent */
        if (find_drawbridge(&dbx, &dby))
            destroy_drawbridge(dbx, dby);
        trap = t_at(x, y); /* expected to be null after destruction */
    }
    /* convert landmine into pit */
    if (trap) {
        if (Is_waterlevel(&u.uz) || Is_airlevel(&u.uz)) {
            /* no pits here */
            deltrap(trap);
        } else {
            trap->ttyp = PIT;       /* explosion creates a pit */
            trap->madeby_u = FALSE; /* resulting pit isn't yours */
            seetrap(trap);          /* and it isn't concealed */
        }
    }
}

static void
launch_drop_spot(obj, x, y)
struct obj *obj;
xchar x, y;
{
    if (!obj) {
        g.launchplace.obj = (struct obj *) 0;
        g.launchplace.x = 0;
        g.launchplace.y = 0;
    } else {
        g.launchplace.obj = obj;
        g.launchplace.x = x;
        g.launchplace.y = y;
    }
}

boolean
launch_in_progress()
{
    if (g.launchplace.obj)
        return TRUE;
    return FALSE;
}

void
force_launch_placement()
{
    if (g.launchplace.obj) {
        g.launchplace.obj->otrapped = 0;
        place_object(g.launchplace.obj, g.launchplace.x, g.launchplace.y);
    }
}

/*
 * Move obj from (x1,y1) to (x2,y2)
 *
 * Return 0 if no object was launched.
 *        1 if an object was launched and placed somewhere.
 *        2 if an object was launched, but used up.
 */
int
launch_obj(otyp, x1, y1, x2, y2, style)
short otyp;
register int x1, y1, x2, y2;
int style;
{
    register struct monst *mtmp;
    register struct obj *otmp, *otmp2;
    register int dx, dy;
    struct obj *singleobj;
    boolean used_up = FALSE;
    boolean otherside = FALSE;
    int dist;
    int tmp;
    int delaycnt = 0;

    otmp = sobj_at(otyp, x1, y1);
    /* Try the other side too, for rolling boulder traps */
    if (!otmp && otyp == BOULDER) {
        otherside = TRUE;
        otmp = sobj_at(otyp, x2, y2);
    }
    if (!otmp)
        return 0;
    if (otherside) { /* swap 'em */
        int tx, ty;

        tx = x1;
        ty = y1;
        x1 = x2;
        y1 = y2;
        x2 = tx;
        y2 = ty;
    }

    if (otmp->quan == 1L) {
        obj_extract_self(otmp);
        singleobj = otmp;
        otmp = (struct obj *) 0;
    } else {
        singleobj = splitobj(otmp, 1L);
        obj_extract_self(singleobj);
    }
    newsym(x1, y1);
    /* in case you're using a pick-axe to chop the boulder that's being
       launched (perhaps a monster triggered it), destroy context so that
       next dig attempt never thinks you're resuming previous effort */
    if ((otyp == BOULDER || otyp == STATUE)
        && singleobj->ox == g.context.digging.pos.x
        && singleobj->oy == g.context.digging.pos.y)
        (void) memset((genericptr_t) &g.context.digging, 0,
                      sizeof(struct dig_info));

    dist = distmin(x1, y1, x2, y2);
    g.bhitpos.x = x1;
    g.bhitpos.y = y1;
    dx = sgn(x2 - x1);
    dy = sgn(y2 - y1);
    switch (style) {
    case ROLL | LAUNCH_UNSEEN:
        if (otyp == BOULDER) {
            You_hear(Hallucination ? "someone bowling."
                                   : "rumbling in the distance.");
        }
        style &= ~LAUNCH_UNSEEN;
        goto roll;
    case ROLL | LAUNCH_KNOWN:
        /* use otrapped as a flag to ohitmon */
        singleobj->otrapped = 1;
        style &= ~LAUNCH_KNOWN;
    /*FALLTHRU*/
 roll:
    case ROLL:
        delaycnt = 2;
    /*FALLTHRU*/
    default:
        if (!delaycnt)
            delaycnt = 1;
        if (!cansee(g.bhitpos.x, g.bhitpos.y))
            curs_on_u();
        tmp_at(DISP_FLASH, obj_to_glyph(singleobj, rn2_on_display_rng));
        tmp_at(g.bhitpos.x, g.bhitpos.y);
    }
    /* Mark a spot to place object in bones files to prevent
     * loss of object. Use the starting spot to ensure that
     * a rolling boulder will still launch, which it wouldn't
     * do if left midstream. Unfortunately we can't use the
     * target resting spot, because there are some things/situations
     * that would prevent it from ever getting there (bars), and we
     * can't tell that yet.
     */
    launch_drop_spot(singleobj, g.bhitpos.x, g.bhitpos.y);

    /* Set the object in motion */
    while (dist-- > 0 && !used_up) {
        struct trap *t;

        tmp_at(g.bhitpos.x, g.bhitpos.y);
        tmp = delaycnt;

        /* dstage@u.washington.edu -- Delay only if hero sees it */
        if (cansee(g.bhitpos.x, g.bhitpos.y))
            while (tmp-- > 0)
                delay_output();

        g.bhitpos.x += dx;
        g.bhitpos.y += dy;

        if ((mtmp = m_at(g.bhitpos.x, g.bhitpos.y)) != 0) {
            if (otyp == BOULDER && throws_rocks(mtmp->data)) {
                if (rn2(3)) {
                    if (cansee(g.bhitpos.x, g.bhitpos.y))
                        pline("%s snatches the boulder.", Monnam(mtmp));
                    singleobj->otrapped = 0;
                    (void) mpickobj(mtmp, singleobj);
                    used_up = TRUE;
                    launch_drop_spot((struct obj *) 0, 0, 0);
                    break;
                }
            }
            if (ohitmon(mtmp, singleobj, (style == ROLL) ? -1 : dist,
                        FALSE)) {
                used_up = TRUE;
                launch_drop_spot((struct obj *) 0, 0, 0);
                break;
            }
        } else if (g.bhitpos.x == u.ux && g.bhitpos.y == u.uy) {
            if (g.multi)
                nomul(0);
            if (thitu(9 + singleobj->spe, dmgval(singleobj, &g.youmonst),
                      &singleobj, (char *) 0))
                stop_occupation();
        }
        if (style == ROLL) {
            if (down_gate(g.bhitpos.x, g.bhitpos.y) != -1) {
                if (ship_object(singleobj, g.bhitpos.x, g.bhitpos.y, FALSE)) {
                    used_up = TRUE;
                    launch_drop_spot((struct obj *) 0, 0, 0);
                    break;
                }
            }
            if ((t = t_at(g.bhitpos.x, g.bhitpos.y)) != 0 && otyp == BOULDER) {
                switch (t->ttyp) {
                case LANDMINE:
                    if (rn2(10) > 2) {
                        pline("KAABLAMM!!!%s",
                              cansee(g.bhitpos.x, g.bhitpos.y)
                               ? "  The rolling boulder triggers a land mine."
                               : "");
                        deltrap(t);
                        del_engr_at(g.bhitpos.x, g.bhitpos.y);
                        place_object(singleobj, g.bhitpos.x, g.bhitpos.y);
                        singleobj->otrapped = 0;
                        fracture_rock(singleobj);
                        (void) scatter(g.bhitpos.x, g.bhitpos.y, 4,
                                       MAY_DESTROY | MAY_HIT | MAY_FRACTURE
                                           | VIS_EFFECTS,
                                       (struct obj *) 0);
                        if (cansee(g.bhitpos.x, g.bhitpos.y))
                            newsym(g.bhitpos.x, g.bhitpos.y);
                        used_up = TRUE;
                        launch_drop_spot((struct obj *) 0, 0, 0);
                    }
                    break;
                case LEVEL_TELEP:
                case TELEP_TRAP:
                    if (cansee(g.bhitpos.x, g.bhitpos.y))
                        pline("Suddenly the rolling boulder disappears!");
                    else
                        You_hear("a rumbling stop abruptly.");
                    singleobj->otrapped = 0;
                    if (t->ttyp == TELEP_TRAP)
                        (void) rloco(singleobj);
                    else {
                        int newlev = random_teleport_level();
                        d_level dest;

                        if (newlev == depth(&u.uz) || In_endgame(&u.uz))
                            continue;
                        add_to_migration(singleobj);
                        get_level(&dest, newlev);
                        singleobj->ox = dest.dnum;
                        singleobj->oy = dest.dlevel;
                        singleobj->migrateflags = (long) MIGR_RANDOM;
                    }
                    seetrap(t);
                    used_up = TRUE;
                    launch_drop_spot((struct obj *) 0, 0, 0);
                    break;
                case PIT:
                case SPIKED_PIT:
                case HOLE:
                case TRAPDOOR:
                    /* the boulder won't be used up if there is a
                       monster in the trap; stop rolling anyway */
                    x2 = g.bhitpos.x, y2 = g.bhitpos.y; /* stops here */
                    if (flooreffects(singleobj, x2, y2, "fall")) {
                        used_up = TRUE;
                        launch_drop_spot((struct obj *) 0, 0, 0);
                    }
                    dist = -1; /* stop rolling immediately */
                    break;
                }
                if (used_up || dist == -1)
                    break;
            }
            if (flooreffects(singleobj, g.bhitpos.x, g.bhitpos.y, "fall")) {
                used_up = TRUE;
                launch_drop_spot((struct obj *) 0, 0, 0);
                break;
            }
            if (otyp == BOULDER
                && (otmp2 = sobj_at(BOULDER, g.bhitpos.x, g.bhitpos.y)) != 0) {
                const char *bmsg = " as one boulder sets another in motion";

                if (!isok(g.bhitpos.x + dx, g.bhitpos.y + dy) || !dist
                    || IS_ROCK(levl[g.bhitpos.x + dx][g.bhitpos.y + dy].typ))
                    bmsg = " as one boulder hits another";

                You_hear("a loud crash%s!",
                         cansee(g.bhitpos.x, g.bhitpos.y) ? bmsg : "");
                obj_extract_self(otmp2);
                /* pass off the otrapped flag to the next boulder */
                otmp2->otrapped = singleobj->otrapped;
                singleobj->otrapped = 0;
                place_object(singleobj, g.bhitpos.x, g.bhitpos.y);
                singleobj = otmp2;
                otmp2 = (struct obj *) 0;
                wake_nearto(g.bhitpos.x, g.bhitpos.y, 10 * 10);
            }
        }
        if (otyp == BOULDER && closed_door(g.bhitpos.x, g.bhitpos.y)) {
            if (!door_is_iron(&levl[g.bhitpos.x][g.bhitpos.y])) {
                if (cansee(g.bhitpos.x, g.bhitpos.y))
                    pline_The("boulder crashes through a door.");
                set_doorstate(&levl[g.bhitpos.x][g.bhitpos.y], D_BROKEN);
                if (dist)
                    unblock_point(g.bhitpos.x, g.bhitpos.y);
            }
            else {
                /* boulder hits iron door, stop the boulder */
                dist = 0;
            }
        }

        /* if about to hit iron bars, do so now */
        if (dist > 0 && isok(g.bhitpos.x + dx, g.bhitpos.y + dy)
            && levl[g.bhitpos.x + dx][g.bhitpos.y + dy].typ == IRONBARS) {
            x2 = g.bhitpos.x, y2 = g.bhitpos.y; /* object stops here */
            if (hits_bars(&singleobj, x2, y2, x2 + dx, y2 + dy, !rn2(20), 0)) {
                if (!singleobj) {
                    used_up = TRUE;
                    launch_drop_spot((struct obj *) 0, 0, 0);
                }
                break;
            }
        }
    }
    tmp_at(DISP_END, 0);
    launch_drop_spot((struct obj *) 0, 0, 0);
    if (!used_up) {
        singleobj->otrapped = 0;
        place_object(singleobj, x2, y2);
        newsym(x2, y2);
        return 1;
    } else
        return 2;
}

void
seetrap(trap)
struct trap *trap;
{
    if (!trap->tseen) {
        trap->tseen = 1;
        newsym(trap->tx, trap->ty);
    }
}

/* like seetrap() but overrides vision */
void
feeltrap(trap)
struct trap *trap;
{
    trap->tseen = 1;
    map_trap(trap, 1);
    /* in case it's beneath something, redisplay the something */
    newsym(trap->tx, trap->ty);
}

static int
mkroll_launch(ttmp, x, y, otyp, ocount)
struct trap *ttmp;
xchar x, y;
short otyp;
long ocount;
{
    struct obj *otmp;
    register int tmp;
    schar dx, dy;
    int distance;
    coord cc;
    coord bcc;
    int trycount = 0;
    boolean success = FALSE;
    int mindist = 4;

    if (ttmp->ttyp == ROLLING_BOULDER_TRAP)
        mindist = 2;
    distance = rn1(5, 4); /* 4..8 away */
    tmp = rn2(8);         /* randomly pick a direction to try first */
    while (distance >= mindist) {
        dx = xdir[tmp];
        dy = ydir[tmp];
        cc.x = x;
        cc.y = y;
        /* Prevent boulder from being placed on water */
        if (ttmp->ttyp == ROLLING_BOULDER_TRAP
            && is_pool_or_lava(x + distance * dx, y + distance * dy))
            success = FALSE;
        else
            success = isclearpath(&cc, distance, dx, dy);
        if (ttmp->ttyp == ROLLING_BOULDER_TRAP) {
            boolean success_otherway;

            bcc.x = x;
            bcc.y = y;
            success_otherway = isclearpath(&bcc, distance, -(dx), -(dy));
            if (!success_otherway)
                success = FALSE;
        }
        if (success)
            break;
        if (++tmp > 7)
            tmp = 0;
        if ((++trycount % 8) == 0)
            --distance;
    }
    if (!success) {
        /* create the trap without any ammo, launch pt at trap location */
        cc.x = bcc.x = x;
        cc.y = bcc.y = y;
    } else {
        otmp = mksobj(otyp, TRUE, FALSE);
        otmp->quan = ocount;
        otmp->owt = weight(otmp);
        place_object(otmp, cc.x, cc.y);
        stackobj(otmp);
    }
    ttmp->launch.x = cc.x;
    ttmp->launch.y = cc.y;
    if (ttmp->ttyp == ROLLING_BOULDER_TRAP) {
        ttmp->launch2.x = bcc.x;
        ttmp->launch2.y = bcc.y;
    } else
        ttmp->launch_otyp = otyp;
    newsym(ttmp->launch.x, ttmp->launch.y);
    return 1;
}

static boolean
isclearpath(cc, distance, dx, dy)
coord *cc;
int distance;
schar dx, dy;
{
    uchar typ;
    xchar x, y;

    x = cc->x;
    y = cc->y;
    while (distance-- > 0) {
        x += dx;
        y += dy;
        typ = levl[x][y].typ;
        if (!isok(x, y) || !ZAP_POS(typ) || closed_door(x, y))
            return FALSE;
    }
    cc->x = x;
    cc->y = y;
    return TRUE;
}

int
mintrap(mtmp)
register struct monst *mtmp;
{
    register struct trap *trap = t_at(mtmp->mx, mtmp->my);
    boolean trapkilled = FALSE;
    struct permonst *mptr = mtmp->data;
    struct obj *otmp;

    if (!trap) {
        mtmp->mtrapped = 0;      /* perhaps teleported? */
    } else if (mtmp->mtrapped) { /* is currently in the trap */
        if (!trap->tseen && cansee(mtmp->mx, mtmp->my) && canseemon(mtmp)
            && (is_pit(trap->ttyp) || trap->ttyp == BEAR_TRAP
                || trap->ttyp == HOLE
                || trap->ttyp == WEB)) {
            /* If you come upon an obviously trapped monster, then
             * you must be able to see the trap it's in too.
             */
            seetrap(trap);
        }

        if (!rn2(40)) {
            if (sobj_at(BOULDER, mtmp->mx, mtmp->my)
                && is_pit(trap->ttyp)) {
                if (!rn2(2)) {
                    mtmp->mtrapped = 0;
                    if (canseemon(mtmp))
                        pline("%s pulls free...", Monnam(mtmp));
                    fill_pit(mtmp->mx, mtmp->my);
                }
            } else {
                mtmp->mtrapped = 0;
            }
        } else if (metallivorous(mptr)) {
            if (trap->ttyp == BEAR_TRAP) {
                if (canseemon(mtmp))
                    pline("%s eats a bear trap!", Monnam(mtmp));
                deltrap_with_ammo(trap, DELTRAP_DESTROY_AMMO);
                mtmp->meating = 5;
                mtmp->mtrapped = 0;
            } else if (trap->ttyp == SPIKED_PIT) {
                if (canseemon(mtmp))
                    pline("%s munches on some spikes!", Monnam(mtmp));
                trap->ttyp = PIT;
                mtmp->meating = 5;
            }
        }
    } else {
        register int tt = trap->ttyp;
        boolean in_sight, tear_web, see_it,
                inescapable = (g.force_mintrap
                               || ((tt == HOLE || tt == PIT)
                                   && Sokoban && !trap->madeby_u));
        const char *fallverb;
        xchar tx = trap->tx, ty = trap->ty;

        /* true when called from dotrap, inescapable is not an option */
        if (mtmp == u.usteed)
            inescapable = TRUE;
        if (!inescapable && ((mtmp->mtrapseen & (1 << (tt - 1))) != 0
                             || (tt == HOLE && !mindless(mptr)))) {
            /* it has been in such a trap - perhaps it escapes */
            if (rn2(4))
                return 0;
        } else {
            mtmp->mtrapseen |= (1 << (tt - 1));
        }
        /* Monster is aggravated by being trapped by you.
           Recognizing who made the trap isn't completely
           unreasonable; everybody has their own style. */
        if (trap->madeby_u && rnl(5))
            setmangry(mtmp, TRUE);

        in_sight = canseemon(mtmp);
        see_it = cansee(mtmp->mx, mtmp->my);
        /* assume hero can tell what's going on for the steed */
        if (mtmp == u.usteed)
            in_sight = TRUE;
        switch (tt) {
        case ARROW_TRAP:
        case DART_TRAP:
            if (!trap->ammo) {
                if (in_sight && see_it)
                    pline("%s triggers a trap but nothing happens.",
                          Monnam(mtmp));
                deltrap(trap);
                newsym(mtmp->mx, mtmp->my);
                break;
            }
            otmp = trap->ammo;
            if (trap->ammo->quan > 1) {
                otmp = splitobj(trap->ammo, 1);
            }
            extract_nobj(otmp, &trap->ammo);
            if (in_sight)
                seetrap(trap);
            if (thitm(8, mtmp, otmp, 0, FALSE))
                trapkilled = TRUE;
            break;
        case ROCKTRAP:
            if (!trap->ammo) {
                if (in_sight && see_it)
                    pline("A trap door above %s opens, but nothing falls out!",
                          mon_nam(mtmp));
                deltrap(trap);
                newsym(mtmp->mx, mtmp->my);
                break;
            }
            otmp = trap->ammo;
            if (trap->ammo->quan > 1) {
                otmp = splitobj(trap->ammo, 1);
            }
            extract_nobj(otmp, &trap->ammo);
            if (in_sight)
                seetrap(trap);
            if (thitm(0, mtmp, otmp, d(2, 6), FALSE))
                trapkilled = TRUE;
            break;
        case SQKY_BOARD:
            if (!grounded(mptr))
                break;
            /* stepped on a squeaky board */
            if (in_sight) {
                if (!Deaf) {
                    if (Hallucination)
                        pline("%s accidentally steps on a squeaky toy!",
                              Monnam(mtmp));
                    else
                        pline("A board beneath %s squeaks %s loudly.",
                            mon_nam(mtmp), trapnote(trap, 0));
                    seetrap(trap);
                } else {
                    pline("%s stops momentarily and appears to cringe.",
                          Monnam(mtmp));
                }
            } else {
                /* same near/far threshold as mzapmsg() */
                int range = couldsee(mtmp->mx, mtmp->my) /* 9 or 5 */
                               ? (BOLT_LIM + 1) : (BOLT_LIM - 3);

                You_hear("a %s squeak %s.", trapnote(trap, 1),
                         (distu(mtmp->mx, mtmp->my) <= range * range)
                            ? "nearby" : "in the distance");
            }
            /* wake up nearby monsters */
            wake_nearto(mtmp->mx, mtmp->my, 40);
            break;
        case BEAR_TRAP:
            if (mptr->msize > MZ_SMALL && !amorphous(mptr) && grounded(mptr)
                && !is_whirly(mptr) && !unsolid(mptr)) {
                mtmp->mtrapped = 1;
                if (in_sight) {
                    pline("%s is caught in %s bear trap!", Monnam(mtmp),
                          a_your[trap->madeby_u]);
                    seetrap(trap);
                } else {
                    if (mptr == &mons[PM_OWLBEAR]
                        || mptr == &mons[PM_BUGBEAR])
                        You_hear("the roaring of an angry bear!");
                }
            } else if (g.force_mintrap) {
                if (in_sight) {
                    pline("%s evades %s bear trap!", Monnam(mtmp),
                          a_your[trap->madeby_u]);
                    seetrap(trap);
                }
            }
            if (mtmp->mtrapped)
                trapkilled = thitm(0, mtmp, (struct obj *) 0, d(2, 4), FALSE);
            break;
        case SLP_GAS_TRAP:
            if (!resists_sleep(mtmp) && !breathless(mptr) && !mtmp->msleeping
                && mtmp->mcanmove) {
                if (sleep_monst(mtmp, rnd(25), -1) && in_sight) {
                    pline("%s suddenly falls asleep!", Monnam(mtmp));
                    seetrap(trap);
                }
            }
            break;
        case RUST_TRAP: {
            struct obj *target;

            if (in_sight)
                seetrap(trap);
            switch (rn2(5)) {
            case 0:
                if (in_sight)
                    pline("%s %s on the %s!", A_gush_of_water_hits,
                          mon_nam(mtmp), mbodypart(mtmp, HEAD));
                target = which_armor(mtmp, W_ARMH);
                (void) water_damage(target, helm_simple_name(target), TRUE);
                break;
            case 1:
                if (in_sight)
                    pline("%s %s's left %s!", A_gush_of_water_hits,
                          mon_nam(mtmp), mbodypart(mtmp, ARM));
                target = which_armor(mtmp, W_ARMS);
                if (water_damage(target, "shield", TRUE) != ER_NOTHING)
                    break;
                target = MON_WEP(mtmp);
                if (target && bimanual(target))
                    (void) water_damage(target, 0, TRUE);
 glovecheck:
                target = which_armor(mtmp, W_ARMG);
                (void) water_damage(target, "gauntlets", TRUE);
                break;
            case 2:
                if (in_sight)
                    pline("%s %s's right %s!", A_gush_of_water_hits,
                          mon_nam(mtmp), mbodypart(mtmp, ARM));
                (void) water_damage(MON_WEP(mtmp), 0, TRUE);
                goto glovecheck;
            default:
                if (in_sight)
                    pline("%s %s!", A_gush_of_water_hits, mon_nam(mtmp));
                for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
                    if (otmp->lamplit
                        && (otmp->owornmask & (W_WEP | W_SWAPWEP)) == 0)
                        (void) snuff_lit(otmp);
                if ((target = which_armor(mtmp, W_ARMC)) != 0)
                    (void) water_damage(target, cloak_simple_name(target),
                                        TRUE);
                else if ((target = which_armor(mtmp, W_ARM)) != 0)
                    (void) water_damage(target, suit_simple_name(target),
                                        TRUE);
                else if ((target = which_armor(mtmp, W_ARMU)) != 0)
                    (void) water_damage(target, "shirt", TRUE);
            }

            if (mptr == &mons[PM_IRON_GOLEM]) {
                if (in_sight)
                    pline("%s falls to pieces!", Monnam(mtmp));
                else if (mtmp->mtame)
                    pline("May %s rust in peace.", mon_nam(mtmp));
                mondied(mtmp);
                if (DEADMONSTER(mtmp))
                    trapkilled = TRUE;
            } else if (mptr == &mons[PM_GREMLIN] && rn2(3)) {
                (void) split_mon(mtmp, (struct monst *) 0);
            }
            break;
        } /* RUST_TRAP */
        case FIRE_TRAP:
 mfiretrap:
            if (in_sight)
                pline("A %s erupts from the %s under %s!", tower_of_flame,
                      surface(mtmp->mx, mtmp->my), mon_nam(mtmp));
            else if (see_it) /* evidently `mtmp' is invisible */
                You_see("a %s erupt from the %s!", tower_of_flame,
                        surface(mtmp->mx, mtmp->my));

            if (resists_fire(mtmp)) {
                if (in_sight) {
                    shieldeff(mtmp->mx, mtmp->my);
                    pline("%s is uninjured.", Monnam(mtmp));
                }
            } else {
                int num = d(2, 4), alt;
                boolean immolate = FALSE;

                /* paper burns very fast, assume straw is tightly
                 * packed and burns a bit slower */
                switch (monsndx(mptr)) {
                case PM_PAPER_GOLEM:
                    immolate = TRUE;
                    alt = mtmp->mhpmax;
                    break;
                case PM_STRAW_GOLEM:
                    alt = mtmp->mhpmax / 2;
                    break;
                case PM_WOOD_GOLEM:
                    alt = mtmp->mhpmax / 4;
                    break;
                case PM_LEATHER_GOLEM:
                    alt = mtmp->mhpmax / 8;
                    break;
                default:
                    alt = 0;
                    break;
                }
                if (alt > num)
                    num = alt;

                if (thitm(0, mtmp, (struct obj *) 0, num, immolate))
                    trapkilled = TRUE;
                else
                    /* we know mhp is at least `num' below mhpmax,
                       so no (mhp > mhpmax) check is needed here */
                    mtmp->mhpmax -= rn2(num + 1);
            }
            if (burnarmor(mtmp) || rn2(3)) {
                (void) destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
                (void) destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
                (void) destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
                ignite_items(mtmp->minvent);
            }
            if (burn_floor_objects(mtmp->mx, mtmp->my, see_it, FALSE)
                && !see_it && distu(mtmp->mx, mtmp->my) <= 3 * 3)
                You("smell smoke.");
            if (is_ice(mtmp->mx, mtmp->my))
                melt_ice(mtmp->mx, mtmp->my, (char *) 0);
            if (see_it && t_at(mtmp->mx, mtmp->my))
                seetrap(trap);
            break;
        case PIT:
        case SPIKED_PIT:
            fallverb = "falls";
            if (!grounded(mptr) || (mtmp->wormno && count_wsegs(mtmp) > 5)) {
                if (g.force_mintrap && !Sokoban) {
                    /* openfallingtrap; not inescapable here */
                    if (in_sight) {
                        seetrap(trap);
                        pline("%s doesn't fall into the pit.", Monnam(mtmp));
                    }
                    break; /* inescapable = FALSE; */
                }
                if (!inescapable)
                    break;               /* avoids trap */
                fallverb = "is dragged"; /* sokoban pit */
            }
            if (!passes_walls(mptr))
                mtmp->mtrapped = 1;
            if (in_sight) {
                pline("%s %s into %s pit!", Monnam(mtmp), fallverb,
                      a_your[trap->madeby_u]);
                if (mptr == &mons[PM_PIT_VIPER]
                    || mptr == &mons[PM_PIT_FIEND])
                    pline("How pitiful.  Isn't that the pits?");
                seetrap(trap);
            }
            mselftouch(mtmp, "Falling, ", FALSE);
            if (DEADMONSTER(mtmp) || thitm(0, mtmp, (struct obj *) 0,
                                        rnd((tt == PIT) ? 6 : 10), FALSE))
                trapkilled = TRUE;
            break;
        case HOLE:
        case TRAPDOOR:
            if (!Can_fall_thru(&u.uz)) {
                impossible("mintrap: %ss cannot exist on this level.",
                           trapname(tt, TRUE));
                break; /* don't activate it after all */
            }
            if (!grounded(mptr) || (mtmp->wormno && count_wsegs(mtmp) > 5)
                || mptr->msize >= MZ_HUGE) {
                if (g.force_mintrap && !Sokoban) {
                    /* openfallingtrap; not inescapable here */
                    if (in_sight) {
                        seetrap(trap);
                        if (tt == TRAPDOOR)
                            pline(
                            "A trap door opens, but %s doesn't fall through.",
                                  mon_nam(mtmp));
                        else /* (tt == HOLE) */
                            pline("%s doesn't fall through the hole.",
                                  Monnam(mtmp));
                    }
                    break; /* inescapable = FALSE; */
                }
                if (inescapable) { /* sokoban hole */
                    if (in_sight) {
                        pline("%s seems to be yanked down!", Monnam(mtmp));
                        /* suppress message in mlevel_tele_trap() */
                        in_sight = FALSE;
                        seetrap(trap);
                    }
                } else
                    break;
            }
            /*FALLTHRU*/
        case LEVEL_TELEP:
        case MAGIC_PORTAL: {
            int mlev_res;

            mlev_res = mlevel_tele_trap(mtmp, trap, inescapable, in_sight);
            if (mlev_res)
                return mlev_res;
            break;
        }
        case TELEP_TRAP:
            mtele_trap(mtmp, trap, in_sight);
            break;
        case WEB:
            /* Monster in a web. */
            if (webmaker(mptr))
                break;
            if (mu_maybe_destroy_web(mtmp, in_sight, trap))
                break;
            tear_web = FALSE;
            switch (monsndx(mptr)) {
            case PM_OWLBEAR: /* Eric Backus */
            case PM_BUGBEAR:
                if (!in_sight) {
                    You_hear("the roaring of a confused bear!");
                    mtmp->mtrapped = 1;
                    break;
                }
                /*FALLTHRU*/
            default:
                if (mptr->mlet == S_GIANT
                    /* exclude baby dragons and relatively short worms */
                    || (mptr->mlet == S_DRAGON && extra_nasty(mptr))
                    || (mtmp->wormno && count_wsegs(mtmp) > 5)) {
                    tear_web = TRUE;
                } else if (in_sight) {
                    pline("%s is caught in %s spider web.", Monnam(mtmp),
                          a_your[trap->madeby_u]);
                    seetrap(trap);
                }
                mtmp->mtrapped = tear_web ? 0 : 1;
                break;
            /* this list is fairly arbitrary; it deliberately
               excludes wumpus & giant/ettin zombies/mummies */
            case PM_TITANOTHERE:
            case PM_BALUCHITHERIUM:
            case PM_PURPLE_WORM:
            case PM_JABBERWOCK:
            case PM_IRON_GOLEM:
            case PM_BALROG:
            case PM_KRAKEN:
            case PM_MASTODON:
            case PM_ORION:
            case PM_NORN:
            case PM_CYCLOPS:
            case PM_LORD_SURTUR:
                tear_web = TRUE;
                break;
            }
            if (tear_web) {
                if (in_sight)
                    pline("%s tears through %s spider web!", Monnam(mtmp),
                          a_your[trap->madeby_u]);
                deltrap(trap);
                newsym(mtmp->mx, mtmp->my);
            } else if (g.force_mintrap && !mtmp->mtrapped) {
                if (in_sight) {
                    pline("%s avoids %s spider web!", Monnam(mtmp),
                          a_your[trap->madeby_u]);
                    seetrap(trap);
                }
            }
            break;
        case STATUE_TRAP:
            break;
        case MAGIC_TRAP:
            /* A magic trap.  Monsters usually immune. */
            if (!rn2(21))
                goto mfiretrap;
            break;
        case ANTI_MAGIC:
            /* similar to hero's case, more or less */
            if (!resists_magm(mtmp)) { /* lose spell energy */
                if (!mtmp->mcan && (attacktype(mptr, AT_MAGC)
                                    || attacktype(mptr, AT_BREA))) {
                    mtmp->mspec_used += d(2, 2);
                    if (in_sight) {
                        seetrap(trap);
                        pline("%s seems lethargic.", Monnam(mtmp));
                    }
                }
            } else { /* take some damage */
                int dmgval2 = rnd(4);

                if ((otmp = MON_WEP(mtmp)) != 0
                    && otmp->oartifact == ART_MAGICBANE)
                    dmgval2 += rnd(4);
                for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
                    if (otmp->oartifact
                        && defends_when_carried(AD_MAGM, otmp))
                        break;
                if (otmp)
                    dmgval2 += rnd(4);
                if (passes_walls(mptr))
                    dmgval2 = (dmgval2 + 3) / 4;

                if (in_sight)
                    seetrap(trap);
                mtmp->mhp -= dmgval2;
                if (DEADMONSTER(mtmp))
                    monkilled(mtmp,
                              in_sight
                                  ? "compression from an anti-magic field"
                                  : (const char *) 0,
                              -AD_MAGM);
                if (DEADMONSTER(mtmp))
                    trapkilled = TRUE;
                if (see_it)
                    newsym(trap->tx, trap->ty);
            }
            break;
        case LANDMINE:
            if (rn2(3))
                break; /* monsters usually don't set it off */
            if (!grounded(mptr)) {
                boolean already_seen = trap->tseen;

                if (in_sight && !already_seen) {
                    pline("A trigger appears in a pile of soil below %s.",
                          mon_nam(mtmp));
                    seetrap(trap);
                }
                if (rn2(3))
                    break;
                if (in_sight) {
                    newsym(mtmp->mx, mtmp->my);
                    pline_The("air currents set %s off!",
                              already_seen ? "a land mine" : "it");
                }
            } else if (in_sight) {
                newsym(mtmp->mx, mtmp->my);
                pline("%s%s triggers %s land mine!",
                      !Deaf ? "KAABLAMM!!!  " : "", Monnam(mtmp),
                      a_your[trap->madeby_u]);
            }
            if (!in_sight && !Deaf)
                pline("Kaablamm!  %s an explosion in the distance!",
                      "You hear");  /* Deaf-aware */
            blow_up_landmine(trap);
            /* explosion might have destroyed a drawbridge; don't
               dish out more damage if monster is already dead */
            if (DEADMONSTER(mtmp)
                || thitm(0, mtmp, (struct obj *) 0, rnd(16), FALSE)) {
                trapkilled = TRUE;
            } else {
                /* monsters recursively fall into new pit */
                if (mintrap(mtmp) == 2)
                    trapkilled = TRUE;
            }
            /* a boulder may fill the new pit, crushing monster */
            fill_pit(tx, ty); /* thitm may have already destroyed the trap */
            if (DEADMONSTER(mtmp))
                trapkilled = TRUE;
            if (unconscious()) {
                g.multi = -1;
                g.nomovemsg = "The explosion awakens you!";
            }
            break;
        case POLY_TRAP:
            if (resists_magm(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
            } else if (!resist(mtmp, WAND_CLASS, 0, NOTELL)) {
                if (newcham(mtmp, (struct permonst *) 0, FALSE, TRUE)) {
                    /* we're done with mptr but keep it up to date */
                    mptr = mtmp->data;
                    deltrap(trap);
                }
            }
            break;
        case ROLLING_BOULDER_TRAP:
            if (grounded(mptr)) {
                int style = ROLL | (in_sight ? 0 : LAUNCH_UNSEEN);

                newsym(mtmp->mx, mtmp->my);
                if (in_sight)
                    pline("Click!  %s triggers %s.", Monnam(mtmp),
                          trap->tseen ? "a rolling boulder trap" : something);
                if (launch_obj(BOULDER, trap->launch.x, trap->launch.y,
                               trap->launch2.x, trap->launch2.y, style)) {
                    if (in_sight)
                        trap->tseen = TRUE;
                    if (DEADMONSTER(mtmp))
                        trapkilled = TRUE;
                } else {
                    deltrap(trap);
                    newsym(mtmp->mx, mtmp->my);
                }
            }
            break;
        case VIBRATING_SQUARE:
            if (see_it && !Blind) {
                seetrap(trap); /* before messages */
                if (in_sight) {
                    char buf[BUFSZ], *p, *monnm = mon_nam(mtmp);

                    if (nolimbs(mtmp->data)
                        || is_floater(mtmp->data) || is_flyer(mtmp->data)) {
                        /* just "beneath <mon>" */
                        Strcpy(buf, monnm);
                    } else {
                        Strcpy(buf, s_suffix(monnm));
                        p = eos(strcat(buf, " "));
                        Strcpy(p, makeplural(mbodypart(mtmp, FOOT)));
                        /* avoid "beneath 'rear paws'" or 'rear hooves' */
                        (void) strsubst(p, "rear ", "");
                    }
                    You_see("a strange vibration beneath %s.", buf);
                } else {
                    /* notice something (hearing uses a larger threshold
                       for 'nearby') */
                    You_see("the ground vibrate %s.",
                            (distu(mtmp->mx, mtmp->my) <= 2 * 2)
                               ? "nearby" : "in the distance");
                }
            }
            break;
        default:
            impossible("Some monster encountered a strange trap of type %d.",
                       tt);
        }
    }
    if (trapkilled)
        return 2;
    return mtmp->mtrapped;
}

/* Combine cockatrice checks into single functions to avoid repeating code. */
void
instapetrify(str)
const char *str;
{
    if (Stone_resistance)
        return;
    if (Hallucination) {
        /* This will follow messages such as "Touching a cockatrice corpse is a
         * fatal mistake" that imply the game is ending; contradict that */
        pline("But you are already stoned.");
        return;
    }
    if (poly_when_stoned(g.youmonst.data)
        && polymon(PM_STONE_GOLEM, POLYMON_ALL_MSGS))
        return;
    You("turn to stone...");
    g.killer.format = KILLED_BY;
    if (str != g.killer.name)
        Strcpy(g.killer.name, str ? str : "");
    done(STONING);
}

void
minstapetrify(mon, byplayer)
struct monst *mon;
boolean byplayer;
{
    if (resists_ston(mon))
        return;
    if (poly_when_stoned(mon->data)) {
        mon_to_stone(mon);
        return;
    }
    if (!vamp_stone(mon))
        return;

    /* give a "<mon> is slowing down" message and also remove
       intrinsic speed (comparable to similar effect on the hero) */
    mon_adjust_speed(mon, -3, (struct obj *) 0);

    if (cansee(mon->mx, mon->my))
        pline("%s turns to stone.", Monnam(mon));
    if (byplayer) {
        g.stoned = TRUE;
        xkilled(mon, XKILL_NOMSG);
    } else
        monstone(mon);
}

void
selftouch(arg)
const char *arg;
{
    char kbuf[BUFSZ];

    if (uwep && uwep->otyp == CORPSE && touch_petrifies(&mons[uwep->corpsenm])
        && !Stone_resistance) {
        pline("%s touch the %s corpse.", arg, mons[uwep->corpsenm].mname);
        Sprintf(kbuf, "%s corpse", an(mons[uwep->corpsenm].mname));
        instapetrify(kbuf);
        /* life-saved; unwield the corpse if we can't handle it */
        if (!uarmg && !Stone_resistance && !Hallucination)
            uwepgone();
    }
    /* Or your secondary weapon, if wielded [hypothetical; we don't
       allow two-weapon combat when either weapon is a corpse] */
    if (u.twoweap && uswapwep && uswapwep->otyp == CORPSE
        && touch_petrifies(&mons[uswapwep->corpsenm]) && !Stone_resistance) {
        pline("%s touch the %s corpse.", arg, mons[uswapwep->corpsenm].mname);
        Sprintf(kbuf, "%s corpse", an(mons[uswapwep->corpsenm].mname));
        instapetrify(kbuf);
        /* life-saved; unwield the corpse */
        if (!uarmg && !Stone_resistance && !Hallucination)
            uswapwepgone();
    }
}

void
mselftouch(mon, arg, byplayer)
struct monst *mon;
const char *arg;
boolean byplayer;
{
    struct obj *mwep = MON_WEP(mon);

    if (mwep && mwep->otyp == CORPSE && touch_petrifies(&mons[mwep->corpsenm])
        && !resists_ston(mon)) {
        if (cansee(mon->mx, mon->my)) {
            pline("%s%s touches %s.", arg ? arg : "",
                  arg ? mon_nam(mon) : Monnam(mon),
                  corpse_xname(mwep, (const char *) 0, CXN_PFX_THE));
        }
        minstapetrify(mon, byplayer);
        /* if life-saved, might not be able to continue wielding */
        if (!DEADMONSTER(mon)
            && !which_armor(mon, W_ARMG) && !resists_ston(mon))
            mwepgone(mon);
    }
}

/* start levitating */
void
float_up()
{
    g.context.botl = TRUE;
    if (u.utrap) {
        if (u.utraptype == TT_PIT) {
            reset_utrap(FALSE);
            You("float up, out of the %s!", trapname(PIT, FALSE));
            g.vision_full_recalc = 1; /* vision limits change */
            fill_pit(u.ux, u.uy);
        } else if (u.utraptype == TT_LAVA /* molten lava */
                   || u.utraptype == TT_INFLOOR) { /* solidified lava */
            Your("body pulls upward, but your %s are still stuck.",
                 makeplural(body_part(LEG)));
        } else if (u.utraptype == TT_BURIEDBALL) { /* tethered */
            coord cc;

            cc.x = u.ux, cc.y = u.uy;
            /* caveat: this finds the first buried iron ball within
               one step of the specified location, not necessarily the
               buried [former] uball at the original anchor point */
            (void) buried_ball(&cc);
            /* being chained to the floor blocks levitation from floating
               above that floor but not from enhancing carrying capacity */
            You("feel lighter, but your %s is still chained to the %s.",
                body_part(LEG),
                IS_ROOM(levl[cc.x][cc.y].typ) ? "floor" : "ground");
        } else if (u.utraptype == WEB) {
            You("float up slightly, but you are still stuck in the %s.",
                trapname(WEB, FALSE));
        } else { /* bear trap */
            You("float up slightly, but your %s is still stuck.",
                body_part(LEG));
        }
        /* when still trapped, float_vs_flight() below will block levitation */
#if 0
    } else if (Is_waterlevel(&u.uz)) {
        pline("It feels as though you've lost some weight.");
#endif
    } else if (u.uinwater) {
        spoteffects(TRUE);
    } else if (u.uswallow) {
        You(is_animal(u.ustuck->data) ? "float away from the %s."
                                      : "spiral up into %s.",
            is_animal(u.ustuck->data) ? surface(u.ux, u.uy)
                                      : mon_nam(u.ustuck));
    } else if (Hallucination) {
        pline("Up, up, and awaaaay!  You're walking on air!");
    } else if (Is_airlevel(&u.uz)) {
        You("gain control over your movements.");
    } else {
        You("start to float in the air!");
    }
    if (u.usteed && !is_floater(u.usteed->data) && !is_flyer(u.usteed->data)) {
        if (Lev_at_will) {
            pline("%s magically floats up!", Monnam(u.usteed));
        } else {
            You("cannot stay on %s.", mon_nam(u.usteed));
            dismount_steed(DISMOUNT_GENERIC);
        }
    }
    if (Flying)
        You("are no longer able to control your flight.");
    float_vs_flight(); /* set BFlying, also BLevitation if still trapped */
    /* levitation gives maximum carrying capacity, so encumbrance
       state might be reduced */
    (void) encumber_msg();
    return;
}

void
fill_pit(x, y)
int x, y;
{
    struct obj *otmp;
    struct trap *t;

    if ((t = t_at(x, y)) != 0 && is_pit(t->ttyp)
        && (otmp = sobj_at(BOULDER, x, y)) != 0) {
        obj_extract_self(otmp);
        (void) flooreffects(otmp, x, y, "settle");
    }
}

/* stop levitating */
int
float_down(hmask, emask)
long hmask, emask; /* might cancel timeout */
{
    register struct trap *trap = (struct trap *) 0;
    d_level current_dungeon_level;
    boolean no_msg = FALSE;

    HLevitation &= ~hmask;
    ELevitation &= ~emask;
    if (Levitation)
        return 0; /* maybe another ring/potion/boots */
    if (BLevitation) {
        /* if blocked by terrain, we haven't actually been levitating so
           we don't give any end-of-levitation feedback or side-effects,
           but if blocking is solely due to being trapped in/on floor,
           do give some feedback but skip other float_down() effects */
        boolean trapped = (BLevitation == I_SPECIAL);

        float_vs_flight();
        if (trapped && u.utrap) /* u.utrap => paranoia */
            You("are no longer trying to float up from the %s.",
                (u.utraptype == TT_BEARTRAP) ? "trap's jaws"
                  : (u.utraptype == TT_WEB) ? "web"
                      : (u.utraptype == TT_BURIEDBALL) ? "chain"
                          : (u.utraptype == TT_LAVA) ? "lava"
                              : "ground"); /* TT_INFLOOR */
        (void) encumber_msg(); /* carrying capacity might have changed */
        return 0;
    }
    g.context.botl = TRUE;
    nomul(0); /* stop running or resting */
    if (BFlying) {
        /* controlled flight no longer overridden by levitation */
        float_vs_flight(); /* clears BFlying & I_SPECIAL
                            * unless hero is stuck in floor */
        if (Flying) {
            You("have stopped levitating and are now flying.");
            (void) encumber_msg(); /* carrying capacity might have changed */
            return 1;
        }
    }
    if (u.uswallow) {
        You("float down, but you are still %s.",
            is_animal(u.ustuck->data) ? "swallowed" : "engulfed");
        (void) encumber_msg();
        return 1;
    }

    if (Punished && !carried(uball)
        && (is_pool(uball->ox, uball->oy)
            || ((trap = t_at(uball->ox, uball->oy))
                && (is_pit(trap->ttyp) || is_hole(trap->ttyp))))) {
        u.ux0 = u.ux;
        u.uy0 = u.uy;
        u.ux = uball->ox;
        u.uy = uball->oy;
        movobj(uchain, uball->ox, uball->oy);
        newsym(u.ux0, u.uy0);
        g.vision_full_recalc = 1; /* in case the hero moved. */
    }
    /* check for falling into pool - added by GAN 10/20/86 */
    if (!Flying) {
        if (!u.uswallow && u.ustuck) {
            if (sticks(g.youmonst.data))
                You("aren't able to maintain your hold on %s.",
                    mon_nam(u.ustuck));
            else
                pline("Startled, %s can no longer hold you!",
                      mon_nam(u.ustuck));
            set_ustuck((struct monst *) 0);
        }
        /* kludge alert:
         * drown() and lava_effects() print various messages almost
         * every time they're called which conflict with the "fall
         * into" message below.  Thus, we want to avoid printing
         * confusing, duplicate or out-of-order messages.
         * Use knowledge of the two routines as a hack -- this
         * should really be handled differently -dlc
         */
        if (is_pool(u.ux, u.uy) && !Swimming && !u.uinwater)
            no_msg = drown();

        if (is_lava(u.ux, u.uy)) {
            (void) lava_effects();
            no_msg = TRUE;
        }
    }
    if (!trap) {
        trap = t_at(u.ux, u.uy);
        if (Is_airlevel(&u.uz)) {
            You("begin to tumble in place.");
        } else if (Is_waterlevel(&u.uz) && !no_msg) {
            You_feel("heavier.");
        /* u.uinwater msgs already in spoteffects()/drown() */
        } else if (!u.uinwater && !no_msg) {
            if (!(emask & W_SADDLE)) {
                if (Sokoban && trap) {
                    /* Justification elsewhere for Sokoban traps is based
                     * on air currents.  This is consistent with that.
                     * The unexpected additional force of the air currents
                     * once levitation ceases knocks you off your feet.
                     */
                    if (Hallucination)
                        pline("Bummer!  You've crashed.");
                    else
                        You("fall over.");
                    losehp(rnd(2), "dangerous winds", KILLED_BY);
                    if (u.usteed)
                        dismount_steed(DISMOUNT_FELL);
                    selftouch("As you fall, you");
                } else if (u.usteed && (is_floater(u.usteed->data)
                                        || is_flyer(u.usteed->data))) {
                    You("settle more firmly in the saddle.");
                } else if (Hallucination) {
                    pline("Bummer!  You've %s.",
                          is_pool(u.ux, u.uy)
                             ? "splashed down"
                             : "hit the ground");
                } else {
                    You("float gently to the %s.", surface(u.ux, u.uy));
                }
            }
        }
    }

    /* levitation gives maximum carrying capacity, so having it end
       potentially triggers greater encumbrance; do this after
       'come down' messages, before trap activation or autopickup */
    (void) encumber_msg();

    /* can't rely on u.uz0 for detecting trap door-induced level change;
       it gets changed to reflect the new level before we can check it */
    assign_level(&current_dungeon_level, &u.uz);
    if (trap) {
        switch (trap->ttyp) {
        case STATUE_TRAP:
            break;
        case HOLE:
        case TRAPDOOR:
            if (!Can_fall_thru(&u.uz) || u.ustuck)
                break;
            /*FALLTHRU*/
        default:
            if (!u.utrap) /* not already in the trap */
                dotrap(trap, 0);
        }
    }
    if (!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz) && !u.uswallow
        /* falling through trap door calls goto_level,
           and goto_level does its own pickup() call */
        && on_level(&u.uz, &current_dungeon_level))
        (void) pickup(1);
    return 1;
}

/* shared code for climbing out of a pit */
void
climb_pit()
{
    const char *pitname;

    if (!u.utrap || u.utraptype != TT_PIT)
        return;

    pitname = trapname(PIT, FALSE);
    if (Passes_walls) {
        /* marked as trapped so they can pick things up */
        You("ascend from the %s.", pitname);
        reset_utrap(FALSE);
        fill_pit(u.ux, u.uy);
        g.vision_full_recalc = 1; /* vision limits change */
    } else if (!rn2(2) && sobj_at(BOULDER, u.ux, u.uy)) {
        Your("%s gets stuck in a crevice.", body_part(LEG));
        display_nhwindow(WIN_MESSAGE, FALSE);
        clear_nhwindow(WIN_MESSAGE);
        You("free your %s.", body_part(LEG));
    } else if ((Flying || is_clinger(g.youmonst.data)) && !Sokoban) {
        /* eg fell in pit, then poly'd to a flying monster;
           or used '>' to deliberately enter it */
        You("%s from the %s.", Flying ? "fly" : "climb", pitname);
        reset_utrap(FALSE);
        fill_pit(u.ux, u.uy);
        g.vision_full_recalc = 1; /* vision limits change */
    } else if (!(--u.utrap)) {
        reset_utrap(FALSE);
        You("%s to the edge of the %s.",
            (Sokoban && Levitation)
                ? "struggle against the air currents and float"
                : u.usteed ? "ride" : "crawl",
            pitname);
        fill_pit(u.ux, u.uy);
        g.vision_full_recalc = 1; /* vision limits change */
    } else if (u.dz || flags.verbose) {
        /* these should use 'pitname' rather than "pit" for hallucination
           but that would nullify Norep (this message can be repeated
           many times without further user intervention by using a run
           attempt to keep retrying to escape from the pit) */
        if (u.usteed)
            Norep("%s is still in a pit.", upstart(y_monnam(u.usteed)));
        else
            Norep((Hallucination && !rn2(5))
                      ? "You've fallen, and you can't get up."
                      : "You are still in a pit.");
    }
}

static void
dofiretrap(box)
struct obj *box; /* null for floor trap */
{
    boolean see_it = !Blind;
    int num, alt;

    /* Bug: for box case, the equivalent of burn_floor_objects() ought
     * to be done upon its contents.
     */

    if ((box && !carried(box)) ? is_pool(box->ox, box->oy) : Underwater) {
        pline("A cascade of steamy bubbles erupts from %s!",
              the(box ? xname(box) : surface(u.ux, u.uy)));
        if (Fire_resistance)
            You("are uninjured.");
        else
            losehp(rnd(3), "boiling water", KILLED_BY);
        return;
    }
    pline("A %s %s from %s!", tower_of_flame, box ? "bursts" : "erupts",
          the(box ? xname(box) : surface(u.ux, u.uy)));
    if (Fire_resistance) {
        shieldeff(u.ux, u.uy);
        num = rn2(2);
    } else if (Upolyd) {
        num = d(2, 4);
        switch (u.umonnum) {
        case PM_PAPER_GOLEM:
            alt = u.mhmax;
            break;
        case PM_STRAW_GOLEM:
            alt = u.mhmax / 2;
            break;
        case PM_WOOD_GOLEM:
            alt = u.mhmax / 4;
            break;
        case PM_LEATHER_GOLEM:
            alt = u.mhmax / 8;
            break;
        default:
            alt = 0;
            break;
        }
        if (alt > num)
            num = alt;
        if (u.mhmax > mons[u.umonnum].mlevel)
            u.mhmax -= rn2(min(u.mhmax, num + 1)), g.context.botl = 1;
    } else {
        num = d(2, 4);
        if (u.uhpmax > u.ulevel)
            u.uhpmax -= rn2(min(u.uhpmax, num + 1)), g.context.botl = 1;
    }
    if (!num)
        You("are uninjured.");
    else
        losehp(num, tower_of_flame, KILLED_BY_AN); /* fire damage */
    burn_away_slime();

    if (burnarmor(&g.youmonst) || rn2(3)) {
        destroy_item(SCROLL_CLASS, AD_FIRE);
        destroy_item(SPBOOK_CLASS, AD_FIRE);
        destroy_item(POTION_CLASS, AD_FIRE);
        ignite_items(g.invent);
    }
    if (!box && burn_floor_objects(u.ux, u.uy, see_it, TRUE) && !see_it)
        You("smell paper burning.");
    if (is_ice(u.ux, u.uy))
        melt_ice(u.ux, u.uy, (char *) 0);
}

static void
domagictrap()
{
    register int fate = rnd(20);

    /* What happened to the poor sucker? */

    if (fate < 10) {
        /* Most of the time, it creates some monsters. */
        int cnt = rnd(4);

        /* blindness effects */
        if (!resists_blnd(&g.youmonst)) {
            You("are momentarily blinded by a flash of light!");
            make_blinded((long) rn1(5, 10), FALSE);
            if (!Blind)
                Your1(vision_clears);
        } else if (!Blind) {
            You_see("a flash of light!");
        }

        /* deafness effects */
        if (!Deaf) {
            You_hear("a deafening roar!");
            incr_itimeout(&HDeaf, rn1(20, 30));
            g.context.botl = TRUE;
        } else {
            /* magic vibrations still hit you */
            You_feel("rankled.");
            incr_itimeout(&HDeaf, rn1(5, 15));
            g.context.botl = TRUE;
        }
        while (cnt--)
            (void) makemon((struct permonst *) 0, u.ux, u.uy, NO_MM_FLAGS);
        /* roar: wake monsters in vicinity, after placing trap-created ones */
        wake_nearto(u.ux, u.uy, 7 * 7);
        /* [flash: should probably also hit nearby gremlins with light] */
    } else {
        switch (fate) {
        case 10:
        case 11:
            /* sometimes nothing happens */
            break;
        case 12: /* a flash of fire */
            dofiretrap((struct obj *) 0);
            break;

        /* odd feelings */
        case 13:
            pline("A shiver runs up and down your %s!", body_part(SPINE));
            break;
        case 14:
            You_hear(Hallucination ? "the moon howling at you."
                                   : "distant howling.");
            break;
        case 15:
            if (on_level(&u.uz, &qstart_level))
                You_feel(
                    "%slike the prodigal son.",
                    (flags.female || (Upolyd && is_neuter(g.youmonst.data)))
                        ? "oddly "
                        : "");
            else if (Role_if(PM_VALKYRIE) && Hallucination)
                You("pine for the fjords.");
            else
                You("suddenly yearn for %s.",
                    Hallucination
                        ? "Cleveland"
                        : (In_quest(&u.uz) || at_dgn_entrance("The Quest"))
                              ? "your nearby homeland"
                              : "your distant homeland");
            break;
        case 16:
            Your("pack shakes violently!");
            break;
        case 17:
            You(Hallucination ? "smell hamburgers." : "smell charred flesh.");
            break;
        case 18:
            You_feel("tired.");
            break;

        /* very occasionally something nice happens. */
        case 19: { /* tame nearby monsters */
            int i, j;
            struct monst *mtmp;

            (void) adjattrib(A_CHA, 1, FALSE);
            for (i = -1; i <= 1; i++)
                for (j = -1; j <= 1; j++) {
                    if (!isok(u.ux + i, u.uy + j))
                        continue;
                    mtmp = m_at(u.ux + i, u.uy + j);
                    if (mtmp)
                        (void) tamedog(mtmp, (struct obj *) 0, TRUE);
                }
            break;
        }
        case 20: { /* uncurse stuff */
            struct obj pseudo;
            long save_conf = HConfusion;

            pseudo = cg.zeroobj; /* neither cursed nor blessed,
                                    and zero out oextra */
            pseudo.otyp = SCR_REMOVE_CURSE;
            HConfusion = 0L;
            (void) seffects(&pseudo);
            HConfusion = save_conf;
            break;
        }
        default:
            break;
        }
    }
}

/* Set an item on fire.
 *   "force" means not to roll a luck-based protection check for the
 *     item.
 *   "x" and "y" are the coordinates to dump the contents of a
 *     container, if it burns up.
 *
 * Return whether the object was destroyed.
 */
boolean
fire_damage(obj, force, x, y)
struct obj *obj;
boolean force;
xchar x, y;
{
    int chance;
    struct obj *otmp, *ncobj;
    int in_sight = !Blind && couldsee(x, y); /* Don't care if it's lit */
    int dindx;

    /* object might light in a controlled manner */
    if (catch_lit(obj))
        return FALSE;

    if (Is_container(obj)) {
        if (!is_flammable(obj)) {
            return FALSE; /* immune */
        }
        switch (obj->otyp) {
        case ICE_BOX:
            return FALSE; /* Immune */
        case CHEST:
            chance = 40;
            break;
        case LARGE_BOX:
            chance = 30;
            break;
        default:
            chance = 20;
            break;
        }
        if ((!force && (Luck + 5) > rn2(chance))
            || (is_flammable(obj) && obj->oerodeproof))
            return FALSE;
        /* Container is burnt up - dump contents out */
        if (in_sight)
            pline("%s catches fire and burns.", Yname2(obj));
        if (Has_contents(obj)) {
            if (in_sight)
                pline("Its contents fall out.");
            for (otmp = obj->cobj; otmp; otmp = ncobj) {
                ncobj = otmp->nobj;
                obj_extract_self(otmp);
                if (!flooreffects(otmp, x, y, ""))
                    place_object(otmp, x, y);
            }
        }
        setnotworn(obj);
        delobj(obj);
        return TRUE;
    } else if (!force && (Luck + 5) > rn2(20)) {
        /*  chance per item of sustaining damage:
          *     max luck (Luck==13):    10%
          *     avg luck (Luck==0):     75%
          *     awful luck (Luck<-4):  100%
          */
        return FALSE;
    } else if (obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS) {
        if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL)
            return FALSE;
        if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
            if (in_sight)
                pline("Smoke rises from %s.", the(xname(obj)));
            return FALSE;
        }
        dindx = (obj->oclass == SCROLL_CLASS) ? 3 : 4;
        if (in_sight)
            pline("%s %s.", Yname2(obj),
                  destroy_strings[dindx][(obj->quan > 1L)]);
        setnotworn(obj);
        delobj(obj);
        return TRUE;
    } else if (obj->oclass == POTION_CLASS) {
        dindx = (obj->otyp != POT_OIL) ? 1 : 2;
        if (in_sight)
            pline("%s %s.", Yname2(obj),
                  destroy_strings[dindx][(obj->quan > 1L)]);
        setnotworn(obj);
        delobj(obj);
        return TRUE;
    } else if (erode_obj(obj, (char *) 0, ERODE_BURN, EF_DESTROY)
               == ER_DESTROYED) {
        return TRUE;
    }
    return FALSE;
}

/*
 * Apply fire_damage() to an entire chain.
 *
 * Return number of objects destroyed. --ALI
 */
int
fire_damage_chain(chain, force, here, x, y)
struct obj *chain;
boolean force, here;
xchar x, y;
{
    struct obj *obj, *nobj;
    int num = 0;

    /* erode_obj() relies on bhitpos if target objects aren't carried by
       the hero or a monster, to check visibility controlling feedback */
    g.bhitpos.x = x, g.bhitpos.y = y;

    for (obj = chain; obj; obj = nobj) {
        nobj = here ? obj->nexthere : obj->nobj;
        if (fire_damage(obj, force, x, y))
            ++num;
    }

    if (num && (Blind && !couldsee(x, y)))
        You("smell smoke.");
    return num;
}

/* obj has been thrown or dropped into lava; damage is worse than mere fire */
boolean
lava_damage(obj, x, y)
struct obj *obj;
xchar x, y;
{
    int otyp = obj->otyp, ocls = obj->oclass;

    /* the Amulet, invocation items, and Rider corpses are never destroyed
       (let Book of the Dead fall through to fire_damage() to get feedback) */
    if (obj_resists(obj, 0, 0) && otyp != SPE_BOOK_OF_THE_DEAD)
        return FALSE;
    /* destroy liquid (venom), wax, veggy, flesh, paper (except for scrolls
       and books--let fire damage deal with them), cloth, leather, wood, bone
       unless it's inherently or explicitly fireproof or contains something;
       note: potions are glass so fall through to fire_damage() and boil */
    if (obj->material < DRAGON_HIDE
        && ocls != SCROLL_CLASS && ocls != SPBOOK_CLASS
        && objects[otyp].oc_oprop != FIRE_RES
        && otyp != WAN_FIRE && otyp != FIRE_HORN
        /* assumes oerodeproof isn't overloaded for some other purpose on
           non-eroding items */
        && !obj->oerodeproof
        /* fire_damage() knows how to deal with containers and contents */
        && !Has_contents(obj)) {
        if (cansee(x, y)) {
            /* this feedback is pretty clunky and can become very verbose
               when former contents of a burned container get here via
               flooreffects() */
            if (obj == g.thrownobj || obj == g.kickedobj)
                pline("%s %s up!", is_plural(obj) ? "They" : "It",
                      otense(obj, "burn"));
            else
                You_see("%s hit lava and burn up!", doname(obj));
        }
        if (carried(obj)) { /* shouldn't happen */
            remove_worn_item(obj, TRUE);
            useupall(obj);
        } else
            delobj(obj);
        return TRUE;
    }
    return fire_damage(obj, TRUE, x, y);
}

void
acid_damage(obj)
struct obj *obj;
{
    /* Scrolls but not spellbooks can be erased by acid. */
    struct monst *victim;
    boolean vismon;

    if (!obj)
        return;

    victim = carried(obj) ? &g.youmonst : mcarried(obj) ? obj->ocarry : NULL;
    vismon = victim && (victim != &g.youmonst) && canseemon(victim);

    if (obj->greased) {
        grease_protect(obj, (char *) 0, victim);
    } else if (obj->oclass == SCROLL_CLASS && obj->otyp != SCR_BLANK_PAPER) {
        if (obj->otyp != SCR_BLANK_PAPER
#ifdef MAIL_STRUCTURES
            && obj->otyp != SCR_MAIL
#endif
            ) {
            if (!Blind) {
                if (victim == &g.youmonst)
                    pline("Your %s.", aobjnam(obj, "fade"));
                else if (vismon)
                    pline("%s %s.", s_suffix(Monnam(victim)),
                          aobjnam(obj, "fade"));
            }
        }
        obj->otyp = SCR_BLANK_PAPER;
        obj->spe = 0;
        obj->dknown = 0;
    } else
        erode_obj(obj, (char *) 0, ERODE_CORRODE, EF_GREASE | EF_VERBOSE);
}

/* Get an object wet and damage it appropriately.
 *   "ostr", if present, is used instead of the object name in some
 *     messages.
 *   "force" means not to roll luck to protect some objects.
 * Returns an erosion return value (ER_*)
 */
int
water_damage(obj, ostr, force)
struct obj *obj;
const char *ostr;
boolean force;
{
    if (!obj)
        return ER_NOTHING;

    if (snuff_lit(obj))
        return ER_DAMAGED;

    if (!ostr)
        ostr = cxname(obj);

    if (obj->otyp == CAN_OF_GREASE && obj->spe > 0) {
        return ER_NOTHING;
    } else if (obj->otyp == TOWEL && obj->spe < 7) {
        wet_a_towel(obj, rnd(7), TRUE);
        return ER_NOTHING;
    } else if (obj->greased) {
        if (!rn2(2))
            obj->greased = 0;
        if (carried(obj))
            update_inventory();
        return ER_GREASED;
    } else if (Is_container(obj) && !Is_box(obj)
               && (obj->otyp != OILSKIN_SACK || (obj->cursed && !rn2(3)))) {
        if (carried(obj))
            pline("Water gets into your %s!", ostr);

        /* assume that if we're getting water into a container, further water
         * damage should also get inside nested containers */
        water_damage_chain(obj->cobj, FALSE, 0, TRUE);
        return ER_DAMAGED; /* contents were damaged */
    } else if (obj->otyp == OILSKIN_SACK) {
        if (carried(obj))
            pline("Some water slides right off your %s.", ostr);
        makeknown(OILSKIN_SACK);
        /* not actually damaged, but because we /didn't/ get the "water
           gets into!" message, the player now has more information and
           thus we need to waste any potion they may have used (also,
           flavourwise the water is now on the floor) */
        return ER_DAMAGED;
    } else if (!force && (Luck + 5) > rn2(20)) {
        /*  chance per item of sustaining damage:
            *   max luck:               10%
            *   avg luck (Luck==0):     75%
            *   awful luck (Luck<-4):  100%
            */
        return ER_NOTHING;
    } else if (obj->oclass == SCROLL_CLASS) {
        if (obj->otyp == SCR_BLANK_PAPER || obj->otyp == SCR_WATER
#ifdef MAIL_STRUCTURES
            || obj->otyp == SCR_MAIL
#endif
           ) return 0;
        if (carried(obj))
            pline("Your %s %s.", ostr, vtense(ostr, "fade"));

        obj->otyp = SCR_BLANK_PAPER;
        obj->dknown = 0;
        obj->spe = 0;
        if (carried(obj))
            update_inventory();
        return ER_DAMAGED;
    } else if (obj->oclass == SPBOOK_CLASS) {
        if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
            pline("Steam rises from %s.", the(xname(obj)));
            return 0;
        } else if (obj->otyp == SPE_BLANK_PAPER) {
            return 0;
        }
        if (carried(obj))
            pline("Your %s %s.", ostr, vtense(ostr, "fade"));

        if (obj->otyp == SPE_NOVEL) {
            obj->novelidx = 0;
            free_oname(obj);
        }

        obj->otyp = SPE_BLANK_PAPER;
        obj->dknown = 0;
        if (carried(obj))
            update_inventory();
        return ER_DAMAGED;
    } else if (obj->oclass == POTION_CLASS) {
        if (obj->otyp == POT_ACID) {
            char *bufp;
            boolean one = (obj->quan == 1L), update = carried(obj),
                    exploded = FALSE;

            if (Blind && !carried(obj))
                obj->dknown = 0;
            if (g.acid_ctx.ctx_valid)
                exploded = ((obj->dknown ? g.acid_ctx.dkn_boom
                                         : g.acid_ctx.unk_boom) > 0);
            /* First message is
             * "a [potion|<color> potion|potion of acid] explodes"
             * depending on obj->dknown (potion has been seen) and
             * objects[POT_ACID].oc_name_known (fully discovered),
             * or "some {plural version} explode" when relevant.
             * Second and subsequent messages for same chain and
             * matching dknown status are
             * "another [potion|<color> &c] explodes" or plural
             * variant.
             */
            bufp = simpleonames(obj);
            pline("%s %s %s!", /* "A potion explodes!" */
                  !exploded ? (one ? "A" : "Some")
                            : (one ? "Another" : "More"),
                  bufp, vtense(bufp, "explode"));
            if (g.acid_ctx.ctx_valid) {
                if (obj->dknown)
                    g.acid_ctx.dkn_boom++;
                else
                    g.acid_ctx.unk_boom++;
            }
            if (obj->dknown)
                makeknown(POT_ACID);
            setnotworn(obj);
            delobj(obj);
            if (update)
                update_inventory();
            return ER_DESTROYED;
        } else if (obj->odiluted) {
            if (carried(obj))
                pline("Your %s %s further.", ostr, vtense(ostr, "dilute"));

            obj->otyp = POT_WATER;
            obj->dknown = 0;
            obj->blessed = obj->cursed = 0;
            obj->odiluted = 0;
            if (carried(obj))
                update_inventory();
            return ER_DAMAGED;
        } else if (obj->otyp == POT_OIL) {
            pline("The water doesn't seem to mix with your %s.", ostr);
            makeknown(POT_OIL);
        } else if (obj->otyp != POT_WATER) {
            if (carried(obj))
                pline("Your %s %s.", ostr, vtense(ostr, "dilute"));

            obj->odiluted++;
            if (carried(obj))
                update_inventory();
            return ER_DAMAGED;
        }
    } else {
        return erode_obj(obj, ostr, ERODE_RUST, EF_NONE);
    }
    return ER_NOTHING;
}

/* Apply water damage to a chain (inventory, floor, monster inventory) of
 * objects.
 * here is TRUE if the chain represents objects on the floor.
 * count, if positive, is the number of items that will be damaged by
 * this function.
 * If trying to wet everything (e.g. falling into water), set count < 1.
 */
void
water_damage_chain(obj, here, count, do_container)
struct obj *obj;
boolean here;
int count;
boolean do_container;
{
    struct obj *otmp = obj;
    struct obj *nobj;
    int i = 0, j;
    struct obj** to_damage = NULL;
    xchar x, y;

    if (!obj)
        return;

    if (count >= 1) {
        /* reservoir sampling: setup */
        to_damage = (struct obj**) alloc(sizeof(struct obj*) * count);
        for (otmp = obj; otmp; otmp = (here ? otmp->nexthere : otmp->nobj)) {
            if (!do_container && Is_container(otmp))
                continue;
            to_damage[i] = otmp;
            i++;
            if (i == count)
                break;
        }
        if (i < count) {
            /* fewer than count items in chain, so just damage everything */
            free((genericptr_t) to_damage);
            count = -1; /* let the rest of this function handle it */
        }
    }

    /* initialize acid context: so far, neither seen (dknown) potions of
       acid nor unseen have exploded during this water damage sequence */
    g.acid_ctx.dkn_boom = g.acid_ctx.unk_boom = 0;
    g.acid_ctx.ctx_valid = TRUE;

    /* erode_obj() relies on bhitpos if target objects aren't carried by
       the hero or a monster, to check visibility controlling feedback */
    if (get_obj_location(obj, &x, &y, CONTAINED_TOO))
        g.bhitpos.x = x, g.bhitpos.y = y;

    for (otmp = obj; otmp; otmp = nobj) {
        /* if acid explodes or other item destruction happens, otmp will be
         * deleted. Avoid reading garbage data from it. */
        nobj = (here ? otmp->nexthere : otmp->nobj);
        if (!do_container && Is_container(otmp))
            continue;
        if (count < 1) {
            water_damage(otmp, (char *) 0, FALSE);
        }
        else {
            /* reservoir sampling: replace elements with lowering probability */
            i++; /* i should start this loop equal to count */
            j = rn2(i);
            if (j < count) {
                to_damage[j] = otmp;
            }
        }
    }

    if (count >= 1) {
        for (i = 0; i < count; i++) {
            water_damage(to_damage[i], (char *) 0, FALSE);
        }
        free((genericptr_t) to_damage);
    }

    /* reset acid context */
    g.acid_ctx.dkn_boom = g.acid_ctx.unk_boom = 0;
    g.acid_ctx.ctx_valid = FALSE;
}

/*
 * This function is potentially expensive - rolling
 * inventory list multiple times.  Luckily it's seldom needed.
 * Returns TRUE if disrobing made player unencumbered enough to
 * crawl out of the current predicament.
 */
static boolean
emergency_disrobe(lostsome)
boolean *lostsome;
{
    int invc = inv_cnt(TRUE);

    while (near_capacity() > (Punished ? UNENCUMBERED : SLT_ENCUMBER)) {
        register struct obj *obj, *otmp = (struct obj *) 0;
        register int i;

        /* Pick a random object */
        if (invc > 0) {
            i = rn2(invc);
            for (obj = g.invent; obj; obj = obj->nobj) {
                /*
                 * Undroppables are: body armor, boots, gloves,
                 * amulets, and rings because of the time and effort
                 * in removing them and other cursed stuff
                 * for obvious reasons.
                 */
                if (!(undroppable(obj) || obj == uamul
                      || obj == uleft || obj == uright || obj == ublindf
                      || obj == uarm || obj == uarmc || obj == uarmg
                      || obj == uarmf || obj == uarmu
                      || (obj->cursed && (obj == uarmh || obj == uarms))
                      || welded(obj)))
                    otmp = obj;
                /* reached the mark and found some stuff to drop? */
                if (--i < 0 && otmp)
                    break;

                /* else continue */
            }
        }
        if (!otmp)
            return FALSE; /* nothing to drop! */
        if (otmp->owornmask)
            remove_worn_item(otmp, FALSE);
        *lostsome = TRUE;
        dropx(otmp);
        invc--;
    }
    return TRUE;
}


/*  return TRUE iff player relocated */
boolean
drown()
{
    const char *pool_of_water;
    boolean inpool_ok = FALSE, crawl_ok;
    int i, x, y;

    if (Levitation || Flying) {
        /* shouldn't drown */
        return FALSE;
    }
    feel_newsym(u.ux, u.uy); /* in case Blind, map the water here */
    /* Note: drown() callers should NOT check !Wwalking as a condition of
     * calling it. If water walking boots prevent the player from falling in,
     * they should become identified. */
    if (Wwalking) {
        if (u.uinwater) {
            impossible("drown: in water but also water walking?");
        }
        /* maybe we were called because the hero moved or fell into a pool; if
         * so, assuming the only source of water walking is water walking
         * boots, identify them. */
        if (!objects[WATER_WALKING_BOOTS].oc_name_known) {
            Your("boots don't sink into the water!");
        }
        makeknown(WATER_WALKING_BOOTS);
        return FALSE;
    }

    /* happily wading in the same contiguous pool */
    if (u.uinwater && is_pool(u.ux - u.dx, u.uy - u.dy)
        && (Swimming || Amphibious)) {
        /* water effects on objects every now and then */
        if (!rn2(5))
            inpool_ok = TRUE;
        else
            return FALSE;
    }

    if (!u.uinwater) {
        You("%s into the %s%c", Is_waterlevel(&u.uz) ? "plunge" : "fall",
            hliquid("water"),
            Amphibious || Swimming ? '.' : '!');
        if (!Swimming && !Is_waterlevel(&u.uz))
            You("sink like %s.", Hallucination ? "the Titanic" : "a rock");
    }

    water_damage_chain(g.invent, FALSE, 0, TRUE);

    if (u.umonnum == PM_GREMLIN && rn2(3))
        (void) split_mon(&g.youmonst, (struct monst *) 0);
    else if (u.umonnum == PM_IRON_GOLEM) {
        You("rust!");
        i = Maybe_Half_Phys(d(2, 6));
        if (u.mhmax > i)
            u.mhmax -= i;
        losehp(i, "rusting away", KILLED_BY);
    }
    if (inpool_ok)
        return FALSE;

    if ((i = number_leashed()) > 0) {
        pline_The("leash%s slip%s loose.", (i > 1) ? "es" : "",
                  (i > 1) ? "" : "s");
        unleash_all();
    }

    if (Amphibious || Swimming) {
        if (Amphibious) {
            if (flags.verbose)
                pline("But you aren't drowning.");
            if (!Is_waterlevel(&u.uz)) {
                if (Hallucination)
                    Your("keel hits the bottom.");
                else
                    You("touch bottom.");
            }
        }
        if (Punished) {
            unplacebc();
            placebc();
        }
        vision_recalc(2); /* unsee old position */
        set_uinwater(1); /* u.uinwater = 1 */
        under_water(1);
        g.vision_full_recalc = 1;
        return FALSE;
    }
    if ((Teleportation || can_teleport(g.youmonst.data)) && !Unaware
        && (Teleport_control || rn2(3) < Luck + 2)) {
        You("attempt a teleport spell."); /* utcsri!carroll */
        if (!noteleport_level(&g.youmonst)) {
            (void) dotele(FALSE);
            if (!is_pool(u.ux, u.uy))
                return TRUE;
        } else
            pline_The("attempted teleport spell fails.");
    }
    if (u.usteed) {
        dismount_steed(DISMOUNT_GENERIC);
        if (!is_pool(u.ux, u.uy))
            return TRUE;
    }
    crawl_ok = FALSE;
    x = y = 0; /* lint suppression */
    /* if sleeping, wake up now so that we don't crawl out of water
       while still asleep; we can't do that the same way that waking
       due to combat is handled; note unmul() clears u.usleep */
    if (u.usleep)
        unmul("Suddenly you wake up!");
    /* being doused will revive from fainting */
    if (is_fainted())
        reset_faint();
    /* can't crawl if unable to move (crawl_ok flag stays false) */
    if (g.multi < 0 || (Upolyd && !g.youmonst.data->mmove))
        goto crawl;
    /* look around for a place to crawl to */
    for (i = 0; i < 100; i++) {
        x = rn1(3, u.ux - 1);
        y = rn1(3, u.uy - 1);
        if (crawl_destination(x, y)) {
            crawl_ok = TRUE;
            goto crawl;
        }
    }
    /* one more scan */
    for (x = u.ux - 1; x <= u.ux + 1; x++)
        for (y = u.uy - 1; y <= u.uy + 1; y++)
            if (crawl_destination(x, y)) {
                crawl_ok = TRUE;
                goto crawl;
            }
 crawl:
    if (crawl_ok) {
        boolean lost = FALSE;
        /* time to do some strip-tease... */
        boolean succ = Is_waterlevel(&u.uz) ? TRUE : emergency_disrobe(&lost);

        You("try to crawl out of the %s.", hliquid("water"));
        if (lost)
            You("dump some of your gear to lose weight...");
        if (succ) {
            pline("Pheew!  That was close.");
            teleds(x, y, TELEDS_ALLOW_DRAG);
            return TRUE;
        }
        /* still too much weight */
        pline("But in vain.");
    }
    set_uinwater(1); /* u.uinwater = 1 */
    You("drown.");
    for (i = 0; i < 5; i++) { /* arbitrary number of loops */
        /* killer format and name are reconstructed every iteration
           because lifesaving resets them */
        pool_of_water = waterbody_name(u.ux, u.uy);
        g.killer.format = KILLED_BY_AN;
        /* avoid "drowned in [a] water" */
        if (!strcmp(pool_of_water, "water"))
            pool_of_water = "deep water", g.killer.format = KILLED_BY;
        Strcpy(g.killer.name, pool_of_water);
        done(DROWNING);
        /* oops, we're still alive.  better get out of the water. */
        if (safe_teleds(TELEDS_ALLOW_DRAG | TELEDS_TELEPORT))
            break; /* successful life-save */
        /* nowhere safe to land; repeat drowning loop... */
        pline("You're still drowning.");
    }
    if (u.uinwater) {
        set_uinwater(0); /* u.uinwater = 0 */
        You("find yourself back %s.",
            Is_waterlevel(&u.uz) ? "in an air bubble" : "on land");
    }
    return TRUE;
}

void
drain_en(n)
int n;
{
    if (!u.uenmax) {
        /* energy is completely gone */
        You_feel("momentarily lethargic.");
    } else {
        /* throttle further loss a bit when there's not much left to lose */
        if (n > u.uenmax || n > u.ulevel)
            n = rnd(n);

        You_feel("your magical energy drain away%c", (n > u.uen) ? '!' : '.');
        u.uen -= n;
        if (u.uen < 0) {
            u.uenmax -= rnd(-u.uen);
            if (u.uenmax < 0)
                u.uenmax = 0;
            u.uen = 0;
        }
        g.context.botl = 1;
    }
}

/* disarm a trap */
int
dountrap()
{
    if (near_capacity() >= HVY_ENCUMBER) {
        pline("You're too strained to do that.");
        return 0;
    }
    if ((nohands(g.youmonst.data) && !webmaker(g.youmonst.data))
        || !g.youmonst.data->mmove) {
        pline("And just how do you expect to do that?");
        return 0;
    } else if (u.ustuck && sticks(g.youmonst.data)) {
        pline("You'll have to let go of %s first.", mon_nam(u.ustuck));
        return 0;
    }
    if (u.ustuck || (welded(uwep) && bimanual(uwep))) {
        Your("%s seem to be too busy for that.", makeplural(body_part(HAND)));
        return 0;
    }
    return untrap(FALSE);
}

/* Probability of disabling a trap.  Helge Hafting */
static int
untrap_prob(ttmp)
struct trap *ttmp;
{
    int chance = 3;

    /* Only spiders know how to deal with webs reliably */
    if (ttmp->ttyp == WEB && !webmaker(g.youmonst.data))
        chance = 30;
    if (Confusion || Hallucination)
        chance++;
    if (Blind)
        chance++;
    if (Stunned)
        chance += 2;
    if (Fumbling)
        chance *= 2;
    /* Your own traps are better known than others. */
    if (ttmp && ttmp->madeby_u)
        chance--;
    if (Role_if(PM_ROGUE)) {
        if (rn2(2 * MAXULEV) < u.ulevel)
            chance--;
        if (u.uhave.questart && chance > 1)
            chance--;
    } else if (Role_if(PM_RANGER) && chance > 1)
        chance--;
    return rn2(chance);
}

/* while attempting to disarm an adjacent trap, we've fallen into it */
static void
move_into_trap(ttmp)
struct trap *ttmp;
{
    int bc = 0;
    xchar x = ttmp->tx, y = ttmp->ty, bx, by, cx, cy;
    boolean unused;

    bx = by = cx = cy = 0; /* lint suppression */
    /* we know there's no monster in the way, and we're not trapped */
    if (!Punished
        || drag_ball(x, y, &bc, &bx, &by, &cx, &cy, &unused, TRUE)) {
        u.ux0 = u.ux, u.uy0 = u.uy;
        /* set u.ux,u.uy and u.usteed->mx,my plus handle CLIPPING */
        u_on_newpos(x, y);
        u.umoved = TRUE;
        newsym(u.ux0, u.uy0);
        vision_recalc(1);
        check_leash(u.ux0, u.uy0);
        if (Punished)
            move_bc(0, bc, bx, by, cx, cy);
        /* marking the trap unseen forces dotrap() to treat it like a new
           discovery and prevents pickup() -> look_here() -> check_here()
           from giving a redundant "there is a <trap> here" message when
           there are objects covering this trap */
        ttmp->tseen = 0; /* hack for check_here() */
        /* trigger the trap */
        iflags.failing_untrap++; /* spoteffects() -> dotrap(,FAILEDUNTRAP) */
        spoteffects(TRUE); /* pickup() + dotrap() */
        iflags.failing_untrap--;
        /* this should no longer be necessary; before the failing_untrap
           hack, Flying hero would not trigger an unseen bear trap and
           setting it not-yet-seen above resulted in leaving it hidden */
        if ((ttmp = t_at(u.ux, u.uy)) != 0)
            ttmp->tseen = 1;
        exercise(A_WIS, FALSE);
    }
}

/* 0: doesn't even try
 * 1: tries and fails
 * 2: succeeds
 */
static int
try_disarm(ttmp, force_failure)
struct trap *ttmp;
boolean force_failure;
{
    struct monst *mtmp = m_at(ttmp->tx, ttmp->ty);
    int ttype = ttmp->ttyp;
    boolean under_u = (!u.dx && !u.dy);
    boolean holdingtrap = (ttype == BEAR_TRAP || ttype == WEB);

    /* Test for monster first, monsters are displayed instead of trap. */
    if (mtmp && (!mtmp->mtrapped || !holdingtrap)) {
        pline("%s is in the way.", Monnam(mtmp));
        return 0;
    }
    /* We might be forced to move onto the trap's location. */
    if (sobj_at(BOULDER, ttmp->tx, ttmp->ty) && !Passes_walls && !under_u) {
        There("is a boulder in your way.");
        return 0;
    }
    /* duplicate tight-space checks from test_move */
    if (u.dx && u.dy && bad_rock(g.youmonst.data, u.ux, ttmp->ty)
        && bad_rock(g.youmonst.data, ttmp->tx, u.uy)) {
        if ((g.invent && (inv_weight() + weight_cap() > 600))
            || bigmonst(g.youmonst.data)) {
            /* don't allow untrap if they can't get thru to it */
            You("are unable to reach the %s!", trapname(ttype, FALSE));
            return 0;
        }
    }
    /* untrappable traps are located on the ground. */
    if (!can_reach_floor(under_u)) {
        if (u.usteed && P_SKILL(P_RIDING) < P_BASIC)
            rider_cant_reach();
        else
            You("are unable to reach the %s!", trapname(ttype, FALSE));
        return 0;
    }

    /* Will our hero succeed? */
    if (force_failure || untrap_prob(ttmp)) {
        if (rnl(5)) {
            pline("Whoops...");
            if (mtmp) { /* must be a trap that holds monsters */
                if (ttype == BEAR_TRAP) {
                    if (mtmp->mtame)
                        abuse_dog(mtmp);
                    mtmp->mhp -= rnd(4);
                    if (DEADMONSTER(mtmp))
                        killed(mtmp);
                } else if (ttype == WEB) {
                    if (!webmaker(g.youmonst.data)) {
                        struct trap *ttmp2 = maketrap(u.ux, u.uy, WEB);

                        if (ttmp2) {
                            pline_The(
                                "webbing sticks to you.  You're caught too!");
                            dotrap(ttmp2, NOWEBMSG);
                            if (u.usteed && u.utrap) {
                                /* you, not steed, are trapped */
                                dismount_steed(DISMOUNT_FELL);
                            }
                        }
                    } else
                        pline("%s remains entangled.", Monnam(mtmp));
                }
            } else if (under_u) {
                /* [don't need the iflags.failing_untrap hack here] */
                dotrap(ttmp, FAILEDUNTRAP);
            } else {
                move_into_trap(ttmp);
            }
        } else {
            pline("%s %s is difficult to %s.",
                  ttmp->madeby_u ? "Your" : under_u ? "This" : "That",
                  trapname(ttype, FALSE),
                  (ttype == WEB) ? "remove" : "disarm");
        }
        return 1;
    }
    return 2;
}

static void
reward_untrap(ttmp, mtmp)
struct trap *ttmp;
struct monst *mtmp;
{
    if (!ttmp->madeby_u) {
        if (rnl(10) < 8 && !mtmp->mpeaceful && !mtmp->msleeping
            && !mtmp->mfrozen && !mindless(mtmp->data)
            && mtmp->data->mlet != S_HUMAN) {
            mtmp->mpeaceful = 1;
            set_malign(mtmp); /* reset alignment */
            pline("%s is grateful.", Monnam(mtmp));
        }
        /* Helping someone out of a trap is a nice thing to do,
         * A lawful may be rewarded, but not too often.  */
        if (!rn2(3) && !rnl(8) && u.ualign.type == A_LAWFUL) {
            adjalign(1);
            You_feel("that you did the right thing.");
        }
    }
}

static int
disarm_holdingtrap(ttmp) /* Helge Hafting */
struct trap *ttmp;
{
    struct monst *mtmp;
    int fails = try_disarm(ttmp, FALSE);

    if (fails < 2)
        return fails;

    /* ok, disarm it. */

    /* untrap the monster, if any.
       There's no need for a cockatrice test, only the trap is touched */
    if ((mtmp = m_at(ttmp->tx, ttmp->ty)) != 0) {
        mtmp->mtrapped = 0;
        You("remove %s %s from %s.", the_your[ttmp->madeby_u],
            (ttmp->ttyp == BEAR_TRAP) ? "bear trap" : "webbing",
            mon_nam(mtmp));
        reward_untrap(ttmp, mtmp);
    } else {
        if (ttmp->ttyp == BEAR_TRAP) {
            You("disarm %s bear trap.", the_your[ttmp->madeby_u]);
            deltrap_with_ammo(ttmp, DELTRAP_PLACE_AMMO);
        } else /* if (ttmp->ttyp == WEB) */ {
            You("succeed in removing %s web.", the_your[ttmp->madeby_u]);
            deltrap(ttmp);
        }
    }
    newsym(u.ux + u.dx, u.uy + u.dy);
    return 1;
}

static int
disarm_landmine(ttmp) /* Helge Hafting */
struct trap *ttmp;
{
    int fails = try_disarm(ttmp, FALSE);

    if (fails < 2)
        return fails;
    You("disarm %s land mine.", the_your[ttmp->madeby_u]);
    deltrap_with_ammo(ttmp, DELTRAP_PLACE_AMMO);
    return 1;
}

static int
oil_ok(obj)
struct obj *obj;
{
    if (!obj)
        return 0;

    if (obj->otyp == CAN_OF_GREASE)
        return 2;

    if (obj->otyp == POT_OIL && obj->dknown &&
        objects[POT_OIL].oc_name_known)
        return 2;

    if (obj->oclass == POTION_CLASS)
        return 1; /* let players try any potion, but don't encourage it */

    return 0;
}

/* it may not make much sense to use grease on floor boards, but so what? */
static int
disarm_squeaky_board(ttmp)
struct trap *ttmp;
{
    struct obj *obj;
    boolean bad_tool;
    int fails;

    obj = getobj("untrap with", oil_ok, FALSE, FALSE);
    if (!obj)
        return 0;

    bad_tool = (obj->cursed
                || ((obj->otyp != POT_OIL || obj->lamplit)
                    && (obj->otyp != CAN_OF_GREASE || !obj->spe)));
    fails = try_disarm(ttmp, bad_tool);
    if (fails < 2)
        return fails;

    /* successfully used oil or grease to fix squeaky board */
    if (obj->otyp == CAN_OF_GREASE) {
        consume_obj_charge(obj, TRUE);
    } else {
        useup(obj); /* oil */
        makeknown(POT_OIL);
    }
    You("repair the squeaky board."); /* no madeby_u */
    deltrap(ttmp);
    newsym(u.ux + u.dx, u.uy + u.dy);
    more_experienced(1, 5);
    newexplevel();
    return 1;
}

/* removes traps that shoot arrows, darts, etc. */
static int
disarm_shooting_trap(ttmp)
struct trap *ttmp;
{
    int fails = try_disarm(ttmp, FALSE);

    if (fails < 2)
        return fails;
    You("disarm %s trap.", the_your[ttmp->madeby_u]);
    deltrap_with_ammo(ttmp, DELTRAP_TAKE_AMMO);
    return 1;
}

/* Is the weight too heavy?
 * Formula as in near_capacity() & check_capacity() */
static int
try_lift(mtmp, ttmp, wt, stuff)
struct monst *mtmp;
struct trap *ttmp;
int wt;
boolean stuff;
{
    int wc = weight_cap();

    if (((wt * 2) / wc) >= HVY_ENCUMBER) {
        pline("%s is %s for you to lift.", Monnam(mtmp),
              stuff ? "carrying too much" : "too heavy");
        if (!ttmp->madeby_u && !mtmp->mpeaceful && mtmp->mcanmove
            && !mindless(mtmp->data) && mtmp->data->mlet != S_HUMAN
            && rnl(10) < 3) {
            mtmp->mpeaceful = 1;
            set_malign(mtmp); /* reset alignment */
            pline("%s thinks it was nice of you to try.", Monnam(mtmp));
        }
        return 0;
    }
    return 1;
}

/* Help trapped monster (out of a (spiked) pit) */
static int
help_monster_out(mtmp, ttmp)
struct monst *mtmp;
struct trap *ttmp;
{
    int wt;
    struct obj *otmp;
    boolean uprob;

    /*
     * This works when levitating too -- consistent with the ability
     * to hit monsters while levitating.
     *
     * Should perhaps check that our hero has arms/hands at the
     * moment.  Helping can also be done by engulfing...
     *
     * Test the monster first - monsters are displayed before traps.
     */
    if (!mtmp->mtrapped) {
        pline("%s isn't trapped.", Monnam(mtmp));
        return 0;
    }
    /* Do you have the necessary capacity to lift anything? */
    if (check_capacity((char *) 0))
        return 1;

    /* Will our hero succeed? */
    if ((uprob = untrap_prob(ttmp)) && !mtmp->msleeping && mtmp->mcanmove) {
        You("try to reach out your %s, but %s backs away skeptically.",
            makeplural(body_part(ARM)), mon_nam(mtmp));
        return 1;
    }

    /* is it a cockatrice?... */
    if (touch_petrifies(mtmp->data) && !uarmg && !Stone_resistance) {
        You("grab the trapped %s using your bare %s.", mtmp->data->mname,
            makeplural(body_part(HAND)));

        if (poly_when_stoned(g.youmonst.data)
            && polymon(PM_STONE_GOLEM, POLYMON_ALL_MSGS)) {
            display_nhwindow(WIN_MESSAGE, FALSE);
        } else {
            char kbuf[BUFSZ];

            Sprintf(kbuf, "trying to help %s out of a pit",
                    an(mtmp->data->mname));
            instapetrify(kbuf);
            return 1;
        }
    }
    /* need to do cockatrice check first if sleeping or paralyzed */
    if (uprob) {
        You("try to grab %s, but cannot get a firm grasp.", mon_nam(mtmp));
        if (mtmp->msleeping) {
            wakeup(mtmp, FALSE);
        }
        return 1;
    }

    You("reach out your %s and grab %s.", makeplural(body_part(ARM)),
        mon_nam(mtmp));

    if (mtmp->msleeping) {
        wakeup(mtmp, FALSE);
    } else if (mtmp->mfrozen && !rn2(mtmp->mfrozen)) {
        /* After such manhandling, perhaps the effect wears off */
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
        pline("%s stirs.", Monnam(mtmp));
    }

    /* is the monster too heavy? */
    wt = inv_weight() + mtmp->data->cwt;
    if (!try_lift(mtmp, ttmp, wt, FALSE))
        return 1;

    /* is the monster with inventory too heavy? */
    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
        wt += otmp->owt;
    if (!try_lift(mtmp, ttmp, wt, TRUE))
        return 1;

    You("pull %s out of the pit.", mon_nam(mtmp));
    mtmp->mtrapped = 0;
    reward_untrap(ttmp, mtmp);
    fill_pit(mtmp->mx, mtmp->my);
    return 1;
}

int
untrap(force)
boolean force;
{
    register struct obj *otmp;
    register int x, y;
    int ch;
    struct trap *ttmp;
    struct monst *mtmp;
    const char *trapdescr;
    boolean here, useplural, deal_with_floor_trap,
            confused = (Confusion || Hallucination),
            trap_skipped = FALSE;
    int boxcnt = 0;
    char the_trap[BUFSZ], qbuf[QBUFSZ];

    if (!getdir((char *) 0))
        return 0;
    x = u.ux + u.dx;
    y = u.uy + u.dy;
    if (!isok(x, y)) {
        pline_The("perils lurking there are beyond your grasp.");
        return 0;
    }
    /* 'force' is true for #invoke; make it be true for #untrap if
       carrying MKoT */
    if (!force && has_magic_key(&g.youmonst))
        force = TRUE;

    ttmp = t_at(x, y);
    if (ttmp && !ttmp->tseen)
        ttmp = 0;
    trapdescr = ttmp ? trapname(ttmp->ttyp, FALSE) : 0;
    here = (x == u.ux && y == u.uy); /* !u.dx && !u.dy */

    if (here) /* are there are one or more containers here? */
        for (otmp = g.level.objects[x][y]; otmp; otmp = otmp->nexthere)
            if (Is_box(otmp)) {
                if (++boxcnt > 1)
                    break;
            }

    deal_with_floor_trap = can_reach_floor(FALSE);
    if (!deal_with_floor_trap) {
        *the_trap = '\0';
        if (ttmp)
            Strcat(the_trap, an(trapdescr));
        if (ttmp && boxcnt)
            Strcat(the_trap, " and ");
        if (boxcnt)
            Strcat(the_trap, (boxcnt == 1) ? "a container" : "containers");
        useplural = ((ttmp && boxcnt > 0) || boxcnt > 1);
        /* note: boxcnt and useplural will always be 0 for !here case */
        if (ttmp || boxcnt)
            There("%s %s %s but you can't reach %s%s.",
                  useplural ? "are" : "is", the_trap, here ? "here" : "there",
                  useplural ? "them" : "it",
                  u.usteed ? " while mounted" : "");
        trap_skipped = (ttmp != 0);
    } else { /* deal_with_floor_trap */

        if (ttmp) {
            Strcpy(the_trap, the(trapdescr));
            if (boxcnt) {
                if (is_pit(ttmp->ttyp)) {
                    You_cant("do much about %s%s.", the_trap,
                             u.utrap ? " that you're stuck in"
                                     : " while standing on the edge of it");
                    trap_skipped = TRUE;
                    deal_with_floor_trap = FALSE;
                } else {
                    Sprintf(
                        qbuf, "There %s and %s here.  %s %s?",
                        (boxcnt == 1) ? "is a container" : "are containers",
                        an(trapdescr),
                        (ttmp->ttyp == WEB) ? "Remove" : "Disarm", the_trap);
                    switch (ynq(qbuf)) {
                    case 'q':
                        return 0;
                    case 'n':
                        trap_skipped = TRUE;
                        deal_with_floor_trap = FALSE;
                        break;
                    }
                }
            }
            if (deal_with_floor_trap) {
                if (u.utrap) {
                    You("cannot deal with %s while trapped%s!", the_trap,
                        (x == u.ux && y == u.uy) ? " in it" : "");
                    return 1;
                }
                if ((mtmp = m_at(x, y)) != 0
                    && (M_AP_TYPE(mtmp) == M_AP_FURNITURE
                        || M_AP_TYPE(mtmp) == M_AP_OBJECT)) {
                    stumble_onto_mimic(mtmp);
                    return 1;
                }
                switch (ttmp->ttyp) {
                case BEAR_TRAP:
                case WEB:
                    return disarm_holdingtrap(ttmp);
                case LANDMINE:
                    return disarm_landmine(ttmp);
                case SQKY_BOARD:
                    return disarm_squeaky_board(ttmp);
                case DART_TRAP:
                case ARROW_TRAP:
                    return disarm_shooting_trap(ttmp);
                case PIT:
                case SPIKED_PIT:
                    if (here) {
                        You("are already on the edge of the pit.");
                        return 0;
                    }
                    if (!mtmp) {
                        pline("Try filling the pit instead.");
                        return 0;
                    }
                    return help_monster_out(mtmp, ttmp);
                default:
                    You("cannot disable %s trap.", !here ? "that" : "this");
                    return 0;
                }
            }
        } /* end if */

        if (boxcnt) {
            for (otmp = g.level.objects[x][y]; otmp; otmp = otmp->nexthere)
                if (Is_box(otmp)) {
                    (void) safe_qbuf(qbuf, "There is ",
                                     " here.  Check it for traps?", otmp,
                                     doname, ansimpleoname, "a box");
                    switch (ynq(qbuf)) {
                    case 'q':
                        return 0;
                    case 'n':
                        continue;
                    }

                    if ((otmp->otrapped
                         && (force || (!confused
                                       && rn2(MAXULEV + 1 - u.ulevel) < 10)))
                        || (!force && confused && !rn2(3))) {
                        You("find a trap on %s!", the(xname(otmp)));
                        if (!confused)
                            exercise(A_WIS, TRUE);

                        switch (ynq("Disarm it?")) {
                        case 'q':
                            return 1;
                        case 'n':
                            trap_skipped = TRUE;
                            continue;
                        }

                        if (otmp->otrapped) {
                            exercise(A_DEX, TRUE);
                            ch = ACURR(A_DEX) + u.ulevel;
                            if (Role_if(PM_ROGUE))
                                ch *= 2;
                            if (!force && (confused || Fumbling
                                           || rnd(75 + level_difficulty() / 2)
                                                  > ch)) {
                                (void) chest_trap(otmp, FINGER, TRUE);
                            } else {
                                You("disarm it!");
                                otmp->otrapped = 0;
                            }
                        } else
                            pline("That %s was not trapped.", xname(otmp));
                        return 1;
                    } else {
                        You("find no traps on %s.", the(xname(otmp)));
                        return 1;
                    }
                }

            You(trap_skipped ? "find no other traps here."
                             : "know of no traps here.");
            return 0;
        }

        if (stumble_on_door_mimic(x, y))
            return 1;

    } /* deal_with_floor_trap */
    /* doors can be manipulated even while levitating/unskilled riding */

    if (!IS_DOOR(levl[x][y].typ)) {
        if (!trap_skipped)
            You("know of no traps there.");
        return 0;
    }

    switch (doorstate(&levl[x][y])) {
    case D_NODOOR:
        You("%s no door there.", Blind ? "feel" : "see");
        return 0;
    case D_BROKEN:
        pline("This door is broken.");
        return 0;
    }

    if ((door_is_trapped(&levl[x][y])
         && (force || (!confused && rn2(MAXULEV - u.ulevel + 11) < 10)))
        || (!force && confused && !rn2(3))) {
        You("find a trap on the door!");
        exercise(A_WIS, TRUE);
        if (ynq("Disarm it?") != 'y')
            return 1;
        if (door_is_trapped(&levl[x][y])) {
            ch = 15 + (Role_if(PM_ROGUE) ? u.ulevel * 3 : u.ulevel);
            exercise(A_DEX, TRUE);
            if (!force && (confused || Fumbling
                           || rnd(75 + level_difficulty() / 2) > ch)) {
                You("slip up...");
                /* don't "set it off"; some traps have no message here */
                doortrapped(x, y, &g.youmonst, FINGER, -D_TRAPPED, 2);
            } else {
                You("disarm it!");
                set_door_trap(&levl[x][y], FALSE);
            }
        } else
            pline("This door was not trapped.");
        return 1;
    } else {
        You("find no traps on the door.");
        return 1;
    }
}

/* for magic unlocking; returns true if targetted monster (which might
   be hero) gets untrapped; the trap remains intact */
boolean
openholdingtrap(mon, noticed)
struct monst *mon;
boolean *noticed; /* set to true iff hero notices the effect; */
{                 /* otherwise left with its previous value intact */
    struct trap *t;
    char buf[BUFSZ], whichbuf[20];
    const char *trapdescr = 0, *which = 0;
    boolean ishero = (mon == &g.youmonst);

    if (!mon)
        return FALSE;
    if (mon == u.usteed)
        ishero = TRUE;

    t = t_at(ishero ? u.ux : mon->mx, ishero ? u.uy : mon->my);

    if (ishero && u.utrap) { /* all u.utraptype values are holding traps */
        which = the_your[(!t || !t->tseen || !t->madeby_u) ? 0 : 1];
        switch (u.utraptype) {
        case TT_LAVA:
            trapdescr = "molten lava";
            break;
        case TT_INFLOOR:
            /* solidified lava, so not "floor" even if within a room */
            trapdescr = "ground";
            break;
        case TT_BURIEDBALL:
            trapdescr = "your anchor";
            which = "";
            break;
        case TT_BEARTRAP:
        case TT_PIT:
        case TT_WEB:
            trapdescr = 0; /* use defsyms[].explanation */
            break;
        default:
            /* lint suppression in case 't' is unexpectedly Null
               or u.utraptype has new value we don't know about yet */
            trapdescr = "trap";
            break;
        }
    } else {
        /* if no trap here or it's not a holding trap, we're done */
        if (!t || (t->ttyp != BEAR_TRAP && t->ttyp != WEB))
            return FALSE;
    }

    if (!trapdescr)
        trapdescr = trapname(t->ttyp, FALSE);
    if (!which)
        which = t->tseen ? the_your[t->madeby_u]
                         : index(vowels, *trapdescr) ? "an" : "a";
    if (*which)
        which = strcat(strcpy(whichbuf, which), " ");

    if (ishero) {
        if (!u.utrap)
            return FALSE;
        *noticed = TRUE;
        if (u.usteed)
            Sprintf(buf, "%s is", noit_Monnam(u.usteed));
        else
            Strcpy(buf, "You are");
        reset_utrap(TRUE);
        g.vision_full_recalc = 1; /* vision limits can change (pit escape) */
        pline("%s released from %s%s.", buf, which, trapdescr);
    } else {
        if (!mon->mtrapped)
            return FALSE;
        mon->mtrapped = 0;
        if (canspotmon(mon)) {
            *noticed = TRUE;
            pline("%s is released from %s%s.", Monnam(mon), which,
                  trapdescr);
        } else if (cansee(t->tx, t->ty) && t->tseen) {
            *noticed = TRUE;
            if (t->ttyp == WEB)
                pline("%s is released from %s%s.", Something, which,
                      trapdescr);
            else /* BEAR_TRAP */
                pline("%s%s opens.", upstart(strcpy(buf, which)), trapdescr);
        }
        /* might pacify monster if adjacent */
        if (rn2(2) && distu(mon->mx, mon->my) <= 2)
            reward_untrap(t, mon);
    }
    return TRUE;
}

/* for magic locking; returns true if targetted monster (which might
   be hero) gets hit by a trap (might avoid actually becoming trapped) */
boolean
closeholdingtrap(mon, noticed)
struct monst *mon;
boolean *noticed; /* set to true iff hero notices the effect; */
{                 /* otherwise left with its previous value intact */
    struct trap *t;
    unsigned dotrapflags;
    boolean ishero = (mon == &g.youmonst), result;

    if (!mon)
        return FALSE;
    if (mon == u.usteed)
        ishero = TRUE;
    t = t_at(ishero ? u.ux : mon->mx, ishero ? u.uy : mon->my);
    /* if no trap here or it's not a holding trap, we're done */
    if (!t || (t->ttyp != BEAR_TRAP && t->ttyp != WEB))
        return FALSE;

    if (ishero) {
        if (u.utrap)
            return FALSE; /* already trapped */
        *noticed = TRUE;
        dotrapflags = FORCETRAP;
        /* dotrap calls mintrap when mounted hero encounters a web */
        if (u.usteed)
            dotrapflags |= NOWEBMSG;
        ++g.force_mintrap;
        dotrap(t, dotrapflags);
        --g.force_mintrap;
        result = (u.utrap != 0);
    } else {
        if (mon->mtrapped)
            return FALSE; /* already trapped */
        /* you notice it if you see the trap close/tremble/whatever
           or if you sense the monster who becomes trapped */
        *noticed = cansee(t->tx, t->ty) || canspotmon(mon);
        ++g.force_mintrap;
        result = (mintrap(mon) != 0);
        --g.force_mintrap;
    }
    return result;
}

/* for magic unlocking; returns true if targetted monster (which might
   be hero) gets hit by a trap (target might avoid its effect) */
boolean
openfallingtrap(mon, trapdoor_only, noticed)
struct monst *mon;
boolean trapdoor_only;
boolean *noticed; /* set to true iff hero notices the effect; */
{                 /* otherwise left with its previous value intact */
    struct trap *t;
    boolean ishero = (mon == &g.youmonst), result;

    if (!mon)
        return FALSE;
    if (mon == u.usteed)
        ishero = TRUE;
    t = t_at(ishero ? u.ux : mon->mx, ishero ? u.uy : mon->my);
    /* if no trap here or it's not a falling trap, we're done
       (note: falling rock traps have a trapdoor in the ceiling) */
    if (!t || ((t->ttyp != TRAPDOOR && t->ttyp != ROCKTRAP)
               && (trapdoor_only || (t->ttyp != HOLE && !is_pit(t->ttyp)))))
        return FALSE;

    if (ishero) {
        if (u.utrap)
            return FALSE; /* already trapped */
        *noticed = TRUE;
        dotrap(t, FORCETRAP);
        result = (u.utrap != 0);
    } else {
        if (mon->mtrapped)
            return FALSE; /* already trapped */
        /* you notice it if you see the trap close/tremble/whatever
           or if you sense the monster who becomes trapped */
        *noticed = cansee(t->tx, t->ty) || canspotmon(mon);
        /* monster will be angered; mintrap doesn't handle that */
        wakeup(mon, TRUE);
        ++g.force_mintrap;
        result = (mintrap(mon) != 0);
        --g.force_mintrap;
        /* mon might now be on the migrating monsters list */
    }
    return result;
}

/* only called when the player is doing something to the chest directly */
boolean
chest_trap(obj, bodypart, disarm)
register struct obj *obj;
register int bodypart;
boolean disarm;
{
    register struct obj *otmp = obj, *otmp2;
    char buf[80];
    const char *msg;
    coord cc;

    if (get_obj_location(obj, &cc.x, &cc.y, 0)) /* might be carried */
        obj->ox = cc.x, obj->oy = cc.y;

    otmp->otrapped = 0; /* trap is one-shot; clear flag first in case
                           chest kills you and ends up in bones file */
    You(disarm ? "set it off!" : "trigger a trap!");
    display_nhwindow(WIN_MESSAGE, FALSE);
    if (Luck > -13 && rn2(13 + Luck) > 7) { /* saved by luck */
        /* trap went off, but good luck prevents damage */
        switch (rn2(13)) {
        case 12:
        case 11:
            msg = "explosive charge is a dud";
            break;
        case 10:
        case 9:
            msg = "electric charge is grounded";
            break;
        case 8:
        case 7:
            msg = "flame fizzles out";
            break;
        case 6:
        case 5:
        case 4:
            msg = "poisoned needle misses";
            break;
        case 3:
        case 2:
        case 1:
        case 0:
            msg = "gas cloud blows away";
            break;
        default:
            impossible("chest disarm bug");
            msg = (char *) 0;
            break;
        }
        if (msg)
            pline("But luckily the %s!", msg);
    } else {
        switch (rn2(20) ? ((Luck >= 13) ? 0 : rn2(13 - Luck)) : rn2(26)) {
        case 25:
        case 24:
        case 23:
        case 22:
        case 21: {
            struct monst *shkp = 0;
            long loss = 0L;
            boolean costly, insider;
            register xchar ox = obj->ox, oy = obj->oy;

            /* the obj location need not be that of player */
            costly = (costly_spot(ox, oy)
                      && (shkp = shop_keeper(*in_rooms(ox, oy, SHOPBASE)))
                             != (struct monst *) 0);
            insider = (*u.ushops && inside_shop(u.ux, u.uy)
                       && *in_rooms(ox, oy, SHOPBASE) == *u.ushops);

            pline("%s!", Tobjnam(obj, "explode"));
            Sprintf(buf, "exploding %s", xname(obj));

            if (costly)
                loss += stolen_value(obj, ox, oy, (boolean) shkp->mpeaceful,
                                     TRUE);
            delete_contents(obj);
            /* unpunish() in advance if either ball or chain (or both)
               is going to be destroyed */
            if (Punished && ((uchain->ox == u.ux && uchain->oy == u.uy)
                             || (uball->where == OBJ_FLOOR
                                 && uball->ox == u.ux && uball->oy == u.uy)))
                unpunish();

            for (otmp = g.level.objects[u.ux][u.uy]; otmp; otmp = otmp2) {
                otmp2 = otmp->nexthere;
                if (costly)
                    loss += stolen_value(otmp, otmp->ox, otmp->oy,
                                         (boolean) shkp->mpeaceful, TRUE);
                delobj(otmp);
            }
            wake_nearby();
            losehp(Maybe_Half_Phys(d(6, 6)), buf, KILLED_BY_AN);
            exercise(A_STR, FALSE);
            if (costly && loss) {
                if (insider)
                    You("owe %ld %s for objects destroyed.", loss,
                        currency(loss));
                else {
                    You("caused %ld %s worth of damage!", loss,
                        currency(loss));
                    make_angry_shk(shkp, ox, oy);
                }
            }
            return TRUE;
        } /* case 21 */
        case 20:
        case 19:
        case 18:
        case 17:
            pline("A cloud of noxious gas billows from %s.", the(xname(obj)));
            if (rn2(3))
                poisoned("gas cloud", A_STR, "cloud of poison gas", 15, FALSE);
            else
                create_gas_cloud(obj->ox, obj->oy, 1, 8);
            exercise(A_CON, FALSE);
            break;
        case 16:
        case 15:
        case 14:
        case 13:
            You_feel("a needle prick your %s.", body_part(bodypart));
            poisoned("needle", A_CON, "poisoned needle", 10, FALSE);
            exercise(A_CON, FALSE);
            break;
        case 12:
        case 11:
        case 10:
        case 9:
            dofiretrap(obj);
            break;
        case 8:
        case 7:
        case 6: {
            int dmg;

            You("are jolted by a surge of electricity!");
            if (Shock_resistance) {
                shieldeff(u.ux, u.uy);
                You("don't seem to be affected.");
                dmg = 0;
            } else
                dmg = d(4, 4);
            destroy_item(RING_CLASS, AD_ELEC);
            destroy_item(WAND_CLASS, AD_ELEC);
            if (dmg)
                losehp(dmg, "electric shock", KILLED_BY_AN);
            break;
        } /* case 6 */
        case 5:
        case 4:
        case 3:
            make_paralyzed(d(5, 6), TRUE, "frozen by a trap");
            break;
        case 2:
        case 1:
        case 0:
            pline("A cloud of %s gas billows from %s.",
                  Blind ? blindgas[rn2(SIZE(blindgas))] : rndcolor(),
                  the(xname(obj)));
            if (!Stunned) {
                if (Hallucination)
                    pline("What a groovy feeling!");
                else
                    You("%s%s...", stagger(g.youmonst.data, "stagger"),
                        Halluc_resistance ? ""
                                          : Blind ? " and get dizzy"
                                                  : " and your vision blurs");
            }
            make_stunned((HStun & TIMEOUT) + (long) rn1(7, 16), FALSE);
            (void) make_hallucinated(
                (HHallucination & TIMEOUT) + (long) rn1(5, 16), FALSE, 0L);
            break;
        default:
            impossible("bad chest trap");
            break;
        }
        bot(); /* to get immediate botl re-display */
    }

    return FALSE;
}

struct trap *
t_at(x, y)
register int x, y;
{
    register struct trap *trap = g.ftrap;

    while (trap) {
        if (trap->tx == x && trap->ty == y)
            return trap;
        trap = trap->ntrap;
    }
    return (struct trap *) 0;
}

void
deltrap(trap)
register struct trap *trap;
{
    register struct trap *ttmp;

    if (trap->ammo) {
        impossible("deleting trap (%d) containing ammo (%d)?",
                   trap->ttyp, trap->ammo->otyp);
        /* deltrap (here) -> deltrap_with_ammo (destroys ammo) -> deltrap */
        deltrap_with_ammo(trap, DELTRAP_DESTROY_AMMO);
        return;
    }
    clear_conjoined_pits(trap);
    if (trap == g.ftrap) {
        g.ftrap = g.ftrap->ntrap;
    } else {
        for (ttmp = g.ftrap; ttmp; ttmp = ttmp->ntrap)
            if (ttmp->ntrap == trap)
                break;
        if (!ttmp)
            panic("deltrap: no preceding trap!");
        ttmp->ntrap = trap->ntrap;
    }
    if (Sokoban && (trap->ttyp == PIT || trap->ttyp == HOLE))
        maybe_finish_sokoban();
    dealloc_trap(trap);
}

/* Delete a trap, but handle any ammo in it.
 * The values for do_what are the DELTRAP_*_AMMO constants.
 * If called with a trap without ammo, this should function like deltrap.
 * If called with DELTRAP_RETURN_AMMO, delete the trap but preserve the ammo as
 * an object chain, and return it. */
struct obj *
deltrap_with_ammo(trap, do_what)
struct trap *trap;
int do_what;
{
    struct obj *otmp, *objchn = NULL;
    xchar tx, ty;
    if (!trap) {
        impossible("deltrap_with_ammo: null trap!");
        return NULL;
    }
    tx = trap->tx;
    ty = trap->ty;
    while (trap->ammo) {
        otmp = trap->ammo;
        extract_nobj(otmp, &trap->ammo);
        if (objchn) {
            otmp->nobj = objchn;
        }
        objchn = otmp;
    }
    if (do_what != DELTRAP_RETURN_AMMO) {
        struct obj *nobj;
        otmp = objchn;
        while (otmp) {
            nobj = otmp->nobj;
            switch (do_what) {
            default:
                impossible("Bad deltrap constant! Destroying ammo instead");
                /* FALLTHRU */
            case DELTRAP_DESTROY_AMMO:
                obfree(otmp, NULL);
                break;
            case DELTRAP_PLACE_AMMO:
                place_object(otmp, trap->tx, trap->ty);
                /* Sell your own traps only... */
                if (trap->madeby_u)
                    sellobj(otmp, trap->tx, trap->ty);
                stackobj(otmp);
                break;
            case DELTRAP_BURY_AMMO:
                place_object(otmp, trap->tx, trap->ty);
                (void) bury_an_obj(otmp, NULL);
                break;
            case DELTRAP_TAKE_AMMO:
                hold_another_object(otmp, "You remove, but drop, %s.",
                                    doname(otmp), NULL);
                break;
            }
            otmp = nobj;
        }
        objchn = NULL;
    }
    if (u.utrap && trap->tx == u.ux && trap->ty == u.uy)
        reset_utrap(TRUE);
    deltrap(trap);
    newsym(tx, ty);
    return objchn;
}

boolean
conjoined_pits(trap2, trap1, u_entering_trap2)
struct trap *trap2, *trap1;
boolean u_entering_trap2;
{
    int dx, dy, diridx, adjidx;

    if (!trap1 || !trap2)
        return FALSE;
    if (!isok(trap2->tx, trap2->ty) || !isok(trap1->tx, trap1->ty)
        || !is_pit(trap2->ttyp)
        || !is_pit(trap1->ttyp)
        || (u_entering_trap2 && !(u.utrap && u.utraptype == TT_PIT)))
        return FALSE;
    dx = sgn(trap2->tx - trap1->tx);
    dy = sgn(trap2->ty - trap1->ty);
    for (diridx = 0; diridx < 8; diridx++)
        if (xdir[diridx] == dx && ydir[diridx] == dy)
            break;
    /* diridx is valid if < 8 */
    if (diridx < 8) {
        adjidx = (diridx + 4) % 8;
        if ((trap1->conjoined & (1 << diridx))
            && (trap2->conjoined & (1 << adjidx)))
            return TRUE;
    }
    return FALSE;
}

static void
clear_conjoined_pits(trap)
struct trap *trap;
{
    int diridx, adjidx, x, y;
    struct trap *t;

    if (trap && is_pit(trap->ttyp)) {
        for (diridx = 0; diridx < 8; ++diridx) {
            if (trap->conjoined & (1 << diridx)) {
                x = trap->tx + xdir[diridx];
                y = trap->ty + ydir[diridx];
                if (isok(x, y)
                    && (t = t_at(x, y)) != 0
                    && is_pit(t->ttyp)) {
                    adjidx = (diridx + 4) % 8;
                    t->conjoined &= ~(1 << adjidx);
                }
                trap->conjoined &= ~(1 << diridx);
            }
        }
    }
}

static boolean
adj_nonconjoined_pit(adjtrap)
struct trap *adjtrap;
{
    struct trap *trap_with_u = t_at(u.ux0, u.uy0);

    if (trap_with_u && adjtrap && u.utrap && u.utraptype == TT_PIT
        && is_pit(trap_with_u->ttyp) && is_pit(adjtrap->ttyp)) {
        int idx;

        for (idx = 0; idx < 8; idx++) {
            if (xdir[idx] == u.dx && ydir[idx] == u.dy)
                return TRUE;
        }
    }
    return FALSE;
}

#if 0
/*
 * Mark all neighboring pits as conjoined pits.
 * (currently not called from anywhere)
 */
static void
join_adjacent_pits(trap)
struct trap *trap;
{
    struct trap *t;
    int diridx, x, y;

    if (!trap)
        return;
    for (diridx = 0; diridx < 8; ++diridx) {
        x = trap->tx + xdir[diridx];
        y = trap->ty + ydir[diridx];
        if (isok(x, y)) {
            if ((t = t_at(x, y)) != 0 && is_pit(t->ttyp)) {
                trap->conjoined |= (1 << diridx);
                join_adjacent_pits(t);
            } else
                trap->conjoined &= ~(1 << diridx);
        }
    }
}
#endif /*0*/

/*
 * Returns TRUE if you escaped a pit and are standing on the precipice.
 */
boolean
uteetering_at_seen_pit(trap)
struct trap *trap;
{
    return (trap && is_pit(trap->ttyp) && trap->tseen
            && trap->tx == u.ux && trap->ty == u.uy
            && !(u.utrap && u.utraptype == TT_PIT));
}

/*
 * Returns TRUE if you didn't fall through a hole or didn't
 * release a trap door
 */
boolean
uescaped_shaft(trap)
struct trap *trap;
{
    return (trap && is_hole(trap->ttyp) && trap->tseen
            && trap->tx == u.ux && trap->ty == u.uy);
}

/* Destroy a trap that emanates from the floor. */
boolean
delfloortrap(ttmp)
register struct trap *ttmp;
{
    /* some of these are arbitrary -dlc */
    if (ttmp && ((ttmp->ttyp == SQKY_BOARD) || (ttmp->ttyp == BEAR_TRAP)
                 || (ttmp->ttyp == LANDMINE) || (ttmp->ttyp == FIRE_TRAP)
                 || is_pit(ttmp->ttyp)
                 || is_hole(ttmp->ttyp)
                 || (ttmp->ttyp == TELEP_TRAP) || (ttmp->ttyp == LEVEL_TELEP)
                 || (ttmp->ttyp == WEB) || (ttmp->ttyp == MAGIC_TRAP)
                 || (ttmp->ttyp == ANTI_MAGIC))) {
        register struct monst *mtmp;

        if (ttmp->tx == u.ux && ttmp->ty == u.uy) {
            if (u.utraptype != TT_BURIEDBALL)
                reset_utrap(TRUE);
        } else if ((mtmp = m_at(ttmp->tx, ttmp->ty)) != 0) {
            mtmp->mtrapped = 0;
        }
        deltrap(ttmp);
        return TRUE;
    }
    return FALSE;
}

/* Given a door location, return the trap type associated with it. */
int
getdoortrap(x, y)
int x, y;
{
    int lvl = level_difficulty();
    int maxtrap = -1;
    const int trapminlevels[NUMDOORTRAPS][2] = {
        { HINGE_SCREECH,      1 }, /* keep these sorted */
        { SELF_LOCK,          1 },
        { STATIC_SHOCK,       2 },
        { WATER_BUCKET,       3 },
        { HINGELESS_FORWARD,  6 },
        { HINGELESS_BACKWARD, 8 },
        { ROCKFALL,          10 },
        { HOT_KNOB,          12 },
        { FIRE_BLAST,        15 }};

    /* Find the maximum possible effect for a trap, based on level. */
    int i;
    for (i = 0; i < NUMDOORTRAPS; ++i) {
        if (lvl >= trapminlevels[i][1]) {
            maxtrap = i;
        }
        else break;
    }
    if (maxtrap < 0) {
        impossible("getdoortrap: no valid traps");
        return 0;
    }

    /* trap is picked deterministically based on door's coordinates, so that
     * repeat traps on the same door will be the same trap.
     * In order to provide some extra scrambling that the player won't be able
     * to get out of the visible game state, involve birthday as well. */
    return (int) (coord_hash(x, y, ledger_no(&u.uz)) % (maxtrap + 1));
}

/* Interacting with a door triggers a trap.
 * mon is the monster triggering the trap (NULL or youmonst means player)
 * bodypart is the body part used to touch the door, and can affect what the
 * trap does. Note: ARM is sometimes used to denote touching the door with an
 * object (i.e. chopping the door down).
 * action is a doorstate that defines what the player is doing with the door:
 *   D_ISOPEN - opening it
 *   D_CLOSED - closing it, not wizard-locking it
 *   D_BROKEN - destroying it
 *   D_LOCKED - locking it
 *  -D_LOCKED - unlocking it
 *  -D_TRAPPED - untrapping it
 *   D_NODOOR - none of the above; this is for a trap that triggers when the
 *     player moves off the doorway onto another space. (Unimplemented.)
 * Unused ones that might be of some use at some point:
 *  -D_NODOOR - moving onto the doorway. (Note that D_NODOOR is 0 so this won't
 *    work.)
 * when means whether this function is being called before the action is
 * complete, e.g. a STATIC_KNOB should trigger when touching the door but a
 * WATER_BUCKET after opening it.
 *   0 = only trigger before-action traps
 *   1 = only trigger after-action traps
 *   2 = trigger all possible traps (used when e.g. zapping striking at door)
 *
 * Return value is as follows:
 *   0 = doorstate is not changed, the caller can continue to do things with it
 *   1 = doorstate is set by this function, to something non-destroyed
 *   2 = door has been destroyed
 */
xchar
doortrapped(x, y, mon, bodypart, action, when)
int x, y;
struct monst * mon;
int bodypart, action;
int when;
{
    boolean before = (when == 0 || when == 2);
    boolean after = (when == 1 || when == 2);
    boolean byu = (mon == &g.youmonst || mon == NULL);
    boolean touching = (!byu || (bodypart != NO_PART));
    /* note that touching represents either you or the monster touching the
     * door and does not mean "the player touching".
     * However, !byu currently DOES imply touching, since monsters can't
     * currently affect doors from range. */
    boolean canseemon = ((byu || cansee(mon->mx, mon->my)) && !Unaware);
    /* also assume that it's impossible for the player to trigger a door trap
     * while unaware, so assume byu implies !Unaware */
    boolean canseedoor = (cansee(x,y) && !Unaware);
    int selected_trap = getdoortrap(x, y);
    int lvl = level_difficulty();
    int dmg = 0;
    struct rm * door = &levl[x][y];
    int saved_doorstate = doorstate(door);

    if (door->typ != DOOR && door->typ != SDOOR) {
        impossible("doortrapped: called on a non-door");
        return 0;
    }
    if (selected_trap < HINGE_SCREECH || selected_trap >= NUMDOORTRAPS) {
        impossible("doortrapped: bad trap %d", selected_trap);
        return 0;
    }
    if (action != D_NODOOR && action != D_ISOPEN && action != D_BROKEN
        && action != D_CLOSED && action != -D_TRAPPED
        && action != -D_LOCKED && action != D_LOCKED) {
        impossible("doortrapped: bad action %d", action);
        return 0;
    }
    /* ok to call this on a non-trapped door, since it lets us call the
     * function unconditionally whenever something interacts with a door */
    if (!door_is_trapped(door)) {
        return 0;
    }

    if (mon == NULL) {
        /* useful for calling player/monster agnostic stuff later */
        mon = &g.youmonst;
    }

    /* It is safe to do a direct return in any of these conditions IF the
     * condition contains zero chance of destroying the door. */
    if (selected_trap == HINGE_SCREECH && after
        && (action == D_ISOPEN || action == D_CLOSED)) {
        int range = (11 * lvl) + (11 * lvl);
        if (!Deaf) {
            if (byu || canseedoor) {
                pline("The hinges screech loudly.");
            }
            else {
                You_hear("a %s screech.",
                        dist2(u.ux, u.uy, x, y) < range ? "faint" : "nearby");
            }
        }
        wake_nearto(x, y, range);
        /* trap not disarmed */
    }
    else if (selected_trap == SELF_LOCK && after) {
        boolean do_lock = FALSE;
        if (action == D_ISOPEN) {
            if (byu || canseedoor) {
                pline("But it swings closed again!");
            }
            do_lock = TRUE;
        }
        else if (action == D_CLOSED ||
                 (action == -D_TRAPPED && door_is_closed(door))) {
            do_lock = TRUE;
        }
        else if (action == D_NODOOR) {
            if (byu || canseedoor) {
                pline("The door swings itself shut!");
            }
            do_lock = TRUE;
        }
        else if (action == -D_LOCKED) {
            if (byu && bodypart == FINGER) {
                You("disarm a self-locking mechanism.");
            }
            set_door_trap(door, FALSE);
            return 0; /* not changed at all */
        }
        /* no case for action == D_LOCKED
         * not much point doing anything when the player *wants* to lock it */
        if (do_lock) {
            if (!Deaf && canseedoor) {
                You_hear("the lock click by itself!");
            }
            set_doorstate(door, D_CLOSED);
            set_door_lock(door, TRUE);
        }
        /* trap not disarmed, except if trying to unlock */
        feel_newsym(x, y); /* the hero knows it is closed */
        block_point(x, y); /* vision: no longer see there */
        return 1; /* apart from disarming, all of these effects change door state
                     even temporarily */
    }
    else if (selected_trap == STATIC_SHOCK && before && bodypart == FINGER
             && (action == D_ISOPEN || action == D_CLOSED
                 || action == -D_LOCKED || action == -D_TRAPPED)) {
        boolean resists = (byu ? Shock_resistance : resists_elec(mon));
        dmg = rnd(lvl * 2) / (resists ? 4 : 1);
        dmg += 1;
        if (byu) {
            pline("An electric spark from the doorknob zaps you!");
            losehp(dmg, "charged doorknob", KILLED_BY_AN);
            exercise(A_WIS, FALSE);
        }
        else {
            if (canseemon) {
                pline("%s is zapped by a doorknob!", Monnam(mon));
            }
            mon->mhp -= dmg;
            if (DEADMONSTER(mon)) {
                monkilled(mon, "", AD_ELEC);
            }
        }
        /* trap goes away */
        set_door_trap(door, FALSE);
    }
    else if (selected_trap == WATER_BUCKET && after
             && (action == D_ISOPEN || action == D_BROKEN
                 || action == -D_TRAPPED)) {
        if (canseemon) {
            pline("A bucket of water splashes down on %s!",
                  (!byu ? mon_nam(mon) : (touching ? "you" : "the floor")));
        }
        else {
            You_hear("a distant splash.");
        }
        if (byu && touching) {
            /* TODO: no iron golem rust/gremlin multiplying as of yet,
                * waiting to hear from DT on this */
            water_damage_chain(g.invent, FALSE, (lvl/5)+1, FALSE);
            exercise(A_WIS, FALSE);
        }
        else if (!byu) {
            water_damage_chain(mon->minvent, FALSE, (lvl/5)+1, FALSE);
        }
        set_door_trap(door, FALSE); /* trap is gone */
    }
    else if ((selected_trap == HINGELESS_FORWARD && before
             && (action == D_ISOPEN || action == D_BROKEN
                 || action == -D_TRAPPED))
            /* if you're trying to *break* the door, and it doesn't have
             * hinges, it's going to fall backward regardless of which way it
             * was rigged. */
             || (selected_trap == HINGELESS_BACKWARD && before
                 && action == D_BROKEN)) {
        dmg = rnd((lvl/4) + 1);
        /* necessary to set this up front; otherwise we hurtle into the closed
         * door and don't actually move */
        set_doorstate(door, D_BROKEN);
        if (byu) {
            pline("The door %s forward off its hinges!",
                  (action == D_BROKEN ? "is knocked" : "falls"));
            if (touching) {
                You("crash on top of it!");
                /* move onto the door */
                hurtle(x - u.ux, y - u.uy, 1, FALSE);
                g.multi_reason = "falling on top of a door";

                make_stunned((HStun & TIMEOUT) + d(2,4), FALSE);
                losehp(Maybe_Half_Phys(dmg), "falling on top of a door",
                        KILLED_BY);
                exercise(A_STR, FALSE);
            }
        }
        else {
            if (canseedoor) {
                You_see("a door %s off its hinges!",
                        (action == D_BROKEN ? "smashed" : "fall"));
            }
            if (touching) {
                if (canseedoor) {
                    pline("%s crashes on top of it!", Monnam(mon));
                }
                else {
                    You_hear("a nearby crash.");
                }
                mhurtle(mon, x - mon->mx, y - mon->my, 1);
                mon->mstun = 1;
                mon->mhp -= dmg;
                if (DEADMONSTER(mon)) {
                    monkilled(mon, "", AD_PHYS);
                }
            }
        }
        set_door_trap(door, FALSE); /* trap is gone */
        wake_nearto(x, y, 8 * 8);
    }
    else if (selected_trap == HINGELESS_BACKWARD && before
             && (action == D_ISOPEN || action == -D_TRAPPED)) {
        dmg = rnd((lvl/2) + 1);
        if (byu) {
            pline("The door falls backward off its hinges!");
            if (touching) {
                You("are flattened by it!");
                losehp(Maybe_Half_Phys(dmg), "crushed by a falling door",
                        NO_KILLER_PREFIX);
                /* takes a while to get out from the door */
                nomul(-3);
                g.multi_reason = "trapped under a door";
                exercise(A_STR, FALSE);
            }
        }
        else {
            if (canseedoor) {
                You_see("a door fall off its hinges!");
            }
            if (touching) {
                if (canseemon) {
                    pline("It crashes on top of %s!", Monnam(mon));
                }
                else {
                    You_hear("a nearby crash.");
                }
                mon->mcanmove = 0;
                mon->mfrozen = 3;
                mon->mhp -= dmg;
                if (DEADMONSTER(mon)) {
                    monkilled(mon, "", AD_PHYS);
                }
            }
        }
        set_door_trap(door, FALSE); /* trap is gone */
        set_doorstate(door, D_BROKEN);
        wake_nearto(x, y, 8 * 8);
    }
    else if (selected_trap == ROCKFALL && after
             && (action == D_ISOPEN || action == D_BROKEN
                 || action == -D_TRAPPED)) {
        if (canseedoor) {
            You("see a tripwire snap!");
        }
        You_hear("something rumbling in the ceiling.");
        if (touching) {
            if (byu) {
                drop_boulder_on_player(FALSE, FALSE, FALSE, FALSE);
                exercise(A_STR, FALSE);
            }
            else {
                drop_boulder_on_monster(mon->mx, mon->my, FALSE, FALSE);
            }
        }
        else {
            /* drop on door square */
            drop_boulder_on_monster(x, y, FALSE, FALSE);
        }
        set_door_trap(door, FALSE); /* trap is gone */
    }
    else if (selected_trap == HOT_KNOB && before && bodypart == FINGER
             && (action == D_ISOPEN || action == D_CLOSED
                 || action == -D_TRAPPED)) {
        int dmg = rnd(lvl);
        struct obj* gloves = which_armor(mon, W_ARMG);
        if (byu ? Fire_resistance : resists_fire(mon)) {
            dmg /= 2;
            dmg += 1;
        }
        /* gloves give half damage but burn if burnable;
         * padded gloves are designed for heat and negate all damage */
        if (gloves) {
            dmg /= 2;
            if (objdescr_is(gloves, "padded gloves")) {
                dmg = 0;
            }
        }
        if (byu) {
            pline("Ouch!  The knob is red-hot!");
            if (dmg <= 0) {
                pline("Fortunately, your glove protects your %s.", body_part(HAND));
            }
            else {
                losehp(dmg, "a red-hot doorknob", KILLED_BY);
            }
        }
        else {
            if (canseemon && dmg) {
                pline("%s is burned by a red-hot doorknob!", Monnam(mon));
            }
            mon->mhp -= dmg;
            if (DEADMONSTER(mon)) {
                monkilled(mon, "", AD_FIRE);
            }
        }

        /* put this after just so a death won't be preceded by a misleading
            * message about gloves burning */
        if (gloves) {
            erode_obj(gloves, NULL, ERODE_BURN, EF_GREASE);
        }
    }
    else if (selected_trap == FIRE_BLAST && after
             && (action == D_ISOPEN || action == D_BROKEN
                 || action == -D_TRAPPED)) {
        /* TODO: ensure explode() wakes monsters without having to call it
         * here */
        if (byu || canseedoor) {
            pline("KABOOM!!  The door was booby-trapped!");
        }
        else {
            You_hear("a distant explosion.");
        }
        /* need to remove trap now because otherwise explode() will destroy the
         * door and call doortrapped() again */
        set_door_trap(door, FALSE); /* trap is gone */
        explode(x, y, 11, rnd(lvl), TRAPPED_DOOR, EXPL_FIERY);
        /* don't need to set doorstate since the door should be consumed in
         * flames */
    }
    if ((saved_doorstate != D_NODOOR && saved_doorstate != D_BROKEN)
        && (doorstate(door) == D_NODOOR || doorstate(door) == D_BROKEN)) {
        /* door was destroyed during this function */
        if (*in_rooms(x, y, SHOPBASE)) {
            /* always add to shk fix list, but only add to player cost if the
             * player is responsible */
            add_damage(x, y, (byu ? SHOP_DOOR_COST : 0L));
        }
        newsym(x, y);
        unblock_point(x, y); /* can now see through it */
        return 2;
    }
    else if (saved_doorstate != doorstate(door)) {
        newsym(x, y);
        /* take care of vision */
        if (!door_is_closed(door))
            unblock_point(x, y);
        else
            block_point(x, y);
        return 1;
    }
    else {
        return 0;
    }
}

/* Monster is hit by trap. */
/* Note: doesn't work if both obj and d_override are null */
static boolean
thitm(tlev, mon, obj, d_override, nocorpse)
int tlev;
struct monst *mon;
struct obj *obj;
int d_override;
boolean nocorpse;
{
    int strike;
    boolean trapkilled = FALSE;

    if (d_override)
        strike = 1;
    else if (obj)
        strike = (find_mac(mon) + tlev + obj->spe <= rnd(20));
    else
        strike = (find_mac(mon) + tlev <= rnd(20));

    /* Actually more accurate than thitu, which doesn't take
     * obj->spe into account.
     */
    if (!strike) {
        if (obj && cansee(mon->mx, mon->my))
            pline("%s is almost hit by %s!", Monnam(mon), doname(obj));
    } else {
        int dam = 1;

        if (obj && cansee(mon->mx, mon->my))
            pline("%s is hit by %s!", Monnam(mon), doname(obj));
        if (d_override) {
            dam = d_override;
        } else if (obj) {
            dam = dmgval(obj, mon);
            if (dam < 1)
                dam = 1;
            if (mon_hates_material(mon, obj->material)) {
                /* extra damage already applied by dmgval() */
                searmsg(NULL, mon, obj, TRUE);
            }
        }
        mon->mhp -= dam;
        if (DEADMONSTER(mon)) {
            int xx = mon->mx, yy = mon->my;

            /* If a monster dies in a trap on the player's turn (e.g. forced
             * onto one by jousting or staggering blow), the player is probably
             * responsible. */
            if (g.context.mon_moving) {
                monkilled(mon, "", nocorpse ? -AD_RBRE : AD_PHYS);
            }
            else {
                xkilled(mon, nocorpse ? XKILL_NOCORPSE : 0);
            }
            if (DEADMONSTER(mon)) {
                newsym(xx, yy);
                trapkilled = TRUE;
            }
        }
    }
    if (obj && (!strike || d_override)) {
        place_object(obj, mon->mx, mon->my);
        stackobj(obj);
    } else if (obj)
        dealloc_obj(obj);

    return trapkilled;
}

boolean
unconscious()
{
    if (g.multi >= 0)
        return FALSE;

    return (u.usleep
            || (g.nomovemsg
                && (!strncmp(g.nomovemsg, "You awake", 9)
                    || !strncmp(g.nomovemsg, "You regain con", 14)
                    || !strncmp(g.nomovemsg, "You are consci", 14))));
}

static const char lava_killer[] = "molten lava";

boolean
lava_effects()
{
    register struct obj *obj, *obj2;
    int dmg = d(6, 6); /* only applicable for water walking */
    boolean usurvive, boil_away;

    feel_newsym(u.ux, u.uy); /* in case Blind, map the lava here */
    burn_away_slime();
    if (likes_lava(g.youmonst.data))
        return FALSE;

    usurvive = Fire_resistance || (Wwalking && dmg < u.uhp);
    /*
     * A timely interrupt might manage to salvage your life
     * but not your gear.  For scrolls and potions this
     * will destroy whole stacks, where fire resistant hero
     * survivor only loses partial stacks via destroy_item().
     *
     * Flag items to be destroyed before any messages so
     * that player causing hangup at --More-- won't get an
     * emergency save file created before item destruction.
     */
    if (!usurvive)
        for (obj = g.invent; obj; obj = obj->nobj)
            if ((is_organic(obj) || obj->oclass == POTION_CLASS)
                && !obj->oerodeproof
                && objects[obj->otyp].oc_oprop != FIRE_RES
                && obj->otyp != SCR_FIRE && obj->otyp != SPE_FIREBALL
                && !obj_resists(obj, 0, 0)) /* for invocation items */
                obj->in_use = 1;

    /* Check whether we should burn away boots *first* so we know whether to
     * make the player sink into the lava. Assumption: water walking only
     * comes from boots.
     */
    if (uarmf && is_organic(uarmf) && !uarmf->oerodeproof) {
        obj = uarmf;
        pline("%s into flame!", Yobjnam2(obj, "burst"));
        iflags.in_lava_effects++; /* (see above) */
        (void) Boots_off();
        useup(obj);
        iflags.in_lava_effects--;
    }

    if (!Fire_resistance) {
        if (Wwalking) {
            /* Assume three things:
             * 1. The hero is wearing water walking boots (they are the only
             *    source of the water walking property).
             * 2. Water walking boots are always burnable.
             * 3. To be walking on lava, they must be fireproof.
             */
            if (!objects[WATER_WALKING_BOOTS].oc_name_known) {
                Your("boots don't sink into the lava!");
            }
            makeknown(WATER_WALKING_BOOTS);
            uarmf->rknown = 1;
            pline_The("%s here burns you!", hliquid("lava"));
            if (usurvive) {
                losehp(dmg, lava_killer, KILLED_BY); /* lava damage */
                goto burn_stuff;
            }
        } else
            You("fall into the %s!", hliquid("lava"));

        usurvive = Lifesaved || discover;
        if (wizard)
            usurvive = TRUE;

        /* prevent remove_worn_item() -> Boots_off(WATER_WALKING_BOOTS) ->
           spoteffects() -> lava_effects() recursion which would
           successfully delete (via useupall) the no-longer-worn boots;
           once recursive call returned, we would try to delete them again
           here in the outer call (and access stale memory, probably panic) */
        iflags.in_lava_effects++;

        for (obj = g.invent; obj; obj = obj2) {
            obj2 = obj->nobj;
            /* above, we set in_use for objects which are to be destroyed */
            if (obj->otyp == SPE_BOOK_OF_THE_DEAD && !Blind) {
                if (usurvive)
                    pline("%s glows a strange %s, but remains intact.",
                          The(xname(obj)), hcolor("dark red"));
            } else if (obj->in_use) {
                if (obj->owornmask) {
                    if (usurvive)
                        pline("%s into flame!", Yobjnam2(obj, "burst"));
                    remove_worn_item(obj, TRUE);
                }
                useupall(obj);
            }
        }

        iflags.in_lava_effects--;

        /* s/he died... */
        boil_away = (u.umonnum == PM_WATER_ELEMENTAL
                     || u.umonnum == PM_STEAM_VORTEX
                     || u.umonnum == PM_FOG_CLOUD);
        for (;;) {
            u.uhp = -1;
            /* killer format and name are reconstructed every iteration
               because lifesaving resets them */
            g.killer.format = KILLED_BY;
            Strcpy(g.killer.name, lava_killer);
            You("%s...", boil_away ? "boil away" : "burn to a crisp");
            done(BURNING);
            if (safe_teleds(TELEDS_ALLOW_DRAG | TELEDS_TELEPORT))
                break; /* successful life-save */
            /* nowhere safe to land; repeat burning loop */
            pline("You're still burning.");
        }
        You("find yourself back on solid %s.", surface(u.ux, u.uy));
        iflags.last_msg = PLNMSG_BACK_ON_GROUND;
        return TRUE;
    } else if (!Wwalking && (!u.utrap || u.utraptype != TT_LAVA)) {
        boil_away = !Fire_resistance;
        /* if not fire resistant, sink_into_lava() will quickly be fatal;
           hero needs to escape immediately */
        set_utrap((unsigned) (rn1(4, 4) + ((boil_away ? 2 : rn1(4, 12)) << 8)),
                  TT_LAVA);
        You("sink into the %s%s!", hliquid("lava"),
            !boil_away ? ", but it only burns slightly"
                       : " and are about to be immolated");
        if (u.uhp > 1)
            losehp(!boil_away ? 1 : (u.uhp / 2), lava_killer,
                   KILLED_BY); /* lava damage */
    }

 burn_stuff:
    destroy_item(SCROLL_CLASS, AD_FIRE);
    destroy_item(SPBOOK_CLASS, AD_FIRE);
    destroy_item(POTION_CLASS, AD_FIRE);
    ignite_items(g.invent);
    return FALSE;
}

/* called each turn when trapped in lava */
void
sink_into_lava()
{
    static const char sink_deeper[] = "You sink deeper into the lava.";

    if (!u.utrap || u.utraptype != TT_LAVA) {
        ; /* do nothing; this usually won't happen but could after
           * polymorphing from a flier into a ceiling hider and then hiding;
           * allmain() only checks whether the hero is at a lava location,
           * not whether he or she is currently sinking */
    } else if (!is_lava(u.ux, u.uy)) {
        reset_utrap(FALSE); /* this shouldn't happen either */
    } else if (!u.uinvulnerable) {
        /* ordinarily we'd have to be fire resistant to survive long
           enough to become stuck in lava, but it can happen without
           resistance if water walking boots allow survival and then
           get burned up; u.utrap time will be quite short in that case */
        if (!Fire_resistance)
            u.uhp = (u.uhp + 2) / 3;

        u.utrap -= (1 << 8);
        if (u.utrap < (1 << 8)) {
            g.killer.format = KILLED_BY;
            Strcpy(g.killer.name, "molten lava");
            You("sink below the surface and die.");
            burn_away_slime(); /* add insult to injury? */
            done(DISSOLVED);
            /* can only get here via life-saving; try to get away from lava */
            reset_utrap(TRUE);
            /* levitation or flight have become unblocked, otherwise Tport */
            if (!Levitation && !Flying)
                (void) safe_teleds(TELEDS_ALLOW_DRAG | TELEDS_TELEPORT);
        } else if (!u.umoved) {
            /* can't fully turn into slime while in lava, but might not
               have it be burned away until you've come awfully close */
            if (Slimed && rnd(10 - 1) >= (int) (Slimed & TIMEOUT)) {
                pline(sink_deeper);
                burn_away_slime();
            } else {
                Norep(sink_deeper);
            }
            u.utrap += rnd(4);
        }
    }
}

/* called when something has been done (breaking a boulder, for instance)
   which entails a luck penalty if performed on a sokoban level */
void
sokoban_guilt()
{
    if (Sokoban) {
        u.uconduct.sokocheat++;
        change_luck(-1);
        /*
         * TODO:
         *  Issue some feedback so that player can learn that whatever
         *  he/she just did is a naughty thing to do in sokoban and
         *  should probably be avoided in future....
         *
         *  Caveat:  doing this might introduce message sequencing
         *  issues, depending upon feedback during the various actions
         *  which trigger Sokoban luck penalties.
         */
    }
}

/* called when a trap has been deleted or had its ttyp replaced */
static void
maybe_finish_sokoban()
{
    struct trap *t;

    if (Sokoban && !g.in_mklev) {
        /* scan all remaining traps, ignoring any created by the hero;
           if this level has no more pits or holes, the current sokoban
           puzzle has been solved */
        for (t = g.ftrap; t; t = t->ntrap) {
            if (t->madeby_u)
                continue;
            if (t->ttyp == PIT || t->ttyp == HOLE)
                break;
        }
        if (!t) {
            /* we've passed the last trap without finding a pit or hole;
               clear the sokoban_rules flag so that luck penalties for
               things like breaking boulders or jumping will no longer
               be given, and restrictions on diagonal moves are lifted */
            Sokoban = 0; /* clear g.level.flags.sokoban_rules */
            /* TODO: give some feedback about solving the sokoban puzzle
               (perhaps say "congratulations" in Japanese?) */
        }
    }
}

/* Return the string name of the trap type passed in, unless the player is
 * hallucinating, in which case return a random or hallucinatory trap name.
 * If the second argument is true, return the correct trap name even when
 * hallucinating (for things like wizard mode wishing for traps and impossible
 * calls).
 */
const char *
trapname(ttyp, override)
int ttyp;
boolean override;
{
    static const char *halu_trapnames[] = {
        /* riffs on actual nethack traps */
        "bottomless pit", "polymorphism trap", "devil teleporter",
        "falling boulder trap", "anti-anti-magic field", "weeping gas trap",
        "queasy board", "electrified web", "owlbear trap", "sand mine",
        "vibrating triangle",
        /* some traps found in nethack variants */
        "death trap", "disintegration trap", "ice trap", "monochrome trap",
        /* plausible real-life traps */
        "axeblade trap", "pool of boiling oil", "pool of quicksand",
        "field of caltrops", "buzzsaw trap", "spiked floor", "revolving wall",
        "uneven floor", "finger trap", "jack-in-a-box", "yellow snow",
        "booby trap", "rat trap", "poisoned nail", "snare", "whirlpool",
        "trip wire", "roach motel (tm)",
        /* sci-fi */
        "negative space", "tensor field", "singularity", "imperial fleet",
        "black hole", "thermal detonator", "event horizon",
        "entoptic phenomenon",
        /* miscellaneous suggestions */
        "sweet-smelling gas vent", "phone booth", "exploding runes",
        "never-ending elevator", "slime pit", "warp zone", "illusory floor",
        "pile of poo", "honey trap", "tourist trap",
    };
    static char roletrap[33]; /* [17 + 5 + 1] should suffice */

    if (Hallucination && !override) {
        int total_names = TRAPNUM + SIZE(halu_trapnames),
            nameidx = rn2_on_display_rng(total_names + 1);

        if (nameidx == total_names) {
            boolean fem = Upolyd ? u.mfemale : flags.female;

            /* inspired by "tourist trap" */
            copynchars(roletrap,
                       rn2(3) ? ((fem && g.urole.name.f) ? g.urole.name.f
                                                         : g.urole.name.m)
                              : rank_of(u.ulevel, Role_switch, fem),
                       (int) (sizeof roletrap - sizeof " trap"));
            Strcat(roletrap, " trap");
            return lcase(roletrap);
        } else if (nameidx >= TRAPNUM) {
            nameidx -= TRAPNUM;
            return halu_trapnames[nameidx];
        } /* else use an actual trap type */
        if (nameidx != NO_TRAP)
            ttyp = nameidx;
    }
    return defsyms[trap_to_defsym(ttyp)].explanation;
}

/* Ignite ignitable items in the given object chain, due to some external source
 * of fire. The object chain should be somewhere exposed, like someone's open
 * inventory or the floor.
 * This is modeled after destroy_item() somewhat and hopefully will be able to
 * merge into it in the future.
 */
void
ignite_items(objchn)
struct obj *objchn;
{
    struct obj *obj;
    boolean vis = FALSE;
    if (!objchn) {
        return;
    }
    if (objchn->where == OBJ_INVENT) {
        vis = TRUE;
    }
    else if (objchn->where == OBJ_MINVENT) {
        vis = canseemon(objchn->ocarry);
    }
    else if (objchn->where == OBJ_FLOOR) {
        vis = cansee(objchn->ox, objchn->oy);
    }
    else {
        impossible("igniting item in a weird location %d", objchn->where);
        return;
    }
    for (obj = objchn; obj; obj = obj->nobj) {
        if (!(ignitable(obj) || obj->otyp == MAGIC_LAMP)
            /* The Candelabrum requires intention to be lit */
            || obj->otyp == CANDELABRUM_OF_INVOCATION
            || obj->otyp == LANTERN /* doesn't ignite via fire */
            || obj->in_use     /* not available */
            || obj->lamplit) { /* already burning */
            continue;
        }
        begin_burn(obj, FALSE);
        if (vis) {
            pline("%s on fire!", Yobjnam2(obj, "catch"));
        }
    }
}


/*trap.c*/
