/* NetHack 5.0	mcastu.c	$NHDT-Date: 1770949988 2026/02/12 18:33:08 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.111 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#define MCASTU_ENUM
enum mcast_spells {
    #include "mcastu.h"
};
#undef MCASTU_ENUM

struct _mcast_data {
    int level;
    int flags;
};

#define MCASTU_INIT
static struct _mcast_data mcast_data[] = {
    #include "mcastu.h"
};
#undef MCASTU_INIT

/* spell lists for specific monster casters */
/* the spells in the list should be in ascending level order */
static int mon_cleric_spells[] = {
    MCAST_OPEN_WOUNDS, MCAST_CURE_SELF, MCAST_CONFUSE_YOU, MCAST_PARALYZE,
    MCAST_BLIND_YOU, MCAST_INSECTS, MCAST_CURSE_ITEMS, MCAST_LIGHTNING,
    MCAST_FIRE_PILLAR, MCAST_GEYSER
};
static int mon_wizard_spells[] = {
    MCAST_PSI_BOLT, MCAST_CURE_SELF, MCAST_HASTE_SELF, MCAST_STUN_YOU,
    MCAST_DISAPPEAR, MCAST_WEAKEN_YOU, MCAST_DESTRY_ARMR, MCAST_CURSE_ITEMS,
    MCAST_AGGRAVATION, MCAST_SUMMON_MONS, MCAST_CLONE_WIZ, MCAST_DEATH_TOUCH
};

staticfn void cursetxt(struct monst *, boolean);
staticfn int choose_monster_spell(struct monst *, int);
staticfn int m_cure_self(struct monst *, int);
staticfn void mcast_death_touch(struct monst *);
staticfn void mcast_clone_wiz(struct monst *);
staticfn void mcast_summon_mons(struct monst *);
staticfn void mcast_destroy_armor(void);
staticfn void mcast_weaken_you(struct monst *, int);
staticfn void mcast_disappear(struct monst *);
staticfn void mcast_stun_you(struct monst *, int);
staticfn int mcast_geyser(int);
staticfn int mcast_fire_pillar(struct monst *, int);
staticfn int mcast_lightning(struct monst *, int);
staticfn int mcast_psi_bolt(int);
staticfn int mcast_open_wounds(int);
staticfn void mcast_insects(struct monst *);
staticfn void mcast_blind_you(void);
staticfn int mcast_paralyze(struct monst *);
staticfn void mcast_confuse_you(struct monst *);
staticfn void mcast_spell(struct monst *, int, int);
staticfn boolean is_undirected_spell(int);
staticfn boolean spell_would_be_useless(struct monst *, int);
/* xNetHack additions: */
staticfn void mcast_entomb(void);
staticfn void mcast_tport_away(struct monst *);
staticfn void mcast_dark_speech(struct monst *);
staticfn void mcast_sheer_cold(int *);
staticfn void mcast_blight(void);
staticfn void mcast_disenchant(struct monst *);
staticfn boolean has_special_spell_list(struct permonst *);
staticfn int choose_special_spell(struct monst *);
staticfn boolean is_entombed(coordxy, coordxy);

/* feedback when frustrated monster couldn't cast a spell */
staticfn void
cursetxt(struct monst *mtmp, boolean undirected)
{
    if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
        const char *point_msg; /* spellcasting monsters are impolite */

        if (undirected)
            point_msg = "all around, then curses";
        else if ((Invis && !perceives(mtmp->data)
                  && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                 || is_obj_mappear(&gy.youmonst, STRANGE_OBJECT)
                 || u.uundetected)
            point_msg = "and curses in your general direction";
        else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
            point_msg = "and curses at your displaced image";
        else
            point_msg = "at you, then curses";

        pline_mon(mtmp, "%s points %s.", Monnam(mtmp), point_msg);
    } else if ((!(svm.moves % 4) || !rn2(4))) {
        if (!Deaf)
            Norep("You hear a mumbled curse.");   /* Deaf-aware */
    }
}

/* choose a spell for monster to cast */
staticfn int
choose_monster_spell(struct monst *mtmp, int adtyp)
{
    int *list = NULL;
    int i, spellval, len = 0;
    int maxlev;

    /* which spell list to use? */
    if (adtyp == AD_SPEL) {
        list = mon_wizard_spells;
        len = SIZE(mon_wizard_spells);
    } else if (adtyp == AD_CLRC) {
        list = mon_cleric_spells;
        len = SIZE(mon_cleric_spells);
    }

    if (!list || len < 1)
        return MCAST_PSI_BOLT;

    /* max spell level in this monster spell list */
    maxlev = mcast_data[list[len - 1]].level;

    /* which level spell to cast? */
    spellval = rn2(mtmp->m_lev);
    if (spellval > maxlev && rn2(maxlev))
        spellval = rn2(maxlev);

    /* find the highest spell in the list we could cast */
    for (i = len-1; i >= 0; i--)
        if (mcast_data[list[i]].level <= spellval
            && !spell_would_be_useless(mtmp, list[i]))
            return list[i];

    /* or return the first spell in the list */
    return list[0];
}

/* does mdat, a spellcaster, use a special spell list and avoid the normal mage
 * spell / cleric spell dichotomy?
 * Note that because of the logic in castmu() calling either cast_wizard_spell
 * or cast_cleric_spell based on the adtyp, it's not currently possible to
 * construct a custom spell with spells mismatching the adtyp (if one were to
 * try, the spells from the type not matching the adtyp would be treated as
 * those matching the adtyp - a mage-spell caster attempting to cast
 * MCAST_OPEN_WOUNDS would instead cast MCAST_PSI_BOLT, etc.) Refactoring would be
 * needed to make this work.
 */
staticfn boolean
has_special_spell_list(struct permonst *mdat)
{
    /* Add more monsters to this list as needed. */
    switch (monsndx(mdat)) {
    case PM_DISPATER:
    case PM_ASMODEUS:
    case PM_DEMOGORGON:
    case PM_ORCUS:
        return TRUE;
    default:
        return FALSE;
    }
}

/* mdat is a spellcaster with a special spell list; return a spell from its list
 */
staticfn int
choose_special_spell(struct monst *mtmp)
{
    if (mtmp->data == &mons[PM_DISPATER]) {
        /* Dispater is defense-oriented and doesn't really cast direct attack
         * spells. Instead he tries to keep away from you and prevent you from
         * getting to him. */
        if (mtmp->mhp * 8 < mtmp->mhpmax || monnear(mtmp, mtmp->mux, mtmp->muy))
            return MCAST_TPORT_AWAY;

        static const int dispater_list[] = {
            MCAST_CURE_SELF, MCAST_AGGRAVATION, MCAST_HASTE_SELF, MCAST_DISAPPEAR,
            MCAST_ENTOMB, MCAST_SUMMON_MONS, MCAST_TPORT_AWAY
        };
        return ROLL_FROM(dispater_list);
    }
    else if (mtmp->data == &mons[PM_ASMODEUS]) {
        if ((mtmp->mhp * 8 < mtmp->mhpmax)
            || (mtmp->mhp * 3 < mtmp->mhpmax && !rn2(3)))
            return MCAST_CURE_SELF;
        else if (!rn2(4))
            return MCAST_DARK_SPEECH;
        else
            return MCAST_SHEER_COLD;
    }
    else if (mtmp->data == &mons[PM_DEMOGORGON]) {
        int spellnum;
        if ((mtmp->mhp * 8 < mtmp->mhpmax)
            || (mtmp->mhp * 3 < mtmp->mhpmax && !rn2(3)))
            return MCAST_CURE_SELF;
        /* Use the regular magic spells as the base, with a few filtered out,
         * and allow casting dark speech. */
        do {
            spellnum = choose_monster_spell(mtmp, AD_SPEL);
        } while (spellnum == MCAST_DISAPPEAR
                 || spellnum == MCAST_AGGRAVATION);
        if (!rn2(4))
            return MCAST_DARK_SPEECH;
        else
            return spellnum;
    }
    else if (mtmp->data == &mons[PM_ORCUS]) {
        int selection;
        if ((mtmp->mhp * 8 < mtmp->mhpmax)
            || (mtmp->mhp * 3 < mtmp->mhpmax && !rn2(3)))
            return MCAST_CURE_SELF;
        static const int orcus_list[] = {
            MCAST_PSI_BOLT, MCAST_BLIGHT, MCAST_STUN_YOU, MCAST_WEAKEN_YOU,
            MCAST_CURSE_ITEMS, MCAST_SUMMON_MONS, MCAST_DESTRY_ARMR, MCAST_DEATH_TOUCH,
            MCAST_DISENCHANT
        };
        /* be intelligent; orcus's spells uniquely can penetrate Antimagic in a
         * lot of cases, but not for certain spells that would be too nasty
         * (such as destroying armor) */
        do {
            selection = ROLL_FROM(orcus_list);
        } while (selection == MCAST_DESTRY_ARMR && m_seenres(mtmp, M_SEEN_MAGR));
        return selection;
    }
    impossible("no special spell list for mon %s",
               mtmp->data->pmnames[NEUTRAL]);
    return MCAST_PSI_BOLT; /* arbitrary since this should never be reached */
}

/* return values:
 * 1: successful spell
 * 0: unsuccessful spell
 */
int
castmu(
    struct monst *mtmp,   /* caster */
    struct attack *mattk, /* caster's current attack */
    boolean thinks_it_foundyou,    /* might be mistaken if displaced */
    boolean foundyou)              /* knows hero's precise location */
{
    int dmg, orig_dmg, ml = mtmp->m_lev;
    int ret;
    int spellnum = 0;

    /* Three cases:
     * -- monster is attacking you.  Search for a useful spell.
     * -- monster thinks it's attacking you.  Search for a useful spell,
     *    without checking for undirected.  If the spell found is directed,
     *    it fails with cursetxt() and loss of mspec_used.
     * -- monster isn't trying to attack.  Select a spell once.  Don't keep
     *    searching; if that spell is not useful (or if it's directed),
     *    return and do something else.
     * Since most spells are directed, this means that a monster that isn't
     * attacking casts spells only a small portion of the time that an
     * attacking monster does.
     */
    if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
        int cnt = 40;

        do {
            if (has_special_spell_list(mtmp->data)) {
                /* is archfiend or other special spellcaster */
                spellnum = choose_special_spell(mtmp);
            }
            else {
                spellnum = choose_monster_spell(mtmp, mattk->adtyp);
            }
            /* not trying to attack?  don't allow directed spells */
            if (!thinks_it_foundyou) {
                if (!is_undirected_spell(spellnum)
                    || spell_would_be_useless(mtmp, spellnum)) {
                    if (foundyou)
                        impossible(
                       "spellcasting monster found you and doesn't know it?");
                    return M_ATTK_MISS;
                }
                break;
            }
        } while (--cnt > 0
                 && spell_would_be_useless(mtmp, spellnum));
        if (cnt == 0)
            return M_ATTK_MISS;
    }

    /* monster unable to cast spells? */
    if (mtmp->mcan || mtmp->mspec_used || !ml
        || m_seenres(mtmp, cvt_adtyp_to_mseenres(mattk->adtyp))) {
        cursetxt(mtmp, is_undirected_spell(spellnum));
        return M_ATTK_MISS;
    }

    debugpline3("castmu:%s,lvl:%i,spell:%i", noit_Monnam(mtmp), ml, spellnum);

    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        /* monst->m_lev is unsigned (uchar), monst->mspec_used is int */
        mtmp->mspec_used = (int) ((mtmp->m_lev < 8) ? (10 - mtmp->m_lev) : 2);
    }

    /* Monster can cast spells, but is casting a directed spell at the
     * wrong place?  If so, give a message, and return.
     * Do this *after* penalizing mspec_used.
     *
     * FIXME?
     *  Shouldn't wall of lava have a case similar to wall of water?
     *  And should cold damage hit water or lava instead of missing
     *  even when the caster has targeted the wrong spot?  Likewise
     *  for fire mis-aimed at ice.
     */
    if (!foundyou && thinks_it_foundyou
        && !is_undirected_spell(spellnum)) {
        pline_mon(mtmp, "%s casts a spell at %s!",
                 canseemon(mtmp) ? Monnam(mtmp) : "Something",
                 is_waterwall(mtmp->mux, mtmp->muy) ? "empty water"
                                                    : "thin air");
        return M_ATTK_MISS;
    }

    nomul(0);
    if (rn2(ml * 10) < (mtmp->mconf ? 100 : 20)) { /* fumbled attack */
        Soundeffect(se_air_crackles, 60);
        if (canseemon(mtmp) && !Deaf) {
            set_msg_xy(mtmp->mx, mtmp->my);
            pline_The("air crackles around %s.", mon_nam(mtmp));
        }
        return M_ATTK_MISS;
    }
    if ((canspotmon(mtmp) || !is_undirected_spell(spellnum))
        /* dark speech has its own casting message */
        && spellnum != MCAST_DARK_SPEECH) {
        pline_mon(mtmp, "%s casts a spell%s!",
                 canspotmon(mtmp) ? Monnam(mtmp) : "Something",
                 is_undirected_spell(spellnum) ? ""
                 : (Invis && !perceives(mtmp->data)
                    && !u_at(mtmp->mux, mtmp->muy))
                   ? " at a spot near you"
                   : (Displaced && !u_at(mtmp->mux, mtmp->muy))
                     ? " at your displaced image"
                     : " at you");
    }

    /*
     * As these are spells, the damage is related to the level
     * of the monster casting the spell.
     */
    if (!foundyou) {
        dmg = 0;
        if (mattk->adtyp != AD_SPEL && mattk->adtyp != AD_CLRC) {
            impossible(
              "%s casting non-hand-to-hand version of hand-to-hand spell %d?",
                       Monnam(mtmp), mattk->adtyp);
            return M_ATTK_MISS;
        }
    } else if (mattk->damd)
        dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
    else
        dmg = d((int) ((ml / 2) + 1), 6);
    if (Half_spell_damage)
        dmg = (dmg + 1) / 2;

    orig_dmg = dmg;
    ret = M_ATTK_HIT;
    /*
     * FIXME: none of these hit the steed when hero is riding, nor do
     *  they inflict damage on carried items.
     */
    switch (mattk->adtyp) {
    case AD_FIRE:
        pline("You're enveloped in flames.");
        if (Fire_resistance) {
            shieldeff(u.ux, u.uy);
            pline("But you resist the effects.");
            monstseesu(M_SEEN_FIRE);
            dmg = 0;
        } else {
            monstunseesu(M_SEEN_FIRE);
        }
        burn_away_slime();
        /* burn up flammable items on the floor, melt ice terrain */
        mon_spell_hits_spot(mtmp, AD_FIRE, u.ux, u.uy);
        break;
    case AD_COLD:
        mcast_sheer_cold(&dmg);
        break;
    case AD_ELEC:
        if (Shock_resistance)
            dmg = 0;
        You("are blasted with electricity%s", exclam(dmg));
        if (Shock_resistance) {
            shieldeff(u.ux, u.uy);
            pline("But you resist the effects.");
            monstseesu(M_SEEN_ELEC);
        }
        else {
            monstunseesu(M_SEEN_ELEC);
        }
        ugolemeffects(AD_ELEC, orig_dmg);
        /* creates a lightning-like flash */
        (void) flashburn((long) rnd(100), TRUE);
        break;
    case AD_MAGM:
        You("are hit by a shower of missiles!");
        dmg = d((int) mtmp->m_lev / 2 + 1, 6);
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            pline("Some missiles bounce off!");
            dmg = (dmg + 1) / 2;
            monstseesu(M_SEEN_MAGR);
        } else {
            monstunseesu(M_SEEN_MAGR);
        }
        if (Half_spell_damage) { /* stacks with Antimagic */
            dmg = (dmg + 1) / 2;
        }
        /* shower of magic missiles scuffs an engraving */
        mon_spell_hits_spot(mtmp, AD_MAGM, u.ux, u.uy);
        break;
    case AD_SPEL: /* wizard spell */
    case AD_CLRC: /* clerical spell */
        mcast_spell(mtmp, dmg, spellnum);
        dmg = 0; /* done by the spell casting functions */
        break;
    } /* switch */
    if (dmg)
        mdamageu(mtmp, dmg);
    return ret;
}

staticfn int
m_cure_self(struct monst *mtmp, int dmg)
{
    if (mtmp->mhp < mtmp->mhpmax) {
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s looks better.", Monnam(mtmp));
        /* note: player healing does 6d4; this used to do 1d8 */
        healmon(mtmp, d(3, 6), 0);
        dmg = 0;
    }
    return dmg;
}

/* unlike the finger of death spell which behaves like a wand of death,
   this monster spell only attacks the hero */
void
touch_of_death(struct monst *mtmp)
{
    char kbuf[BUFSZ];
    int dmg = 50 + d(8, 6);
    int drain = dmg / 2;

    /* if we get here, we know that hero isn't magic resistant and isn't
       poly'd into an undead or demon */
    You_feel("drained...");
    (void) death_inflicted_by(kbuf, "the touch of death", mtmp);

    if (Upolyd) {
        u.mh = 0;
        rehumanize(); /* fatal iff Unchanging */
    } else if (drain >= u.uhpmax) {
        svk.killer.format = KILLED_BY;
        Strcpy(svk.killer.name, kbuf);
        done(DIED);
    } else {
        /* HP manipulation similar to poisoned(attrib.c) */
        int olduhp = u.uhp,
            uhpmin = minuhpmax(3),
            newuhpmax = u.uhpmax - drain;

        setuhpmax(max(newuhpmax, uhpmin), FALSE);
        dmg = adjuhploss(dmg, olduhp); /* reduce pending damage if uhp has
                                        * already been reduced due to drop
                                        * in uhpmax */
        losehp(dmg, kbuf, KILLED_BY);
    }
    svk.killer.name[0] = '\0'; /* not killed if we get here... */
}

/* give a reason for death by some monster spells */
char *
death_inflicted_by(
    char *outbuf,            /* assumed big enough; pm_names are short */
    const char *deathreason, /* cause of death */
    struct monst *mtmp)      /* monster who caused it */
{
    Strcpy(outbuf, deathreason);
    if (mtmp) {
        struct permonst *mptr = mtmp->data,
            *champtr = (ismnum(mtmp->cham)) ? &mons[mtmp->cham] : mptr;
        const char *realnm = pmname(champtr, Mgender(mtmp)),
            *fakenm = pmname(mptr, Mgender(mtmp));

        /* greatly simplified extract from done_in_by(), primarily for
           reason for death due to 'touch of death' spell; if mtmp is
           shape changed, it won't be a vampshifter or mimic since they
           can't cast spells */
        if (!type_is_pname(champtr) && !the_unique_pm(mptr))
            realnm = an(realnm);
        Sprintf(eos(outbuf), " inflicted by %s%s",
                the_unique_pm(mptr) ? "the " : "", realnm);
        if (champtr != mptr)
            Sprintf(eos(outbuf), " imitating %s", an(fakenm));
    }
    return outbuf;
}

/*
 * Monster wizard and cleric spellcasting functions.
 */

staticfn void
mcast_death_touch(struct monst *mtmp)
{
    boolean orcus = (monsndx(mtmp->data) == PM_ORCUS);
    pline("Oh no, %s's using the touch of death!", mhe(mtmp));
    if (nonliving(gy.youmonst.data) || is_demon(gy.youmonst.data)) {
        You("seem no deader than before.");
    } else if ((!Antimagic || orcus) && rn2(mtmp->m_lev) > 12) {
        if (Hallucination) {
            You("have an out of body experience.");
        } else {
            touch_of_death(mtmp);
        }
        monstunseesu(M_SEEN_MAGR);
    } else {
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
        }
        pline("Lucky for you, it didn't work!");
    }
}

staticfn void
mcast_clone_wiz(struct monst *mtmp)
{
    if (mtmp->iswiz && svc.context.no_of_wizards == 1) {
        pline("Double Trouble...");
        clonewiz();
    } else
        impossible("bad wizard cloning?");
}

staticfn void
mcast_summon_mons(struct monst *mtmp)
{
    int count;
    if (monsndx(mtmp->data) == PM_ORCUS) {
        coord yourloc = { u.ux, u.uy };
        mkundead(&yourloc, TRUE, NO_MINVENT);
        /* TODO: "The dead emerge from the ground!" message? suppress existing
         * msg with mm flags? */
    }

    count = nasty(mtmp);
    if (!count) {
        ; /* nothing was created? */
    } else if (mtmp->iswiz) {
        SetVoice(mtmp, 0, 80, 0);
        verbalize("Destroy the thief, my pet%s!", plur(count));
    } else {
        boolean one = (count == 1);
        const char *mappear = one ? "A monster appears"
                                  : "Monsters appear";

        /* messages not quite right if plural monsters created but
           only a single monster is seen */
        if (Invis && !perceives(mtmp->data)
            && (mtmp->mux != u.ux || mtmp->muy != u.uy))
            pline("%s %s a spot near you!", mappear,
                  one ? "at" : "around");
        else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
            pline("%s %s your displaced image!", mappear,
                  one ? "by" : "around");
        else
            pline("%s from nowhere!", mappear);
    }
}

staticfn void
mcast_destroy_armor(void)
{
    if (Antimagic) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        pline("A field of force surrounds you!");
    } else if (!destroy_arm()) {
        Your("skin itches.");
    } else {
        /* monsters only realize you aren't magic-protected if armor is
           actually destroyed */
        monstunseesu(M_SEEN_MAGR);
    }
}

staticfn void
mcast_weaken_you(struct monst *mtmp, int dmg)
{
    if (Antimagic && monsndx(mtmp->data) != PM_ORCUS) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        You_feel("momentarily weakened.");
    } else {
        char kbuf[BUFSZ];

        You("suddenly feel weaker!");
        dmg = mtmp->m_lev - 6;
        if (dmg < 1) /* paranoia since only chosen when m_lev is high */
            dmg = 1;
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        losestr(rnd(dmg),
                death_inflicted_by(kbuf, "strength loss", mtmp),
                KILLED_BY);
        svk.killer.name[0] = '\0'; /* not killed if we get here... */
        monstunseesu(M_SEEN_MAGR);
    }
}

staticfn void
mcast_disappear(struct monst *mtmp)
{
    if (!mtmp->minvis && !mtmp->invis_blkd) {
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s suddenly %s!", Monnam(mtmp),
                      !See_invisible ? "disappears" : "becomes transparent");
        mon_set_minvis(mtmp, FALSE);
        if (cansee(mtmp->mx, mtmp->my) && !canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
    } else
        impossible("no reason for monster to cast disappear spell?");
}

staticfn void
mcast_stun_you(struct monst *mtmp, int dmg)
{
    if ((Antimagic && monsndx(mtmp->data) != PM_ORCUS) || Free_action) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        if (!Stunned)
            You_feel("momentarily disoriented.");
        make_stunned(1L, FALSE);
    } else {
        You(Stunned ? "struggle to keep your balance." : "reel...");
        dmg = d(ACURR(A_DEX) < 12 ? 6 : 4, 4);
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        make_stunned((HStun & TIMEOUT) + (long) dmg, FALSE);
        monstunseesu(M_SEEN_MAGR);
    }
}

staticfn int
mcast_geyser(int dmg)
{
    /* this is physical damage (force not heat),
     * not magical damage or fire damage
     */
    pline("A sudden geyser slams into you from nowhere!");
    dmg = d(8, 6);
    if (Half_physical_damage)
        dmg = (dmg + 1) / 2;
    if (u.umonnum == PM_IRON_GOLEM) {
        You("rust!");
        Strcpy(svk.killer.name, "rusted away");
        svk.killer.format = NO_KILLER_PREFIX;
        rehumanize();
        dmg = 0; /* prevent further damage after rehumanization */
    }
#if 0   /* since inventory items aren't affected, don't include this */
        /* make floor items wet */
    water_damage_chain(level.objects[u.ux][u.uy], TRUE);
#endif
    return dmg;
}

staticfn int
mcast_fire_pillar(struct monst *mtmp, int dmg)
{
    int orig_dmg;

    pline("A pillar of fire strikes all around you!");
    orig_dmg = dmg = d(8, 6);
    if (Fire_resistance) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_FIRE);
        dmg = 0;
    } else {
        monstunseesu(M_SEEN_FIRE);
    }
    if (Half_spell_damage)
        dmg = (dmg + 1) / 2;
    burn_away_slime();
    (void) burnarmor(&gy.youmonst);
    /* item destruction dmg */
    (void) destroy_items(&gy.youmonst, AD_FIRE, orig_dmg);
    ignite_items(gi.invent);
    /* burn up flammable items on the floor, melt ice terrain */
    mon_spell_hits_spot(mtmp, AD_FIRE, u.ux, u.uy);
    return dmg;
}

staticfn int
mcast_lightning(struct monst *mtmp, int dmg)
{
    int orig_dmg;
    const char *reflectsrc;

    Soundeffect(se_bolt_of_lightning, 80);
    pline("A bolt of lightning strikes down at you from above!");
    reflectsrc = ureflectsrc();
    orig_dmg = dmg = d(8, 6);
    if (reflectsrc || Shock_resistance) {
        shieldeff(u.ux, u.uy);
        dmg = 0;
        if (reflectsrc) {
            pline("It bounces off your %s.", reflectsrc);
            monstseesu(M_SEEN_REFL);
            return dmg;
        }
        monstunseesu(M_SEEN_REFL);
        monstseesu(M_SEEN_ELEC);
    } else {
        monstunseesu(M_SEEN_ELEC | M_SEEN_REFL);
    }
    if (Half_spell_damage)
        dmg = (dmg + 1) / 2;
    (void) destroy_items(&gy.youmonst, AD_ELEC, orig_dmg);
    /* lightning might destroy iron bars if hero is on such a spot;
       reflection protects terrain here [execution won't get here due
       to 'if (reflects) break' above] but hero resistance doesn't;
       do this before maybe blinding the hero via flashburn() */
    mon_spell_hits_spot(mtmp, AD_ELEC, u.ux, u.uy);
    /* blind hero; no effect if already blind */
    (void) flashburn((long) rnd(100), TRUE);
    return dmg;
}

staticfn int
mcast_psi_bolt(int dmg)
{
    /* prior to 3.4.0 Antimagic was setting the damage to 1--this
       made the spell virtually harmless to players with magic res. */
    if (Antimagic) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        dmg = (dmg + 1) / 2;
    } else {
        monstunseesu(M_SEEN_MAGR);
    }
    if (dmg <= 5)
        You("get a slight %sache.", body_part(HEAD));
    else if (dmg <= 10)
        Your("brain is on fire!");
    else if (dmg <= 20)
        Your("%s suddenly aches painfully!", body_part(HEAD));
    else
        Your("%s suddenly aches very painfully!", body_part(HEAD));
    return dmg;
}

staticfn int
mcast_open_wounds(int dmg)
{
    if (Antimagic) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        dmg = (dmg + 1) / 2;
    } else {
        monstunseesu(M_SEEN_MAGR);
    }
    if (dmg <= 5)
        Your("skin itches badly for a moment.");
    else if (dmg <= 10)
        pline("Wounds appear on your body!");
    else if (dmg <= 20)
        pline("Severe wounds appear on your body!");
    else
        Your("body is covered with painful wounds!");
    return dmg;
}

staticfn void
mcast_insects(struct monst *mtmp)
{
    /* Try for insects, and if there are none
       left, go for (sticks to) snakes.  -3. */
    struct permonst *pm = mkclass(S_ANT, 0);
    struct monst *mtmp2 = (struct monst *) 0;
    char whatbuf[QBUFSZ], let = (pm ? S_ANT : S_SNAKE);
    boolean success = FALSE, seecaster;
    int i, quan, oldseen, newseen;
    coord bypos;
    const char *fmt, *what;

    oldseen = monster_census(TRUE);
    quan = (mtmp->m_lev < 2) ? 1 : rnd((int) mtmp->m_lev / 2);
    if (quan < 3)
        quan = 3;
    if (mtmp->data == &mons[PM_MASTER_KAEN]) {
        /* Master Kaen can cast this spell, but summoning insects doesn't
         * make much sense for him. Create elementals instead. */
        let = S_ELEMENTAL;
        if (!m_next2u(mtmp)) {
            /* you are probably already surrounded by some elementals, don't
             * dogpile too many more on */
            quan = rnd(2);
        }
    }
    for (i = 0; i <= quan; i++) {
        if (!enexto(&bypos, mtmp->mux, mtmp->muy, mtmp->data))
            return;
        if ((pm = mkclass(let, 0)) != 0
            && (mtmp2 = makemon(pm, bypos.x, bypos.y, MM_ANGRY | MM_NOMSG))
            != 0) {
            success = TRUE;
            mtmp2->msleeping = mtmp2->mpeaceful = mtmp2->mtame = 0;
            set_malign(mtmp2);
        }
    }
    newseen = monster_census(TRUE);

    /* not canspotmon() which includes unseen things sensed via warning */
    seecaster = canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters;
    what = (let == S_ELEMENTAL) ? "elementals"
                                : (let == S_SNAKE) ? "snakes" : "insects";
    if (Hallucination)
        what = makeplural(bogusmon(whatbuf, (char *) 0, -1));

    fmt = 0;
    if (!seecaster) {
        if (newseen <= oldseen || Unaware) {
            /* unseen caster fails or summons unseen critters,
               or unconscious hero ("You dream that you hear...") */
            You_hear("someone summoning %s.", what);
        } else {
            char *arg;

            if (what != whatbuf)
                what = strcpy(whatbuf, what);
            /* unseen caster summoned seen critter(s) */
            arg = (newseen == oldseen + 1) ? an(makesingular(what))
                                           : whatbuf;
            if (!Deaf) {
                Soundeffect(se_someone_summoning, 100);
                You_hear("someone summoning something, and %s %s.", arg,
                         vtense(arg, "appear"));
            } else {
                pline("%s %s.", upstart(arg), vtense(arg, "appear"));
            }
        }

        /* seen caster, possibly producing unseen--or just one--critters;
           hero is told what the caster is doing and doesn't necessarily
           observe complete accuracy of that caster's results (in other
           words, no need to fuss with visibility or singularization;
           player is told what's happening even if hero is unconscious) */
    } else if (!success) {
        fmt = "%s casts at a clump of sticks, but nothing happens.%s";
        what = "";
    } else if (let == S_SNAKE) {
        fmt = "%s transforms a clump of sticks into %s!";
    } else if (Invis && !perceives(mtmp->data)
               && (mtmp->mux != u.ux || mtmp->muy != u.uy)) {
        fmt = "%s summons %s around a spot near you!";
    } else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy)) {
        fmt = "%s summons %s around your displaced image!";
    } else {
        fmt = "%s summons %s!";
    }
    if (fmt) {
        DISABLE_WARNING_FORMAT_NONLITERAL;
        pline_mon(mtmp, fmt, Monnam(mtmp), what);
        RESTORE_WARNING_FORMAT_NONLITERAL;
    }
}

staticfn void
mcast_blind_you(void)
{
    /* note: resists_blnd() doesn't apply here */
    if (!Blinded) {
        int num_eyes = eyecount(gy.youmonst.data);

        pline("Scales cover your %s!", (num_eyes == 1)
                                       ? body_part(EYE)
                                       : makeplural(body_part(EYE)));
        make_blinded(Half_spell_damage ? 100L : 200L, FALSE);
        if (!Blind)
            Your1(vision_clears);
    } else
        impossible("no reason for monster to cast blindness spell?");
}

staticfn int
mcast_paralyze(struct monst *mtmp)
{
    int dmg = 0;

    if (Antimagic) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        dmg = 1; /* to produce nomul(-1), not actual damage */
    } else {
        dmg = 4 + (int) mtmp->m_lev;
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        monstunseesu(M_SEEN_MAGR);
    }
    dynamic_multi_reason(mtmp, "paralyzed", FALSE);
    make_paralyzed(dmg, TRUE, (const char *) 0);
    return dmg;
}

staticfn void
mcast_confuse_you(struct monst *mtmp)
{
    if (Antimagic) {
        shieldeff(u.ux, u.uy);
        monstseesu(M_SEEN_MAGR);
        You_feel("momentarily dizzy.");
    } else {
        boolean oldprop = !!Confusion;
        int dmg = (int) mtmp->m_lev;

        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        make_confused(HConfusion + dmg, TRUE);
        if (Hallucination)
            You_feel("%s!", oldprop ? "trippier" : "trippy");
        else
            You_feel("%sconfused!", oldprop ? "more " : "");
        monstunseesu(M_SEEN_MAGR);
    }
}

/* entomb you in rocks (and maybe a couple diggable walls) to delay you and
 * allow some time for the caster to get away */
staticfn void
mcast_entomb(void)
{
    coordxy x, y;
    pline_The("ground shakes violently%s!",
              Blind ? "" : " and twists into walls");
    if (!Blind)
        pline("Boulders fall from above!");
    for (x = u.ux - 1; x <= u.ux + 1; ++x) {
        for (y = u.uy - 1; y <= u.uy + 1; ++y) {
            if (!SPACE_POS(levl[x][y].typ))
                continue;
            if (x == u.ux && y == u.uy)
                continue;
            /* Only create actual walls where there is no monster or object
             * or trap in the way. */
            if (!rn2(6) && levl[x][y].typ == ROOM && !m_at(x, y)
                && !svl.level.objects[x][y] && !t_at(x, y)) {
                levl[x][y].typ = rn2(2) ? HWALL : VWALL;
                levl[x][y].wall_info &= ~W_NONDIGGABLE;
                block_point(x, y);
                newsym(x, y);
            }
            else {
                if (rn2(5))
                    drop_boulder_on_monster(x, y, FALSE, FALSE);
                if (rn2(3))
                    drop_boulder_on_monster(x, y, FALSE, FALSE);
            }
        }
    }
    if (rn2(4))
        drop_boulder_on_player(FALSE, FALSE, FALSE, FALSE);
}

staticfn void
mcast_tport_away(struct monst *mtmp)
{
    /* this is better than reimplementing the logic of rloc to pick a random
     * spot that is sufficiently far away from (mux, muy) */
    xint8 tries = 3;
    do {
        rloc(mtmp, RLOC_MSG);
    } while (--tries
                && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) < 10);
}

staticfn void
mcast_dark_speech(struct monst * mtmp)
{
    if (Blind) {
        if (Deaf)
            ; /* nothing */
        else
            pline("Something intones a terrible chant!");
    }
    else {
        pline("%s raises a %s towards you and %s", Monnam(mtmp),
                mbodypart(mtmp, HAND),
                Deaf ? "appears to chant something."
                    : "intones a terrible chant!");
    }
    pline("Dark energy surrounds you...");
    switch (rn2(5)) {
    case 0:
        attrcurse();
        break;
    case 1:
        mcast_blight();
        break;
    case 2:
        Your("mind twists!");
        losehp(d((Deaf ? 4 : 8), 6), "hearing the Dark Speech", KILLED_BY);
        make_confused((HConfusion & TIMEOUT) + rnd(30), FALSE);
        make_stunned((HStun & TIMEOUT) + rnd(30), TRUE);
        break;
    case 3:
        You("are overwhelmed with a sense of doom...");
        if (Doomed)
            change_luck(-2);
        else
            set_itimeout(&Doomed, rn1(2000, 500));
        break;
    case 4:
        {
            boolean was_blind_before = Blind;
            /* this handles all the vision recalc stuff */
            make_blinded(1L, FALSE);
            HBlinded |= FROMOUTSIDE;
            if (!Blind)
                /* wearing Eyes of the Overworld - no effect, undo it */
                HBlinded &= ~FROMOUTSIDE;
            else if (!was_blind_before)
                You("can no longer see.");
            break;
        }
    }
}

/* extracted from castmu; if the corresponding flame spell is ever used and
 * treated as one of several possible spells in a demon lord's repertoire, it
 * should also probably be extracted.
 * The function expects *dmg to be the already rolled amount of damage the spell
 * will deliver by default. It may adjust *dmg in the process; the caller should
 * anticipate this. */
staticfn void
mcast_sheer_cold(int *dmg)
{
    int orig_dmg = *dmg;
    pline("You're covered in frigid frost.");
    if (Cold_resistance) {
        shieldeff(u.ux, u.uy);
        pline("You partially resist the effects.");
        monstseesu(M_SEEN_COLD);
        *dmg /= 4;
    }
    else {
        monstunseesu(M_SEEN_COLD);
    }
    destroy_items(&gy.youmonst, AD_COLD, orig_dmg);
}

/* this is also one of dark speech's effects */
staticfn void
mcast_blight(void)
{
    You("%s rapidly decomposing!", Withering ? "continue" : "begin");
    incr_itimeout(&HWithering, rn1(40, 100));
}

/* 40% chance of zapping enchantment from current wielded weapon
 * 45% chance from random piece of worn gear
 * 15% chance of taking it from a random charged ring, charged tool, wand, or
 * unequipped weapon or armor */
staticfn void
mcast_disenchant(struct monst *mtmp)
{
    struct obj *targ = (struct obj *) 0;
    short loss = rnd(3);
    const schar MIN_SPE1 = -7; /* for worn gear */
    const schar MIN_SPE2 = 0;  /* for tools & wands */
    schar floor = MIN_SPE1;
    if (uwep && uwep->spe > MIN_SPE1 && percent(40))
        targ = uwep;
    else if ((targ = some_armor(&gy.youmonst)) && targ->spe > MIN_SPE1
             && percent(75))
        ; /* targ already selected */
    else {
        struct obj *otmp;
        int choices = 0;
        for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
            short oclass = objects[otmp->otyp].oc_class;
            if ((oclass == RING_CLASS && objects[otmp->otyp].oc_charged)
                || (oclass == TOOL_CLASS && is_weptool(otmp))) {
                /* weptools and charged rings use the same rules for weapons and
                 * armor */
                if (otmp->spe > MIN_SPE1 && !rn2(++choices)) {
                    targ = otmp;
                    floor = MIN_SPE1;
                }
            }
            else if (oclass == WAND_CLASS
                     || (oclass == TOOL_CLASS
                         && objects[otmp->otyp].oc_charged
                         && objects[otmp->otyp].oc_magic)) {
                /* wands and charged tools do not use the same rules since
                 * negative spe doesn't make sense for them (well, it does for
                 * wands, but that would mix this up with cancellation) */
                if (otmp->spe > MIN_SPE2 && !rn2(++choices)) {
                    targ = otmp;
                    floor = MIN_SPE2;
                    /* account for tools and wands which have a higher number of
                     * charges than normal, or have been recharged beyond their
                     * normal amount */
                    loss = max(loss, otmp->spe / 3);
                }
            }
        }
    }
    if (!targ)
        /* couldn't find anything to disenchant... */
        return;
    if (targ->spe > 0) {
        pline("%s absorbs magic energies from %s!", Monnam(mtmp),
              yname(targ));
        mtmp->mspec_used = max(mtmp->mspec_used - loss, 0);
        floor = 0;
    }
    else {
        pline("%s glows black.", Yname2(targ));
    }
    targ->spe = max(floor, targ->spe - loss);
    if (targ->spe < 0)
        curse(targ);
}

/*
   If dmg is zero, then the monster is not casting at you.
   If the monster is intentionally not casting at you, we have previously
   called spell_would_be_useless() and spellnum should always be a valid
   undirected spell.
   If you modify either of these, be sure to change is_undirected_spell()
   and spell_would_be_useless().
 */
staticfn void
mcast_spell(struct monst *mtmp, int dmg, int spellnum)
{
    if (dmg < 0) {
        impossible("monster cast spell (%d) with negative dmg (%d)?",
                   spellnum, dmg);
        return;
    }
    if (dmg == 0 && !is_undirected_spell(spellnum)) {
        impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case MCAST_DEATH_TOUCH:
        mcast_death_touch(mtmp);
        dmg = 0;
        break;
    case MCAST_CLONE_WIZ:
        mcast_clone_wiz(mtmp);
        dmg = 0;
        break;
    case MCAST_SUMMON_MONS:
        mcast_summon_mons(mtmp);
        dmg = 0;
        break;
    case MCAST_AGGRAVATION:
        You_feel("that monsters are aware of your presence.");
        aggravate();
        dmg = 0;
        break;
    case MCAST_CURSE_ITEMS:
        You_feel("as if you need some help.");
        rndcurse();
        dmg = 0;
        break;
    case MCAST_DESTRY_ARMR:
        mcast_destroy_armor();
        dmg = 0;
        break;
    case MCAST_WEAKEN_YOU: /* drain strength */
        mcast_weaken_you(mtmp, dmg);
        dmg = 0;
        break;
    case MCAST_DISAPPEAR: /* makes self invisible */
        mcast_disappear(mtmp);
        dmg = 0;
        break;
    case MCAST_STUN_YOU:
        mcast_stun_you(mtmp, dmg);
        dmg = 0;
        break;
    case MCAST_HASTE_SELF:
        mon_adjust_speed(mtmp, 1, (struct obj *) 0);
        dmg = 0;
        break;
    case MCAST_CURE_SELF:
        dmg = m_cure_self(mtmp, dmg);
        break;
    case MCAST_PSI_BOLT:
        dmg = mcast_psi_bolt(dmg);
        break;
    case MCAST_GEYSER:
        dmg = mcast_geyser(dmg);
        break;
    case MCAST_FIRE_PILLAR:
        dmg = mcast_fire_pillar(mtmp, dmg);
        break;
    case MCAST_LIGHTNING:
        dmg = mcast_lightning(mtmp, dmg);
        break;
    case MCAST_INSECTS:
        mcast_insects(mtmp);
        dmg = 0;
        break;
    case MCAST_BLIND_YOU:
        mcast_blind_you();
        dmg = 0;
        break;
    case MCAST_PARALYZE:
        dmg = mcast_paralyze(mtmp);
        break;
    case MCAST_CONFUSE_YOU:
        mcast_confuse_you(mtmp);
        dmg = 0;
        break;
    case MCAST_OPEN_WOUNDS:
        dmg = mcast_open_wounds(dmg);
        break;
    case MCAST_TPORT_AWAY:
        mcast_tport_away(mtmp);
        dmg = 0;
        break;
    case MCAST_ENTOMB:
        mcast_entomb();
        dmg = 0;
        break;
    case MCAST_DARK_SPEECH:
        mcast_dark_speech(mtmp);
        dmg = 0;
        break;
    case MCAST_SHEER_COLD:
        mcast_sheer_cold(&dmg);
        break;
    case MCAST_BLIGHT:
        mcast_blight();
        dmg = 0;
        break;
    case MCAST_DISENCHANT:
        mcast_disenchant(mtmp);
        dmg = 0;
        break;
    default:
        impossible("mcastu: invalid magic spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    if (dmg)
        mdamageu(mtmp, dmg);
}

staticfn boolean
is_undirected_spell(int spellnum)
{
    if ((mcast_data[spellnum].flags & MCF_INDIRECT) != 0)
        return TRUE;
    return FALSE;
}

/* Some spells are useless under some circumstances. */
staticfn boolean
spell_would_be_useless(struct monst *mtmp, int spellnum)
{
    /* Some spells don't require the player to really be there and can be cast
     * by the monster when you're invisible, yet still shouldn't be cast when
     * the monster doesn't even think you're there.
     * This check isn't quite right because it always uses your real position.
     * We really want something like "if the monster could see mux, muy".
     */

    /* spell is only cast by hostile monsters */
    if ((mcast_data[spellnum].flags & MCF_HOSTILE) != 0) {
        if (mtmp->mpeaceful)
            return TRUE;
    }

    /* spell needs the monster to see hero */
    if ((mcast_data[spellnum].flags & MCF_SIGHT) != 0) {
        boolean mcouldseeu = couldsee(mtmp->mx, mtmp->my);

        if (!mcouldseeu)
            return TRUE;
    }

    switch (spellnum) {
    case MCAST_DEATH_TOUCH:
        if ((Antimagic || Hallucination) && !rn2(2))
            return TRUE;
        break;
    case MCAST_GEYSER:
        if (!rn2(5))
            return TRUE;
        break;
    case MCAST_CLONE_WIZ:
        /* only the Wizard is allowed to clone himself */
        if (!mtmp->iswiz || svc.context.no_of_wizards > 1)
            return TRUE;
        break;
    case MCAST_AGGRAVATION:
        /* aggravation (global wakeup) when everyone is already active */
        /* if nothing needs to be awakened then this spell is useless
           but caster might not realize that [chance to pick it then
           must be very small otherwise caller's many retry attempts
           will eventually end up picking it too often] */
        if (!has_aggravatables(mtmp))
            return rn2(100) ? TRUE : FALSE;
        break;
    case MCAST_HASTE_SELF:
        /* haste self when already fast */
        if (mtmp->permspeed == MFAST)
            return TRUE;
        break;
    case MCAST_DISAPPEAR:
        /* invisibility when already invisible */
        if (mtmp->minvis || mtmp->invis_blkd)
            return TRUE;
        /* peaceful monster won't cast invisibility if you can't see
           invisible,
           same as when monsters drink potions of invisibility.  This doesn't
           really make a lot of sense, but lets the player avoid hitting
           peaceful monsters by mistake */
        if (mtmp->mpeaceful && !See_invisible)
            return TRUE;
        break;
    case MCAST_CURE_SELF:
        /* healing when already healed */
        if (mtmp->mhp == mtmp->mhpmax)
            return TRUE;
        break;
    case MCAST_BLIND_YOU:
        if (Blinded)
            return TRUE;
        break;
    case MCAST_TPORT_AWAY:
        /* don't teleport away if already sufficiently far away */
        if (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) >= 10)
            return TRUE;
        break;
    case MCAST_ENTOMB:
        /* don't entomb if hero is already entombed */
        if (spellnum == MCAST_ENTOMB && is_entombed(u.ux, u.uy))
            return TRUE;
    default:
        break;
    }
    return FALSE;
}

/* monster uses spell (ranged) */
int
buzzmu(struct monst *mtmp, struct attack *mattk)
{
    /* don't print constant stream of curse messages for 'normal'
       spellcasting monsters at range */
    if (!BZ_VALID_ADTYP(mattk->adtyp))
        return M_ATTK_MISS;

    if (mtmp->mcan || m_seenres(mtmp, cvt_adtyp_to_mseenres(mattk->adtyp))) {
        cursetxt(mtmp, FALSE);
        return M_ATTK_MISS;
    }
    if (lined_up(mtmp) && rn2(3)) {
        nomul(0);
        if (canseemon(mtmp))
            pline_mon(mtmp, "%s zaps you with a %s!", Monnam(mtmp),
                  flash_str(BZ_OFS_AD(mattk->adtyp), FALSE));
        gb.buzzer = mtmp;
        buzz(BZ_M_SPELL(BZ_OFS_AD(mattk->adtyp)), (int) mattk->damn,
             mtmp->mx, mtmp->my, sgn(gt.tbx), sgn(gt.tby));
        gb.buzzer = 0;
        return M_ATTK_HIT;
    }
    return M_ATTK_MISS;
}

/* is (x,y) entombed (completely surrounded by boulders or nonwalkable spaces)?
 * note that (x,y) itself is not checked */
staticfn boolean
is_entombed(coordxy x, coordxy y)
{
    coordxy xx, yy;
    for (xx = x - 1; xx <= x + 1; xx++) {
        for (yy = y - 1; yy <= y + 1; yy++) {
            if (isok(xx, yy) && xx != x && yy != y
                && SPACE_POS(levl[xx][yy].typ) && !sobj_at(BOULDER, xx, yy))
                return FALSE;
        }
    }
    return TRUE;
}

/*mcastu.c*/
