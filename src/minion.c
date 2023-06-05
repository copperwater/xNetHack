/* NetHack 3.7	minion.c	$NHDT-Date: 1624322864 2021/06/22 00:47:44 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.60 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2008. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* used to pick among the four basic elementals without worrying whether
   they've been reordered (difficulty reassessment?) or any new ones have
   been introduced (hybrid types added to 'E'-class?) */
static const int elementals[4] = {
    PM_AIR_ELEMENTAL, PM_FIRE_ELEMENTAL,
    PM_EARTH_ELEMENTAL, PM_WATER_ELEMENTAL
};

void
newemin(struct monst *mtmp)
{
    if (!mtmp->mextra)
        mtmp->mextra = newmextra();
    if (!EMIN(mtmp)) {
        EMIN(mtmp) = (struct emin *) alloc(sizeof(struct emin));
        (void) memset((genericptr_t) EMIN(mtmp), 0, sizeof(struct emin));
    }
}

void
free_emin(struct monst *mtmp)
{
    if (mtmp->mextra && EMIN(mtmp)) {
        free((genericptr_t) EMIN(mtmp));
        EMIN(mtmp) = (struct emin *) 0;
    }
    mtmp->isminion = 0;
}

/* count the number of monsters on the level */
int
monster_census(boolean spotted) /* seen|sensed vs all */
{
    struct monst *mtmp;
    int count = 0;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (mtmp->isgd && mtmp->mx == 0)
            continue;
        if (spotted && !canspotmon(mtmp))
            continue;
        if (spotted && !sensemon(mtmp) && M_AP_TYPE(mtmp) != M_AP_NOTHING
            && M_AP_TYPE(mtmp) != M_AP_MONSTER)
            continue; /* concealed mimic */
        ++count;
    }
    return count;
}

/* mon summons a monster */
int
msummon(struct monst *mon)
{
    struct permonst *ptr;
    int dtype = NON_PM, cnt = 0, result = 0, census;
    boolean xlight;
    aligntyp atyp;
    struct monst *mtmp;

    if (mon) {
        ptr = mon->data;

        if (u_wield_art(ART_DEMONBANE) && is_demon(ptr)) {
            if (canseemon(mon))
                pline("%s looks puzzled for a moment.", Monnam(mon));
            return 0;
        }

        atyp = mon->ispriest ? EPRI(mon)->shralign
               : mon->isminion ? EMIN(mon)->min_align
                 : (ptr->maligntyp == A_NONE) ? A_NONE
                   : sgn(ptr->maligntyp);
    } else {
        ptr = &mons[PM_WIZARD_OF_YENDOR];
        atyp = (ptr->maligntyp == A_NONE) ? A_NONE : sgn(ptr->maligntyp);
    }

    if (is_dprince(ptr) || (ptr == &mons[PM_WIZARD_OF_YENDOR])) {
        dtype = (!rn2(20)) ? dprince(atyp) : (!rn2(4)) ? dlord(atyp)
                                                       : ndemon(atyp);
        cnt = !rn2(4) ? 4 : 2;
    } else if (is_dlord(ptr)) {
        dtype = (!rn2(50)) ? dprince(atyp) : (!rn2(20)) ? dlord(atyp)
                                                        : ndemon(atyp);
        cnt = !rn2(4) ? rn1(3, 2) : rn1(2, 1);
    } else if (is_ndemon(ptr)) {
        dtype = (!rn2(20)) ? dlord(atyp) : (!rn2(6)) ? ndemon(atyp)
                                                     : monsndx(ptr);
        cnt = 1;
    } else if (is_lminion(mon)) {
        dtype = (is_lord(ptr) && !rn2(20))
                    ? llord()
                    : (is_lord(ptr) || !rn2(6)) ? lminion() : monsndx(ptr);
        cnt = ((dtype != NON_PM)
               && !rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
    } else if (ptr == &mons[PM_ANGEL]) {
        /* non-lawful angels can also summon */
        if (!rn2(6)) {
            switch (atyp) { /* see summon_minion */
            case A_NEUTRAL:
                dtype = elementals[rn2(SIZE(elementals))];
                break;
            case A_CHAOTIC:
            case A_NONE:
                dtype = ndemon(atyp);
                break;
            }
        } else {
            dtype = PM_ANGEL;
        }
        cnt = ((dtype != NON_PM)
               && !rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
    }

    /* put overrides for specific summoners here, but note they have to check
     * for dtype being NON_PM before using it */
    if (ptr == &mons[PM_DISPATER]
        && ((gm.mvitals[PM_PIT_FIEND].mvflags & G_GONE) == 0)
        && (dtype == NON_PM || is_ndemon(&mons[dtype])) && !rn2(2)) {
        /* Dispater favors pit fiends, despite them being chaotic */
        dtype = PM_PIT_FIEND;
    }

    if (dtype == NON_PM)
        return 0;

    /* sanity checks */
    if (cnt > 1 && (mons[dtype].geno & G_UNIQ) != 0)
        cnt = 1;
    /*
     * If this daemon is unique and being re-summoned (the only way we
     * could get this far with an extinct dtype), try another.
     */
    if ((gm.mvitals[dtype].mvflags & G_GONE) != 0) {
        dtype = ndemon(atyp);
        if (dtype == NON_PM)
            return 0;
    }

    /* some candidates can generate a group of monsters, so simple
       count of non-null makemon() result is not sufficient */
    census = monster_census(FALSE);
    xlight = FALSE;

    while (cnt > 0) {
        mtmp = makemon(&mons[dtype], u.ux, u.uy, MM_EMIN|MM_NOMSG);
        if (mtmp) {
            result++;
            /* an angel's alignment should match the summoner */
            if (dtype == PM_ANGEL) {
                mtmp->isminion = 1;
                EMIN(mtmp)->min_align = atyp;
                /* renegade if same alignment but not peaceful
                   or peaceful but different alignment */
                EMIN(mtmp)->renegade =
                    (atyp != u.ualign.type) ^ !mtmp->mpeaceful;
            }

            if (mtmp->data->mlet == S_ANGEL && !Blind) {
                /* for any 'A', 'cloud of smoke' will be 'flash of light';
                   if more than one monster is being created, that message
                   might be skipped for this monster but show 'mtmp' anyway */
                show_transient_light((struct obj *) 0, mtmp->mx, mtmp->my);
                xlight = TRUE;
                /* we don't do this for 'burst of flame' (fire elemental)
                   because those monsters become their own light source */
            }

            if (cnt == 1 && canseemon(mtmp)) {
                const char *cloud = 0,
                           *what = msummon_environ(mtmp->data, &cloud);

                if (!boss_entrance(mtmp))
                    pline("%s appears in a %s of %s!", Amonnam(mtmp),
                          cloud, what);
            }
        }
        cnt--;
    }

    if (xlight) {
        /* Note: if we forced --More-- here, the 'A's would be visible for
           long enough to be seen, but like with clairvoyance, some players
           would be annoyed at the disruption of having to acknowledge it */
        transient_light_cleanup();
    }

    /* how many monsters exist now compared to before? */
    if (result)
        result = monster_census(FALSE) - census;

    return result;
}

void
summon_minion(aligntyp alignment, boolean talk)
{
    register struct monst *mon;
    int mnum;

    switch ((int) alignment) {
    case A_LAWFUL:
        mnum = lminion();
        break;
    case A_NEUTRAL:
        mnum = elementals[rn2(SIZE(elementals))];
        break;
    case A_CHAOTIC:
    case A_NONE:
        mnum = ndemon(alignment);
        break;
    default:
        impossible("unaligned player?");
        mnum = ndemon(A_NONE);
        break;
    }
    if (mnum == NON_PM) {
        mon = 0;
    } else if (mnum == PM_ANGEL) {
        mon = makemon(&mons[mnum], u.ux, u.uy, MM_EMIN|MM_NOMSG);
        if (mon) {
            mon->isminion = 1;
            EMIN(mon)->min_align = alignment;
            EMIN(mon)->renegade = FALSE;
        }
    } else if (mnum != PM_SHOPKEEPER && mnum != PM_GUARD
               && mnum != PM_ALIGNED_CLERIC && mnum != PM_HIGH_CLERIC) {
        /* This was mons[mnum].pxlth == 0 but is this restriction
           appropriate or necessary now that the structures are separate? */
        mon = makemon(&mons[mnum], u.ux, u.uy, MM_EMIN|MM_NOMSG);
        if (mon) {
            mon->isminion = 1;
            EMIN(mon)->min_align = alignment;
            EMIN(mon)->renegade = FALSE;
        }
    } else {
        mon = makemon(&mons[mnum], u.ux, u.uy, MM_NOMSG);
    }
    if (mon) {
        if (talk) {
            if (!Deaf)
                pline_The("voice of %s booms:", align_gname(alignment));
            else
                You_feel("%s booming voice:",
                         s_suffix(align_gname(alignment)));
            SetVoice(mon, 0, 80, 0);
            verbalize("Thou shalt pay for thine indiscretion!");
            if (canspotmon(mon))
                pline("%s appears before you.", Amonnam(mon));
            mon->mstrategy &= ~STRAT_APPEARMSG;
        }
        mon->mpeaceful = FALSE;
        /* don't call set_malign(); player was naughty */
    }
}

/* A boss monster (if not yet encountered, as determined by STRAT_APPEARMSG)
 * makes a dramatic entrance.
 * Might not actually be passed a boss. Return TRUE if it is a boss and we did
 * print the dramatic entrance; FALSE otherwise. */
boolean
boss_entrance(struct monst* mtmp)
{
    struct permonst* mdat = mtmp->data;
    int mondx = monsndx(mdat);
    char name_appears[BUFSZ]; /* holds "Foo_appears", as lua key */
    char* iter;

    if (!(is_archfiend(mdat) || is_rider(mdat)
          || mondx == PM_VLAD_THE_IMPALER || mondx == PM_WIZARD_OF_YENDOR)) {
        /* no appearance message exists for this type of monster */
        return FALSE;
    }

    if (mdat->msound == MS_NEMESIS || mdat->msound == MS_LEADER) {
        /* this could be a demon quest nemesis/leader, who will have their own
         * dialog */
        return FALSE;
    }

    if (gm.mvitals[mondx].died > 0) {
        /* Never print entrance message if the player already killed it. */
        return FALSE;
    }

    if (!canspotmon(mtmp)) {
        /* Assume the messages depend on you being able to spot it, so no
         * dramatic entrance if you can't. */
        return FALSE;
    }

    /* Never print message if this monster isn't marked to give one. */
    if ((mtmp->mstrategy & STRAT_APPEARMSG) == 0) {
        return FALSE;
    }
    /* ... and then turn off any other appearance message they were going to
     * get. */
    mtmp->mstrategy &= ~STRAT_APPEARMSG;

    Sprintf(name_appears, "%s_appears", mdat->pmnames[NEUTRAL]);
    while ((iter = strstr(name_appears, " ")) != (char*) 0) {
        *iter = '_';
    }
    com_pager((const char*) name_appears);
    return TRUE;
}

#define Athome (Inhell && (mtmp->cham == NON_PM))

/* How much is obj worth to demon lord if they demand it as a bribe?
 * Return a number whose units are zorkmids (the amount they will take off their
 * original bribe amount if given this). This loosely relates to cost, but
 * doesn't have to; in particular, the demon lord would rather have rare items
 * than more coins, so they may be worth more here than their equivalent if
 * bought at a shop.
 * Return 0 if the demon doesn't care about this item, or it's ineligible and
 * they should never consider taking it. */
static long
demon_value(struct obj *obj)
{
    const short otyp = obj->otyp;
    long baseval = 0;

    /* Ineligible stuff goes first.
     * (should cursed welded items be ineligible? or handwave it as the demon
     * lord easily removing the curse for free?) */
    if (objects[otyp].oc_unique)
        return 0;
    if (obj == uskin)
        return 0;

    baseval = objects[otyp].oc_cost * 3; /* baseline */

    /* cases with some complexity in calculation */
    if (obj->oartifact)
        return arti_cost(obj);
    else if (Has_contents(obj)) {
        struct obj *otmp;
        for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
            baseval += demon_value(otmp); /* recurse further into containers */
        }
        return baseval;
    }
    else if (obj->oclass == ARMOR_CLASS && objects[otyp].oc_magic) {
        if (Is_dragon_scaled_armor(obj))
            baseval += (objects[obj->dragonscales].oc_cost * 3);
        return baseval;
    }
    else if (obj->oclass == WEAPON_CLASS && obj->spe >= 6) {
        /* baseval is probably pretty low, the high enchantment is the prize
         * here */
        return baseval + (obj->spe * 50);
    }
    /* Cases where we want to inflate the value even more than 3 * base cost. */
    else if (otyp == WAN_WISHING || otyp == MAGIC_LAMP
             || (In_cocytus(&u.uz) && otyp == RIN_COLD_RESISTANCE))
        return baseval * 2;
    /* Then, any case where the demon lord is interested in something for the
     * plain base value.
     * For rings and amulets, they focus on ones the player is wearing rather
     * than random ones they may happen to have, on the guess that those are
     * more valuable to the player. */
    else if ((obj->oclass == SPBOOK_CLASS && objects[otyp].oc_level >= 6)
             || (obj->oclass == WAND_CLASS && obj->spe > 3)
             || otyp == MAGIC_MARKER
             || obj == uleft || obj == uright || obj == uamul)
        return baseval;

    return 0;
}

/* returns 1 if it won't attack. */
int
demon_talk(register struct monst *mtmp)
{
    long cash, demand, offer = 0L;
    struct obj *otmp, *shiny = (struct obj *) 0;
    int items_given = 0;

    if (u_wield_art(ART_EXCALIBUR) || u_wield_art(ART_DEMONBANE)) {
        if (canspotmon(mtmp))
            pline("%s looks very angry.", Amonnam(mtmp));
        else
            You_feel("tension building.");
        mtmp->mpeaceful = mtmp->mtame = 0;
        set_malign(mtmp);
        newsym(mtmp->mx, mtmp->my);
        return 0;
    }

    if (is_fainted()) {
        reset_faint(); /* if fainted - wake up */
    } else {
        stop_occupation();
        if (gm.multi > 0) {
            nomul(0);
            unmul((char *) 0);
        }
    }

    /* Slight advantage given. */
    if (is_dprince(mtmp->data)) {
        mtmp->minvis = mtmp->perminvis = 0;
        if ((mtmp->mstrategy & STRAT_APPEARMSG) && !boss_entrance(mtmp)) {
            /* you still can't see the demon; this can happen if you are blind
             * and non-telepathic. */
            mtmp->mstrategy &= ~STRAT_APPEARMSG;
        }
        newsym(mtmp->mx, mtmp->my);
    }
    if (gy.youmonst.data->mlet == S_DEMON) { /* Won't blackmail their own. */
        if (!Deaf)
            pline("%s says, \"Good hunting, %s.\"", Amonnam(mtmp),
                  flags.female ? "Sister" : "Brother");
        else if (canseemon(mtmp))
            pline("%s says something.", Amonnam(mtmp));
        if (!tele_restrict(mtmp))
            (void) rloc(mtmp, RLOC_MSG);
        return 1;
    }

    /* Bribable monsters are very greedy, and don't care how much gold you
     * appear to be carrying. They are capricious, and may demand truly
     * exorbitant amounts; however, it might be risky to challenge a hero who
     * has reached them, since they do not want to end up dead. */
    demand = d(50,1000);
    cash = money_cnt(gi.invent);
    verbalize("Mortal, if thou canst pay, I shall not hinder thee later.");

    /* First, they may want some of your valuables more than gold. See if they
     * do. */
    do {
        long total_value = 0;
        shiny = (struct obj *) 0;

        for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
            long value = demon_value(otmp);
            if (value < 1)
                continue;
            total_value += value;
            if (rn2(total_value) < value)
                shiny = otmp;
        }

        if (shiny) {
            verbalize("I see thou hast %s in thy possession...",
                      an(xname(shiny)));
            if (y_n("Give up your item?") != 'y') {
                You("refuse.");
                pline("%s gets angry...", Amonnam(mtmp));
                mtmp->mpeaceful = 0;
                set_malign(mtmp);
                newsym(mtmp->mx, mtmp->my);
                return 0;
            }

            if (shiny->owornmask) {
                remove_outer_gear(shiny);
                remove_worn_item(shiny, TRUE);
            }
            items_given++;
            demand -= demon_value(shiny);
            freeinv(shiny);
            if (shiny->oartifact == ART_DEMONBANE) {
                pline("%s seizes it!", Monnam(mtmp));
                pline("Laughing evilly, %s engulfs it in flames, melting it.",
                      mhe(mtmp));
                obfree(shiny, (struct obj *) 0);
            }
            else {
                pline("%s greedily takes it.", Monnam(mtmp));
                (void) mpickobj(mtmp, shiny); /* could merge and free shiny but
                                               * won't */ }
        }
    } while (demand > 0 && rn2(4));

    if (demand <= 0) { /* forfeited enough items to satisfy mtmp */
        verbalize("Very well, mortal. I shall not impede thy quest.");
        pline("%s vanishes, laughing to %sself.", Amonnam(mtmp), mhis(mtmp));
        livelog_printf(LL_UMONST, "bribed %s with %d items for safe passage",
                        Amonnam(mtmp), items_given);
        /* fall through to mongone() */
    }
    else if (items_given == 0 && (cash == 0 || gm.multi < 0)) {
        /* you have no gold or can't move */
        pline("%s roars:", Amonnam(mtmp));
        verbalize("You bring me no tribute?  Then you must die!");
        mtmp->mpeaceful = 0;
        set_malign(mtmp);
        newsym(mtmp->mx, mtmp->my);
        return 0;
    } else {
        /* make sure that the demand is unmeetable if the monster
           has the Amulet, preventing monster from being satisfied
           and removed from the game (along with said Amulet...) */
        /* [actually the Amulet is safe; it would be dropped when
           mongone() gets rid of the monster; force combat anyway;
           also make it unmeetable if the player is Deaf, to simplify
           handling that case as player-won't-pay] */
        if (mon_has_amulet(mtmp) || Deaf)
            /* 125: 5*25 in case hero has maximum possible charisma */
            demand = cash + (long) rn1(1000, 125);

        if (!Deaf)
            pline("%s%s demands %ld %s for safe passage.",
                  Amonnam(mtmp),
                  (items_given > 0) ? " also" : "",
                  demand, currency(demand));
        else if (canseemon(mtmp))
            pline("%s seems to be demanding your money.", Amonnam(mtmp));

        offer = bribe(mtmp);
        if (offer >= demand) {
            verbalize("Very well, mortal. I shall not impede thy quest.");
            pline("%s vanishes, laughing about cowardly mortals.",
                  Amonnam(mtmp));
            livelog_printf(LL_UMONST, "bribed %s with %ld %s for safe passage",
                           Amonnam(mtmp), offer, currency(offer));
        }
        else if (offer == cash && offer > 0 && cash >= demand / 2 && !rn2(50)) {
            /* monster may rarely take pity on you if you hand over everything
             * you have */
            pline("%s grudgingly grabs your money bag, then vanishes.",
                    Amonnam(mtmp));
            livelog_printf(LL_UMONST,
                           "turned out their pockets for %s for safe passage",
                           Amonnam(mtmp));
        }
        else if((long) rnd(100 * ACURR(A_CHA)) > (demand - offer)) {
            /* monster may also decide that being shortchanged by a small
             * amount isn't worth risking their life */
            pline("%s scowls at you menacingly, then vanishes.",
                  Amonnam(mtmp));
            livelog_printf(LL_UMONST,
                           "shortchanged %s with %ld %s for safe passage",
                           Amonnam(mtmp), offer, currency(offer));
        }
        else {
            pline("%s gets angry...", Amonnam(mtmp));
            mtmp->mpeaceful = 0;
            set_malign(mtmp);
            newsym(mtmp->mx, mtmp->my);
            return 0;
        }
    }
    mongone(mtmp);
    return 1;
}

long
bribe(struct monst *mtmp)
{
    char buf[BUFSZ] = DUMMY;
    long offer;
    long umoney = money_cnt(gi.invent);

    getlin("How much will you offer?", buf);
    if (sscanf(buf, "%ld", &offer) != 1)
        offer = 0L;

    /*Michael Paddon -- fix for negative offer to monster*/
    /*JAR880815 - */
    if (offer < 0L) {
        You("try to shortchange %s, but fumble.", mon_nam(mtmp));
        return 0L;
    } else if (offer == 0L) {
        You("refuse.");
        return 0L;
    } else if (offer >= umoney) {
        You("give %s all your gold.", mon_nam(mtmp));
        offer = umoney;
    } else if (offer == umoney / 10 && mtmp->ispriest) {
        You("pay your tithe to %s.", mon_nam(mtmp));
    } else {
        You("give %s %ld %s.", mon_nam(mtmp), offer, currency(offer));
    }
    (void) money2mon(mtmp, offer);
    gc.context.botl = 1;
    return offer;
}

int
dprince(aligntyp atyp)
{
    int tryct, pm;

    for (tryct = !In_endgame(&u.uz) ? 20 : 0; tryct > 0; --tryct) {
        pm = rn1(PM_DEMOGORGON + 1 - PM_ORCUS, PM_ORCUS);
        if (!(gm.mvitals[pm].mvflags & G_GONE)
            && (atyp == A_NONE || sgn(mons[pm].maligntyp) == sgn(atyp)))
            return pm;
    }
    return dlord(atyp); /* approximate */
}

int
dlord(aligntyp atyp)
{
    int tryct, pm;

    for (tryct = !In_endgame(&u.uz) ? 20 : 0; tryct > 0; --tryct) {
        pm = rn1(PM_YEENOGHU + 1 - PM_JUIBLEX, PM_JUIBLEX);
        /* This previously checked G_GONE (as dprince still does), but Juiblex
         * is weird in that he splits, and in order to split he doesn't get
         * G_EXTINCT set. Instead, check the born counter (which also works fine
         * for Yeenoghu or any other archfiend since the outcome is the same -
         * if they have already been generated, they can't be summoned. */
        if (!(gm.mvitals[pm].born)
            && (atyp == A_NONE || sgn(mons[pm].maligntyp) == sgn(atyp)))
            return pm;
    }
    return ndemon(atyp); /* approximate */
}

/* create lawful (good) lord */
int
llord(void)
{
    if (!(gm.mvitals[PM_ARCHON].mvflags & G_GONE))
        return PM_ARCHON;

    return lminion(); /* approximate */
}

int
lminion(void)
{
    int tryct;
    struct permonst *ptr;

    for (tryct = 0; tryct < 20; tryct++) {
        ptr = mkclass(S_ANGEL, 0);
        if (ptr && !is_lord(ptr))
            return monsndx(ptr);
    }

    return NON_PM;
}

int
ndemon(aligntyp atyp) /* A_NONE is used for 'any alignment' */
{
    struct permonst *ptr;

    /*
     * 3.6.2:  [fixed #H2204, 22-Dec-2010, eight years later...]
     * pick a correctly aligned demon in one try.  This used to
     * use mkclass() to choose a random demon type and keep trying
     * (up to 20 times) until it got one with the desired alignment.
     * mkclass_aligned() skips wrongly aligned potential candidates.
     * [The only neutral demons are djinni and mail daemon and
     * mkclass() won't pick them, but call it anyway in case either
     * aspect of that changes someday.]
     */
#if 0
    if (atyp == A_NEUTRAL)
        return NON_PM;
#endif
    ptr = mkclass_aligned(S_DEMON, 0, atyp);
    return (ptr && is_ndemon(ptr)) ? monsndx(ptr) : NON_PM;
}

/* guardian angel has been affected by conflict so is abandoning hero */
void
lose_guardian_angel(struct monst *mon) /* if null, angel hasn't been created yet */
{
    coord mm;
    int i;

    if (mon) {
        if (canspotmon(mon)) {
            if (!Deaf) {
                pline("%s rebukes you, saying:", Monnam(mon));
                SetVoice(mon, 0, 80, 0);
                verbalize("Since you desire conflict, have some more!");
            } else {
                pline("%s vanishes!", Monnam(mon));
            }
        }
        mongone(mon);
    }
    /* create 2 to 4 hostile angels to replace the lost guardian */
    for (i = rn1(3, 2); i > 0; --i) {
        mm.x = u.ux;
        mm.y = u.uy;
        if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL]))
            (void) mk_roamer(&mons[PM_ANGEL], u.ualign.type, mm.x, mm.y,
                             FALSE);
    }
}

/* just entered the Astral Plane; receive tame guardian angel if worthy */
void
gain_guardian_angel(void)
{
    struct monst *mtmp;
    struct obj *otmp;
    coord mm;

    Hear_again(); /* attempt to cure any deafness now (divine
                     message will be heard even if that fails) */
    if (Conflict) {
       if (!Deaf)
            pline("A voice booms:");
        else
            You_feel("a booming voice:");
        SetVoice((struct monst *) 0, 0, 80, voice_deity);
        verbalize("Thy desire for conflict shall be fulfilled!");
        /* send in some hostile angels instead */
        lose_guardian_angel((struct monst *) 0);
    } else if (u.ualign.record > 8 && u.uconduct.pets) { /* fervent */
        /* Too nasty for the game to unexpectedly break petless conduct on
         * the final level of the game. If you have been petless thus far, your
         * god will respect your decision, and won't send an angel. */
        if (!Deaf)
            pline("A voice whispers:");
        else
            You_feel("a soft voice:");
        SetVoice((struct monst *) 0, 0, 80, voice_deity);
        verbalize("Thou hast been worthy of me!");
        mm.x = u.ux;
        mm.y = u.uy;
        if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL])
            && (mtmp = mk_roamer(&mons[PM_ANGEL], u.ualign.type, mm.x, mm.y,
                                 TRUE)) != 0) {
            mtmp->mstrategy &= ~STRAT_APPEARMSG;
            /* guardian angel -- the one case mtame doesn't imply an
             * edog structure, so we don't want to call tamedog().
             * [Note: this predates mon->mextra which allows a monster
             * to have both emin and edog at the same time.]
             */
            mtmp->mtame = 10;
            u.uconduct.pets++;
            /* for 'hilite_pet'; after making tame, before next message */
            newsym(mtmp->mx, mtmp->my);
            if (!Blind)
                pline("An angel appears near you.");
            else
                You_feel("the presence of a friendly angel near you.");
            /* make him strong enough vs. endgame foes */
            mtmp->m_lev = rn1(8, 15);
            mtmp->mhp = mtmp->mhpmax =
                d((int) mtmp->m_lev, 10) + 30 + rnd(30);
            if ((otmp = select_hwep(mtmp)) == 0) {
                otmp = mksobj(SABER, FALSE, FALSE);
                if (mpickobj(mtmp, otmp))
                    panic("merged weapon?");
            }
            bless(otmp);
            if (otmp->spe < 4)
                otmp->spe += rnd(4);
            if ((otmp = which_armor(mtmp, W_ARMS)) == 0
                || otmp->otyp != SHIELD_OF_REFLECTION) {
                (void) mongets(mtmp, AMULET_OF_REFLECTION);
                m_dowear(mtmp, TRUE);
            }
        }
    }
}

/* special bonus for Geryon towards damage and regeneration on his level, based
 * on the number of his herd that is present */
int
geryon_bonus(void)
{
    static long lastturncheck = -1;
    static int bonus = 0;
    struct monst *mtmp;

    if (gm.moves == lastturncheck)
        return bonus; /* already calculated this turn */

    lastturncheck = gm.moves;
    bonus = 0;
    if (Is_geryon_level(&u.uz)) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (mtmp->data->mlet == S_QUADRUPED && !mtmp->mtame) {
                bonus++;
            }
        }
    }
    else {
        /* if he was summoned away from his level, give a middling amount
         * (assuming it typically ranges from 40 to 0) */
        bonus = 20;
    }
    return bonus;
}

/* you have ticked off Geryon by killing a member of his herd; assumes caller
 * has checked for the conditions like being on his level already */
void
angry_geryon(void)
{
    struct monst *geryon;
    boolean givemsg = FALSE;
    for (geryon = fmon; geryon; geryon = geryon->nmon) {
        if (geryon->data == &mons[PM_GERYON]) {
            if (geryon->mhp * 5 > geryon->mhpmax
                && !monnear(geryon, u.ux, u.uy)) {
                if (geryon->mpeaceful)
                    givemsg = TRUE;
                geryon->mpeaceful = FALSE;
                mnexto(geryon, RLOC_NONE);
            }
            break;
        }
    }
    /* Cases where Geryon comes back mad and should be spawned in, assuming
     * he is not present on the level and num_in_dgn is not indicating he's
     * somewhere else:
     *
     * spawn geryon if
     * born = 0, died = 0 (never generated)
     * born = 1, died = 0 (bribed or escaped dungeon)
     * born = X+1, died = X (repeated resurrecting followed by bribe/escape)
     *
     * DON'T spawn geryon if
     * born = 1, died = 1
     * born = X, died = X (repeated resurrecting ending with a kill)
     */
    if (!geryon && (lookup_fiend(PM_GERYON)->num_in_dgn == 0)
        && (gm.mvitals[PM_GERYON].born == 0
            || gm.mvitals[PM_GERYON].born > gm.mvitals[PM_GERYON].died)) {
        /* Geryon hasn't generated yet OR has generated and been bribed
         * away, in which case he comes back mad; generate him now */
        boolean never_generated = (gm.mvitals[PM_GERYON].born == 0);
        geryon = makemon(&mons[PM_GERYON], u.ux, u.uy,
                         MM_NOMSG | MM_ADJACENTOK);
        if (!geryon) {
            impossible("Geryon spawning failed");
        }
        if (never_generated)
            boss_entrance(geryon);
        geryon->mpeaceful = FALSE;
        newsym(geryon->mx, geryon->my); /* remove peaceful symbol */
        givemsg = TRUE;
    }
    if (givemsg) {
        verbalize("Thou durst harm my herds, mortal?  Die!");
    }
}

/* At the start of the game, set up the archfiend structures and randomly select
 * which archfiends should have wands of wishing in their lairs. */
#define NUM_ARCHFIEND_WISHES 3
void
init_archfiends(void)
{
    int i;
    /* first just set the basic constants */
    for (i = FIRST_ARCHFIEND; i <= LAST_ARCHFIEND; ++i) {
        struct fiend_info *tmpinfo = lookup_fiend(i);
        tmpinfo->mndx = i;
        tmpinfo->num_in_dgn = 0;
        tmpinfo->escaped = FALSE;
    }
    /* now randomly choose some of them to have wishes
     * Juiblex is omitted here because his lair is open and doesn't really have
     * anywhere to stash a wand of wishing safely. */
    static int elig_fiends[7] = {
        PM_YEENOGHU, PM_ORCUS, PM_GERYON, PM_DISPATER, PM_ASMODEUS,
        PM_BAALZEBUB, PM_DEMOGORGON
    };
    shuffle_int_array(elig_fiends, 7);
    for (i = 0; i < NUM_ARCHFIEND_WISHES; ++i) {
        lookup_fiend(elig_fiends[i])->has_wish = TRUE;
    }
}

/* Obtain a pointer to the fiend_info struct that stores data about the
 * given archfiend. */
struct fiend_info *
lookup_fiend(int mndx) {
    return &gc.context.archfiends[mndx - FIRST_ARCHFIEND];
}

/* Should the given archfiend be giving the player a bad effect?
 * Up to the caller to decide what the bad effect is and when it should be
 * triggered. This function only tests whether the fiend is out of the picture
 * or not, or whether they haven't yet started leaning on the scales. */
boolean
fiend_adversity(int mndx)
{
    struct fiend_info *fnd = lookup_fiend(mndx);

    if (!u.uevent.uamultouch)
        /* this only kicks in after you have obtained the Amulet */
        return FALSE;

    if (fnd->escaped)
        /* they are always going to be exerting their influence and you can't do
         * anything about it. It's this way because if them escaping meant they
         * wouldn't bother you anymore, it would probably be easier to do that
         * than actually dealing with them */
        return TRUE;
    else if (fnd->num_in_dgn > 0)
        /* they are alive and kicking somewhere in the dungeon */
        return TRUE;
    else if (gm.mvitals[mndx].born == 0)
        /* they were never encountered, thus not dealt with */
        return TRUE;

    /* at this point, we could differentiate bribing from killing if we wanted
     * because bribing cases will have born > died, but we don't care because
     * either one means they've been dealt with */
    return FALSE;
}

/* helper function that returns the number of fiends still in play */
int
active_fiends(void)
{
    int mndx;
    int num_active = 0;
    for (mndx = FIRST_ARCHFIEND; mndx <= LAST_ARCHFIEND; ++mndx) {
        if (fiend_adversity(mndx))
            num_active++;
    }
    return num_active;
}

/* When the player first acquires the Amulet of Yendor, un-dealt-with archfiends
 * start applying debuffs. Give an indication of this, and take care of any
 * debuffs that should trigger immediately. */
void
start_fiend_harassment(void)
{
    /* first just see how many fiends are still in play so we can give the
     * message first */
    int num_active = active_fiends();
    if (num_active > 0) {
        You("feel the malevolent attention of %s archfiend%s.",
            num_active > 3 ? "several"
                           : num_active > 1 ? "some" : "an",
            num_active == 1 ? "" : "s");
        pline("An oppressive weight seems to settle on you.");
    }
    /* now do specific fiend effects or checks that should happen immediately */
    if (fiend_adversity(PM_GERYON))
        (void) encumber_msg();
}

/*minion.c*/
