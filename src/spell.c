/* NetHack 3.6	spell.c	$NHDT-Date: 1508479722 2017/10/20 06:08:42 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.84 $ */
/*      Copyright (c) M. Stephenson 1988                          */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* spellmenu arguments; 0 thru n-1 used as spl_book[] index when swapping */
#define SPELLMENU_CAST (-2)
#define SPELLMENU_VIEW (-1)
#define SPELLMENU_SORT (MAXSPELL) /* special menu entry */

/* spell retention period, in turns; at 10% of this value, player becomes
   eligible to reread the spellbook and regain 100% retention (the threshold
   used to be 1000 turns, which was 10% of the original 10000 turn retention
   period but didn't get adjusted when that period got doubled to 20000) */
#define KEEN 20000
/* x: need to add 1 when used for reading a spellbook rather than for hero
   initialization; spell memory is decremented at the end of each turn,
   including the turn on which the spellbook is read; without the extra
   increment, the hero used to get cheated out of 1 turn of retention */
#define incrnknow(spell, x) (spl_book[spell].sp_know = KEEN + (x))

#define spellev(spell) spl_book[spell].sp_lev
#define spellname(spell) OBJ_NAME(objects[spellid(spell)])
#define spellet(spell) \
    ((char) ((spell < 26) ? ('a' + spell) : ('A' + spell - 26)))

STATIC_DCL int FDECL(spell_let_to_idx, (CHAR_P));
STATIC_DCL boolean FDECL(cursed_book, (struct obj * bp));
STATIC_DCL boolean FDECL(confused_book, (struct obj *));
STATIC_DCL void FDECL(deadbook, (struct obj *));
STATIC_PTR int NDECL(learn);
STATIC_DCL boolean NDECL(rejectcasting);
STATIC_DCL boolean FDECL(getspell, (int *));
STATIC_PTR int FDECL(CFDECLSPEC spell_cmp, (const genericptr,
                                            const genericptr));
STATIC_DCL void NDECL(sortspells);
STATIC_DCL boolean NDECL(spellsortmenu);
STATIC_DCL boolean FDECL(dospellmenu, (const char *, int, int *));
STATIC_DCL int FDECL(percent_success, (int));
STATIC_DCL int FDECL(energy_cost, (int));
STATIC_DCL char *FDECL(spellretention, (int, char *));
STATIC_DCL int NDECL(throwspell);
STATIC_DCL void NDECL(cast_protection);
STATIC_DCL void FDECL(spell_backfire, (int));
STATIC_DCL boolean FDECL(spell_aim_step, (genericptr_t, int, int));

/* The roles[] table lists the role-specific values for tuning
 * percent_success().
 *
 * Reasoning:
 *   spelbase, spelheal:
 *      Arc are aware of magic through historical research
 *      Bar abhor magic (Conan finds it "interferes with his animal instincts")
 *      Cav are ignorant to magic
 *      Hea are very aware of healing magic through medical research
 *      Kni are moderately aware of healing from Paladin training
 *      Mon use magic to attack and defend in lieu of weapons and armor
 *      Pri are very aware of healing magic through theological research
 *      Ran avoid magic, preferring to fight unseen and unheard
 *      Rog are moderately aware of magic through trickery
 *      Sam have limited magical awareness, preferring meditation to conjuring
 *      Tou are aware of magic from all the great films they have seen
 *      Val have limited magical awareness, preferring fighting
 *      Wiz are trained mages
 *
 *      The arms penalty is lessened for trained fighters Bar, Kni, Ran,
 *      Sam, Val -- the penalty is its metal interference, not encumbrance.
 *      The `spelspec' is a single spell which is fundamentally easier
 *      for that role to cast.
 *
 */

/* since the spellbook itself doesn't blow up, don't say just "explodes" */
static const char explodes[] = "radiates explosive energy";

/* convert a letter into a number in the range 0..51, or -1 if not a letter */
STATIC_OVL int
spell_let_to_idx(ilet)
char ilet;
{
    int indx;

    indx = ilet - 'a';
    if (indx >= 0 && indx < 26)
        return indx;
    indx = ilet - 'A';
    if (indx >= 0 && indx < 26)
        return indx + 26;
    return -1;
}

/* TRUE: book should be destroyed by caller */
STATIC_OVL boolean
cursed_book(bp)
struct obj *bp;
{
    boolean was_in_use;
    int lev = objects[bp->otyp].oc_level;
    int dmg = 0;
    boolean already_cursed = bp->cursed;

    switch (rn2(lev)) {
    case 0:
        if (already_cursed || rn2(4)) {
            You_feel("a wrenching sensation.");
            tele(); /* teleport him */
        }
        else {
            if (bp->blessed) {
                unbless(bp);
                pline_The("book glows brown.");
            }
            else {
                /* can't call curse() here because it will call cursed_book */
                bp->cursed = 1;
                pline_The("book glows %s.", NH_BLACK);
            }
            bp->bknown = TRUE;
        }
        break;
    case 1:
        You_feel("threatened.");
        aggravate();
        break;
    case 2:
        make_blinded(Blinded + rn1(30, 10), TRUE);
        break;
    case 3:
        pline_The("book develops a huge set of teeth and bites you!");
        /* temp disable in_use; death should not destroy the book */
        was_in_use = bp->in_use;
        bp->in_use = FALSE;
        losehp(rn1(5, 3), "carnivorous book", KILLED_BY_AN);
        bp->in_use = was_in_use;
        break;
    case 4:
        make_paralyzed(rn1(16,16), TRUE, "frozen by a book");
        break;
    case 5:
        pline_The("book was coated with contact poison!");
        if (uarmg) {
            erode_obj(uarmg, "gloves", ERODE_CORRODE, EF_GREASE | EF_VERBOSE);
            break;
        }
        /* temp disable in_use; death should not destroy the book */
        was_in_use = bp->in_use;
        bp->in_use = FALSE;
        losestr(Poison_resistance ? rn1(2, 1) : rn1(4, 3));
        losehp(rnd(Poison_resistance ? 6 : 10), "contact-poisoned spellbook",
               KILLED_BY_AN);
        bp->in_use = was_in_use;
        break;
    case 6:
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
            pline_The("book %s, but you are unharmed!", explodes);
        } else {
            pline("As you read the book, it %s in your %s!", explodes,
                  body_part(FACE));
            dmg = 2 * rnd(10) + 5;
            losehp(Maybe_Half_Phys(dmg), "exploding rune", KILLED_BY_AN);
        }
        return TRUE;
    default:
        impossible("spellbook level out of bounds");
        rndcurse();
        break;
    }
    if (already_cursed) {
        pline_The("spellbook crumbles to dust!");
        return TRUE;
    }
    return FALSE;
}

/* study while confused: returns TRUE if the book is destroyed */
STATIC_OVL boolean
confused_book(spellbook)
struct obj *spellbook;
{
    if (!rn2(3) && spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
        spellbook->in_use = TRUE; /* in case called from learn */
        pline(
         "Being confused you have difficulties in controlling your actions.");
        display_nhwindow(WIN_MESSAGE, FALSE);
        You("accidentally tear the spellbook to pieces.");
        if (!objects[spellbook->otyp].oc_name_known
            && !objects[spellbook->otyp].oc_uname)
            docall(spellbook);
        useup(spellbook);
        return TRUE;
    } else {
        You("find yourself reading the %s line over and over again.",
            spellbook == context.spbook.book ? "next" : "first");
    }
    return FALSE;
}

/* special effects for The Book of the Dead */
STATIC_OVL void
deadbook(book2)
struct obj *book2;
{
    struct monst *mtmp, *mtmp2;
    coord mm;

    You("turn the pages of the Book of the Dead...");
    makeknown(SPE_BOOK_OF_THE_DEAD);
    /* KMH -- Need ->known to avoid "_a_ Book of the Dead" */
    book2->known = 1;
    if (invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
        register struct obj *otmp;
        register boolean arti1_primed = FALSE, arti2_primed = FALSE,
                         arti_cursed = FALSE;

        if (book2->cursed) {
            pline_The("runes appear scrambled.  You can't read them!");
            return;
        }

        if (!u.uhave.bell || !u.uhave.menorah) {
            pline("A chill runs down your %s.", body_part(SPINE));
            if (!u.uhave.bell)
                You_hear("a faint chime...");
            if (!u.uhave.menorah)
                pline("Vlad's doppelganger is amused.");
            return;
        }

        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (otmp->otyp == CANDELABRUM_OF_INVOCATION && otmp->spe == 7
                && otmp->lamplit) {
                if (!otmp->cursed)
                    arti1_primed = TRUE;
                else
                    arti_cursed = TRUE;
            }
            if (otmp->otyp == BELL_OF_OPENING
                && (moves - otmp->age) < 5L) { /* you rang it recently */
                if (!otmp->cursed)
                    arti2_primed = TRUE;
                else
                    arti_cursed = TRUE;
            }
        }

        if (arti_cursed) {
            pline_The("invocation fails!");
            pline("At least one of your artifacts is cursed...");
        } else if (arti1_primed && arti2_primed) {
            unsigned soon =
                (unsigned) d(2, 6); /* time til next intervene() */

            /* successful invocation */
            mkinvokearea();
            u.uevent.invoked = 1;
            /* in case you haven't killed the Wizard yet, behave as if
               you just did */
            u.uevent.udemigod = 1; /* wizdead() */
            if (!u.udg_cnt || u.udg_cnt > soon)
                u.udg_cnt = soon;
        } else { /* at least one artifact not prepared properly */
            You("have a feeling that %s is amiss...", something);
            goto raise_dead;
        }
        return;
    }

    /* when not an invocation situation */
    if (book2->cursed) {
    raise_dead:

        You("raised the dead!");
        /* first maybe place a dangerous adversary */
        if (!rn2(3) && ((mtmp = makemon(&mons[PM_MASTER_LICH], u.ux, u.uy,
                                        NO_MINVENT)) != 0
                        || (mtmp = makemon(&mons[PM_NALFESHNEE], u.ux, u.uy,
                                           NO_MINVENT)) != 0)) {
            mtmp->mpeaceful = 0;
            set_malign(mtmp);
        }
        /* next handle the affect on things you're carrying */
        (void) unturn_dead(&youmonst);
        /* last place some monsters around you */
        mm.x = u.ux;
        mm.y = u.uy;
        mkundead(&mm, TRUE, NO_MINVENT);
    } else if (book2->blessed) {
        for (mtmp = fmon; mtmp; mtmp = mtmp2) {
            mtmp2 = mtmp->nmon; /* tamedog() changes chain */
            if (DEADMONSTER(mtmp))
                continue;

            if ((is_undead(mtmp->data) || is_vampshifter(mtmp))
                && cansee(mtmp->mx, mtmp->my)) {
                mtmp->mpeaceful = TRUE;
                if (sgn(mtmp->data->maligntyp) == sgn(u.ualign.type)
                    && distu(mtmp->mx, mtmp->my) < 4)
                    if (mtmp->mtame) {
                        if (mtmp->mtame < 20)
                            mtmp->mtame++;
                    } else
                        (void) tamedog(mtmp, (struct obj *) 0, FALSE);
                else
                    monflee(mtmp, 0, FALSE, TRUE);
            }
        }
    } else {
        switch (rn2(3)) {
        case 0:
            Your("ancestors are annoyed with you!");
            break;
        case 1:
            pline_The("headstones in the cemetery begin to move!");
            break;
        default:
            pline("Oh my!  Your name appears in the book!");
        }
    }
    return;
}

/* 'book' has just become cursed; if we're reading it and realize it is
   now cursed, interrupt */
void
book_cursed(book)
struct obj *book;
{
    if (occupation == learn && context.spbook.book == book
        && book->cursed && book->bknown && multi >= 0)
        stop_occupation();
}

STATIC_PTR int
learn(VOID_ARGS)
{
    int i;
    short booktype;
    char splname[BUFSZ];
    boolean costly = TRUE;
    struct obj *book = context.spbook.book;

    /* JDS: lenses give 50% faster reading; 33% smaller read time */
    if (context.spbook.delay && ublindf && ublindf->otyp == LENSES && rn2(2))
        context.spbook.delay++;
    if (Confusion) { /* became confused while learning */
        if (!confused_book(book)) {
            You("can no longer focus on the book.");
        }
        context.spbook.book = 0; /* no longer studying */
        context.spbook.o_id = 0;
        context.spbook.delay = 0;
        return 0;
    }
    if (context.spbook.delay) {
        /* not if (context.spbook.delay++), so at end delay == 0 */
        context.spbook.delay++;
        return 1; /* still busy */
    }
    exercise(A_WIS, TRUE); /* you're studying. */
    booktype = book->otyp;
    if (booktype == SPE_BOOK_OF_THE_DEAD) {
        deadbook(book);
        return 0;
    }

    Sprintf(splname,
            objects[booktype].oc_name_known ? "\"%s\"" : "the \"%s\" spell",
            OBJ_NAME(objects[booktype]));
    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == booktype || spellid(i) == NO_SPELL)
            break;

    if (i == MAXSPELL) {
        impossible("Too many spells memorized!");
    } else if (spellid(i) == booktype) {
        /* normal book can be read and re-read a total of 4 times */
        if (book->spestudied > MAX_SPELL_STUDY) {
            pline("This spellbook is too faint to be read any more.");
            book->otyp = booktype = SPE_BLANK_PAPER;
            /* reset spestudied as if polymorph had taken place */
            book->spestudied = rn2(book->spestudied);
        } else if (spellknow(i) > KEEN / 10) {
            You("know %s quite well already.", splname);
            costly = FALSE;
        } else { /* spellknow(i) <= KEEN/10 */
            Your("knowledge of %s is %s.", splname,
                 spellknow(i) ? "keener" : "restored");
            incrnknow(i, 1);
            book->spestudied++;
            exercise(A_WIS, TRUE); /* extra study */
        }
        /* make book become known even when spell is already
           known, in case amnesia made you forget the book */
        makeknown((int) booktype);
    } else { /* (spellid(i) == NO_SPELL) */
        /* for a normal book, spestudied will be zero, but for
           a polymorphed one, spestudied will be non-zero and
           one less reading is available than when re-learning */
        if (book->spestudied >= MAX_SPELL_STUDY) {
            /* pre-used due to being the product of polymorph */
            pline("This spellbook is too faint to read even once.");
            book->otyp = booktype = SPE_BLANK_PAPER;
            /* reset spestudied as if polymorph had taken place */
            book->spestudied = rn2(book->spestudied);
        } else {
            spl_book[i].sp_id = booktype;
            spl_book[i].sp_lev = objects[booktype].oc_level;
            incrnknow(i, 1);
            book->spestudied++;
            You(i > 0 ? "add %s to your repertoire." : "learn %s.", splname);
        }
        makeknown((int) booktype);
    }

    if (book->cursed) { /* maybe a demon cursed it */
        if (cursed_book(book)) {
            useup(book);
            context.spbook.book = 0;
            context.spbook.o_id = 0;
            return 0;
        }
    }
    if (costly)
        check_unpaid(book);
    context.spbook.book = 0;
    context.spbook.o_id = 0;
    return 0;
}

int
study_book(spellbook)
register struct obj *spellbook;
{
    int booktype = spellbook->otyp;
    boolean confused = (Confusion != 0);
    boolean too_hard = FALSE;

    /* attempting to read dull book may make hero fall asleep */
    if (!confused && !Sleep_resistance && objdescr_is(spellbook, "dull")) {
        const char *eyes;
        int dullbook = rnd(25) - ACURR(A_WIS);

        /* adjust chance if hero stayed awake, got interrupted, retries */
        if (context.spbook.delay && spellbook == context.spbook.book)
            dullbook -= rnd(objects[booktype].oc_level);

        if (dullbook > 0) {
            eyes = body_part(EYE);
            if (eyecount(youmonst.data) > 1)
                eyes = makeplural(eyes);
            pline("This book is so dull that you can't keep your %s open.",
                  eyes);
            dullbook += rnd(2 * objects[booktype].oc_level);
            fall_asleep(-dullbook, TRUE);
            return 1;
        }
    }

    if (context.spbook.delay && !confused && spellbook == context.spbook.book
        /* handle the sequence: start reading, get interrupted, have
           context.spbook.book become erased somehow, resume reading it */
        && booktype != SPE_BLANK_PAPER) {
        You("continue your efforts to %s.",
            (booktype == SPE_NOVEL) ? "read the novel" : "memorize the spell");
    } else {
        /* KMH -- Simplified this code */
        if (booktype == SPE_BLANK_PAPER) {
            pline("This spellbook is all blank.");
            makeknown(booktype);
            return 1;
        }

        /* 3.6 tribute */
        if (booktype == SPE_NOVEL) {
            /* Obtain current Terry Pratchett book title */
            const char *tribtitle = noveltitle(&spellbook->novelidx);

            if (read_tribute("books", tribtitle, 0, (char *) 0, 0,
                             spellbook->o_id)) {
                if(!u.uconduct.literate++)
                    livelog_printf(LL_CONDUCT,
                            "became literate by reading %s", tribtitle);
                check_unpaid(spellbook);
                makeknown(booktype);
                if (!u.uevent.read_tribute) {
                    /* give bonus of 20 xp and 4*20+0 pts */
                    more_experienced(20, 0);
                    newexplevel();
                    u.uevent.read_tribute = 1; /* only once */
                }
            }
            return 1;
        }

        if (objects[booktype].oc_level >= 8) {
            impossible("Unknown spellbook level %d, book %d;",
                       objects[booktype].oc_level, booktype);
            return 0;
        }
        /* currently level * 10 */
        context.spbook.delay = -objects[booktype].oc_delay;

        /* Books are often wiser than their readers (Rus.) */
        spellbook->in_use = TRUE;
        if (spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
            /* uncursed - chance to fail */
            int read_ability = ACURR(A_INT) + 4 + u.ulevel / 2
                                - 2 * objects[booktype].oc_level
                                + (spellbook->blessed ? 10 : 0)
                                - (spellbook->cursed ? 10 : 0)
                            + ((ublindf && ublindf->otyp == LENSES) ? 2 : 0);

            /* only wizards know if a spell is too difficult */
            if (Role_if(PM_WIZARD) && read_ability < 20 && !confused) {
                char qbuf[QBUFSZ];

                Sprintf(qbuf,
                "This spellbook is %sdifficult to comprehend.  Continue?",
                        (read_ability < 12 ? "very " : ""));
                if (yn(qbuf) != 'y') {
                    spellbook->in_use = FALSE;
                    return 1;
                }
            }
            /* its up to random luck now */
            if (rnd(20) > read_ability) {
                too_hard = TRUE;
            }
        }

        if (too_hard) {
            You("can't comprehend the runes!");
            make_confused(-context.spbook.delay, FALSE); /* study time */
            bot(); /* show Conf on status line before tele, crumbling, etc */

            boolean gone = cursed_book(spellbook);
            if (gone) {
                if (!objects[spellbook->otyp].oc_name_known
                    && !objects[spellbook->otyp].oc_uname)
                    docall(spellbook);
                useup(spellbook);
            } else
                spellbook->in_use = FALSE;
            return 1;
        } else if (confused) {
            if (!confused_book(spellbook)) {
                spellbook->in_use = FALSE;
            }
            return 1;
        }
        spellbook->in_use = FALSE;

        You("begin to %s the runes.",
            spellbook->otyp == SPE_BOOK_OF_THE_DEAD ? "recite" : "memorize");
    }

    context.spbook.book = spellbook;
    if (context.spbook.book)
        context.spbook.o_id = context.spbook.book->o_id;
    set_occupation(learn, "studying", 0);
    return 1;
}

/* a spellbook has been destroyed or the character has changed levels;
   the stored address for the current book is no longer valid */
void
book_disappears(obj)
struct obj *obj;
{
    if (obj == context.spbook.book) {
        context.spbook.book = (struct obj *) 0;
        context.spbook.o_id = 0;
    }
}

/* renaming an object usually results in it having a different address;
   so the sequence start reading, get interrupted, name the book, resume
   reading would read the "new" book from scratch */
void
book_substitution(old_obj, new_obj)
struct obj *old_obj, *new_obj;
{
    if (old_obj == context.spbook.book) {
        context.spbook.book = new_obj;
        if (context.spbook.book)
            context.spbook.o_id = context.spbook.book->o_id;
    }
}

/* called from moveloop() */
void
age_spells()
{
    int i;
    /*
     * The time relative to the hero (a pass through move
     * loop) causes all spell knowledge to be decremented.
     * The hero's speed, rest status, conscious status etc.
     * does not alter the loss of memory.
     */
    for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++)
        if (spellknow(i))
            decrnknow(i);
    return;
}

/* return True if spellcasting is inhibited;
   only covers a small subset of reasons why casting won't work */
STATIC_OVL boolean
rejectcasting()
{
    /* rejections which take place before selecting a particular spell */
    if (Stunned) {
        You("are too impaired to cast a spell.");
        return TRUE;
    } else if (!can_chant(&youmonst)) {
        You("are unable to chant the incantation.");
        return TRUE;
    } else if (!freehand()) {
        /* Note: !freehand() occurs when weapon and shield (or two-handed
         * weapon) are welded to hands, so "arms" probably doesn't need
         * to be makeplural(bodypart(ARM)).
         *
         * But why isn't lack of free arms (for gesturing) an issue when
         * poly'd hero has no limbs?
         */
        Your("arms are not free to cast!");
        return TRUE;
    }
    return FALSE;
}

/*
 * Return TRUE if a spell was picked, with the spell index in the return
 * parameter.  Otherwise return FALSE.
 */
STATIC_OVL boolean
getspell(spell_no)
int *spell_no;
{
    int nspells, idx;
    char ilet, lets[BUFSZ], qbuf[QBUFSZ];

    if (spellid(0) == NO_SPELL) {
        You("don't know any spells right now.");
        return FALSE;
    }
    if (rejectcasting())
        return FALSE; /* no spell chosen */

    if (flags.menu_style == MENU_TRADITIONAL) {
        /* we know there is at least 1 known spell */
        for (nspells = 1; nspells < MAXSPELL && spellid(nspells) != NO_SPELL;
             nspells++)
            continue;

        if (nspells == 1)
            Strcpy(lets, "a");
        else if (nspells < 27)
            Sprintf(lets, "a-%c", 'a' + nspells - 1);
        else if (nspells == 27)
            Sprintf(lets, "a-zA");
        /* this assumes that there are at most 52 spells... */
        else
            Sprintf(lets, "a-zA-%c", 'A' + nspells - 27);

        for (;;) {
            Sprintf(qbuf, "Cast which spell? [%s *?]", lets);
            ilet = yn_function(qbuf, (char *) 0, '\0');
            if (ilet == '*' || ilet == '?')
                break; /* use menu mode */
            if (index(quitchars, ilet))
                return FALSE;

            idx = spell_let_to_idx(ilet);
            if (idx < 0 || idx >= nspells) {
                You("don't know that spell.");
                continue; /* ask again */
            }
            *spell_no = idx;
            return TRUE;
        }
    }
    return dospellmenu("Choose which spell to cast", SPELLMENU_CAST,
                       spell_no);
}

/* the 'Z' command -- cast a spell */
int
docast()
{
    int spell_no;

    if (getspell(&spell_no))
        return spelleffects(spell_no, FALSE);
    return 0;
}

const char *
spelltypemnemonic(skill)
int skill;
{
    switch (skill) {
    case P_ATTACK_SPELL:
        return "attack";
    case P_HEALING_SPELL:
        return "healing";
    case P_DIVINATION_SPELL:
        return "divination";
    case P_ENCHANTMENT_SPELL:
        return "enchantment";
    case P_CLERIC_SPELL:
        return "clerical";
    case P_ESCAPE_SPELL:
        return "escape";
    case P_MATTER_SPELL:
        return "matter";
    default:
        impossible("Unknown spell skill, %d;", skill);
        return "";
    }
}

int
spell_skilltype(booktype)
int booktype;
{
    return objects[booktype].oc_skill;
}

STATIC_OVL void
cast_protection()
{
    int l = u.ulevel, loglev = 0,
        gain, natac = u.uac + u.uspellprot;
    /* note: u.uspellprot is subtracted when find_ac() factors it into u.uac,
       so adding here factors it back out
       (versions prior to 3.6 had this backwards) */

    /* loglev=log2(u.ulevel)+1 (1..5) */
    while (l) {
        loglev++;
        l /= 2;
    }

    /* The more u.uspellprot you already have, the less you get,
     * and the better your natural ac, the less you get.
     *
     *  LEVEL AC    SPELLPROT from successive SPE_PROTECTION casts
     *      1     10    0,  1,  2,  3,  4
     *      1      0    0,  1,  2,  3
     *      1    -10    0,  1,  2
     *      2-3   10    0,  2,  4,  5,  6,  7,  8
     *      2-3    0    0,  2,  4,  5,  6
     *      2-3  -10    0,  2,  3,  4
     *      4-7   10    0,  3,  6,  8,  9, 10, 11, 12
     *      4-7    0    0,  3,  5,  7,  8,  9
     *      4-7  -10    0,  3,  5,  6
     *      7-15 -10    0,  3,  5,  6
     *      8-15  10    0,  4,  7, 10, 12, 13, 14, 15, 16
     *      8-15   0    0,  4,  7,  9, 10, 11, 12
     *      8-15 -10    0,  4,  6,  7,  8
     *     16-30  10    0,  5,  9, 12, 14, 16, 17, 18, 19, 20
     *     16-30   0    0,  5,  9, 11, 13, 14, 15
     *     16-30 -10    0,  5,  8,  9, 10
     */
    natac = (10 - natac) / 10; /* convert to positive and scale down */
    gain = loglev - (int) u.uspellprot / (4 - min(3, natac));

    if (gain > 0) {
        if (!Blind) {
            int rmtyp;
            const char *hgolden = hcolor(NH_GOLDEN), *atmosphere;

            if (u.uspellprot) {
                pline_The("%s haze around you becomes more dense.", hgolden);
            } else {
                rmtyp = levl[u.ux][u.uy].typ;
                atmosphere = u.uswallow
                                ? ((u.ustuck->data == &mons[PM_FOG_CLOUD])
                                   ? "mist"
                                   : is_whirly(u.ustuck->data)
                                      ? "maelstrom"
                                      : is_animal(u.ustuck->data)
                                         ? "maw"
                                         : "ooze")
                                : (u.uinwater
                                   ? hliquid("water")
                                   : (rmtyp == CLOUD)
                                      ? "cloud"
                                      : IS_TREE(rmtyp)
                                         ? "vegetation"
                                         : IS_STWALL(rmtyp)
                                            ? "stone"
                                            : "air");
                pline_The("%s around you begins to shimmer with %s haze.",
                          atmosphere, an(hgolden));
            }
        }
        u.uspellprot += gain;
        u.uspmtime = (P_SKILL(spell_skilltype(SPE_PROTECTION)) == P_EXPERT)
                        ? 20 : 10;
        if (!u.usptime)
            u.usptime = u.uspmtime;
        find_ac();
    } else {
        Your("skin feels warm for a moment.");
    }
}

/* attempting to cast a forgotten spell will cause disorientation */
STATIC_OVL void
spell_backfire(spell)
int spell;
{
    long duration = (long) ((spellev(spell) + 1) * 3), /* 6..24 */
         old_stun = (HStun & TIMEOUT), old_conf = (HConfusion & TIMEOUT);

    /* Prior to 3.4.1, only effect was confusion; it still predominates.
     *
     * 3.6.0: this used to override pre-existing confusion duration
     * (cases 0..8) and pre-existing stun duration (cases 4..9);
     * increase them instead.   (Hero can no longer cast spells while
     * Stunned, so the potential increment to stun duration here is
     * just hypothetical.)
     */
    switch (rn2(10)) {
    case 0:
    case 1:
    case 2:
    case 3:
        make_confused(old_conf + duration, FALSE); /* 40% */
        break;
    case 4:
    case 5:
    case 6:
        make_confused(old_conf + 2L * duration / 3L, FALSE); /* 30% */
        make_stunned(old_stun + duration / 3L, FALSE);
        break;
    case 7:
    case 8:
        make_stunned(old_stun + 2L * duration / 3L, FALSE); /* 20% */
        make_confused(old_conf + duration / 3L, FALSE);
        break;
    case 9:
        make_stunned(old_stun + duration, FALSE); /* 10% */
        break;
    }
    return;
}

int
spelleffects(spell, atme)
int spell;
boolean atme;
{
    int energy, damage, n, intell;
    int otyp, skill, role_skill, res = 0;
    boolean confused = (Confusion != 0);
    boolean physical_damage = FALSE;
    struct obj *pseudo;
    coord cc;

    /*
     * Reject attempting to cast while stunned or with no free hands.
     * Already done in getspell() to stop casting before choosing
     * which spell, but duplicated here for cases where spelleffects()
     * gets called directly for ^T without intrinsic teleport capability
     * or #turn for non-priest/non-knight.
     * (There's no duplication of messages; when the rejection takes
     * place in getspell(), we don't get called.)
     */
    if (rejectcasting()) {
        return 0; /* no time elapses */
    }

    /*
     * Spell casting no longer affects knowledge of the spell. A
     * decrement of spell knowledge is done every turn.
     */
    if (spellknow(spell) <= 0) {
        Your("knowledge of this spell is twisted.");
        pline("It invokes nightmarish images in your mind...");
        spell_backfire(spell);
        return 1;
    } else if (spellknow(spell) <= KEEN / 200) { /* 100 turns left */
        You("strain to recall the spell.");
    } else if (spellknow(spell) <= KEEN / 40) { /* 500 turns left */
        You("have difficulty remembering the spell.");
    } else if (spellknow(spell) <= KEEN / 20) { /* 1000 turns left */
        Your("knowledge of this spell is growing faint.");
    } else if (spellknow(spell) <= KEEN / 10) { /* 2000 turns left */
        Your("recall of this spell is gradually fading.");
    }

    if (u.uhunger <= 10 && spellid(spell) != SPE_DETECT_FOOD) {
        You("are too hungry to cast that spell.");
        return 0;
    } else if (ACURR(A_STR) < 4 && spellid(spell) != SPE_RESTORE_ABILITY) {
        You("lack the strength to cast spells.");
        return 0;
    } else if (check_capacity(
                "Your concentration falters while carrying so much stuff.")) {
        return 1;
    }

    energy = energy_cost(spell);
    /* if spell is impossible to cast, kludge this to u.uen + 1 to make it fail
     * the checks below as not having enough energy. */
    if (energy < 0) {
        energy = u.uen + 1;
    }

    /* if the cast attempt is already going to fail due to insufficient
       energy (ie, u.uen < energy), the Amulet's drain effect won't kick
       in and no turn will be consumed; however, when it does kick in,
       the attempt may fail due to lack of energy after the draining, in
       which case a turn will be used up in addition to the energy loss */
    if (u.uhave.amulet && u.uen >= energy) {
        You_feel("the amulet draining your energy away.");
        /* this used to be 'energy += rnd(2 * energy)' (without 'res'),
           so if amulet-induced cost was more than u.uen, nothing
           (except the "don't have enough energy" message) happened
           and player could just try again (and again and again...);
           now we drain some energy immediately, which has a
           side-effect of not increasing the hunger aspect of casting */
        u.uen -= rnd(2 * energy);
        if (u.uen < 0)
            u.uen = 0;
        context.botl = 1;
        res = 1; /* time is going to elapse even if spell doesn't get cast */
    }

    if (energy > u.uen) {
        You("don't have enough energy to cast that spell.");
        return res;
    } else {
        if (spellid(spell) != SPE_DETECT_FOOD) {
            int hungr = energy * 2;

            /* If hero is a wizard, their current intelligence
             * (bonuses + temporary + current)
             * affects hunger reduction in casting a spell.
             * 1. int = 17-18 no reduction
             * 2. int = 16    1/4 hungr
             * 3. int = 15    1/2 hungr
             * 4. int = 1-14  normal reduction
             * The reason for this is:
             * a) Intelligence affects the amount of exertion
             * in thinking.
             * b) Wizards have spent their life at magic and
             * understand quite well how to cast spells.
             */
            intell = acurr(A_INT);
            if (!Role_if(PM_WIZARD))
                intell = 10;
            switch (intell) {
            case 25:
            case 24:
            case 23:
            case 22:
            case 21:
            case 20:
            case 19:
            case 18:
            case 17:
                hungr = 0;
                break;
            case 16:
                hungr /= 4;
                break;
            case 15:
                hungr /= 2;
                break;
            }
            /* don't put player (quite) into fainting from
             * casting a spell, particularly since they might
             * not even be hungry at the beginning; however,
             * this is low enough that they must eat before
             * casting anything else except detect food
             */
            if (hungr > u.uhunger - 3)
                hungr = u.uhunger - 3;
            morehungry(hungr);
        }
    }

    /* confused casting always fails, and is assessed after hunger penalty */
    if (confused) {
        You("fail to cast the spell correctly.");
        u.uen -= energy / 2;
        context.botl = 1;
        return 1;
    }

    u.uen -= energy;
    context.botl = 1;
    exercise(A_WIS, TRUE);
    /* pseudo is a temporary "false" object containing the spell stats */
    pseudo = mksobj(spellid(spell), FALSE, FALSE);
    pseudo->blessed = pseudo->cursed = 0;
    pseudo->quan = 20L; /* do not let useup get it */
    /*
     * Find the skill the hero has in a spell type category.
     * See spell_skilltype for categories.
     */
    otyp = pseudo->otyp;
    skill = spell_skilltype(otyp);
    role_skill = P_SKILL(skill);

    switch (otyp) {
    /*
     * At first spells act as expected.  As the hero increases in skill
     * with the appropriate spell type, some spells increase in their
     * effects, e.g. more damage, further distance, and so on, without
     * additional cost to the spellcaster.
     */
    case SPE_FIREBALL:
    case SPE_CONE_OF_COLD:
        if (role_skill >= P_SKILLED) {
            if (throwspell()) {
                cc.x = u.dx;
                cc.y = u.dy;
                n = rnd(8) + 1;
                while (n--) {
                    if (!u.dx && !u.dy && !u.dz) {
                        if ((damage = zapyourself(pseudo, TRUE)) != 0) {
                            char buf[BUFSZ];
                            Sprintf(buf, "zapped %sself with a spell",
                                    uhim());
                            losehp(damage, buf, NO_KILLER_PREFIX);
                        }
                    } else {
                        explode(u.dx, u.dy,
                                otyp - SPE_MAGIC_MISSILE + 10,
                                spell_damage_bonus(u.ulevel / 2 + 1), 0,
                                (otyp == SPE_CONE_OF_COLD)
                                   ? EXPL_FROSTY
                                   : EXPL_FIERY);
                    }
                    u.dx = cc.x + rnd(3) - 2;
                    u.dy = cc.y + rnd(3) - 2;
                    if (!isok(u.dx, u.dy) || !cansee(u.dx, u.dy)
                        || IS_STWALL(levl[u.dx][u.dy].typ) || u.uswallow) {
                        /* Spell is reflected back to center */
                        u.dx = cc.x;
                        u.dy = cc.y;
                    }
                }
            }
            break;
        } /* else */
        /*FALLTHRU*/

    /* these spells are all duplicates of wand effects */
    case SPE_FORCE_BOLT:
        physical_damage = TRUE;
    /*FALLTHRU*/
    case SPE_SLEEP:
    case SPE_MAGIC_MISSILE:
    case SPE_KNOCK:
    case SPE_SLOW_MONSTER:
    case SPE_WIZARD_LOCK:
    case SPE_DIG:
    case SPE_TURN_UNDEAD:
    case SPE_POLYMORPH:
    case SPE_TELEPORT_AWAY:
    case SPE_CANCELLATION:
    case SPE_FINGER_OF_DEATH:
    case SPE_LIGHT:
    case SPE_DETECT_UNSEEN:
    case SPE_HEALING:
    case SPE_EXTRA_HEALING:
    case SPE_DRAIN_LIFE:
    case SPE_STONE_TO_FLESH:
        if (objects[otyp].oc_dir != NODIR) {
            if (otyp == SPE_HEALING || otyp == SPE_EXTRA_HEALING) {
                /* healing and extra healing are actually potion effects,
                   but they've been extended to take a direction like wands */
                if (role_skill >= P_SKILLED)
                    pseudo->blessed = 1;
            }
            if (atme) {
                u.dx = u.dy = u.dz = 0;
            } else if (!getdir((char *) 0)) {
                /* getdir cancelled, re-use previous direction */
                /*
                 * FIXME:  reusing previous direction only makes sense
                 * if there is an actual previous direction.  When there
                 * isn't one, the spell gets cast at self which is rarely
                 * what the player intended.  Unfortunately, the way
                 * spelleffects() is organized means that aborting with
                 * "nevermind" is not an option.
                 */
                pline_The("magical energy is released!");
            }
            if (!u.dx && !u.dy && !u.dz) {
                if ((damage = zapyourself(pseudo, TRUE)) != 0) {
                    char buf[BUFSZ];

                    Sprintf(buf, "zapped %sself with a spell", uhim());
                    if (physical_damage)
                        damage = Maybe_Half_Phys(damage);
                    losehp(damage, buf, NO_KILLER_PREFIX);
                }
            } else
                weffects(pseudo);
        } else
            weffects(pseudo);
        update_inventory(); /* spell may modify inventory */
        break;

    /* these are all duplicates of scroll effects */
    case SPE_REMOVE_CURSE:
    case SPE_CONFUSE_MONSTER:
    case SPE_DETECT_FOOD:
    case SPE_CAUSE_FEAR:
        /* high skill yields effect equivalent to blessed scroll */
        if (role_skill >= P_SKILLED)
            pseudo->blessed = 1;
    /*FALLTHRU*/
    case SPE_CHARM_MONSTER:
    case SPE_MAGIC_MAPPING:
    case SPE_CREATE_MONSTER:
        (void) seffects(pseudo);
        break;

    /* these are all duplicates of potion effects */
    case SPE_HASTE_SELF:
    case SPE_DETECT_TREASURE:
    case SPE_DETECT_MONSTERS:
    case SPE_LEVITATION:
    case SPE_RESTORE_ABILITY:
        /* high skill yields effect equivalent to blessed potion */
        if (role_skill >= P_SKILLED)
            pseudo->blessed = 1;
    /*FALLTHRU*/
    case SPE_INVISIBILITY:
        (void) peffects(pseudo);
        break;
    /* end of potion-like spells */

    case SPE_CURE_BLINDNESS:
        healup(0, 0, FALSE, TRUE);
        break;
    case SPE_CURE_SICKNESS:
        if (Sick)
            You("are no longer ill.");
        if (Slimed)
            make_slimed(0L, "The slime disappears!");
        healup(0, 0, TRUE, FALSE);
        break;
    case SPE_CREATE_FAMILIAR:
        (void) make_familiar((struct obj *) 0, u.ux, u.uy, FALSE);
        break;
    case SPE_CLAIRVOYANCE:
        if (!BClairvoyant) {
            if (role_skill >= P_SKILLED)
                pseudo->blessed = 1; /* detect monsters as well as map */
            do_vicinity_map(pseudo);
        /* at present, only one thing blocks clairvoyance */
        } else if (uarmh && uarmh->otyp == CORNUTHAUM)
            You("sense a pointy hat on top of your %s.", body_part(HEAD));
        break;
    case SPE_PROTECTION:
        cast_protection();
        break;
    case SPE_JUMPING:
        if (!jump(max(role_skill, 1)))
            pline1(nothing_happens);
        break;
    default:
        impossible("Unknown spell %d attempted.", spell);
        obfree(pseudo, (struct obj *) 0);
        return 0;
    }

    /* gain skill for successful cast */
    use_skill(skill, spellev(spell));

    obfree(pseudo, (struct obj *) 0); /* now, get rid of it */
    return 1;
}

/*ARGSUSED*/
STATIC_OVL boolean
spell_aim_step(arg, x, y)
genericptr_t arg UNUSED;
int x, y;
{
    if (!isok(x,y))
        return FALSE;
    if (!ZAP_POS(levl[x][y].typ)
        && !(IS_DOOR(levl[x][y].typ) && doorstate(&levl[x][y]) == D_ISOPEN))
        return FALSE;
    return TRUE;
}

/* Choose location where spell takes effect. */
STATIC_OVL int
throwspell()
{
    coord cc, uc;
    struct monst *mtmp;

    if (u.uinwater) {
        pline("You're joking! In this weather?");
        return 0;
    } else if (Is_waterlevel(&u.uz)) {
        You("had better wait for the sun to come out.");
        return 0;
    }

    pline("Where do you want to cast the spell?");
    cc.x = u.ux;
    cc.y = u.uy;
    if (getpos(&cc, TRUE, "the desired position") < 0)
        return 0; /* user pressed ESC */
    /* The number of moves from hero to where the spell drops.*/
    if (distmin(u.ux, u.uy, cc.x, cc.y) > 10) {
        pline_The("spell dissipates over the distance!");
        return 0;
    } else if (u.uswallow) {
        pline_The("spell is cut short!");
        exercise(A_WIS, FALSE); /* What were you THINKING! */
        u.dx = 0;
        u.dy = 0;
        return 1;
    } else if ((!cansee(cc.x, cc.y)
                && (!(mtmp = m_at(cc.x, cc.y)) || !canspotmon(mtmp)))
               || IS_STWALL(levl[cc.x][cc.y].typ)) {
        Your("mind fails to lock onto that location!");
        return 0;
    }

    uc.x = u.ux;
    uc.y = u.uy;

    walk_path(&uc, &cc, spell_aim_step, (genericptr_t) 0);

    u.dx = cc.x;
    u.dy = cc.y;
    return 1;
}


/*
 * Allow player to sort the list of known spells.  Manually swapping
 * pairs of them becomes very tedious once the list reaches two pages.
 *
 * Possible extensions:
 *      provide means for player to control ordering of skill classes;
 *      provide means to supply value N such that first N entries stick
 *      while rest of list is being sorted;
 *      make chosen sort order be persistent such that when new spells
 *      are learned, they get inserted into sorted order rather than be
 *      appended to the end of the list?
 */
enum spl_sort_types {
    SORTBY_LETTER = 0,
    SORTBY_ALPHA,
    SORTBY_LVL_LO,
    SORTBY_LVL_HI,
    SORTBY_SKL_AL,
    SORTBY_SKL_LO,
    SORTBY_SKL_HI,
    SORTBY_CURRENT,
    SORTRETAINORDER,

    NUM_SPELL_SORTBY
};

static const char *spl_sortchoices[NUM_SPELL_SORTBY] = {
    "by casting letter",
    "alphabetically",
    "by level, low to high",
    "by level, high to low",
    "by skill group, alphabetized within each group",
    "by skill group, low to high level within group",
    "by skill group, high to low level within group",
    "maintain current ordering",
    /* a menu choice rather than a sort choice */
    "reassign casting letters to retain current order",
};
static int spl_sortmode = 0;   /* index into spl_sortchoices[] */
static int *spl_orderindx = 0; /* array of spl_book[] indices */

/* qsort callback routine */
STATIC_PTR int CFDECLSPEC
spell_cmp(vptr1, vptr2)
const genericptr vptr1;
const genericptr vptr2;
{
    /*
     * gather up all of the possible parameters except spell name
     * in advance, even though some might not be needed:
     *  indx. = spl_orderindx[] index into spl_book[];
     *  otyp. = spl_book[] index into objects[];
     *  levl. = spell level;
     *  skil. = skill group aka spell class.
     */
    int indx1 = *(int *) vptr1, indx2 = *(int *) vptr2,
        otyp1 = spl_book[indx1].sp_id, otyp2 = spl_book[indx2].sp_id,
        levl1 = objects[otyp1].oc_level, levl2 = objects[otyp2].oc_level,
        skil1 = objects[otyp1].oc_skill, skil2 = objects[otyp2].oc_skill;

    switch (spl_sortmode) {
    case SORTBY_LETTER:
        return indx1 - indx2;
    case SORTBY_ALPHA:
        break;
    case SORTBY_LVL_LO:
        if (levl1 != levl2)
            return levl1 - levl2;
        break;
    case SORTBY_LVL_HI:
        if (levl1 != levl2)
            return levl2 - levl1;
        break;
    case SORTBY_SKL_AL:
        if (skil1 != skil2)
            return skil1 - skil2;
        break;
    case SORTBY_SKL_LO:
        if (skil1 != skil2)
            return skil1 - skil2;
        if (levl1 != levl2)
            return levl1 - levl2;
        break;
    case SORTBY_SKL_HI:
        if (skil1 != skil2)
            return skil1 - skil2;
        if (levl1 != levl2)
            return levl2 - levl1;
        break;
    case SORTBY_CURRENT:
    default:
        return (vptr1 < vptr2) ? -1
                               : (vptr1 > vptr2); /* keep current order */
    }
    /* tie-breaker for most sorts--alphabetical by spell name */
    return strcmpi(OBJ_NAME(objects[otyp1]), OBJ_NAME(objects[otyp2]));
}

/* sort the index used for display order of the "view known spells"
   list (sortmode == SORTBY_xxx), or sort the spellbook itself to make
   the current display order stick (sortmode == SORTRETAINORDER) */
STATIC_OVL void
sortspells()
{
    int i;
#if defined(SYSV) || defined(DGUX)
    unsigned n;
#else
    int n;
#endif

    if (spl_sortmode == SORTBY_CURRENT)
        return;
    for (n = 0; n < MAXSPELL && spellid(n) != NO_SPELL; ++n)
        continue;
    if (n < 2)
        return; /* not enough entries to need sorting */

    if (!spl_orderindx) {
        /* we haven't done any sorting yet; list is in casting order */
        if (spl_sortmode == SORTBY_LETTER /* default */
            || spl_sortmode == SORTRETAINORDER)
            return;
        /* allocate enough for full spellbook rather than just N spells */
        spl_orderindx = (int *) alloc(MAXSPELL * sizeof(int));
        for (i = 0; i < MAXSPELL; i++)
            spl_orderindx[i] = i;
    }

    if (spl_sortmode == SORTRETAINORDER) {
        struct spell tmp_book[MAXSPELL];

        /* sort spl_book[] rather than spl_orderindx[];
           this also updates the index to reflect the new ordering (we
           could just free it since that ordering becomes the default) */
        for (i = 0; i < MAXSPELL; i++)
            tmp_book[i] = spl_book[spl_orderindx[i]];
        for (i = 0; i < MAXSPELL; i++)
            spl_book[i] = tmp_book[i], spl_orderindx[i] = i;
        spl_sortmode = SORTBY_LETTER; /* reset */
        return;
    }

    /* usual case, sort the index rather than the spells themselves */
    qsort((genericptr_t) spl_orderindx, n, sizeof *spl_orderindx, spell_cmp);
    return;
}

/* called if the [sort spells] entry in the view spells menu gets chosen */
STATIC_OVL boolean
spellsortmenu()
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    char let;
    int i, n, choice;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    any = zeroany; /* zero out all bits */

    for (i = 0; i < SIZE(spl_sortchoices); i++) {
        if (i == SORTRETAINORDER) {
            let = 'z'; /* assumes fewer than 26 sort choices... */
            /* separate final choice from others with a blank line */
            any.a_int = 0;
            add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "",
                     MENU_UNSELECTED);
        } else {
            let = 'a' + i;
        }
        any.a_int = i + 1;
        add_menu(tmpwin, NO_GLYPH, &any, let, 0, ATR_NONE, spl_sortchoices[i],
                 (i == spl_sortmode) ? MENU_SELECTED : MENU_UNSELECTED);
    }
    end_menu(tmpwin, "View known spells list sorted");

    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        choice = selected[0].item.a_int - 1;
        /* skip preselected entry if we have more than one item chosen */
        if (n > 1 && choice == spl_sortmode)
            choice = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        spl_sortmode = choice;
        return TRUE;
    }
    return FALSE;
}

/* the '+' command -- view known spells */
int
dovspell()
{
    char qbuf[QBUFSZ];
    int splnum, othnum;
    struct spell spl_tmp;

    if (spellid(0) == NO_SPELL) {
        You("don't know any spells right now.");
    } else {
        while (dospellmenu("Currently known spells",
                           SPELLMENU_VIEW, &splnum)) {
            if (splnum == SPELLMENU_SORT) {
                if (spellsortmenu())
                    sortspells();
            } else {
                Sprintf(qbuf, "Reordering spells; swap '%c' with",
                        spellet(splnum));
                if (!dospellmenu(qbuf, splnum, &othnum))
                    break;

                spl_tmp = spl_book[splnum];
                spl_book[splnum] = spl_book[othnum];
                spl_book[othnum] = spl_tmp;
            }
        }
    }
    if (spl_orderindx) {
        free((genericptr_t) spl_orderindx);
        spl_orderindx = 0;
    }
    spl_sortmode = SORTBY_LETTER; /* 0 */
    return 0;
}

STATIC_OVL boolean
dospellmenu(prompt, splaction, spell_no)
const char *prompt;
int splaction; /* SPELLMENU_CAST, SPELLMENU_VIEW, or spl_book[] index */
int *spell_no;
{
    winid tmpwin;
    int i, n, how, splnum;
    char buf[BUFSZ], retentionbuf[24], pw_buf[5];
    const char *fmt;
    menu_item *selected;
    anything any;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    any = zeroany; /* zero out all bits */

    /*
     * The correct spacing of the columns when not using
     * tab separation depends on the following:
     * (1) that the font is monospaced, and
     * (2) that selection letters are pre-pended to the
     * given string and are of the form "a - ".
     */
    if (!iflags.menu_tab_sep) {
        Sprintf(buf, "%-20s     Level %-12s   Pw Retention", "    Name",
                "Category");
        fmt = "%-20s  %2d   %-12s %4s %9s";
    } else {
        Sprintf(buf, "Name\tLevel\tCategory\tPw\tRetention");
        fmt = "%s\t%-d\t%s\t%s\t%s";
    }
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings, buf,
             MENU_UNSELECTED);
    for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
        splnum = !spl_orderindx ? i : spl_orderindx[i];
        if (energy_cost(splnum) < 0) {
            strcpy(pw_buf, "Inf");
        } else {
            /* maximum possible should be 3500 */
            Sprintf(pw_buf, "%d", energy_cost(splnum));
        }
        Sprintf(buf, fmt, spellname(splnum), spellev(splnum),
                spelltypemnemonic(spell_skilltype(spellid(splnum))),
                pw_buf, spellretention(splnum, retentionbuf));

        any.a_int = splnum + 1; /* must be non-zero */
        add_menu(tmpwin, NO_GLYPH, &any, spellet(splnum), 0, ATR_NONE, buf,
                 (splnum == splaction) ? MENU_SELECTED : MENU_UNSELECTED);
    }
    how = PICK_ONE;
    if (splaction == SPELLMENU_VIEW) {
        if (spellid(1) == NO_SPELL) {
            /* only one spell => nothing to swap with */
            how = PICK_NONE;
        } else {
            /* more than 1 spell, add an extra menu entry */
            any.a_int = SPELLMENU_SORT + 1;
            add_menu(tmpwin, NO_GLYPH, &any, '+', 0, ATR_NONE,
                     "[sort spells]", MENU_UNSELECTED);
        }
    }
    end_menu(tmpwin, prompt);

    n = select_menu(tmpwin, how, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        *spell_no = selected[0].item.a_int - 1;
        /* menu selection for `PICK_ONE' does not
           de-select any preselected entry */
        if (n > 1 && *spell_no == splaction)
            *spell_no = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        /* default selection of preselected spell means that
           user chose not to swap it with anything */
        if (*spell_no == splaction)
            return FALSE;
        return TRUE;
    } else if (splaction >= 0) {
        /* explicit de-selection of preselected spell means that
           user is still swapping but not for the current spell */
        *spell_no = splaction;
        return TRUE;
    }
    return FALSE;
}

struct spellwand {
    short spell;
    short wand;  /* both of these are otyps */
};
static const struct spellwand wand_combos[] = {
    { SPE_LIGHT,           WAN_LIGHT },
    { SPE_FIREBALL,        WAN_FIRE },
    { SPE_CONE_OF_COLD,    WAN_COLD },
    { SPE_CANCELLATION,    WAN_CANCELLATION },
    { SPE_FINGER_OF_DEATH, WAN_DEATH },
    { SPE_SLEEP,           WAN_SLEEP },
    { SPE_DIG,             WAN_DIGGING },
    { SPE_KNOCK,           WAN_OPENING },
    { SPE_WIZARD_LOCK,     WAN_LOCKING },
    { SPE_DETECT_UNSEEN,   WAN_SECRET_DOOR_DETECTION },
    { SPE_MAGIC_MISSILE,   WAN_MAGIC_MISSILE },
    { SPE_INVISIBILITY,    WAN_MAKE_INVISIBLE },
    { SPE_SLOW_MONSTER,    WAN_SLOW_MONSTER },
    { SPE_HASTE_SELF,      WAN_SPEED_MONSTER },
    { SPE_FORCE_BOLT,      WAN_STRIKING },
    { SPE_TURN_UNDEAD,     WAN_UNDEAD_TURNING },
    { SPE_CREATE_MONSTER,  WAN_CREATE_MONSTER },
    { SPE_POLYMORPH,       WAN_POLYMORPH },
    { SPE_TELEPORT_AWAY,   WAN_TELEPORTATION },
    { 0, 0}
};

/* Compute a percentage chance of a spell succeeding, based on skill,
 * Intelligence, XL, armor getting in the way, the spell level, and base role
 * spellcasting ability.
 * In xNetHack this number isn't actually used directly, but rather influences
 * how much extra Pw a difficult spell will take to cast.
 * Original formula by FIQ, with some modifications. */
STATIC_OVL int
percent_success(spell)
int spell;
{
    int chance;
    int skill;
    int cap;

    /* Calculate effective Int: may be boosted by certain items */
    unsigned char intel = ACURR(A_INT);
    if (uwep && uwep->oclass == WAND_CLASS && uwep->otyp != WAN_NOTHING) {
        /* can get a boost from a wand whose magic is similar
         * however, you need to have formally identified it, otherwise you can
         * cheese wand ID by wielding different wands and seeing if this stat
         * changes */
        int i = 0;
        for (i = 0; wand_combos[i].spell != 0; ++i) {
            if (spellid(spell) == wand_combos[i].spell
                && uwep->otyp == wand_combos[i].wand
                && objects[uwep->otyp].oc_name_known) {
                intel += 7;
                break;
            }
        }
    }
    /* can't be an else if - otherwise, wielding an unidentified non-nothing
     * wand will prevent this bonus from being assessed */
    if (intel == ACURR(A_INT)
        && ((uarmc && uarmc->otyp == ROBE)
            || (uwep && (uwep->otyp == QUARTERSTAFF
                         || uwep->otyp == WAN_NOTHING)))) {
        intel += 5;
    }

    /* Don't get over-powerful with these boosts */
    if (intel >= 20) {
        intel = 20;
    }

    /* At base, chance is your base role spellcasting ability. */
    chance = urole.spelbase;

    /* Int and XL increase this. */
    chance = (intel * 5) + (u.ulevel * 5);

    /* Higher level spells will reduce this chance, though. */
    chance -= 25 * spellev(spell);

    /* Calculate penalty from armor. Metal armor and shields hurt chance. */
    if (uarm && is_metallic(uarm)) {
        chance -= 50;
    }
    if (uarms) {
        if (objects[uarms->otyp].oc_bulky)
            chance -= 30;
        else
            chance -= 15;

        if (is_metallic(uarms))
            chance -= 15;
    }
    if (uarmh && is_metallic(uarmh) && uarmh->otyp != HELM_OF_BRILLIANCE) {
        chance -= 20;
    }
    if (uarmg && is_metallic(uarmg)) {
        chance -= 35;
    }
    if (uarmf && is_metallic(uarmf)) {
        chance -= 10;
    }

    /* The less skilled you are, the worse the cap on your spellcasting ability. */
    cap = 30; /* restricted */
    skill = P_SKILL(spell_skilltype(spellid(spell)));
    if (skill == P_EXPERT)
        cap = 100;
    else if (skill == P_SKILLED)
        cap = 80;
    else if (skill == P_BASIC)
        cap = 60;
    else if (skill == P_UNSKILLED)
        cap = 40;

    /* Clamp to percentile */
    if (chance > cap)
        chance = cap;
    if (chance < 0)
        chance = 0;

    return chance;
}

/* Return the amount of energy a spell will take to cast.
   Spells can no longer fail. Instead, the percent_success() function is used
   to increase the required energy of the spell, so a spell with a 100% success
   chance costs the same as always, whereas a spell with a 50% success chance
   costs twice as much Pw, and a spell with a 1% success chance costs 100 times
   as much Pw.
   Return -1 if the success rate would be 0 and the spell cannot be cast.
*/
STATIC_OVL int
energy_cost(spell)
int spell;
{
    int energy = (spellev(spell) * 5); /* 5 <= energy <= 35 */
    int old_success_rate = percent_success(spell);

    if (old_success_rate == 0) {
        /* With a 0% success chance, the spell should take infinite power to
         * cast, and is thus still uncastable. However, this should work well
         * enough to prevent it from being cast. */
        return -1;
    } else {
        energy = (energy * 100) / old_success_rate;
    }

    /* If currently wielding the spellbook containing the spell that we're
     * trying to cast, reduce the Pw cost of casting by half rounded up. */
    if (uwep && uwep->otyp == spellid(spell)) {
        energy = (energy + 1) / 2;
    }

    return energy;
}

STATIC_OVL char *
spellretention(idx, outbuf)
int idx;
char *outbuf;
{
    long turnsleft, percent, accuracy;
    int skill;

    skill = P_SKILL(spell_skilltype(spellid(idx)));
    skill = max(skill, P_UNSKILLED); /* restricted same as unskilled */
    turnsleft = spellknow(idx);
    *outbuf = '\0'; /* lint suppression */

    if (turnsleft < 1L) {
        /* spell has expired; hero can't successfully cast it anymore */
        Strcpy(outbuf, "(gone)");
    } else if (turnsleft >= (long) KEEN) {
        /* full retention, first turn or immediately after reading book */
        Strcpy(outbuf, "100%");
    } else {
        /*
         * Retention is displayed as a range of percentages of
         * amount of time left until memory of the spell expires;
         * the precision of the range depends upon hero's skill
         * in this spell.
         *    expert:  2% intervals; 1-2,   3-4,  ...,   99-100;
         *   skilled:  5% intervals; 1-5,   6-10, ...,   95-100;
         *     basic: 10% intervals; 1-10, 11-20, ...,   91-100;
         * unskilled: 25% intervals; 1-25, 26-50, 51-75, 76-100.
         *
         * At the low end of each range, a value of N% really means
         * (N-1)%+1 through N%; so 1% is "greater than 0, at most 200".
         * KEEN is a multiple of 100; KEEN/100 loses no precision.
         */
        percent = (turnsleft - 1L) / ((long) KEEN / 100L) + 1L;
        accuracy =
            (skill == P_EXPERT) ? 2L : (skill == P_SKILLED)
                                           ? 5L
                                           : (skill == P_BASIC) ? 10L : 25L;
        /* round up to the high end of this range */
        percent = accuracy * ((percent - 1L) / accuracy + 1L);
        Sprintf(outbuf, "%ld%%-%ld%%", percent - accuracy + 1L, percent);
    }
    return outbuf;
}

/* Learn a spell during creation of the initial inventory */
void
initialspell(obj)
struct obj *obj;
{
    int i, otyp = obj->otyp;

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == NO_SPELL || spellid(i) == otyp)
            break;

    if (i == MAXSPELL) {
        impossible("Too many spells memorized!");
    } else if (spellid(i) != NO_SPELL) {
        /* initial inventory shouldn't contain duplicate spellbooks */
        impossible("Spell %s already known.", OBJ_NAME(objects[otyp]));
    } else {
        spl_book[i].sp_id = otyp;
        spl_book[i].sp_lev = objects[otyp].oc_level;
        incrnknow(i, 0);
    }
    return;
}

/*spell.c*/
