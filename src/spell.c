/* NetHack 3.7	spell.c	$NHDT-Date: 1646838390 2022/03/09 15:06:30 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.131 $ */
/*      Copyright (c) M. Stephenson 1988                          */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* spellmenu arguments; 0 thru n-1 used as gs.spl_book[] index when swapping */
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
#define incrnknow(spell, x) (gs.spl_book[spell].sp_know = KEEN + (x))

#define spellev(spell) gs.spl_book[spell].sp_lev
#define spellname(spell) OBJ_NAME(objects[spellid(spell)])
#define spellet(spell) \
    ((char) ((spell < 26) ? ('a' + spell) : ('A' + spell - 26)))

static int spell_let_to_idx(char);
static boolean cursed_book(struct obj * bp);
static boolean confused_book(struct obj *);
static void deadbook(struct obj *);
static int learn(void);
static boolean rejectcasting(void);
static boolean getspell(int *);
static int QSORTCALLBACK spell_cmp(const genericptr, const genericptr);
static void sortspells(void);
static boolean spellsortmenu(void);
static boolean dospellmenu(const char *, int, int *);
static int percent_success(int);
static int energy_cost(int);
static int throwspell(void);
static void cast_protection(void);
static void spell_backfire(int);
static int spell_hunger(int);
static boolean spelleffects_check(int, int *, int *);
static boolean can_center_spell_location(coordxy, coordxy);
static boolean spell_aim_step(genericptr_t, coordxy, coordxy);

/* The roles[] table lists the role-specific values for tuning
 * percent_success().
 *
 * Reasoning for spelbase:
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
static int
spell_let_to_idx(char ilet)
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
static boolean
cursed_book(struct obj* bp)
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
                pline_The("book glows %s.", hcolor("brown"));
            }
            else {
                /* can't call curse() here because it will call cursed_book */
                bp->cursed = 1;
                pline_The("book glows %s.", hcolor(NH_BLACK));
            }
            bp->bknown = TRUE;
        }
        break;
    case 1:
        You_feel("threatened.");
        aggravate();
        break;
    case 2:
        make_blinded(BlindedTimeout + rn1(30, 10), TRUE);
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
        poison_strdmg(Poison_resistance ? rn1(2, 1) : rn1(4, 3),
                      rnd(Poison_resistance ? 6 : 10),
                      "contact-poisoned spellbook", KILLED_BY_AN);
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
static boolean
confused_book(struct obj* spellbook)
{
    if (!rn2(3) && spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
        spellbook->in_use = TRUE; /* in case called from learn */
        pline(
         "Being confused you have difficulties in controlling your actions.");
        display_nhwindow(WIN_MESSAGE, FALSE);
        You("accidentally tear the spellbook to pieces.");
        trycall(spellbook);
        useup(spellbook);
        return TRUE;
    } else {
        You("find yourself reading the %s line over and over again.",
            spellbook == gc.context.spbook.book ? "next" : "first");
    }
    return FALSE;
}

/* special effects for The Book of the Dead; reading it while blind is
   allowed so that needs to be taken into account too */
static void
deadbook(struct obj* book2)
{
    struct monst *mtmp, *mtmp2;
    coord mm;

    You("turn the pages of the Book of the Dead...");
    makeknown(SPE_BOOK_OF_THE_DEAD);
    book2->dknown = 1; /* in case blind now and hasn't been seen yet */
    /* KMH -- Need ->known to avoid "_a_ Book of the Dead" */
    book2->known = 1;
    if (invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
        register struct obj *otmp;
        register boolean arti1_primed = FALSE, arti2_primed = FALSE,
                         arti_cursed = FALSE;

        if (book2->cursed) {
            pline_The("%s!",
                      Blind ? "Book seems to be ignoring you"
                            : "runes appear scrambled.  You can't read them");
            return;
        }

        if (!u.uhave.bell || !u.uhave.menorah) {
            pline("A chill runs down your %s.", body_part(SPINE));
            if (!u.uhave.bell) {
                Soundeffect(se_faint_chime, 30);
                You_hear("a faint chime...");
            }
            if (!u.uhave.menorah)
                pline("Vlad's doppelganger is amused.");
            return;
        }

        for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
            if (otmp->otyp == CANDELABRUM_OF_INVOCATION && otmp->spe == 7
                && otmp->lamplit) {
                if (!otmp->cursed)
                    arti1_primed = TRUE;
                else
                    arti_cursed = TRUE;
            }
            if (otmp->otyp == BELL_OF_OPENING
                && (gm.moves - otmp->age) < 5L) { /* you rang it recently */
                if (!otmp->cursed)
                    arti2_primed = TRUE;
                else
                    arti_cursed = TRUE;
            }
        }

        if (arti_cursed) {
            pline_The("invocation fails!");
            /* this used to say "your artifacts" but the invocation tools
               are not artifacts */
            pline("At least one of your relics is cursed...");
        } else if (arti1_primed && arti2_primed) {
            unsigned soon =
                (unsigned) d(2, 6); /* time til next intervene() */

            /* successful invocation */
            mkinvokearea();
            u.uevent.invoked = 1;
            record_achievement(ACH_INVK);
            /* in case you haven't killed the Wizard yet, behave as if
               you just did */
            u.uevent.udemigod = 1; /* wizdead() */
            if (!u.udg_cnt || u.udg_cnt > soon)
                u.udg_cnt = soon;
        } else { /* at least one relic not prepared properly */
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
        (void) unturn_dead(&gy.youmonst);
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
                    && mdistu(mtmp) < 4)
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

/* 'book' has just become cursed; if we're reading it, interrupt */
void
book_cursed(struct obj *book)
{
    if (book->cursed && gm.multi >= 0
        && go.occupation == learn && gc.context.spbook.book == book) {
        pline("%s shut!", Tobjnam(book, "slam"));
        set_bknown(book, 1);
        stop_occupation();
    }
}

DISABLE_WARNING_FORMAT_NONLITERAL

static int
learn(void)
{
    int i;
    short booktype;
    char splname[BUFSZ];
    boolean costly = TRUE;
    struct obj *book = gc.context.spbook.book;

    /* JDS: lenses give 50% faster reading; 33% smaller read time */
    if (gc.context.spbook.delay && ublindf && ublindf->otyp == LENSES && rn2(2))
        gc.context.spbook.delay++;
    if (Confusion) { /* became confused while learning */
        if (!confused_book(book)) {
            You("can no longer focus on the book.");
        }
        gc.context.spbook.book = 0; /* no longer studying */
        gc.context.spbook.o_id = 0;
        gc.context.spbook.delay = 0;
        return 0;
    }
    if (gc.context.spbook.delay) {
        /* not if (gc.context.spbook.delay++), so at end delay == 0 */
        gc.context.spbook.delay++;
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
        } else {
            Your("knowledge of %s is %s.", splname,
                 spellknow(i) ? "keener" : "restored");
            incrnknow(i, 1);
            book->spestudied++;
            exercise(A_WIS, TRUE); /* extra study */
        }
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
            gs.spl_book[i].sp_id = booktype;
            gs.spl_book[i].sp_lev = objects[booktype].oc_level;
            incrnknow(i, 1);
            book->spestudied++;
            if (!i)
                /* first is always 'a', so no need to mention the letter */
                You("learn %s.", splname);
            else
                You("add %s to your repertoire, as '%c'.",
                    splname, spellet(i));
        }
        makeknown((int) booktype);
    }

    if (book->cursed) { /* maybe a demon cursed it */
        if (cursed_book(book)) {
            useup(book);
            gc.context.spbook.book = 0;
            gc.context.spbook.o_id = 0;
            return 0;
        }
    }
    if (costly)
        check_unpaid(book);
    gc.context.spbook.book = 0;
    gc.context.spbook.o_id = 0;
    return 0;
}

RESTORE_WARNING_FORMAT_NONLITERAL

int
study_book(register struct obj* spellbook)
{
    int booktype = spellbook->otyp, i;
    boolean confused = (Confusion != 0);
    boolean too_hard = FALSE;

    /* attempting to read dull book may make hero fall asleep */
    if (!confused && !Sleep_resistance
        && objdescr_is(spellbook, "dull")) {
        const char *eyes;
        int dullbook = rnd(25) - ACURR(A_WIS);

        /* adjust chance if hero stayed awake, got interrupted, retries */
        if (gc.context.spbook.delay && spellbook == gc.context.spbook.book)
            dullbook -= rnd(objects[booktype].oc_level);

        if (dullbook > 0) {
            eyes = body_part(EYE);
            if (eyecount(gy.youmonst.data) > 1)
                eyes = makeplural(eyes);
            pline("This book is so dull that you can't keep your %s open.",
                  eyes);
            dullbook += rnd(2 * objects[booktype].oc_level);
            fall_asleep(-dullbook, TRUE);
            return 1;
        }
    }

    /* attempting to read a dusty book may result in sneezing */
    if (objdescr_is(spellbook, "dusty") && !rn2(5)
        && olfaction(gy.youmonst.data)) {
        pline("The book's dust gets in your %s and makes you sneeze!",
              body_part(NOSE));
        wake_nearby();
        return 1;
    }

    if (gc.context.spbook.delay && !confused
        && spellbook == gc.context.spbook.book
        /* handle the sequence: start reading, get interrupted, have
           gc.context.spbook.book become erased somehow, resume reading it */
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
                if (!u.uconduct.literate++)
                    livelog_printf(LL_CONDUCT,
                                   "became literate by reading %s", tribtitle);

                check_unpaid(spellbook);
                makeknown(booktype);
                if (!u.uevent.read_tribute) {
                    record_achievement(ACH_NOVL);
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
        gc.context.spbook.delay = -objects[booktype].oc_delay;

        /* check to see if we already know it and want to refresh our memory */
        for (i = 0; i < MAXSPELL; i++)
            if (spellid(i) == booktype || spellid(i) == NO_SPELL)
                break;
        if (spellid(i) == booktype && spellknow(i) > KEEN / 10) {
            You("know \"%s\" quite well already.",
                OBJ_NAME(objects[booktype]));
            /* hero has just been told what spell this book is for; it may
               have been undiscovered if spell was learned via divine gift */
            makeknown(booktype);
            if (y_n("Refresh your memory anyway?") == 'n')
                return 0;
        }

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
                if (y_n(qbuf) != 'y') {
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
            make_confused(-gc.context.spbook.delay, FALSE); /* study time */
            bot(); /* show Conf on status line before tele, crumbling, etc */

            boolean gone = cursed_book(spellbook);
            if (gone) {
                trycall(spellbook);
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

    gc.context.spbook.book = spellbook;
    if (gc.context.spbook.book)
        gc.context.spbook.o_id = gc.context.spbook.book->o_id;
    set_occupation(learn, "studying", 0);
    return 1;
}

/* a spellbook has been destroyed or the character has changed levels;
   the stored address for the current book is no longer valid */
void
book_disappears(struct obj* obj)
{
    if (obj == gc.context.spbook.book) {
        gc.context.spbook.book = (struct obj *) 0;
        gc.context.spbook.o_id = 0;
    }
}

/* renaming an object usually results in it having a different address;
   so the sequence start reading, get interrupted, name the book, resume
   reading would read the "new" book from scratch */
void
book_substitution(struct obj* old_obj, struct obj* new_obj)
{
    if (old_obj == gc.context.spbook.book) {
        gc.context.spbook.book = new_obj;
        if (gc.context.spbook.book)
            gc.context.spbook.o_id = gc.context.spbook.book->o_id;
    }
}

/* called from moveloop() */
void
age_spells(void)
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
static boolean
rejectcasting(void)
{
    /* rejections which take place before selecting a particular spell */
    if (Stunned) {
        You("are too impaired to cast a spell.");
        return TRUE;
    } else if (!can_chant(&gy.youmonst)) {
        You("are unable to chant the incantation.");
        return TRUE;
    } else if (!freehand() && !(uwep && uwep->otyp == QUARTERSTAFF)) {
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
static boolean
getspell(int* spell_no)
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
        nspells = num_spells();

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
            Snprintf(qbuf, sizeof(qbuf), "Cast which spell? [%s *?]",
                     lets);
            ilet = yn_function(qbuf, (char *) 0, '\0', TRUE);
            if (ilet == '*' || ilet == '?')
                break; /* use menu mode */
            if (strchr(quitchars, ilet))
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

/* #wizcast - cast any spell even without knowing it */
int
dowizcast(void)
{
    winid win;
    menu_item *selected;
    anything any;
    int i, n;

    win = create_nhwindow(NHW_MENU);
    start_menu(win, MENU_BEHAVE_STANDARD);
    any = cg.zeroany;

    for (i = 0; i < MAXSPELL; i++) {
        n = (SPE_DIG + i);
        if (n >= SPE_BLANK_PAPER)
            break;
        any.a_int = n;
        add_menu(win, &nul_glyphinfo, &any, 0, 0, ATR_NONE, 0, OBJ_NAME(objects[n]), MENU_ITEMFLAGS_NONE);
    }
    end_menu(win, "Cast which spell?");
    n = select_menu(win, PICK_ONE, &selected);
    destroy_nhwindow(win);
    if (n > 0) {
        i = selected[0].item.a_int;
        free((genericptr_t) selected);
        return spelleffects(i, FALSE, TRUE);
    }
    return ECMD_OK;
}


/* the #cast command -- cast a spell */
int
docast(void)
{
    int spell_no;

    if (getspell(&spell_no))
        return spelleffects(gs.spl_book[spell_no].sp_id, FALSE, FALSE);
    return ECMD_OK;
}

const char *
spelltypemnemonic(int skill)
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
spell_skilltype(int booktype)
{
    return objects[booktype].oc_skill;
}

static void
cast_protection(void)
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
                struct permonst *pm = u.ustuck ? u.ustuck->data : 0;

                rmtyp = levl[u.ux][u.uy].typ;
                atmosphere = (pm && u.uswallow)
                                ? ((pm == &mons[PM_FOG_CLOUD]) ? "mist"
                                   : is_whirly(pm) ? "maelstrom"
                                     : enfolds(pm) ? "folds"
                                       : is_animal(pm) ? "maw"
                                         : "ooze")
                                : (u.uinwater ? hliquid("water")
                                   : (rmtyp == CLOUD) ? "cloud"
                                     : IS_TREE(rmtyp) ? "vegetation"
                                       : IS_STWALL(rmtyp) ? "stone"
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
static void
spell_backfire(int spell)
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

/* Given an expected amount of hunger for a spell, return the amount it should
 * be reduced to. High-intelligence Wizards get to cast spells with less or no
 * hunger penalty. */
static int
spell_hunger(int hungr)
{
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
    if (Role_if(PM_WIZARD)) {
        int intel = acurr(A_INT);
        if (intel >= 17)
            return 0;
        else if (intel == 16)
            return hungr / 4;
        else if (intel == 15)
            return hungr / 2;
    }
    /* no adjustment */
    return hungr;
}
/* for using this function to test whether hunger would be eliminated */
#define spell_would_hunger() (spell_hunger(100) > 0)

static boolean
spelleffects_check(int spell, int *res, int *energy)
{
    boolean confused = (Confusion != 0);

    *energy = 0;

    /*
     * Reject attempting to cast while stunned or with no free hands.
     * Already done in getspell() to stop casting before choosing
     * which spell, but duplicated here for cases where spelleffects()
     * gets called directly for ^T without intrinsic teleport capability
     * or #turn for non-priest/non-knight.
     * (There's no duplication of messages; when the rejection takes
     * place in getspell(), we don't get called.)
     */
    if ((spell == UNKNOWN_SPELL) || rejectcasting()) {
        *res = ECMD_OK; /* no time elapses */
        return TRUE;
    }

    /*
     *  Note: dotele() also calculates energy use and checks nutrition
     *  and strength requirements; if any of these change, update it too.
     */
    *energy = SPELL_LEV_PW(spellev(spell)); /* 5 <= energy <= 35 */

    /*
     * Spell casting no longer affects knowledge of the spell. A
     * decrement of spell knowledge is done every turn.
     */
    if (spellknow(spell) <= 0) {
        Your("knowledge of this spell is twisted.");
        pline("It invokes nightmarish images in your mind...");
        spell_backfire(spell);
        u.uen -= rnd(*energy);
        if (u.uen < 0)
            u.uen = 0;
        gc.context.botl = 1;
        *res = ECMD_TIME;
        return TRUE;
    } else if (spellknow(spell) <= KEEN / 200) { /* 100 turns left */
        You("strain to recall the spell.");
    } else if (spellknow(spell) <= KEEN / 40) { /* 500 turns left */
        You("have difficulty remembering the spell.");
    } else if (spellknow(spell) <= KEEN / 20) { /* 1000 turns left */
        Your("knowledge of this spell is growing faint.");
    } else if (spellknow(spell) <= KEEN / 10) { /* 2000 turns left */
        Your("recall of this spell is gradually fading.");
    }

    if (u.uhunger <= 10 && spellid(spell) != SPE_DETECT_FOOD
        && spell_would_hunger() > 0) {
        You("are too hungry to cast that spell.");
        *res = ECMD_OK;
        return TRUE;
    } else if (ACURR(A_STR) < 4 && spellid(spell) != SPE_RESTORE_ABILITY) {
        You("lack the strength to cast spells.");
        *res = ECMD_OK;
        return TRUE;
    } else if (check_capacity(
                "Your concentration falters while carrying so much stuff.")) {
        *res = ECMD_TIME;
        return TRUE;
    }

    /*
     *  Note: dotele() also calculates energy use and checks nutrition
     *  and strength requirements; it any of these change, update it too.
     */
    *energy = energy_cost(spell);
    /* if spell is impossible to cast, kludge this to u.uen + 1 to make it fail
     * the checks below as not having enough energy. */
    if (*energy < 0) {
        *energy = u.uen + 1;
    }

    /* if the cast attempt is already going to fail due to insufficient
       energy (ie, u.uen < energy), the Amulet's drain effect won't kick
       in and no turn will be consumed; however, when it does kick in,
       the attempt may fail due to lack of energy after the draining, in
       which case a turn will be used up in addition to the energy loss */
    if (u.uhave.amulet && u.uen >= *energy) {
        You_feel("the amulet draining your energy away.");
        /* this used to be 'energy += rnd(2 * energy)' (without 'res'),
           so if amulet-induced cost was more than u.uen, nothing
           (except the "don't have enough energy" message) happened
           and player could just try again (and again and again...);
           now we drain some energy immediately, which has a
           side-effect of not increasing the hunger aspect of casting */
        u.uen -= rnd(2 * *energy);
        if (u.uen < 0)
            u.uen = 0;
        gc.context.botl = 1;
        *res = ECMD_TIME; /* time is used even if spell doesn't get cast */
    }

    if (*energy > u.uen) {
        /*
         * Hero has insufficient energy/power to cast the spell.
         * Augment the message when current energy is at maximum.
         * "yet": mainly for level 1 characters who already know a spell
         * but don't start with enough energy to cast it.
         * "anymore": maximum energy was high enough at some point but
         * isn't now (lost energy when losing levels or polymorphing into
         * new person or had some stripped away by traps or monsters).
         */
        You("don't have enough energy to cast that spell%s.",
            (u.uen < u.uenmax) ? "" /* not at full energy => normal message */
            : (*energy > u.uenpeak) ? " yet" /* haven't ever had enough */
              : " anymore"); /* once had enough but have lost some since */
        return TRUE;
    } else {
        if (spellid(spell) != SPE_DETECT_FOOD) {
            int hungr = spell_hunger(*energy * 2);
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
        u.uen -= *energy / 2;
        gc.context.botl = 1;
        *res = ECMD_TIME;
        return TRUE;
    }
    return FALSE;
}

/* hero casts a spell of type spell_otyp, eg. SPE_SLEEP.
   hero must know the spell (unless force is TRUE). */
int
spelleffects(int spell_otyp, boolean atme, boolean force)
{
    int spell = force ? spell_otyp : spell_idx(spell_otyp);
    int energy = 0, damage, n;
    int otyp, skill, role_skill, res = ECMD_OK;
    boolean physical_damage = FALSE;
    struct obj *pseudo;
    coord cc;

    if (!force && spelleffects_check(spell, &res, &energy))
        return res;

    u.uen -= energy;
    gc.context.botl = 1;
    exercise(A_WIS, TRUE);
    /* pseudo is a temporary "false" object containing the spell stats */
    pseudo = mksobj(force ? spell : spellid(spell), FALSE, FALSE);
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
        if (role_skill >= P_SKILLED && y_n("Cast advanced spell?") == 'y') {
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
        } else if (role_skill >= P_SKILLED) {
            /* player said not to cast advanced spell; return up to half of the
             * magical energy */
            u.uen += rnd(energy / 2);
        }
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
        /* remove externally imposed intrinsic blind; expect healup to call
         * make_blinded(0) to produce messages and recalc vision */
        HBlinded &= ~FROMOUTSIDE;
        set_itimeout(&HBlinded, 1L);
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
        if (!(jump(max(role_skill, 1)) & ECMD_TIME))
            pline1(nothing_happens);
        break;
    default:
        impossible("Unknown spell %d attempted.", spell);
        obfree(pseudo, (struct obj *) 0);
        return ECMD_OK;
    }

    /* gain skill for successful cast */
    use_skill(skill, spellev(spell));

    obfree(pseudo, (struct obj *) 0); /* now, get rid of it */
    return ECMD_TIME;
}

/*ARGSUSED*/
static boolean
spell_aim_step(genericptr_t arg UNUSED, coordxy x, coordxy y)
{
    if (!isok(x,y))
        return FALSE;
    if (!ZAP_POS(levl[x][y].typ)
        && !(IS_DOOR(levl[x][y].typ) && doorstate(&levl[x][y]) == D_ISOPEN))
        return FALSE;
    return TRUE;
}

/* not quite the same as throwspell limits, but close enough */
static boolean
can_center_spell_location(coordxy x, coordxy y)
{
    if (distmin(u.ux, u.uy, x, y) > 10)
        return FALSE;
    return (isok(x, y) && cansee(x, y) && !(IS_STWALL(levl[x][y].typ)));
}

/* Choose location where spell takes effect. */
static int
throwspell(void)
{
    coord cc, uc;
    struct monst *mtmp;

    if (u.uinwater) {
        pline("You're joking!  In this weather?");
        return 0;
    } else if (Is_waterlevel(&u.uz)) {
        You("had better wait for the sun to come out.");
        return 0;
    }

    pline("Where do you want to cast the spell?");
    cc.x = u.ux;
    cc.y = u.uy;
    getpos_sethilite(NULL, can_center_spell_location);
    if (getpos(&cc, TRUE, "the desired position") < 0)
        return 0; /* user pressed ESC */
    clear_nhwindow(WIN_MESSAGE); /* discard any autodescribe feedback */

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
    } else if (((cc.x != u.ux || cc.y != u.uy) && !cansee(cc.x, cc.y)
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

/* add/hide/remove/unhide teleport-away on behalf of dotelecmd() to give
   more control to behavior of ^T when used in wizard mode */
int
tport_spell(int what)
{
    static struct tport_hideaway {
        struct spell savespell;
        int tport_indx;
    } save_tport;
    int i;
/* also defined in teleport.c */
#define NOOP_SPELL  0
#define HIDE_SPELL  1
#define ADD_SPELL   2
#define UNHIDESPELL 3
#define REMOVESPELL 4

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == SPE_TELEPORT_AWAY || spellid(i) == NO_SPELL)
            break;
    if (i == MAXSPELL) {
        impossible("tport_spell: spellbook full");
        /* wizard mode ^T is not able to honor player's menu choice */
    } else if (spellid(i) == NO_SPELL) {
        if (what == HIDE_SPELL || what == REMOVESPELL) {
            save_tport.tport_indx = MAXSPELL;
        } else if (what == UNHIDESPELL) {
            /*assert( save_tport.savespell.sp_id == SPE_TELEPORT_AWAY );*/
            gs.spl_book[save_tport.tport_indx] = save_tport.savespell;
            save_tport.tport_indx = MAXSPELL; /* burn bridge... */
        } else if (what == ADD_SPELL) {
            save_tport.savespell = gs.spl_book[i];
            save_tport.tport_indx = i;
            gs.spl_book[i].sp_id = SPE_TELEPORT_AWAY;
            gs.spl_book[i].sp_lev = objects[SPE_TELEPORT_AWAY].oc_level;
            gs.spl_book[i].sp_know = KEEN;
            return REMOVESPELL; /* operation needed to reverse */
        }
    } else { /* spellid(i) == SPE_TELEPORT_AWAY */
        if (what == ADD_SPELL || what == UNHIDESPELL) {
            save_tport.tport_indx = MAXSPELL;
        } else if (what == REMOVESPELL) {
            /*assert( i == save_tport.tport_indx );*/
            gs.spl_book[i] = save_tport.savespell;
            save_tport.tport_indx = MAXSPELL;
        } else if (what == HIDE_SPELL) {
            save_tport.savespell = gs.spl_book[i];
            save_tport.tport_indx = i;
            gs.spl_book[i].sp_id = NO_SPELL;
            return UNHIDESPELL; /* operation needed to reverse */
        }
    }
    return NOOP_SPELL;
}

/* forget a random selection of known spells due to amnesia;
   they used to be lost entirely, as if never learned, but now we
   just set the memory retention to zero so that they can't be cast */
void
losespells(void)
{
    int n, nzap, i;

    /* in case reading has been interrupted earlier, discard context */
    gc.context.spbook.book = 0;
    gc.context.spbook.o_id = 0;
    /* count the number of known spells */
    for (n = 0; n < MAXSPELL; ++n)
        if (spellid(n) == NO_SPELL)
            break;

    /* lose anywhere from zero to all known spells;
       if confused, use the worse of two die rolls */
    nzap = rn2(n + 1);
    if (Confusion) {
        i = rn2(n + 1);
        if (i > nzap)
            nzap = i;
    }
    /* good Luck might ameliorate spell loss */
    if (nzap > 1 && !rnl(7))
        nzap = rnd(nzap);

    /*
     * Forget 'nzap' out of 'n' known spells by setting their memory
     * retention to zero.  Every spell has the same probability to be
     * forgotten, even if its retention is already zero.
     *
     * Perhaps we should forget the corresponding book too?
     *
     * (3.4.3 removed spells entirely from the list, but always did
     * so from its end, so the 'nzap' most recently learned spells
     * were the ones lost by default.  Player had sort control over
     * the list, so could move the most useful spells to front and
     * only lose them if 'nzap' turned out to be a large value.
     *
     * Discarding from the end of the list had the virtue of making
     * casting letters for lost spells become invalid and retaining
     * the original letter for the ones which weren't lost, so there
     * was no risk to the player of accidentally casting the wrong
     * spell when using a letter that was in use prior to amnesia.
     * That wouldn't be the case if we implemented spell loss spread
     * throughout the list of known spells; every spell located past
     * the first lost spell would end up with new letter assigned.)
     */
    for (i = 0; nzap > 0; ++i) {
        /* when nzap is small relative to the number of spells left,
           the chance to lose spell [i] is small; as the number of
           remaining candidates shrinks, the chance per candidate
           gets bigger; overall, exactly nzap entries are affected */
        if (rn2(n - i) < nzap) {
            /* lose access to spell [i] */
            spellknow(i) = 0;
#if 0
            /* also forget its book */
            forget_single_object(spellid(i));
#endif
            /* and abuse wisdom */
            exercise(A_WIS, FALSE);
            /* there's now one less spell slated to be forgotten */
            --nzap;
        }
    }
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

static const char *const spl_sortchoices[NUM_SPELL_SORTBY] = {
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

/* qsort callback routine */
static int QSORTCALLBACK
spell_cmp(const genericptr vptr1, const genericptr vptr2)
{
    /*
     * gather up all of the possible parameters except spell name
     * in advance, even though some might not be needed:
     *  indx. = spl_orderindx[] index into gs.spl_book[];
     *  otyp. = gs.spl_book[] index into objects[];
     *  levl. = spell level;
     *  skil. = skill group aka spell class.
     */
    int indx1 = *(int *) vptr1, indx2 = *(int *) vptr2,
        otyp1 = gs.spl_book[indx1].sp_id, otyp2 = gs.spl_book[indx2].sp_id,
        levl1 = objects[otyp1].oc_level, levl2 = objects[otyp2].oc_level,
        skil1 = objects[otyp1].oc_skill, skil2 = objects[otyp2].oc_skill;

    switch (gs.spl_sortmode) {
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
static void
sortspells(void)
{
    int i;
#if defined(SYSV) || defined(DGUX)
    unsigned n;
#else
    int n;
#endif

    if (gs.spl_sortmode == SORTBY_CURRENT)
        return;
    for (n = 0; n < MAXSPELL && spellid(n) != NO_SPELL; ++n)
        continue;
    if (n < 2)
        return; /* not enough entries to need sorting */

    if (!gs.spl_orderindx) {
        /* we haven't done any sorting yet; list is in casting order */
        if (gs.spl_sortmode == SORTBY_LETTER /* default */
            || gs.spl_sortmode == SORTRETAINORDER)
            return;
        /* allocate enough for full spellbook rather than just N spells */
        gs.spl_orderindx = (int *) alloc(MAXSPELL * sizeof(int));
        for (i = 0; i < MAXSPELL; i++)
            gs.spl_orderindx[i] = i;
    }

    if (gs.spl_sortmode == SORTRETAINORDER) {
        struct spell tmp_book[MAXSPELL];

        /* sort gs.spl_book[] rather than spl_orderindx[];
           this also updates the index to reflect the new ordering (we
           could just free it since that ordering becomes the default) */
        for (i = 0; i < MAXSPELL; i++)
            tmp_book[i] = gs.spl_book[gs.spl_orderindx[i]];
        for (i = 0; i < MAXSPELL; i++)
            gs.spl_book[i] = tmp_book[i], gs.spl_orderindx[i] = i;
        gs.spl_sortmode = SORTBY_LETTER; /* reset */
        return;
    }

    /* usual case, sort the index rather than the spells themselves */
    qsort((genericptr_t) gs.spl_orderindx, n,
          sizeof *gs.spl_orderindx, spell_cmp);
    return;
}

/* called if the [sort spells] entry in the view spells menu gets chosen */
static boolean
spellsortmenu(void)
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    char let;
    int i, n, choice;
    int clr = 0;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany; /* zero out all bits */

    for (i = 0; i < SIZE(spl_sortchoices); i++) {
        if (i == SORTRETAINORDER) {
            let = 'z'; /* assumes fewer than 26 sort choices... */
            /* separate final choice from others with a blank line */
            any.a_int = 0;
            add_menu(tmpwin, &nul_glyphinfo, &any, 0, 0,
                     ATR_NONE, clr, "", MENU_ITEMFLAGS_NONE);
        } else {
            let = 'a' + i;
        }
        any.a_int = i + 1;
        add_menu(tmpwin, &nul_glyphinfo, &any, let, 0,
                 ATR_NONE, clr, spl_sortchoices[i],
                 (i == gs.spl_sortmode) ? MENU_ITEMFLAGS_SELECTED
                                        : MENU_ITEMFLAGS_NONE);
    }
    end_menu(tmpwin, "View known spells list sorted");

    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        choice = selected[0].item.a_int - 1;
        /* skip preselected entry if we have more than one item chosen */
        if (n > 1 && choice == gs.spl_sortmode)
            choice = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        gs.spl_sortmode = choice;
        return TRUE;
    }
    return FALSE;
}

/* the #showspells command -- view known spells */
int
dovspell(void)
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

                spl_tmp = gs.spl_book[splnum];
                gs.spl_book[splnum] = gs.spl_book[othnum];
                gs.spl_book[othnum] = spl_tmp;
            }
        }
    }
    if (gs.spl_orderindx) {
        free((genericptr_t) gs.spl_orderindx);
        gs.spl_orderindx = 0;
    }
    gs.spl_sortmode = SORTBY_LETTER; /* 0 */
    return ECMD_OK;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* shows menu of known spells, with options to sort them.
   return FALSE on cancel, TRUE otherwise.
   spell_no is set to the internal spl_book index, if any selected */
static boolean
dospellmenu(
    const char *prompt,
    int splaction, /* SPELLMENU_CAST, SPELLMENU_VIEW, or gs.spl_book[] index */
    int *spell_no)
{
    winid tmpwin;
    int i, n, how, splnum;
    char buf[BUFSZ], pw_buf[5], sep;
    const char *fmt;
    menu_item *selected;
    anything any;
    int clr = 0;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany; /* zero out all bits */

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
        fmt = "%-20s  %2d   %-12s %4s     %5d";
        sep = ' ';
    } else {
        Sprintf(buf, "Name\tLevel\tCategory\tPw\tRetention");
        fmt = "%s\t%-d\t%s\t%s\t%d";
        sep = '\t';
    }
    if (wizard)
        Sprintf(eos(buf), "%c%6s", sep, "turns");

    add_menu(tmpwin, &nul_glyphinfo, &any, 0, 0,
             iflags.menu_headings, clr, buf, MENU_ITEMFLAGS_NONE);
    for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
        splnum = !gs.spl_orderindx ? i : gs.spl_orderindx[i];
        if (energy_cost(splnum) < 0) {
            strcpy(pw_buf, "Inf");
        } else {
            /* maximum possible should be 3500 */
            Sprintf(pw_buf, "%d", energy_cost(splnum));
        }
        Sprintf(buf, fmt, spellname(splnum), spellev(splnum),
                spelltypemnemonic(spell_skilltype(spellid(splnum))),
                pw_buf, spellknow(splnum));

        any.a_int = splnum + 1; /* must be non-zero */
        add_menu(tmpwin, &nul_glyphinfo, &any, spellet(splnum), 0,
                 ATR_NONE, clr, buf,
                 (splnum == splaction)
                    ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
    }
    how = PICK_ONE;
    if (splaction == SPELLMENU_VIEW) {
        if (spellid(1) == NO_SPELL) {
            /* only one spell => nothing to swap with */
            how = PICK_NONE;
        } else {
            /* more than 1 spell, add an extra menu entry */
            any.a_int = SPELLMENU_SORT + 1;
            add_menu(tmpwin, &nul_glyphinfo, &any, '+', 0,
                     ATR_NONE, clr, "[sort spells]", MENU_ITEMFLAGS_NONE);
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

RESTORE_WARNING_FORMAT_NONLITERAL

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
static int
percent_success(int spell)
{
    int chance, armor_penalty, cap;

    boolean wield_wand_bonus, wield_book_bonus, gear_bonus;
    wield_wand_bonus = wield_book_bonus = gear_bonus = FALSE;
    int skill, skilltype = spell_skilltype(spellid(spell));

    /* Knights don't get metal armor penalty for clerical spells */
    boolean paladin_bonus = (Role_if(PM_KNIGHT)
                             && skilltype == P_CLERIC_SPELL);

    if (uwep && uwep->oclass == WAND_CLASS && uwep->otyp != WAN_NOTHING) {
        /* can get a boost from a wand whose magic is similar
         * however, you need to have formally identified it, otherwise you can
         * cheese wand ID by wielding different wands and seeing if the
         * spellcasting menu changes */
        int i = 0;
        for (i = 0; wand_combos[i].spell != 0; ++i) {
            if (spellid(spell) == wand_combos[i].spell
                && uwep->otyp == wand_combos[i].wand
                && objects[uwep->otyp].oc_name_known) {
                wield_wand_bonus = TRUE;
                break;
            }
        }
    }
    /* can't be an else if - otherwise, wielding an unidentified non-nothing
     * wand will prevent this bonus from being assessed */
    if (uwep && uwep->oclass == SPBOOK_CLASS && uwep->otyp == spellid(spell)) {
        /* Wielding the spellbook containing the spell that we're trying to cast
         * gives the same benefit as the wand.
         * No check for spellbook identification here, largely because if the
         * hero knows a spell they should know its corresponding book's
         * appearance. */
        wield_book_bonus = TRUE;
    }
    if (uarmc && uarmc->otyp == ROBE) {
        gear_bonus = TRUE;
    }
    if (uwep && (uwep->otyp == QUARTERSTAFF || uwep->otyp == WAN_NOTHING)) {
        gear_bonus = TRUE;
    }

    /* At base, chance is your base role spellcasting ability. */
    chance = gu.urole.spelbase;

    /* Int and XL increase this. */
    chance += (ACURR(A_INT) * 5) + (u.ulevel * 5);

    /* So does wielding the proper wand. */
    if (wield_wand_bonus) {
        chance += 30;
    }

    /* Higher level spells will reduce this chance, though. */
    chance -= 25 * spellev(spell);

    /* Calculate penalty from armor. Metal armor and shields hurt chance. */
    armor_penalty = 0;
    if (uarm && is_metallic(uarm) && !paladin_bonus) {
        armor_penalty += 50;
    }
    if (uarms && !is_quest_artifact(uarms)) {
        /* quest artifact check allows Archeologists to wear Itlachiayaque
         * without penalty; other roles get normal penalty */
        if (objects[uarms->otyp].oc_bulky)
            armor_penalty += 30;
        else
            armor_penalty += 15;

        if (is_metallic(uarms) && !paladin_bonus)
            armor_penalty += 15;
    }
    if (uarmh && is_metallic(uarmh) && !paladin_bonus) {
        /* HELM_OF_BRILLIANCE is now crystal and not checked here anymore */
        armor_penalty += 20;
    }
    if (uarmg && is_metallic(uarmg) && !paladin_bonus) {
        armor_penalty += 35;
    }
    if (uarmf && is_metallic(uarmf) && !paladin_bonus) {
        armor_penalty += 10;
    }

    if (gear_bonus) {
        armor_penalty = (armor_penalty + 1) / 2;
    }
    chance -= armor_penalty;

    /* One of Orcus's negative effects during the ascension run is to apply a
     * fluctuating penalty to spellcasting. */
    if (fiend_adversity(PM_ORCUS)) {
        chance = ((hash1(gm.moves / 20) % 10) + 10);
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
static int
energy_cost(int spell)
{
    int base_energy = (spellev(spell) * 5); /* 5 <= energy <= 35 */
    int energy = base_energy;
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
     * trying to cast, reduce the Pw cost of casting by half rounded up.
     * But don't reduce further than the innate base amount of power the spell
     * normally takes to cast. */
    if (uwep && uwep->otyp == spellid(spell)) {
        int half_energy = (energy + 1) / 2;
        if (half_energy < base_energy)
            half_energy = base_energy;
        energy = half_energy;
    }

    return energy;
}

/* Learn a spell during creation of the initial inventory */
void
initialspell(struct obj* obj)
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
        gs.spl_book[i].sp_id = otyp;
        gs.spl_book[i].sp_lev = objects[otyp].oc_level;
        incrnknow(i, 0);
    }
    return;
}

/* returns one of spe_Unknown, spe_Fresh, spe_GoingStale, spe_Forgotten */
int
known_spell(short otyp)
{
    int i, k;

    for (i = 0; (i < MAXSPELL) && (spellid(i) != NO_SPELL); i++)
        if (spellid(i) == otyp) {
            k = spellknow(i);
            return (k > KEEN / 10) ? spe_Fresh
                   : (k > 0) ? spe_GoingStale
                     : spe_Forgotten;
        }
    return spe_Unknown;
}

/* return index for spell otyp, or UNKNOWN_SPELL if not found */
int
spell_idx(short otyp)
{
    int i;

    for (i = 0; (i < MAXSPELL) && (spellid(i) != NO_SPELL); i++)
        if (spellid(i) == otyp)
            return i;
    return UNKNOWN_SPELL;
}

/* learn or refresh spell otyp, if feasible; return casting letter or '\0' */
char
force_learn_spell(short otyp)
{
    int i;

    if (otyp == SPE_BLANK_PAPER || otyp == SPE_BOOK_OF_THE_DEAD
        || known_spell(otyp) == spe_Fresh)
        return '\0';

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == NO_SPELL || spellid(i) == otyp)
            break;
    if (i == MAXSPELL) {
        impossible("Too many spells memorized");
        return '\0';
    }
    /* for a going-stale or forgotten spell the sp_id and sp_lev assignments
       are redundant but harmless; for an unknown spell, they're essential */
    gs.spl_book[i].sp_id = otyp;
    gs.spl_book[i].sp_lev = objects[otyp].oc_level;
    incrnknow(i, 0); /* set spl_book[i].sp_know to KEEN; unlike when learning
                      * a spell by reading its book, we don't need to add 1 */
    return spellet(i);
}

/* number of spells hero knows */
int
num_spells(void)
{
    int i;

    for (i = 0; i < MAXSPELL; i++)
        if (spellid(i) == NO_SPELL)
            break;
    return i;
}

/*spell.c*/
