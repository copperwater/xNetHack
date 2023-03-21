/* NetHack 3.7	mcastu.c	$NHDT-Date: 1596498177 2020/08/03 23:42:57 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.68 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* monster mage spells */
enum mcast_mage_spells {
    MGC_PSI_BOLT = 0,
    MGC_CURE_SELF,
    MGC_HASTE_SELF,
    MGC_STUN_YOU,
    MGC_DISAPPEAR,
    MGC_WEAKEN_YOU,
    MGC_DESTRY_ARMR,
    MGC_CURSE_ITEMS,
    MGC_AGGRAVATION,
    MGC_SUMMON_MONS,
    MGC_CLONE_WIZ,
    MGC_DEATH_TOUCH,
    MGC_ENTOMB,
    MGC_TPORT_AWAY,
    MGC_DARK_SPEECH,
    MGC_SHEER_COLD /* Asmodeus - emulates an AT_MAGC AD_COLD attack */
};

/* monster cleric spells */
enum mcast_cleric_spells {
    CLC_OPEN_WOUNDS = 0,
    CLC_CURE_SELF,
    CLC_CONFUSE_YOU,
    CLC_PARALYZE,
    CLC_BLIND_YOU,
    CLC_INSECTS,
    CLC_CURSE_ITEMS,
    CLC_LIGHTNING,
    CLC_FIRE_PILLAR,
    CLC_GEYSER
};

static void cursetxt(struct monst *, boolean);
static int choose_magic_spell(int);
static int choose_clerical_spell(int);
static int m_cure_self(struct monst *, int);
static void cast_wizard_spell(struct monst *, int, int);
static void cast_cleric_spell(struct monst *, int, int);
static boolean has_special_spell_list(struct permonst *);
static int choose_special_spell(struct monst *);
static boolean is_undirected_spell(unsigned int, int);
static boolean
spell_would_be_useless(struct monst *, unsigned int, int);
static boolean is_entombed(coordxy, coordxy);
static void sheer_cold(int *dmg);

/* feedback when frustrated monster couldn't cast a spell */
static void
cursetxt(struct monst *mtmp, boolean undirected)
{
    if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
        const char *point_msg; /* spellcasting monsters are impolite */

        if (undirected)
            point_msg = "all around, then curses";
        else if ((Invis && !perceives(mtmp->data)
                  && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                 || is_obj_mappear(&g.youmonst, STRANGE_OBJECT)
                 || u.uundetected)
            point_msg = "and curses in your general direction";
        else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
            point_msg = "and curses at your displaced image";
        else
            point_msg = "at you, then curses";

        pline("%s points %s.", Monnam(mtmp), point_msg);
    } else if ((!(g.moves % 4) || !rn2(4))) {
        if (!Deaf)
            Norep("You hear a mumbled curse.");   /* Deaf-aware */
    }
}

/* convert a level based random selection into a specific mage spell;
   inappropriate choices will be screened out by spell_would_be_useless() */
static int
choose_magic_spell(int spellval)
{
    /* for 3.4.3 and earlier, val greater than 22 selected default spell */
    while (spellval > 24 && rn2(25))
        spellval = rn2(spellval);

    switch (spellval) {
    case 24:
    case 23:
        if (Antimagic || Hallucination)
            return MGC_PSI_BOLT;
        /*FALLTHRU*/
    case 22:
    case 21:
    case 20:
        return MGC_DEATH_TOUCH;
    case 19:
    case 18:
        return MGC_CLONE_WIZ;
    case 17:
    case 16:
    case 15:
        return MGC_SUMMON_MONS;
    case 14:
    case 13:
        return MGC_AGGRAVATION;
    case 12:
    case 11:
    case 10:
        return MGC_CURSE_ITEMS;
    case 9:
    case 8:
        return MGC_DESTRY_ARMR;
    case 7:
    case 6:
        return MGC_WEAKEN_YOU;
    case 5:
    case 4:
        return MGC_DISAPPEAR;
    case 3:
        return MGC_STUN_YOU;
    case 2:
        return MGC_HASTE_SELF;
    case 1:
        return MGC_CURE_SELF;
    case 0:
    default:
        return MGC_PSI_BOLT;
    }
}

/* convert a level based random selection into a specific cleric spell */
static int
choose_clerical_spell(int spellnum)
{
    /* for 3.4.3 and earlier, num greater than 13 selected the default spell
     */
    while (spellnum > 15 && rn2(16))
        spellnum = rn2(spellnum);

    switch (spellnum) {
    case 15:
    case 14:
        if (rn2(3))
            return CLC_OPEN_WOUNDS;
        /*FALLTHRU*/
    case 13:
        return CLC_GEYSER;
    case 12:
        return CLC_FIRE_PILLAR;
    case 11:
        return CLC_LIGHTNING;
    case 10:
    case 9:
        return CLC_CURSE_ITEMS;
    case 8:
        return CLC_INSECTS;
    case 7:
    case 6:
        return CLC_BLIND_YOU;
    case 5:
    case 4:
        return CLC_PARALYZE;
    case 3:
    case 2:
        return CLC_CONFUSE_YOU;
    case 1:
        return CLC_CURE_SELF;
    case 0:
    default:
        return CLC_OPEN_WOUNDS;
    }
}

/* does mdat, a spellcaster, use a special spell list and avoid the normal mage
 * spell / cleric spell dichotomy?
 * Note that because of the logic in castmu() calling either cast_wizard_spell
 * or cast_cleric_spell based on the adtyp, it's not currently possible to
 * construct a custom spell with spells mismatching the adtyp (if one were to
 * try, the spells from the type not matching the adtyp would be treated as
 * those matching the adtyp - a mage-spell caster attempting to cast
 * CLC_OPEN_WOUNDS would instead cast MGC_PSI_BOLT, etc.) Refactoring would be
 * needed to make this work.
 */
static boolean
has_special_spell_list(struct permonst *mdat)
{
    /* Add more monsters to this list as needed. */
    switch (monsndx(mdat)) {
    case PM_DISPATER:
    case PM_ASMODEUS:
        return TRUE;
    }
    return FALSE;
}

/* mdat is a spellcaster with a special spell list; return a spell from its list
 */
static int
choose_special_spell(struct monst *mtmp)
{
    if (mtmp->data == &mons[PM_DISPATER]) {
        /* Dispater is defense-oriented and doesn't really cast direct attack
         * spells. Instead he tries to keep away from you and prevent you from
         * getting to him. */
        if (mtmp->mhp * 8 < mtmp->mhpmax || monnear(mtmp, mtmp->mux, mtmp->muy))
            return MGC_TPORT_AWAY;

        static const int dispater_list[] = {
            MGC_CURE_SELF, MGC_AGGRAVATION, MGC_HASTE_SELF, MGC_DISAPPEAR,
            MGC_ENTOMB, MGC_SUMMON_MONS, MGC_TPORT_AWAY
        };
        return dispater_list[rn2(SIZE(dispater_list))];
    }
    else if (mtmp->data == &mons[PM_ASMODEUS]) {
        if ((mtmp->mhp * 8 < mtmp->mhpmax)
            || (mtmp->mhp * 3 < mtmp->mhpmax && !rn2(3)))
            return MGC_CURE_SELF;
        else if (!rn2(4))
            return MGC_DARK_SPEECH;
        else
            return MGC_SHEER_COLD;
    }
    impossible("no special spell list for mon %s",
               mtmp->data->pmnames[NEUTRAL]);
    return MGC_PSI_BOLT; /* arbitrary since this should never be reached */
}

/* return values:
 * 1: successful spell
 * 0: unsuccessful spell
 */
int
castmu(register struct monst *mtmp,
       register struct attack *mattk,
       boolean thinks_it_foundyou,
       boolean foundyou)
{
    int dmg, ml = mtmp->m_lev;
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
                /* is demon lord or other special spellcaster */
                spellnum = choose_special_spell(mtmp);
            }
            else {
                spellnum = rn2(ml);
                if (mattk->adtyp == AD_SPEL)
                    spellnum = choose_magic_spell(spellnum);
                else
                    spellnum = choose_clerical_spell(spellnum);
            }
            /* not trying to attack?  don't allow directed spells */
            if (!thinks_it_foundyou) {
                if (!is_undirected_spell(mattk->adtyp, spellnum)
                    || spell_would_be_useless(mtmp, mattk->adtyp, spellnum)) {
                    if (foundyou)
                        impossible(
                       "spellcasting monster found you and doesn't know it?");
                    return 0;
                }
                break;
            }
        } while (--cnt > 0
                 && spell_would_be_useless(mtmp, mattk->adtyp, spellnum));
        if (cnt == 0)
            return 0;
    }

    /* monster unable to cast spells? */
    if (mtmp->mcan || mtmp->mspec_used || !ml) {
        cursetxt(mtmp, is_undirected_spell(mattk->adtyp, spellnum));
        return 0;
    }

    if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
        mtmp->mspec_used = 10 - mtmp->m_lev;
        if (mtmp->mspec_used < 2)
            mtmp->mspec_used = 2;
    }

    /* monster can cast spells, but is casting a directed spell at the
       wrong place?  If so, give a message, and return.  Do this *after*
       penalizing mspec_used. */
    if (!foundyou && thinks_it_foundyou
        && !is_undirected_spell(mattk->adtyp, spellnum)) {
        pline("%s casts a spell at %s!",
              canseemon(mtmp) ? Monnam(mtmp) : "Something",
              is_waterwall(mtmp->mux,mtmp->muy) ? "empty water"
                                                : "thin air");
        return 0;
    }

    nomul(0);
    if (rn2(ml * 10) < (mtmp->mconf ? 100 : 20)) { /* fumbled attack */
        if (canseemon(mtmp) && !Deaf)
            pline_The("air crackles around %s.", mon_nam(mtmp));
        return 0;
    }
    if ((canspotmon(mtmp) || !is_undirected_spell(mattk->adtyp, spellnum))
        /* dark speech has its own casting message */
        && spellnum != MGC_DARK_SPEECH) {
        pline("%s casts a spell%s!",
              canspotmon(mtmp) ? Monnam(mtmp) : "Something",
              is_undirected_spell(mattk->adtyp, spellnum)
                  ? ""
                  : (Invis && !perceives(mtmp->data)
                     && (mtmp->mux != u.ux || mtmp->muy != u.uy))
                        ? " at a spot near you"
                        : (Displaced
                           && (mtmp->mux != u.ux || mtmp->muy != u.uy))
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
            return 0;
        }
    } else if (mattk->damd)
        dmg = d((int) ((ml / 2) + mattk->damn), (int) mattk->damd);
    else
        dmg = d((int) ((ml / 2) + 1), 6);
    if (Half_spell_damage)
        dmg = (dmg + 1) / 2;

    ret = 1;
    switch (mattk->adtyp) {
    case AD_FIRE:
        pline("You're enveloped in flames.");
        if (Fire_resistance) {
            shieldeff(u.ux, u.uy);
            pline("But you resist the effects.");
            monstseesu(M_SEEN_FIRE);
            dmg = 0;
        }
        burn_away_slime();
        break;
    case AD_COLD:
        sheer_cold(&dmg);
        break;
    case AD_MAGM:
        You("are hit by a shower of missiles!");
        dmg = d((int) mtmp->m_lev / 2 + 1, 6);
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            pline("Some missiles bounce off!");
            dmg = (dmg + 1) / 2;
            monstseesu(M_SEEN_MAGR);
        }
        if (Half_spell_damage) { /* stacks with Antimagic */
            dmg = (dmg + 1) / 2;
        }
        break;
    case AD_SPEL: /* wizard spell */
    case AD_CLRC: /* clerical spell */
    {
        if (mattk->adtyp == AD_SPEL)
            cast_wizard_spell(mtmp, dmg, spellnum);
        else
            cast_cleric_spell(mtmp, dmg, spellnum);
        dmg = 0; /* done by the spell casting functions */
        break;
    }
    }
    if (dmg)
        mdamageu(mtmp, dmg);
    return ret;
}

static int
m_cure_self(struct monst *mtmp, int dmg)
{
    if (mtmp->mhp < mtmp->mhpmax) {
        if (canseemon(mtmp))
            pline("%s looks better.", Monnam(mtmp));
        /* note: player healing does 6d4; this used to do 1d8 */
        if ((mtmp->mhp += d(3, 6)) > mtmp->mhpmax)
            mtmp->mhp = mtmp->mhpmax;
        dmg = 0;
    }
    return dmg;
}

void
touch_of_death(void)
{
    int dmg = 50 + d(8, 6);
    int drain = dmg / 2;

    You_feel("drained...");

    if (drain >= u.uhpmax) {
        g.killer.format = KILLED_BY_AN;
        Strcpy(g.killer.name, "touch of death");
        done(DIED);
    } else {
        u.uhpmax -= drain;
        losehp(dmg, "touch of death", KILLED_BY_AN);
    }
}

/* monster wizard and cleric spellcasting functions */
/*
   If dmg is zero, then the monster is not casting at you.
   If the monster is intentionally not casting at you, we have previously
   called spell_would_be_useless() and spellnum should always be a valid
   undirected spell.
   If you modify either of these, be sure to change is_undirected_spell()
   and spell_would_be_useless().
 */
static
void
cast_wizard_spell(struct monst *mtmp, int dmg, int spellnum)
{
    if (dmg == 0 && !is_undirected_spell(AD_SPEL, spellnum)) {
        impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case MGC_DEATH_TOUCH:
        pline("Oh no, %s's using the touch of death!", mhe(mtmp));
        if (nonliving(g.youmonst.data) || is_demon(g.youmonst.data)
            || g.youmonst.data->mlet == S_ANGEL) {
            You("seem no deader than before.");
        } else if (!Antimagic && rn2(mtmp->m_lev) > 12) {
            if (Hallucination) {
                You("have an out of body experience.");
            } else {
                touch_of_death();
            }
        } else {
            if (Antimagic) {
                shieldeff(u.ux, u.uy);
                monstseesu(M_SEEN_MAGR);
            }
            pline("Lucky for you, it didn't work!");
        }
        dmg = 0;
        break;
    case MGC_CLONE_WIZ:
        if (mtmp->iswiz && g.context.no_of_wizards == 1) {
            pline("Double Trouble...");
            clonewiz();
            dmg = 0;
        } else
            impossible("bad wizard cloning?");
        break;
    case MGC_SUMMON_MONS:
        (void) nasty(mtmp); /* summon something nasty */
        dmg = 0;
        break;
    case MGC_AGGRAVATION:
        You_feel("that monsters are aware of your presence.");
        aggravate();
        dmg = 0;
        break;
    case MGC_CURSE_ITEMS:
        You_feel("as if you need some help.");
        rndcurse();
        dmg = 0;
        break;
    case MGC_DESTRY_ARMR:
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            pline("A field of force surrounds you!");
        } else if (!destroy_arm(some_armor(&g.youmonst), FALSE)) {
            Your("skin itches.");
        }
        dmg = 0;
        break;
    case MGC_WEAKEN_YOU: /* drain strength */
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            You_feel("momentarily weakened.");
        } else {
            You("suddenly feel weaker!");
            dmg = mtmp->m_lev - 6;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            losestr(rnd(dmg));
            if (u.uhp < 1)
                done_in_by(mtmp, DIED);
        }
        dmg = 0;
        break;
    case MGC_DISAPPEAR: /* makes self invisible */
        if (!mtmp->minvis && !mtmp->invis_blkd) {
            if (canseemon(mtmp))
                pline("%s suddenly %s!", Monnam(mtmp),
                      !See_invisible ? "disappears" : "becomes transparent");
            mon_set_minvis(mtmp);
            if (cansee(mtmp->mx, mtmp->my) && !canspotmon(mtmp))
                map_invisible(mtmp->mx, mtmp->my);
            dmg = 0;
        } else
            impossible("no reason for monster to cast disappear spell?");
        break;
    case MGC_STUN_YOU:
        if (Antimagic || Free_action) {
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
        }
        dmg = 0;
        break;
    case MGC_HASTE_SELF:
        mon_adjust_speed(mtmp, 1, (struct obj *) 0);
        dmg = 0;
        break;
    case MGC_CURE_SELF:
        dmg = m_cure_self(mtmp, dmg);
        break;
    case MGC_PSI_BOLT:
        /* prior to 3.4.0 Antimagic was setting the damage to 1--this
           made the spell virtually harmless to players with magic res. */
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            dmg = (dmg + 1) / 2;
        }
        if (dmg <= 5)
            You("get a slight %sache.", body_part(HEAD));
        else if (dmg <= 10)
            Your("brain is on fire!");
        else if (dmg <= 20)
            Your("%s suddenly aches painfully!", body_part(HEAD));
        else
            Your("%s suddenly aches very painfully!", body_part(HEAD));
        break;
    case MGC_TPORT_AWAY: {
        /* this is better than reimplementing the logic of rloc to pick a random
         * spot that is sufficiently far away from (mux, muy) */
        xchar tries = 3;
        do {
            rloc(mtmp, RLOC_MSG);
        } while (--tries
                 && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) < 10);
        dmg = 0;
        break;
    }
    case MGC_ENTOMB: {
        /* entomb you in rocks (and maybe a couple diggable walls) to delay you
         * and allow some time for the caster to get away */
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
                    && !g.level.objects[x][y] && !t_at(x, y)) {
                    levl[x][y].typ = rn2(2) ? HWALL : VWALL;
                    levl[x][y].wall_info &= ~W_NONDIGGABLE;
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
        dmg = 0;
        break;
    }
    case MGC_DARK_SPEECH:
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
            You("%s rapidly decomposing!", Withering ? "continue" : "begin");
            incr_itimeout(&HWithering, rn1(40, 100));
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
                Blinded |= FROMOUTSIDE;
                if (!Blind)
                    /* wearing Eyes of the Overworld - no effect, undo it */
                    Blinded &= ~FROMOUTSIDE;
                else if (!was_blind_before)
                    You("can no longer see.");
                break;
            }
        }
        dmg = 0;
        break;
    case MGC_SHEER_COLD:
        sheer_cold(&dmg);
        break;
    default:
        impossible("mcastu: invalid magic spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    if (dmg)
        mdamageu(mtmp, dmg);
}

DISABLE_WARNING_FORMAT_NONLITERAL

static void
cast_cleric_spell(struct monst *mtmp, int dmg, int spellnum)
{
    int orig_dmg = 0;
    if (dmg == 0 && !is_undirected_spell(AD_CLRC, spellnum)) {
        impossible("cast directed cleric spell (%d) with dmg=0?", spellnum);
        return;
    }

    switch (spellnum) {
    case CLC_GEYSER:
        /* this is physical damage (force not heat),
         * not magical damage or fire damage
         */
        pline("A sudden geyser slams into you from nowhere!");
        dmg = d(8, 6);
        if (Half_physical_damage)
            dmg = (dmg + 1) / 2;
        if (u.umonnum == PM_IRON_GOLEM) {
            You("rust!");
            Strcpy(g.killer.name, "rusted away");
            g.killer.format = NO_KILLER_PREFIX;
            rehumanize();
            dmg = 0; /* prevent further damage after rehumanization */
        }
        erode_armor(&g.youmonst, ERODE_RUST);
        break;
    case CLC_FIRE_PILLAR:
        pline("A pillar of fire strikes all around you!");
        orig_dmg = dmg = d(8, 6);
        if (Fire_resistance) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_FIRE);
            dmg = 0;
        }
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        burn_away_slime();
        (void) burnarmor(&g.youmonst);
        (void) destroy_items(&g.youmonst, AD_FIRE, orig_dmg);
        ignite_items(g.invent);
        (void) burn_floor_objects(u.ux, u.uy, TRUE, FALSE);
        break;
    case CLC_LIGHTNING: {
        pline("A bolt of lightning strikes down at you from above!");
        const char* reflectsrc = ureflectsrc();
        orig_dmg = dmg = d(8, 6);
        if (reflectsrc || Shock_resistance) {
            shieldeff(u.ux, u.uy);
            dmg = 0;
            if (reflectsrc) {
                pline("It bounces off your %s.", reflectsrc);
                monstseesu(M_SEEN_REFL);
                break;
            }
            monstseesu(M_SEEN_ELEC);
        }
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        (void) destroy_items(&g.youmonst, AD_ELEC, orig_dmg);
        (void) flashburn((long) rnd(100));
        break;
    }
    case CLC_CURSE_ITEMS:
        You_feel("as if you need some help.");
        rndcurse();
        dmg = 0;
        break;
    case CLC_INSECTS: {
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
        for (i = 0; i <= quan; i++) {
            if (!enexto(&bypos, mtmp->mux, mtmp->muy, mtmp->data))
                break;
            if ((pm = mkclass(let, 0)) != 0
                && (mtmp2 = makemon(pm, bypos.x, bypos.y, MM_ANGRY|MM_NOMSG)) != 0) {
                success = TRUE;
                mtmp2->msleeping = mtmp2->mpeaceful = mtmp2->mtame = 0;
                set_malign(mtmp2);
            }
        }
        newseen = monster_census(TRUE);

        /* not canspotmon() which includes unseen things sensed via warning */
        seecaster = canseemon(mtmp) || tp_sensemon(mtmp) || Detect_monsters;
        what = (let == S_SNAKE) ? "snakes" : "insects";
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
                if (!Deaf)
                    You_hear("someone summoning something, and %s %s.", arg,
                             vtense(arg, "appear"));
                else
                    pline("%s %s.", upstart(arg), vtense(arg, "appear"));
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
        if (fmt)
            pline(fmt, Monnam(mtmp), what);

        dmg = 0;
        break;
    }
    case CLC_BLIND_YOU:
        /* note: resists_blnd() doesn't apply here */
        if (!Blinded) {
            int num_eyes = eyecount(g.youmonst.data);
            pline("Scales cover your %s!", (num_eyes == 1)
                                               ? body_part(EYE)
                                               : makeplural(body_part(EYE)));
            make_blinded(Half_spell_damage ? 100L : 200L, FALSE);
            if (!Blind)
                Your1(vision_clears);
            dmg = 0;
        } else
            impossible("no reason for monster to cast blindness spell?");
        break;
    case CLC_PARALYZE:
        dmg = 4 + (int) mtmp->m_lev;
        if (Half_spell_damage)
            dmg = (dmg + 1) / 2;
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
        }
        dynamic_multi_reason(mtmp, "paralyzed", FALSE);
        make_paralyzed(dmg, FALSE, (const char *) 0);
        dmg = 0;
        break;
    case CLC_CONFUSE_YOU:
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            You_feel("momentarily dizzy.");
        } else {
            boolean oldprop = !!Confusion;

            dmg = (int) mtmp->m_lev;
            if (Half_spell_damage)
                dmg = (dmg + 1) / 2;
            make_confused(HConfusion + dmg, TRUE);
            if (Hallucination)
                You_feel("%s!", oldprop ? "trippier" : "trippy");
            else
                You_feel("%sconfused!", oldprop ? "more " : "");
        }
        dmg = 0;
        break;
    case CLC_CURE_SELF:
        dmg = m_cure_self(mtmp, dmg);
        break;
    case CLC_OPEN_WOUNDS:
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_MAGR);
            dmg = (dmg + 1) / 2;
        }
        if (dmg <= 5)
            Your("skin itches badly for a moment.");
        else if (dmg <= 10)
            pline("Wounds appear on your body!");
        else if (dmg <= 20)
            pline("Severe wounds appear on your body!");
        else
            Your("body is covered with painful wounds!");
        break;
    default:
        impossible("mcastu: invalid clerical spell (%d)", spellnum);
        dmg = 0;
        break;
    }

    if (dmg)
        mdamageu(mtmp, dmg);
}

RESTORE_WARNING_FORMAT_NONLITERAL

static boolean
is_undirected_spell(unsigned int adtyp, int spellnum)
{
    if (adtyp == AD_SPEL) {
        switch (spellnum) {
        case MGC_CLONE_WIZ:
        case MGC_SUMMON_MONS:
        case MGC_AGGRAVATION:
        case MGC_DISAPPEAR:
        case MGC_HASTE_SELF:
        case MGC_CURE_SELF:
        case MGC_TPORT_AWAY:
        case MGC_ENTOMB:
        /* note that if MGC_SHEER_COLD were to appear here, it could be cast at
         * range, but would deal 0 damage, because of the castmu code that sets
         * damage to 0 if !foundyou, which it is when castmu is called from
         * monmove. This might be possible to kludge around. */
        case MGC_DARK_SPEECH:
            return TRUE;
        default:
            break;
        }
    } else if (adtyp == AD_CLRC) {
        switch (spellnum) {
        case CLC_INSECTS:
        case CLC_CURE_SELF:
            return TRUE;
        default:
            break;
        }
    }
    return FALSE;
}

/* Some spells are useless under some circumstances. */
static boolean
spell_would_be_useless(struct monst *mtmp, unsigned int adtyp, int spellnum)
{
    /* Some spells don't require the player to really be there and can be cast
     * by the monster when you're invisible, yet still shouldn't be cast when
     * the monster doesn't even think you're there.
     * This check isn't quite right because it always uses your real position.
     * We really want something like "if the monster could see mux, muy".
     */
    boolean mcouldseeu = couldsee(mtmp->mx, mtmp->my);

    if (adtyp == AD_SPEL) {
        /* aggravate monsters, etc. won't be cast by peaceful monsters */
        if (mtmp->mpeaceful
            && (spellnum == MGC_AGGRAVATION || spellnum == MGC_SUMMON_MONS
                || spellnum == MGC_CLONE_WIZ))
            return TRUE;
        /* haste self when already fast */
        if (mtmp->permspeed == MFAST && spellnum == MGC_HASTE_SELF)
            return TRUE;
        /* invisibility when already invisible */
        if ((mtmp->minvis || mtmp->invis_blkd) && spellnum == MGC_DISAPPEAR)
            return TRUE;
        /* peaceful monster won't cast invisibility if you can't see
           invisible,
           same as when monsters drink potions of invisibility.  This doesn't
           really make a lot of sense, but lets the player avoid hitting
           peaceful monsters by mistake */
        if (mtmp->mpeaceful && !See_invisible && spellnum == MGC_DISAPPEAR)
            return TRUE;
        /* healing when already healed */
        if (mtmp->mhp == mtmp->mhpmax && spellnum == MGC_CURE_SELF)
            return TRUE;
        /* don't summon monsters if it doesn't think you're around */
        if (!mcouldseeu && (spellnum == MGC_SUMMON_MONS
                            || (!mtmp->iswiz && spellnum == MGC_CLONE_WIZ)))
            return TRUE;
        if ((!mtmp->iswiz || g.context.no_of_wizards > 1)
            && spellnum == MGC_CLONE_WIZ)
            return TRUE;
        /* aggravation (global wakeup) when everyone is already active */
        if (spellnum == MGC_AGGRAVATION) {
            /* if nothing needs to be awakened then this spell is useless
               but caster might not realize that [chance to pick it then
               must be very small otherwise caller's many retry attempts
               will eventually end up picking it too often] */
            if (!has_aggravatables(mtmp))
                return rn2(100) ? TRUE : FALSE;
        }
        /* don't teleport away if already sufficiently far away */
        if (spellnum == MGC_TPORT_AWAY
            && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) >= 10)
            return TRUE;
        /* don't entomb if hero is already entombed */
        if (spellnum == MGC_ENTOMB && is_entombed(u.ux, u.uy))
            return TRUE;
        /* sheer cold and dark speech require line of sight */
        if (!mcouldseeu && (spellnum == MGC_DARK_SPEECH
                            || spellnum == MGC_SHEER_COLD))
            return TRUE;
    } else if (adtyp == AD_CLRC) {
        /* summon insects/sticks to snakes won't be cast by peaceful monsters
         */
        if (mtmp->mpeaceful && spellnum == CLC_INSECTS)
            return TRUE;
        /* healing when already healed */
        if (mtmp->mhp == mtmp->mhpmax && spellnum == CLC_CURE_SELF)
            return TRUE;
        /* don't summon insects if it doesn't think you're around */
        if (!mcouldseeu && spellnum == CLC_INSECTS)
            return TRUE;
        /* blindness spell on blinded player */
        if (Blinded && spellnum == CLC_BLIND_YOU)
            return TRUE;
    }
    return FALSE;
}

/* convert 1..10 to 0..9; add 10 for second group (spell casting) */
#define ad_to_typ(k) (10 + (int) k - 1)

/* monster uses spell (ranged) */
int
buzzmu(register struct monst *mtmp, register struct attack *mattk)
{
    /* don't print constant stream of curse messages for 'normal'
       spellcasting monsters at range */
    if (mattk->adtyp > AD_SPC2)
        return 0;

    if (mtmp->mcan) {
        cursetxt(mtmp, FALSE);
        return 0;
    }
    if (lined_up(mtmp) && rn2(3)) {
        nomul(0);
        if (mattk->adtyp && (mattk->adtyp < 11)) { /* no cf unsigned >0 */
            if (canseemon(mtmp))
                pline("%s zaps you with a %s!", Monnam(mtmp),
                      flash_str(ad_to_typ(mattk->adtyp), FALSE));
            buzz(-ad_to_typ(mattk->adtyp), (int) mattk->damn, mtmp->mx,
                 mtmp->my, sgn(g.tbx), sgn(g.tby));
        } else
            impossible("Monster spell %d cast", mattk->adtyp - 1);
    }
    return 1;
}

/* is (x,y) entombed (completely surrounded by boulders or nonwalkable spaces)?
 * note that (x,y) itself is not checked */
static boolean
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

/* extracted from castmu; if the corresponding flame spell is ever used and
 * treated as one of several possible spells in a demon lord's repertoire, it
 * should also probably be extracted.
 * The function expects *dmg to be the already rolled amount of damage the spell
 * will deliver by default. It may adjust *dmg in the process; the caller should
 * anticipate this. */
static void
sheer_cold(int *dmg)
{
    pline("You're covered in frigid frost.");
    if (Cold_resistance) {
        shieldeff(u.ux, u.uy);
        pline("You partially resist the effects.");
        monstseesu(M_SEEN_COLD);
        *dmg /= 4;
    }
    destroy_items(&g.youmonst, AD_COLD, *dmg);
}

/*mcastu.c*/
