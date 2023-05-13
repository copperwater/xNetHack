/* NetHack 3.7	trap.h	$NHDT-Date: 1670316586 2022/12/06 08:49:46 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.31 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2016. */
/* NetHack may be freely redistributed.  See license for details. */

/* note for 3.1.0 and later: no longer manipulated by 'makedefs' */

#ifndef TRAP_H
#define TRAP_H

union vlaunchinfo {
    short v_launch_otyp; /* type of object to be triggered */
    coord v_launch2;     /* secondary launch point (for boulders) */
    uchar v_conjoined;   /* conjoined pit locations */
    short v_tnote;       /* boards: 12 notes        */
};

struct trap {
    struct trap *ntrap;
    coordxy tx, ty;
    d_level dst; /* destination for portals/holes/trapdoors */
    coord launch;
#define teledest launch /* x,y destination for teleport traps, if > 0 */
    Bitfield(ttyp, 5);
    Bitfield(tseen, 1);
    Bitfield(once, 1); /* has the trap been triggered previously? */
    Bitfield(madeby_u, 1); /* So monsters may take offence when you trap
                            * them.  Recognizing who made the trap isn't
                            * completely unreasonable, everybody has
                            * their own style.  This flag is also needed
                            * when you untrap a monster.  It would be too
                            * easy to make a monster peaceful if you could
                            * set a trap for it and then untrap it. */
    struct obj* ammo; /* object associated with this trap - darts for a dart
                         trap, arrows for an arrow trap, a beartrap object for a
                         bear trap.  This object does not physically exist in
                         the game until some action creates it, such as the
                         beartrap being untrapped, or one dart being fired.
                         Not all types of traps will need this field - in fact,
                         most don't. Only those which need to store persistent
                         information about the associated object do. */
    union vlaunchinfo vl;
#define launch_otyp vl.v_launch_otyp
#define launch2 vl.v_launch2
#define conjoined vl.v_conjoined
#define tnote vl.v_tnote
};

#define newtrap() (struct trap *) alloc(sizeof(struct trap))
#define dealloc_trap(trap) free((genericptr_t)(trap))

/* reasons for statue animation */
#define ANIMATE_NORMAL 0
#define ANIMATE_SHATTER 1
#define ANIMATE_SPELL 2

/* reasons for animate_statue's failure */
#define AS_OK 0            /* didn't fail */
#define AS_NO_MON 1        /* makemon failed */
#define AS_MON_IS_UNIQUE 2 /* statue monster is unique */

/* Note: if adding/removing a trap, adjust trap_engravings[] in mklev.c */

/* unconditional traps */
enum trap_types {
    ALL_TRAPS    = -1, /* mon_knows_traps(), mon_learns_traps() */
    NO_TRAP      =  0,
    ARROW_TRAP   =  1,
    DART_TRAP    =  2,
    ROCKTRAP     =  3,
    SQKY_BOARD   =  4,
    BEAR_TRAP    =  5,
    LANDMINE     =  6,
    ROLLING_BOULDER_TRAP = 7,
    SLP_GAS_TRAP =  8,
    RUST_TRAP    =  9,
    FIRE_TRAP    = 10,
    COLD_TRAP    = 11,
    PIT          = 12,
    SPIKED_PIT   = 13,
    HOLE         = 14,
    TRAPDOOR     = 15,
    TELEP_TRAP   = 16,
    LEVEL_TELEP  = 17,
    MAGIC_PORTAL = 18,
    WEB          = 19,
    STATUE_TRAP  = 20,
    MAGIC_TRAP   = 21,
    ANTI_MAGIC   = 22,
    POLY_TRAP    = 23,
    VIBRATING_SQUARE = 24, /* not a trap but shown/remembered as if one
                            * once it has been discovered */

    /* trapped door and trapped chest aren't traps on the map, but they
       might be shown/remembered as such after trap detection until hero
       comes in view of them and sees the feature or object;
       key-using or door-busting monsters who survive a door trap learn
       to avoid other such doors [not implemented] */
    TRAPPED_DOOR = 25, /* part of door; not present on map as a trap */
    TRAPPED_CHEST = 26, /* part of object; not on map */

    TRAPNUM = 27
};

/* some trap-related function return results */
enum { Trap_Effect_Finished = 0,
       Trap_Is_Gone = 0,
       Trap_Caught_Mon = 1,
       Trap_Killed_Mon = 2,
       Trap_Moved_Mon = 3, /* new location, or new level */
};

#define is_pit(ttyp) ((ttyp) == PIT || (ttyp) == SPIKED_PIT)
#define is_hole(ttyp)  ((ttyp) == HOLE || (ttyp) == TRAPDOOR)
#define unhideable_trap(ttyp) ((ttyp) == HOLE \
                               || (ttyp) == MAGIC_PORTAL) /* visible traps */
#define undestroyable_trap(ttyp) ((ttyp) == MAGIC_PORTAL         \
                                  || (ttyp) == VIBRATING_SQUARE)
#define is_magical_trap(ttyp) ((ttyp) == TELEP_TRAP     \
                               || (ttyp) == LEVEL_TELEP \
                               || (ttyp) == MAGIC_TRAP  \
                               || (ttyp) == ANTI_MAGIC  \
                               || (ttyp) == POLY_TRAP)
/* "transportation" traps */
#define is_xport(ttyp) ((ttyp) >= TELEP_TRAP && (ttyp) <= MAGIC_PORTAL)

/* List of traps that can be triggered by interacting with a door. */
enum doortrap_types {
    HINGE_SCREECH = 0,
    SELF_LOCK,
    STATIC_SHOCK,
    WATER_BUCKET,
    HINGELESS_FORWARD,
    HINGELESS_BACKWARD,
    ROCKFALL,
    HOT_KNOB,
    FIRE_BLAST,
    NUMDOORTRAPS
    /* other ideas: boxing glove, sticky doorknob
     * hurtle(u.ux - doorx, u.uy - doory, 1, FALSE) */
};

/* Return codes from immune_to_trap. */
enum trap_immunities {
    TRAP_NOT_IMMUNE = 0,
    TRAP_CLEARLY_IMMUNE,
    TRAP_HIDDEN_IMMUNE
};

/* Values for deltrap_with_ammo */
enum deltrap_handle_ammo {
    DELTRAP_RETURN_AMMO = 0, /* return ammo to caller; do nothing with it */
    DELTRAP_DESTROY_AMMO,    /* delete ammo */
    DELTRAP_PLACE_AMMO,      /* place ammo on ground where trap was */
    DELTRAP_BURY_AMMO,       /* bury ammo under where trap was */
    DELTRAP_TAKE_AMMO        /* put ammo into player's inventory */
};

#define predoortrapped(x, y, mon, body, act) \
    doortrapped(x, y, mon, body, act, 0)
#define postdoortrapped(x, y, mon, body, act) \
    doortrapped(x, y, mon, body, act, 1)

#endif /* TRAP_H */
