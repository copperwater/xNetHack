/* NetHack 3.7	steal.c	$NHDT-Date: 1720895742 2024/07/13 18:35:42 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.132 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

staticfn int unstolenarm(void);
staticfn int stealarm(void);
staticfn void worn_item_removal(struct monst *, struct obj *);

/* proportional subset of gold; return value actually fits in an int */
long
somegold(long lmoney)
{
    int igold = (lmoney >= (long) LARGEST_INT) ? LARGEST_INT : (int) lmoney;

    if (igold < 50)
        ; /* all gold */
    else if (igold < 100)
        igold = rn1(igold - 25 + 1, 25);
    else if (igold < 500)
        igold = rn1(igold - 50 + 1, 50);
    else if (igold < 1000)
        igold = rn1(igold - 100 + 1, 100);
    else if (igold < 5000)
        igold = rn1(igold - 500 + 1, 500);
    else if (igold < 10000)
        igold = rn1(igold - 1000 + 1, 1000);
    else
        igold = rn1(igold - 5000 + 1, 5000);

    return (long) igold;
}

/*
 * Find the first gold object in a chain.
 * Used when leprechaun (or you as leprechaun) looks for
 * someone else's gold.  Returns a pointer so the gold may
 * be seized without further searching.
 * May search containers too.
 * Deals in gold only, as leprechauns don't care for lesser materials.
 * If only_coins is FALSE, it will return the first actual gold object in this
 * chain, not just gold pieces. If it's TRUE, it will only look for gold
 * pieces.
*/
struct obj *
findgold(struct obj* argchain, boolean only_coins)
{
    struct obj *chain = argchain; /* allow arg to be nonnull */
    struct obj* gold = (struct obj *) 0;
    int ngoldobjs = 0;
    while (chain) {
        if (only_coins && chain->otyp == GOLD_PIECE) {
            /* assume no multiple gold stacks */
            return chain;
        }
        else if (!only_coins && chain->material == GOLD) {
            ngoldobjs++;
            if (!rn2(ngoldobjs)) {
                gold = chain;
            }
        }
        chain = chain->nobj;
    }
    return gold;
}

/*
 * Steal gold coins only.  Leprechauns don't care for lesser coins.
*/
void
stealgold(struct monst *mtmp)
{
    struct obj *fgold;
    struct obj *ygold;
    long tmp;
    struct monst *who;
    const char *whose, *what;

    /* look for gold on the floor */
    fgold = svl.level.objects[u.ux][u.uy];
    while (fgold && fgold->material != GOLD)
        fgold = fgold->nexthere;

    /* Do you have real gold? */
    ygold = findgold(gi.invent, FALSE);

    if (fgold && (!ygold || fgold->quan > ygold->quan || !rn2(5))) {
        obj_extract_self(fgold);
        add_to_minv(mtmp, fgold);
        newsym(u.ux, u.uy);
        if (u.usteed) {
            who = u.usteed;
            whose = s_suffix(y_monnam(who));
            what = makeplural(mbodypart(who, FOOT));
        } else {
            who = &gy.youmonst;
            whose = "your";
            what = makeplural(body_part(FOOT));
        }
        /* [ avoid "between your rear regions" :-] */
        if (slithy(who->data))
            what = "coils";
        /* reduce "rear hooves/claws" to "hooves/claws" */
        if (!strncmp(what, "rear ", 5))
            what += 5;
        pline("%s quickly snatches some gold from %s %s %s!", Monnam(mtmp),
              (Levitation || Flying) ? "beneath" : "between", whose, what);
        if (!ygold || !rn2(5)) {
            if (!tele_restrict(mtmp))
                (void) rloc(mtmp, RLOC_MSG);
            monflee(mtmp, 0, FALSE, FALSE);
        }
    } else if (ygold) {
        if (ygold->otyp == GOLD_PIECE) {
            const int gold_price = objects[GOLD_PIECE].oc_cost;

            tmp = (somegold(money_cnt(gi.invent)) + gold_price - 1) / gold_price;
            tmp = min(tmp, ygold->quan);
            if (tmp < ygold->quan)
                ygold = splitobj(ygold, tmp);
            else
                setnotworn(ygold);
            Your("purse feels lighter.");
        }
        else {
            pline("%s steals %s!", Monnam(mtmp), yname(ygold));
            remove_worn_item(ygold, TRUE);
        }
        freeinv(ygold);
        add_to_minv(mtmp, ygold);
        if (!tele_restrict(mtmp))
            (void) rloc(mtmp, RLOC_MSG);
        monflee(mtmp, 0, FALSE, FALSE);
        disp.botl = TRUE;
    }
}

/* monster who was stealing from hero has just died */
void
thiefdead(void)
{
    /* hero is busy taking off an item of armor which takes multiple turns */
    gs.stealmid = 0;
    if (ga.afternmv == stealarm) {
        ga.afternmv = unstolenarm;
        gn.nomovemsg = (char *) 0;
    }
}

/* checks whether hero can be responsive to seduction attempts; similar to
   Unaware but also includes paralysis */
boolean
unresponsive(void)
{
    if (gm.multi >= 0)
        return FALSE;

    return (unconscious() || is_fainted()
            || (gm.multi_reason
                && (!strncmp(gm.multi_reason, "frozen", 6)
                    || !strncmp(gm.multi_reason, "paralyzed", 9))));
}

/* called via (*ga.afternmv)() when hero finishes taking off armor that
   was slated to be stolen but the thief died in the interim */
staticfn int
unstolenarm(void)
{
    struct obj *obj;

    /* find the object before clearing stealoid; it has already become
       not-worn and is still in hero's inventory */
    for (obj = gi.invent; obj; obj = obj->nobj)
        if (obj->o_id == gs.stealoid)
            break;
    gs.stealoid = 0;
    if (obj) {
        You("finish taking off your %s.", armor_simple_name(obj));
    }
    return 0;
}

/* finish stealing an item of armor which takes multiple turns to take off */
staticfn int
stealarm(void)
{
    struct monst *mtmp;
    struct obj *otmp, *nextobj;

    if (!gs.stealoid || !gs.stealmid)
        goto botm;

    for (otmp = gi.invent; otmp; otmp = nextobj) {
        nextobj = otmp->nobj;
        if (otmp->o_id == gs.stealoid) {
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (mtmp->m_id == gs.stealmid) {
                    if (DEADMONSTER(mtmp)) {
                        impossible("stealarm(): dead monster stealing");
                        goto botm; /* (could just use 'break' here) */
                    }
                    /* maybe the thief polymorphed into something without a
                       steal attack, or perhaps while stealing hero's suit
                       the thief took away other items causing hero to fall
                       into water or lava and then teleport to safety */
                    if (!dmgtype(mtmp->data, AD_SITM)
                        || distu(mtmp->mx, mtmp->my) > 2)
                        goto botm; /* (could just use 'break' here) */
                    if (otmp->unpaid)
                        subfrombill(otmp, shop_keeper(*u.ushops));
                    freeinv(otmp);
                    pline("%s steals %s!", Monnam(mtmp), doname(otmp));
                    (void) mpickobj(mtmp, otmp); /* may free otmp */
                    /* Implies seduction, "you gladly hand over ..."
                       so we don't set mavenge bit here. */
                    monflee(mtmp, 0, FALSE, FALSE);
                    if (!tele_restrict(mtmp))
                        (void) rloc(mtmp, RLOC_MSG);
                    break;
                }
            }
            break;
        }
    }
 botm:
    gs.stealoid = gs.stealmid = 0; /* in case only one has been reset so far */
    return 0;
}

/* An object you're wearing has been taken off by a monster (theft or
   seduction).  Also used if a worn item gets transformed (stone to flesh). */
void
remove_worn_item(
    struct obj *obj,
    boolean unchain_ball) /* whether to unpunish or just unwield */
{
    unsigned oldinuse;

    if (donning(obj))
        cancel_don();
    if (!obj->owornmask)
        return;

    /*
     * Losing worn gear might drop hero into water or lava or onto a
     * location-changing trap or take away the ability to breathe in water.
     * Marking it 'in_use' prevents emergency_disrobe() from dropping it
     * and lava_effects() from destroying it; other cases impacting object
     * location (or destruction) might still have issues.
     *
     * Note:  if a hangup save occurs when 'in_use' is set, the item will
     * be destroyed via useup() during restore.  Maybe remove_worn_item()
     * and emergency_disrobe() should switch to using obj->bypass instead
     * but that would need a lot more cooperation by callers.  It's a
     * tradeoff between protecting the player against unintentional hangup
     * and defending the game against deliberate hangup when player sees a
     * message about something undesirable followed by --More--.
     */
    oldinuse = obj->in_use;
    obj->in_use = 1;

    if (obj->owornmask & W_ARMOR) {
        if (obj == uskin) {
            impossible("Removing embedded scales?");
            skinback(TRUE); /* uarm = uskin; uskin = 0; */
        }
        if (obj == uarm)
            (void) Armor_off();
        else if (obj == uarmc)
            (void) Cloak_off();
        else if (obj == uarmf)
            (void) Boots_off();
        else if (obj == uarmg)
            (void) Gloves_off();
        else if (obj == uarmh)
            (void) Helmet_off();
        else if (obj == uarms)
            (void) Shield_off();
        else if (obj == uarmu)
            (void) Shirt_off();
        /* catchall -- should never happen */
        else
            setworn((struct obj *) 0, obj->owornmask & W_ARMOR);
    } else if (obj->owornmask & W_AMUL) {
        Amulet_off();
    } else if (obj->owornmask & W_RING) {
        Ring_gone(obj);
    } else if (obj->owornmask & W_TOOL) {
        Blindf_off(obj);
    } else if (obj->owornmask & W_WEAPONS) {
        if (obj == uwep)
            uwepgone();
        if (obj == uswapwep)
            uswapwepgone();
        if (obj == uquiver)
            uqwepgone();
    }

    if (obj->owornmask & (W_BALL | W_CHAIN)) {
        if (unchain_ball)
            unpunish();
    } else if (obj->owornmask) {
        /* catchall */
        setnotworn(obj);
    }

    if (obj->where == OBJ_DELETED)
        debugpline1("remove_worn_item() \"%s\" deleted!", simpleonames(obj));
    obj->in_use = oldinuse;
}

/* during theft of a worn item: remove_worn_item(), prefaced by a message */
staticfn void
worn_item_removal(
    struct monst *mon,
    struct obj *obj)
{
    char objbuf[BUFSZ], article[20], *p;
    const char *verb;
    int strip_art;

    Strcpy(objbuf, doname(obj));
    /* massage the object description */
    strip_art = !strncmp(objbuf, "the ", 4) ? 4
                : !strncmp(objbuf, "an ", 3) ? 3
                  : !strncmp(objbuf, "a ", 2) ? 2
                    : 0;
    if (strip_art) { /* convert "a/an/the <object>" to "your object" */
        copynchars(article, objbuf, strip_art);
        /* when removing attached iron ball, caller passes 'uchain';
           when formatted, it will be "an iron chain (attached to you)";
           change "an" to "the" rather than to "your" in that situation */
        (void) strsubst(objbuf, article, (obj == uchain) ? "the " : "your ");
    }
    /* these ought to be guarded against matching user-supplied name */
    (void) strsubst(objbuf, " (being worn)", "");
    (void) strsubst(objbuf, " (alternate weapon; not wielded)", "");
    /* convert "ring (on left hand)" to "ring (from left hand)" */
    if ((p = strstri(objbuf, " (on "))
        && (!strncmp(p + 5, "left ", 5) || !strncmp(p + 5, "right ", 6)))
        (void) strsubst(p + 2, "on", "from");

    /* slightly iffy for alternate weapon that isn't actively dual-wielded,
       but it's better to alert the player to the change in equipment than
       to suppress the message for that case */
    verb = ((obj->owornmask & W_WEAPONS) != 0L) ? "disarms"
           : ((obj->owornmask & W_ACCESSORY) != 0L) ? "removes"
             : "takes off";
    pline("%s %s %s.", Some_Monnam(mon), verb, objbuf);
    iflags.last_msg = PLNMSG_MON_TAKES_OFF_ITEM;
    /* removal might trigger more messages (due to loss of Lev|Fly;
       descending happens before the theft in progress finishes) */
    remove_worn_item(obj, TRUE);
}

/* Returns 1 when something was stolen (or at least, when N should flee now),
 * returns -1 if the monster died in the attempt.
 * Avoid stealing the object 'stealoid'.
 * Nymphs and monkeys won't steal coins (so that their "steal item" attack
 * doesn't become a superset of leprechaun's "steal gold" attack).
 */
int
steal(struct monst *mtmp, char *objnambuf)
{
    struct obj *otmp;
    char Monnambuf[BUFSZ];
    int tmp, could_petrify, armordelay, olddelay, icnt,
        named = 0, retrycnt = 0;
    boolean monkey_business = is_animal(mtmp->data),
            seen = canspotmon(mtmp),
            was_doffing, was_punished = Punished;

    if (objnambuf)
        *objnambuf = '\0';
    /* the following is true if successful on first of two attacks. */
    if (!monnear(mtmp, u.ux, u.uy))
        return 0;

    /* stealing a worn item might drop hero into water or lava where
       teleporting to safety could result in a previously visible thief
       no longer being visible; it could also be a case of a blinded
       hero being able to see via wearing the Eyes of the Overworld and
       having those stolen; remember the name as it is now; if unseen,
       nymphs will be "Someone" and monkeys will be "Something" */
    Strcpy(Monnambuf, Some_Monnam(mtmp));

    /* food being eaten might already be used up but will not have
       been removed from inventory yet; we don't want to steal that,
       so this will cause it to be removed now */
    if (go.occupation)
        (void) maybe_finished_meal(FALSE);

    icnt = inv_cnt(FALSE); /* don't include gold */
    if (!icnt || (icnt == 1 && uskin)) {
        /* Not even a thousand men in armor can strip a naked man. */
 nothing_to_steal:
        /* nymphs might target uchain if invent is empty; monkeys won't;
           hero becomes unpunished but nymph ends up empty handed */
        if (Punished && !monkey_business && rn2(4)) {
            /* uball is not carried (uchain never is) */
            assert(uball != NULL && uball->where == OBJ_FLOOR);
            worn_item_removal(mtmp, uchain);
        } else if (u.utrap && u.utraptype == TT_BURIEDBALL
                   && !monkey_business && !rn2(4)) {
            boolean dummy;

            /* buried ball is not tracked via 'uball' and there is no chain
               at all (hence no uchain to take off) */
            pline("%s takes off your unseen chain.", Monnambuf);
            (void) openholdingtrap(&gy.youmonst, &dummy);
        } else if (Blind) {
            pline("Somebody tries to rob you, but finds nothing to steal.");
        } else if (inv_cnt(TRUE) > inv_cnt(FALSE)) {
            pline("%s tries to rob you, but isn't interested in gold.",
                  Monnambuf);
        } else {
            pline("%s tries to rob you, but there is nothing to steal!",
                  Monnambuf);
        }
        return 1; /* let her flee */
    }

    if (monkey_business || uarmg) {
        ; /* skip ring special cases */
    } else if (Adornment & LEFT_RING) {
        otmp = uleft;
        goto gotobj;
    } else if (Adornment & RIGHT_RING) {
        otmp = uright;
        goto gotobj;
    }

 retry:
    tmp = 0;
    for (otmp = gi.invent; otmp; otmp = otmp->nobj)
        if ((!uarm || otmp != uarmc) && otmp != uskin
            && otmp->oclass != COIN_CLASS)
            tmp += (otmp->owornmask & (W_ARMOR | W_ACCESSORY)) ? 5 : 1;
    if (!tmp)
        goto nothing_to_steal;
    tmp = rn2(tmp);
    for (otmp = gi.invent; otmp; otmp = otmp->nobj)
        if ((!uarm || otmp != uarmc) && otmp != uskin
            && otmp->oclass != COIN_CLASS) {
            tmp -= (otmp->owornmask & (W_ARMOR | W_ACCESSORY)) ? 5 : 1;
            if (tmp < 0)
                break;
        }
    if (!otmp) {
        impossible("Steal fails!");
        return 0;
    }
    /* can't steal ring(s) while wearing gloves */
    if ((otmp == uleft || otmp == uright) && uarmg)
        otmp = uarmg;
    /* can't steal gloves while wielding - so steal the wielded item. */
    if (otmp == uarmg && uwep)
        otmp = uwep;
    /* can't steal armor while wearing cloak - so steal the cloak. */
    else if (otmp == uarm && uarmc)
        otmp = uarmc;
    /* can't steal shirt while wearing cloak or suit */
    else if (otmp == uarmu && uarmc)
        otmp = uarmc;
    else if (otmp == uarmu && uarm)
        otmp = uarm;

 gotobj:
    if (otmp->o_id == gs.stealoid)
        return 0;

    if (otmp->otyp == BOULDER && !throws_rocks(mtmp->data)) {
        if (!retrycnt++)
            goto retry;
        goto cant_take;
    }
    /* animals can't overcome curse stickiness nor unlock chains */
    if (monkey_business) {
        boolean ostuck;

        /* is the player prevented from voluntarily giving up this item? */
        if (otmp == uball)
            ostuck = TRUE; /* effectively worn; curse is implicit */
        else if (otmp == uquiver || (otmp == uswapwep && !u.twoweap))
            ostuck = FALSE; /* not really worn; curse doesn't matter */
        else
            ostuck = ((otmp->cursed && otmp->owornmask)
                      /* nymphs can steal rings from under
                         cursed weapon but animals can't */
                      || (otmp == RING_ON_PRIMARY && welded(uwep))
                      || (otmp == RING_ON_SECONDARY && welded(uwep)
                          && bimanual(uwep))
                      || undroppable(otmp));

        if (ostuck || can_carry(mtmp, otmp) == 0) {
            static const char *const how[] = {
                "steal", "snatch", "grab", "take"
            };
 cant_take:
            pline("%s tries to %s %s%s but gives up.", Monnambuf,
                  ROLL_FROM(how),
                  (otmp->owornmask & W_ARMOR) ? "your " : "",
                  (otmp->owornmask & W_ARMOR) ? armor_simple_name(otmp)
                                              : yname(otmp));
            /* the fewer items you have, the less likely the thief
               is going to stick around to try again (0) instead of
               running away (1) */
            return !rn2(inv_cnt(FALSE) / 5 + 2);
        }
    }

    if (otmp->otyp == LEASH && otmp->leashmon) {
        if (monkey_business && otmp->cursed)
            goto cant_take;
        o_unleash(otmp);
    }

    was_doffing = doffing(otmp);
    /* stop donning/doffing now so that afternmv won't be clobbered
       below; stop_occupation doesn't handle donning/doffing */
    olddelay = stop_donning(otmp);
    /* you're going to notice the theft... */
    stop_occupation();

    if (otmp->owornmask & (W_ARMOR | W_ACCESSORY)) {
        switch (otmp->oclass) {
        case TOOL_CLASS:
        case AMULET_CLASS:
        case RING_CLASS:
        case FOOD_CLASS: /* meat ring */
            worn_item_removal(mtmp, otmp);
            break;
        case ARMOR_CLASS:
            armordelay = objects[otmp->otyp].oc_delay;
            if (olddelay > 0 && olddelay < armordelay)
                armordelay = olddelay;
            if (monkey_business || unresponsive()) {
                /* animals usually don't have enough patience to take off
                   items which require extra time; unconscious or paralyzed
                   hero can't be charmed into taking off his own armor */
                if (armordelay >= 1 && !olddelay && rn2(10))
                    goto cant_take;
                worn_item_removal(mtmp, otmp);
                break;
            } else {
                int curssv = otmp->cursed;
                int slowly;

                otmp->cursed = 0;
                slowly = (armordelay >= 1 || gm.multi < 0);
                if (flags.female)
                    urgent_pline("%s charms you.  You gladly %s your %s.",
                                 !seen ? "She" : Monnambuf,
                                 curssv ? "let her take"
                                 : !slowly ? "hand over"
                                   : was_doffing ? "continue removing"
                                     : "start removing",
                                 armor_simple_name(otmp));
                else
                    urgent_pline("%s seduces you and %s off your %s.",
                                 !seen ? "She" : Adjmonnam(mtmp, "beautiful"),
                                 curssv ? "helps you to take"
                                 : !slowly ? "you take"
                                   : was_doffing ? "you continue taking"
                                     : "you start taking",
                                 armor_simple_name(otmp));
                named++;
                /* the following is to set multi for later on */
                nomul(-armordelay);
                gm.multi_reason = "taking off clothes";
                gn.nomovemsg = 0;
                remove_worn_item(otmp, TRUE);
                otmp->cursed = curssv;
                if (gm.multi < 0) {
                    gs.stealoid = otmp->o_id;
                    gs.stealmid = mtmp->m_id;
                    ga.afternmv = stealarm;
                    return 0;
                }
            }
            break;
        default:
            impossible("Tried to steal a strange worn thing. [%d]",
                       otmp->oclass);
        }
        /* hero's blindfold might have just been stolen; if so, replace
           cached "Someone" or "Something" with Monnam */
        if (!seen && canspotmon(mtmp))
            Strcpy(Monnambuf, Monnam(mtmp));
    } else if (otmp->owornmask) { /* weapon or ball&chain */
        struct obj *item = otmp;

        if (otmp == uball) /* non-Null uball implies non-Null uchain */
            item = uchain; /* yields a more accurate 'takes off' message */
        worn_item_removal(mtmp, item);
        /* if we switched from uball to uchain for the preface message,
           then unpunish() took place and both those pointers are now Null,
           with 'item' a stale pointer to freed chain; the ball is still
           present though and 'otmp' is still valid; if uball was also
           wielded or quivered, the corresponding weapon pointer hasn't
           been cleared yet; do that, with no preface message this time */
        if ((otmp->owornmask & W_WEAPONS) != 0L)
            remove_worn_item(otmp, FALSE);
    }

    /* do this before removing it from inventory */
    if (objnambuf)
        Strcpy(objnambuf, yname(otmp));
    /* usually set mavenge bit so knights won't suffer an alignment penalty
       during retaliation; not applicable for removing attached iron ball */
    if (!Conflict && !(was_punished && !Punished))
        mtmp->mavenge = 1;

    if (otmp->unpaid)
        subfrombill(otmp, shop_keeper(*u.ushops));
    freeinv(otmp);

    /* if we just gave a message about removing a worn item and there have
       been no intervening messages, shorten '<mon> stole <item>' message */
    if (iflags.last_msg == PLNMSG_MON_TAKES_OFF_ITEM
        && mtmp->data->mlet == S_NYMPH)
        ++named;
    urgent_pline("%s stole %s.", named ? "She" : Monnambuf, doname(otmp));
    (void) encumber_msg();
    could_petrify = (otmp->otyp == CORPSE
                     && touch_petrifies(&mons[otmp->corpsenm]));
    otmp->how_lost = LOST_STOLEN;
    (void) mpickobj(mtmp, otmp); /* may free otmp */
    if (could_petrify && !(mtmp->misc_worn_check & W_ARMG)) {
        minstapetrify(mtmp, TRUE);
        return -1;
    }
    if (monkey_business && gm.multi >= 0) {
        /* check multi here because we only want the speed boost to go towards
         * fleeing, not towards further attacks if player can't move */
        int bonus = NORMAL_SPEED;
        if (mtmp->mspeed == MFAST) {
            bonus *= 2;
        }
        else if (mtmp->mspeed == MSLOW) {
            bonus /= 2;
        }
        mtmp->movement += bonus;
    }
    return (gm.multi < 0) ? 0 : 1;
}

/* Returns 1 if otmp is free'd, 0 otherwise. */
int
mpickobj(struct monst *mtmp, struct obj *otmp)
{
    int freed_otmp;
    boolean snuff_otmp = FALSE;

    if (!otmp) {
        impossible("monster (%s) taking or picking up nothing?",
                   pmname(mtmp->data, Mgender(mtmp)));
        return 1;
    } else if (otmp == uball || otmp == uchain) {
        impossible("monster (%s) taking or picking up attached %s (%s)?",
                   pmname(mtmp->data, Mgender(mtmp)),
                   (otmp == uchain) ? "chain" : "ball", simpleonames(otmp));
        return 0;
    }
    /* if monster is acquiring a thrown or kicked object, the throwing
       or kicking code shouldn't continue to track and place it */
    if (otmp == gt.thrownobj)
        gt.thrownobj = 0;
    else if (otmp == gk.kickedobj)
        gk.kickedobj = 0;
    /* an unpaid item can be on the floor; if a monster picks it up, take
       it off the shop bill */
    if (otmp->unpaid || (Has_contents(otmp) && count_unpaid(otmp->cobj))) {
        subfrombill(otmp, find_objowner(otmp, otmp->ox, otmp->oy));
    }
    /* don't want hidden light source inside the monster; assumes that
       engulfers won't have external inventories; whirly monsters cause
       the light to be extinguished rather than letting it shine thru */
    if (obj_sheds_light(otmp) && attacktype(mtmp->data, AT_ENGL)) {
        /* this is probably a burning object that you dropped or threw */
        if (engulfing_u(mtmp) && !Blind)
            pline("%s out.", Tobjnam(otmp, "go"));
        snuff_otmp = TRUE;
    }
    /* for hero owned object on shop floor, mtmp is taking possession
       and if it's eventually dropped in a shop, shk will claim it */
    otmp->no_charge = 0;
    /* some object handling is only done if mtmp isn't a pet */
    if (!mtmp->mtame) {
        /* if monst is unseen, some info hero knows about this object becomes
           lost; continual pickup and drop by pets makes this too annoying if
           it is applied to them; when engulfed (where monster can't be seen
           because vision is disabled), or when held (or poly'd and holding)
           while blind, behave as if the monster can be 'seen' by touch */
        if (!canseemon(mtmp) && mtmp != u.ustuck)
            unknow_object(otmp);
        /* if otmp has flags set for how it left hero's inventory, change
           those flags; if thrown, now stolen and autopickup might override
           pickup_types and autopickup exceptions based on 'pickup_stolen'
           rather than 'pickup_thrown'; if previously stolen, stays stolen;
           if previously dropped, now forgotten and autopickup will operate
           normally regardless of the setting for 'dropped_nopick' */
        if (otmp->how_lost == LOST_THROWN)
            otmp->how_lost = LOST_STOLEN;
        else if (otmp->how_lost == LOST_DROPPED)
            otmp->how_lost = LOST_NONE;
    }
    /* Must do carrying effects on object prior to add_to_minv() */
    carry_obj_effects(otmp);
    /* add_to_minv() might free otmp [if merged with something else],
       so we have to call it after doing the object checks */
    freed_otmp = add_to_minv(mtmp, otmp);
    /* and we had to defer this until object is in mtmp's inventory */
    if (snuff_otmp)
        snuff_light_source(mtmp->mx, mtmp->my);
    return freed_otmp;
}

/* Take off gear which is blocking the target obj from being removed [by a quest
 * artifact steal, or a demon lord interested in the item] */
void
remove_outer_gear(struct monst *remover, struct obj *target)
{
    if ((target == uarm || target == uarmu) && uarmc)
        worn_item_removal(remover, uarmc);
    if (target == uarmu && uarm)
        worn_item_removal(remover, uarm);
    if ((target == uarmg || ((target == uright || target == uleft) && uarmg))
        && uwep) {
        /* gloves are about to be unworn; unwield weapon(s) first */
        if (u.twoweap)    /* remove_worn_item(uswapwep) indirectly */
            worn_item_removal(remover, uswapwep);
        worn_item_removal(remover, uwep);
    }
    if ((target == uright || target == uleft) && uarmg) {
        /* calls Gloves_off() to handle wielded cockatrice corpse */
        worn_item_removal(remover, uarmg);
    }
}

/* called for AD_SAMU (the Wizard and quest nemeses) */
void
stealamulet(struct monst *mtmp)
{
    char buf[BUFSZ];
    struct obj *otmp = 0, *obj = 0;
    int real = 0, fake = 0, n;

    /* target every quest artifact, not just current role's;
       if hero has more than one, choose randomly so that player
       can't use inventory ordering to influence the theft */
    for (n = 0, obj = gi.invent; obj; obj = obj->nobj)
        if (any_quest_artifact(obj))
            ++n, otmp = obj;
    if (n > 1) {
        n = rnd(n);
        for (otmp = gi.invent; otmp; otmp = otmp->nobj)
            if (any_quest_artifact(otmp) && !--n)
                break;
    }

    if (!otmp) {
        /* if we didn't find any quest artifact, find another valuable item */
        if (u.uhave.amulet) {
            real = AMULET_OF_YENDOR;
            fake = FAKE_AMULET_OF_YENDOR;
        } else if (u.uhave.bell) {
            real = BELL_OF_OPENING;
            fake = BELL;
        } else if (u.uhave.book) {
            real = SPE_BOOK_OF_THE_DEAD;
        } else if (u.uhave.menorah) {
            real = CANDELABRUM_OF_INVOCATION;
        } else
            return; /* you have nothing of special interest */

        /* If we get here, real and fake have been set up. */
        for (n = 0, obj = gi.invent; obj; obj = obj->nobj)
            if (obj->otyp == real || (obj->otyp == fake && !mtmp->iswiz))
                ++n, otmp = obj;
        if (n > 1) {
            n = rnd(n);
            for (otmp = gi.invent; otmp; otmp = otmp->nobj)
                if ((otmp->otyp == real
                     || (otmp->otyp == fake && !mtmp->iswiz)) && !--n)
                    break;
        }
    }

    if (otmp) { /* we have something to snatch */
        remove_outer_gear(mtmp, otmp);

        /* finally, steal the target item */
        if (otmp->owornmask)
            worn_item_removal(mtmp, otmp);
        if (otmp->unpaid)
            subfrombill(otmp, shop_keeper(*u.ushops));
        freeinv(otmp);
        Strcpy(buf, doname(otmp));
        (void) mpickobj(mtmp, otmp); /* could merge and free otmp but won't */
        pline("%s steals %s!", Some_Monnam(mtmp), buf);
        if (can_teleport(mtmp->data) && !tele_restrict(mtmp))
            (void) rloc(mtmp, RLOC_MSG);
        (void) encumber_msg();
    }
}

/* when a mimic gets poked with something, it might take that thing
   (at present, only implemented for when the hero does the poking) */
void
maybe_absorb_item(
    struct monst *mon,
    struct obj *obj,
    int ochance, int achance) /* percent chance for ordinary item, artifact */
{
    if (obj == uball || obj == uchain || obj->oclass == ROCK_CLASS
        || obj_resists(obj, 100 - ochance, 100 - achance)
        || !touch_artifact(obj, mon))
        return;

    if (carried(obj)) {
        if (obj->owornmask)
            remove_worn_item(obj, TRUE);
        if (obj->unpaid)
            subfrombill(obj, shop_keeper(*u.ushops));
        if (cansee(mon->mx, mon->my)) {
            /* Some_Monnam() avoids "It pulls ... and absorbs it!"
               if hero can see the location but not the monster */
            pline("%s pulls %s away from you and absorbs %s!",
                  Some_Monnam(mon), /* Monnam() or "Something" */
                  yname(obj), (obj->quan > 1L) ? "them" : "it");
        } else {
            const char *hand_s = body_part(HAND);

            if (bimanual(obj))
                hand_s = makeplural(hand_s);
            pline("%s %s pulled from your %s!", upstart(yname(obj)),
                  otense(obj, "are"), hand_s);
        }
        freeinv(obj);
        (void) encumber_msg();
    } else {
        /* not carried; presumably thrown or kicked */
        if (canspotmon(mon))
            pline("%s absorbs %s!", Monnam(mon), yname(obj));
    }
    /* add to mon's inventory */
    (void) mpickobj(mon, obj);
}

/* drop one object taken from a (possibly dead) monster's inventory */
void
mdrop_obj(
    struct monst *mon,
    struct obj *obj,
    boolean verbosely)
{
    coordxy omx = mon->mx, omy = mon->my;
    long unwornmask = obj->owornmask;
    /* call distant_name() for its possible side-effects even if the result
       might not be printed, and do it before extracting obj from minvent */
    char *obj_name = distant_name(obj, doname);

    extract_from_minvent(mon, obj, FALSE, TRUE);
    /* don't charge for an owned saddle on dead steed (provided
        that the hero is within the same shop at the time) */
    if (unwornmask && mon->mtame && (unwornmask & W_SADDLE) != 0L
        && !obj->unpaid && costly_spot(omx, omy)
        /* being at costly_spot guarantees lev->roomno is not 0 */
        && strchr(in_rooms(u.ux, u.uy, SHOPBASE), levl[omx][omy].roomno)) {
        obj->no_charge = 1;
    }
    /* obj_no_longer_held(obj); -- done by place_object */
    if (verbosely && cansee(omx, omy))
        pline_mon(mon, "%s drops %s.", Monnam(mon), obj_name);
    if (!flooreffects(obj, omx, omy, "fall")) {
        place_object(obj, omx, omy);
        stackobj(obj);
    }
    /* do this last, after placing obj on floor; removing steed's saddle
       throws rider, possibly inflicting fatal damage and producing bones; this
       is why we had to call extract_from_minvent() with do_intrinsics=FALSE */
    if (!DEADMONSTER(mon) && unwornmask)
        update_mon_extrinsics(mon, obj, FALSE, TRUE);
}

/* some monsters bypass the normal rules for moving between levels or
   even leaving the game entirely; when that happens, prevent them from
   taking the Amulet, invocation items, or quest artifact with them */
void
mdrop_special_objs(struct monst *mon)
{
    struct obj *obj, *otmp;

    for (obj = mon->minvent; obj; obj = otmp) {
        otmp = obj->nobj;
        /* the Amulet, invocation tools, and Rider corpses resist even when
           artifacts and ordinary objects are given 0% resistance chance;
           current role's quest artifact is rescued too--quest artifacts
           for the other roles are not */
        if (obj_resists(obj, 0, 0) || is_quest_artifact(obj)) {
            if (mon->mx) {
                mdrop_obj(mon, obj, FALSE);
            } else { /* migrating monster not on map */
                extract_from_minvent(mon, obj, TRUE, TRUE);
                rloco(obj);
            }
        }
    }
}

/* release the objects the creature is carrying */
void
relobj(
    struct monst *mtmp,
    int show,
    boolean is_pet) /* If true, pet should keep wielded/worn items */
{
    struct obj *otmp;
    struct monst *shkp;
    int omx = mtmp->mx, omy = mtmp->my;

    /* vault guard's gold goes away rather than be dropped... */
    if (mtmp->isgd && (otmp = findgold(mtmp->minvent, TRUE)) != 0) {
        if (canspotmon(mtmp))
            pline("%s gold %s.", s_suffix(Monnam(mtmp)),
                  canseemon(mtmp) ? "vanishes" : "seems to vanish");
        obj_extract_self(otmp);
        obfree(otmp, (struct obj *) 0);
    } /* isgd && has gold */

    while ((otmp = (is_pet ? droppables(mtmp) : mtmp->minvent)) != 0) {
        if (is_unpaid(otmp) && costly_spot(omx, omy)
            && (shkp = shop_keeper(*in_rooms(omx, omy, SHOPBASE))) != 0
            && inhishop(shkp)) {
            subfrombill(otmp, shkp);
        }
        mdrop_obj(mtmp, otmp, is_pet && flags.verbose);
    }

    if (show && cansee(omx, omy))
        newsym(omx, omy);
}

/*steal.c*/
