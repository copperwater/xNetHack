/* NetHack 3.7	mondata.h	$NHDT-Date: 1703845738 2023/12/29 10:28:58 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.63 $ */
/* Copyright (c) 1989 Mike Threepoint                             */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MONDATA_H
#define MONDATA_H

/* The macros in here take a permonst * as an argument */

#define monsndx(ptr) ((ptr)->pmidx)
#define verysmall(ptr) ((ptr)->msize < MZ_SMALL)
#define bigmonst(ptr) ((ptr)->msize >= MZ_LARGE)

#define pm_resistance(ptr, typ) (((ptr)->mresists & (typ)) != 0)

/* for MR2 stuff, the mresists part of mon_resistancebits is 0 because it's a
 * uchar */
#define has_telepathy(mon) \
    (telepathic((mon)->data) || (mon_resistancebits(mon) & MR2_TELEPATHY) != 0)

/* as of 3.2.0:  gray dragons, Angels, Oracle, Yeenoghu
 * this was moved out of resists_magm so monster lookup can show it */
#define resists_mgc(ptr) \
    (dmgtype(ptr, AD_MAGM) || ptr == &mons[PM_BABY_GRAY_DRAGON] \
     || dmgtype(ptr, AD_RBRE)) /* Tiamat */

/* similarly, this was moved out of resists_drli for monster lookup */
#define resists_drain(ptr) \
    (is_undead(ptr) || is_demon(ptr) || is_were(ptr) \
     || ptr == &mons[PM_DEATH])
/* is_were() doesn't handle hero in human form */

#define immune_poisongas(ptr) ((ptr) == &mons[PM_HEZROU]        \
                               || (ptr) == &mons[PM_VROCK])

#define is_flyer(ptr) (((ptr)->mflags1 & M1_FLY) != 0L)
#define is_floater(ptr) ((ptr)->mlet == S_EYE || (ptr)->mlet == S_LIGHT)
/* clinger: piercers, mimics, wumpus -- generally don't fall down holes */
#define is_clinger(ptr) (((ptr)->mflags1 & M1_CLING) != 0L)
#define grounded(ptr) (!is_flyer(ptr) && !is_floater(ptr) \
                       && (!is_clinger(ptr) || !has_ceiling(&u.uz)))
#define is_swimmer(ptr) (((ptr)->mflags1 & M1_SWIM) != 0L)
#define breathless(ptr) (((ptr)->mflags1 & M1_BREATHLESS) != 0L)
#define amphibious(ptr) (((ptr)->mflags1 & M1_AMPHIBIOUS) != 0L)
#define cant_drown(ptr) (is_swimmer(ptr) || amphibious(ptr) || breathless(ptr))
#define passes_walls(ptr) (((ptr)->mflags1 & M1_WALLWALK) != 0L)
#define amorphous(ptr) (((ptr)->mflags1 & M1_AMORPHOUS) != 0L)
#define noncorporeal(ptr) (((ptr)->mflags3 & M3_NONCORPOREAL) != 0L)
#define tunnels(ptr) (((ptr)->mflags1 & M1_TUNNEL) != 0L)
#define needspick(ptr) (((ptr)->mflags1 & M1_NEEDPICK) != 0L)
/* hides_under() requires an object or appropriate terrain at the location in
 * order to hide */
#define hides_under(ptr) (((ptr)->mflags1 & M1_CONCEAL) != 0L)
/* is_hider() is True for mimics but when hiding they appear as something
   else rather than become mon->mundetected, so use is_hider() with care */
#define is_hider(ptr) (((ptr)->mflags1 & M1_HIDE) != 0L)
/* piercers cling to the ceiling; lurkers above are hiders but they fly
   so aren't classified as clingers; unfortunately mimics are classified
   as both hiders and clingers but have nothing to do with ceilings;
   wumpuses (not wumpi :-) cling but aren't hiders */
#define ceiling_hider(ptr) \
    (is_hider(ptr) && ((is_clinger(ptr) && (ptr)->mlet != S_MIMIC) \
                       || is_flyer(ptr))) /* lurker above */
#define haseyes(ptr) (((ptr)->mflags1 & M1_NOEYES) == 0L)
/* used to decide whether plural applies so no need for 'more than 2' */
#define eyecount(ptr) \
    (!haseyes(ptr) ? 0                                                     \
     : ((ptr) == &mons[PM_CYCLOPS] || (ptr) == &mons[PM_FLOATING_EYE]) ? 1 \
       : 2)
#define nohands(ptr) (((ptr)->mflags1 & M1_NOHANDS) != 0L)
#define nolimbs(ptr) (((ptr)->mflags1 & M1_NOLIMBS) == M1_NOLIMBS)
#define notake(ptr) (((ptr)->mflags1 & M1_NOTAKE) != 0L)
#define has_head(ptr) (((ptr)->mflags1 & M1_NOHEAD) == 0L)
#define has_horns(ptr) (num_horns(ptr) > 0)
#define is_whirly(ptr) \
    ((ptr)->mlet == S_VORTEX || (ptr) == &mons[PM_AIR_ELEMENTAL])
#define flaming(ptr)                                                     \
    ((ptr) == &mons[PM_FIRE_VORTEX] || (ptr) == &mons[PM_FLAMING_SPHERE] \
     || (ptr) == &mons[PM_FIRE_ELEMENTAL] || (ptr) == &mons[PM_SALAMANDER])
#define is_silent(ptr) ((ptr)->msound == MS_SILENT)
#define unsolid(ptr) (((ptr)->mflags1 & M1_UNSOLID) != 0L)
#define mindless(ptr) (((ptr)->mflags1 & M1_MINDLESS) != 0L)
#define humanoid(ptr) (((ptr)->mflags1 & M1_HUMANOID) != 0L)
#define is_animal(ptr) (((ptr)->mflags1 & M1_ANIMAL) != 0L)
#define slithy(ptr) (((ptr)->mflags1 & M1_SLITHY) != 0L)
#define is_wooden(ptr) ((ptr) == &mons[PM_WOOD_GOLEM])
#define thick_skinned(ptr) (((ptr)->mflags1 & M1_THICK_HIDE) != 0L)
#define hug_throttles(ptr) ((ptr) == &mons[PM_ROPE_GOLEM])
#define digests(ptr) \
    (dmgtype_fromattack((ptr), AD_DGST, AT_ENGL) != 0) /* purple w*/
#define enfolds(ptr) \
    (dmgtype_fromattack((ptr), AD_WRAP, AT_ENGL) != 0) /* 't' */
#define slimeproof(ptr) \
    ((ptr) == &mons[PM_GREEN_SLIME] || flaming(ptr) || noncorporeal(ptr))
#define lays_eggs(ptr) (((ptr)->mflags1 & M1_OVIPAROUS) != 0L)
#define eggs_in_water(ptr) \
    (lays_eggs(ptr) && (ptr)->mlet == S_EEL && is_swimmer(ptr))
#define regenerates(ptr) (((ptr)->mflags1 & M1_REGEN) != 0L)
#define perceives(ptr) (((ptr)->mflags1 & M1_SEE_INVIS) != 0L)
#define can_teleport(ptr) (((ptr)->mflags1 & M1_TPORT) != 0L)
#define control_teleport(ptr) (((ptr)->mflags1 & M1_TPORT_CNTRL) != 0L)
#define telepathic(ptr)                                                   \
    ((ptr) == &mons[PM_FLOATING_EYE] || (ptr) == &mons[PM_MIND_FLAYER]    \
     || (ptr) == &mons[PM_MASTER_MIND_FLAYER]                             \
     || (ptr) == &mons[PM_ORC_SHAMAN] || (ptr) == &mons[PM_KOBOLD_SHAMAN] \
     || (ptr) == &mons[PM_GNOMISH_WIZARD])
#define is_armed(ptr) attacktype(ptr, AT_WEAP)
#define acidic(ptr) (((ptr)->mflags1 & M1_ACID) != 0L)
#define poisonous(ptr) (((ptr)->mflags1 & M1_POIS) != 0L)
#define carnivorous(ptr) (((ptr)->mflags1 & M1_CARNIVORE) != 0L)
#define herbivorous(ptr) (((ptr)->mflags1 & M1_HERBIVORE) != 0L)
#define metallivorous(ptr) (((ptr)->mflags1 & M1_METALLIVORE) != 0L)
#define lithivorous(ptr) ((ptr) == &mons[PM_ROCK_MOLE])
#define polyok(ptr) (((ptr)->mflags2 & M2_NOPOLY) == 0L)
#define is_shapeshifter(ptr) (((ptr)->mflags2 & M2_SHAPESHIFTER) != 0L)
#define is_undead(ptr) (((ptr)->mflags2 & M2_UNDEAD) != 0L)
#define is_were(ptr) (((ptr)->mflags2 & M2_WERE) != 0L)
#define is_elf(ptr) (((ptr)->mflags2 & M2_ELF) != 0L)
#define is_dwarf(ptr) (((ptr)->mflags2 & M2_DWARF) != 0L)
#define is_gnome(ptr) (((ptr)->mflags2 & M2_GNOME) != 0L)
#define is_orc(ptr) (((ptr)->mflags2 & M2_ORC) != 0L)
#define is_human(ptr) (((ptr)->mflags2 & M2_HUMAN) != 0L)
#define your_race(ptr) (((ptr)->mflags2 & gu.urace.selfmask) != 0L)
#define is_bat(ptr)                                         \
    ((ptr) == &mons[PM_BAT] || (ptr) == &mons[PM_GIANT_BAT] \
     || (ptr) == &mons[PM_VAMPIRE_BAT])
#define is_bee(ptr) ((ptr) == &mons[PM_KILLER_BEE] \
     || (ptr) == &mons[PM_QUEEN_BEE])
#define is_bird(ptr) ((ptr)->mlet == S_BAT && !is_bat(ptr))
/* Jabberwocks not considered to have beaks because they have "jaws". */
#define has_beak(ptr) (is_bird(ptr) || (ptr) == &mons[PM_TENGU] || \
                       (ptr) == &mons[PM_VROCK])
#define is_giant(ptr) (((ptr)->mflags2 & M2_GIANT) != 0L)
#define is_golem(ptr) ((ptr)->mlet == S_GOLEM)
#define is_rat(ptr)                                         \
    (((ptr) == &mons[PM_SEWER_RAT]) || ((ptr) == &mons[PM_GIANT_RAT]) || \
     ((ptr) == &mons[PM_RABID_RAT]) || ((ptr) == &mons[PM_WERERAT]) || \
     ((ptr) == &mons[PM_HUMAN_WERERAT]))
#define is_zombie(ptr) \
    ((ptr)->mlet == S_ZOMBIE && strstri((ptr)->pmnames[NEUTRAL], "zombie"))
#define is_domestic(ptr) (((ptr)->mflags2 & M2_DOMESTIC) != 0L)
#define is_demon(ptr) (((ptr)->mflags2 & M2_DEMON) != 0L)
#define is_mercenary(ptr) (((ptr)->mflags2 & M2_MERC) != 0L)
#define is_male(ptr) (((ptr)->mflags2 & M2_MALE) != 0L)
#define is_female(ptr) (((ptr)->mflags2 & M2_FEMALE) != 0L)
#define is_neuter(ptr) (((ptr)->mflags2 & M2_NEUTER) != 0L)
#define is_wanderer(ptr) (((ptr)->mflags2 & M2_WANDER) != 0L)
#define always_hostile(ptr) (((ptr)->mflags2 & M2_HOSTILE) != 0L)
#define always_peaceful(ptr) (((ptr)->mflags2 & M2_PEACEFUL) != 0L)
#define race_hostile(ptr) (((ptr)->mflags2 & gu.urace.hatemask) != 0L)
#define race_peaceful(ptr) (((ptr)->mflags2 & gu.urace.lovemask) != 0L)
#define extra_nasty(ptr) (((ptr)->mflags2 & M2_NASTY) != 0L)
#define strongmonst(ptr) (((ptr)->mflags2 & M2_STRONG) != 0L)
#define can_breathe(ptr) attacktype(ptr, AT_BREA)
#define cantwield(ptr) (nohands(ptr) || verysmall(ptr))
/* Does this type of monster have multiple weapon attacks?  If so,
   hero poly'd into this form can use two-weapon combat.  It used
   to just check mattk[1] and assume mattk[0], which was suitable
   for mons[] at the time but somewhat fragile.  This is more robust
   without going to the extreme of checking all six slots. */
#define could_twoweap(ptr) \
    ((  ((ptr)->mattk[0].aatyp == AT_WEAP)              \
      + ((ptr)->mattk[1].aatyp == AT_WEAP)              \
      + ((ptr)->mattk[2].aatyp == AT_WEAP)  ) > 1)
#define cantweararm(ptr) (breakarm(ptr) || sliparm(ptr))
#define throws_rocks(ptr) (((ptr)->mflags2 & M2_ROCKTHROW) != 0L)
#define type_is_pname(ptr) (((ptr)->mflags2 & M2_PNAME) != 0L)
#define is_lord(ptr) (((ptr)->mflags2 & M2_LORD) != 0L)
#define is_prince(ptr) (((ptr)->mflags2 & M2_PRINCE) != 0L)
#define is_ndemon(ptr) \
    (is_demon(ptr) && (((ptr)->mflags2 & (M2_LORD | M2_PRINCE)) == 0L))
#define is_dlord(ptr) (is_demon(ptr) && is_lord(ptr))
#define is_dprince(ptr) (is_demon(ptr) && is_prince(ptr))
#define is_archfiend(ptr) (is_dlord(ptr) || is_dprince(ptr))
#define is_minion(ptr) (((ptr)->mflags2 & M2_MINION) != 0L)
#define likes_gold(ptr) (((ptr)->mflags2 & M2_GREEDY) != 0L)
#define likes_gems(ptr) (((ptr)->mflags2 & M2_JEWELS) != 0L)
#define likes_objs(ptr) (((ptr)->mflags2 & M2_COLLECT) != 0L || is_armed(ptr))
#define likes_magic(ptr) (((ptr)->mflags2 & M2_MAGIC) != 0L)
#define webmaker(ptr) \
    ((ptr) == &mons[PM_CAVE_SPIDER] || (ptr) == &mons[PM_GIANT_SPIDER])
#define is_unicorn(ptr) ((ptr)->mlet == S_UNICORN && likes_gems(ptr))
#define is_longworm(ptr)                                                   \
    (((ptr) == &mons[PM_BABY_LONG_WORM]) || ((ptr) == &mons[PM_LONG_WORM]) \
     || ((ptr) == &mons[PM_LONG_WORM_TAIL]))
#define is_covetous(ptr) (((ptr)->mflags3 & M3_COVETOUS))
#define infravision(ptr) (((ptr)->mflags3 & M3_INFRAVISION))
#define infravisible(ptr) (((ptr)->mflags3 & M3_INFRAVISIBLE))
#define is_displacer(ptr) (((ptr)->mflags3 & M3_DISPLACES) != 0L)
#define is_mplayer(ptr) \
    (((ptr) >= &mons[PM_ARCHEOLOGIST]) && ((ptr) <= &mons[PM_WIZARD]))
#define is_watch(ptr) \
    ((ptr) == &mons[PM_WATCHMAN] || (ptr) == &mons[PM_WATCH_CAPTAIN])
#define is_rider(ptr)                                      \
    ((ptr) == &mons[PM_DEATH] || (ptr) == &mons[PM_FAMINE] \
     || (ptr) == &mons[PM_PESTILENCE])
/* note: placeholder monsters are used for corpses of zombies and mummies;
   PM_DWARF and PM_GNOME are normal monsters, not placeholders */
#define is_placeholder(ptr)                             \
    ((ptr) == &mons[PM_ORC] || (ptr) == &mons[PM_GIANT] \
     || (ptr) == &mons[PM_ELF] || (ptr) == &mons[PM_HUMAN])
/* return TRUE if the monster tends to revive */
#define is_reviver(ptr) (is_rider(ptr) || is_zombie(ptr) \
                         || (ptr)->mlet == S_TROLL)
/* monsters whose corpses and statues need special handling;
   note that high priests and the Wizard of Yendor are flagged
   as unique even though they really aren't; that's ok here */
#define unique_corpstat(ptr) (((ptr)->geno & G_UNIQ) != 0)

#define likes_lava(ptr) \
    (ptr == &mons[PM_FIRE_ELEMENTAL] || ptr == &mons[PM_SALAMANDER])
#define pm_invisible(ptr) \
    ((ptr) == &mons[PM_STALKER] || (ptr) == &mons[PM_BLACK_LIGHT])

/* could probably add more */
#define likes_fire(ptr)                                                  \
    ((ptr) == &mons[PM_FIRE_VORTEX] || (ptr) == &mons[PM_FLAMING_SPHERE] \
     || likes_lava(ptr))

#define touch_petrifies(ptr) \
    ((ptr) == &mons[PM_COCKATRICE] || (ptr) == &mons[PM_CHICKATRICE])
/* Medusa doesn't pass touch_petrifies() but does petrify if eaten */
#define flesh_petrifies(pm) (touch_petrifies(pm) || (pm) == &mons[PM_MEDUSA])

/* missiles made of rocks don't harm these: xorns and earth elementals
   (but not ghosts and shades because that would impact all missile use
   and also require an exception for blessed rocks/gems/boulders) */
#define passes_rocks(ptr) (passes_walls(ptr) && !unsolid(ptr))

#define is_mind_flayer(ptr) \
    ((ptr) == &mons[PM_MIND_FLAYER] || (ptr) == &mons[PM_MASTER_MIND_FLAYER])

#define is_vampire(ptr) ((ptr)->mlet == S_VAMPIRE)

#define hates_light(ptr) ((ptr) == &mons[PM_GREMLIN])

/* used to vary a few messages; also nonliving monsters don't get life-saved */
#define weirdnonliving(ptr) (is_golem(ptr) || (ptr)->mlet == S_VORTEX)
#define nonliving(ptr) \
    (is_undead(ptr) || (ptr) == &mons[PM_MANES] || weirdnonliving(ptr))

/* no corpse (ie, blank scrolls) if killed by fire; special case instakill  */
#define completelyburns(ptr) \
    ((ptr) == &mons[PM_PAPER_GOLEM] || (ptr) == &mons[PM_STRAW_GOLEM])
#define completelyrots(ptr) \
    ((ptr) == &mons[PM_WOOD_GOLEM] || (ptr) == &mons[PM_LEATHER_GOLEM] \
     || (ptr) == &mons[PM_PAPER_GOLEM] || (ptr) == &mons[PM_STRAW_GOLEM])
#define completelyrusts(ptr) ((ptr) == &mons[PM_IRON_GOLEM])

/* Used for conduct with corpses, tins, and digestion attacks */
/* G_NOCORPSE monsters might still be swallowed as a purple worm */
/* Maybe someday this could be in mflags... */
#define vegan(ptr)                                                 \
    ((ptr)->mlet == S_BLOB || (ptr)->mlet == S_JELLY               \
     || (ptr)->mlet == S_FUNGUS || (ptr)->mlet == S_VORTEX         \
     || (ptr)->mlet == S_LIGHT                                     \
     || ((ptr)->mlet == S_ELEMENTAL && (ptr) != &mons[PM_STALKER]) \
     || ((ptr)->mlet == S_GOLEM && (ptr) != &mons[PM_FLESH_GOLEM]  \
         && (ptr) != &mons[PM_LEATHER_GOLEM]) || noncorporeal(ptr))
#define vegetarian(ptr) \
    (vegan(ptr)         \
     || ((ptr)->mlet == S_PUDDING && (ptr) != &mons[PM_BLACK_PUDDING]))

#define corpse_eater(ptr)                    \
    (ptr == &mons[PM_PURPLE_WORM]            \
     || ptr == &mons[PM_BABY_PURPLE_WORM]    \
     || ptr == &mons[PM_GHOUL]               \
     || ptr == &mons[PM_PIRANHA])

/* monkeys are tamable via bananas but not pacifiable via food,
   otherwise their theft attack could be nullified too easily;
   dogs and cats can be tamed by anything they like to eat and are
   pacified by any other food;
   horses can be tamed by always-veggy food or lichen corpses but
   not tamed or pacified by other corpses or tins of veggy critters */
#define befriend_with_obj(ptr, obj) \
    (((ptr) == &mons[PM_MONKEY] || (ptr) == &mons[PM_APE])               \
     ? (obj)->otyp == BANANA                                             \
     : (is_domestic(ptr) && (obj)->oclass == FOOD_CLASS                  \
        && ((ptr)->mlet != S_UNICORN                                     \
            || obj->material == VEGGY                 \
            || ((obj)->otyp == CORPSE && (obj)->corpsenm == PM_LICHEN))))

#ifdef PMNAME_MACROS
#define pmname(ptr,g) ((((g) == MALE || (g) == FEMALE) && (ptr)->pmnames[g]) \
                        ? (ptr)->pmnames[g] : (ptr)->pmnames[NEUTRAL])
#endif
#define monsym(ptr) (def_monsyms[(int) (ptr)->mlet].sym)

/* Wielding and opening doors use the same flags: handed and not verysmall.
   Exception: zombies cannot open doors.
*/
#define can_open_doors(ptr) (!cantwield(ptr) && !is_zombie(ptr))

/* Noise that a monster makes when engaged in combat. Assume that vocalizations
 * account for some noise, so monsters capable of vocalizing make more.
 * This gets used as an argument to wake_nearto, which expects a squared value,
 * so we square the result. */
#define combat_noise(ptr) \
    ((ptr)->msound ? ((ptr)->msize*2 + 1) * ((ptr)->msize*2 + 1) \
                   : ((ptr)->msize + 1)   * ((ptr)->msize + 1))

/* The monster prefers to keep its distance rather than charging and engaging
 * you in combat. This was adapted from the M3_SKITTISH flag in SporkHack;
 * however, since only a couple monsters have this behavior, it isn't necessary
 * to add a M3 flag. */
#define keeps_distance(ptr)                                  \
    ((ptr)->mlet == S_CENTAUR                                \
     || ((ptr) == &mons[PM_SCHLIEMANN] && !u.uhave.questart) \
     || ((ptr) == &mons[PM_DISPATER]))

/* The monster is covetous, but should not warp, heal, or otherwise use
 * tactics(). */
#define covetous_nonwarper(ptr)       \
    ((ptr) == &mons[PM_SCHLIEMANN]    \
     || is_archfiend(ptr))

/* The monster knows the difference between a valuable gem and worthless glass.
 * (This is not an innate ability, so player polymorphed into them will not get
 * to identify gems.) */
#define knows_valuable_gems(ptr) \
    ((ptr) == &mons[PM_DWARF_RULER] || (ptr) == &mons[PM_GNOME_RULER])

/* The monster is primarily made of water */
#define is_made_of_water(ptr) \
    ((ptr) == &mons[PM_WATER_ELEMENTAL] || (ptr) == &mons[PM_WATER_TROLL] \
     || (ptr) == &mons[PM_FOG_CLOUD] || (ptr) == &mons[PM_STEAM_VORTEX])

#endif /* MONDATA_H */
