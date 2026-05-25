

#define MCF_NONE     0x0000
#define MCF_INDIRECT 0x0001 /* untargeted/indirect spell */
#define MCF_SIGHT    0x0002 /* monster needs to see hero */
#define MCF_HOSTILE  0x0004 /* cast by hostile monsters only */

#if defined(MCASTU_ENUM)
#define MONSPELL(def, lvl, flags) MCAST_##def
#elif defined(MCASTU_INIT)
#define MONSPELL(def, lvl, flags) { lvl, flags }
#elif defined(DUMP_MCASTU_ENUM1)
#define MONSPELL(def, lvl, flags) MCAST_DUMPENUM_##def
#elif defined(DUMP_MCASTU_ENUM2)
#define MONSPELL(def, lvl, flags) { MCAST_DUMPENUM_##def, #def }
#endif

MONSPELL(PSI_BOLT,     0, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(OPEN_WOUNDS,  0, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(CURE_SELF,    1, MCF_INDIRECT),
MONSPELL(HASTE_SELF,   2, MCF_INDIRECT),
MONSPELL(CONFUSE_YOU,  2, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(STUN_YOU,     3, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(DISAPPEAR,    4, MCF_INDIRECT),
MONSPELL(PARALYZE,     4, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(BLIND_YOU,    6, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(WEAKEN_YOU,   6, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(DESTRY_ARMR,  8, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(INSECTS,      8, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
MONSPELL(CURSE_ITEMS, 10, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(LIGHTNING,   11, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(FIRE_PILLAR, 12, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(GEYSER,      13, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(AGGRAVATION, 13, MCF_INDIRECT|MCF_HOSTILE|MCF_SIGHT),
MONSPELL(SUMMON_MONS, 15, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
MONSPELL(CLONE_WIZ,   18, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
MONSPELL(DEATH_TOUCH, 20, MCF_HOSTILE|MCF_SIGHT),
/* xNetHack-added monster spells, currently all for bosses; level doesn't really
 * matter since they are used in special spell lists and not the wizard/clerical
 * spell lists, and only certain monsters can cast each */
MONSPELL(ENTOMB,          30, MCF_HOSTILE|MCF_INDIRECT),
MONSPELL(TPORT_AWAY,  30, MCF_INDIRECT),
MONSPELL(DARK_SPEECH, 30, MCF_HOSTILE|MCF_INDIRECT|MCF_SIGHT),
/* sheer cold is for Asmodeus - emulates an AT_MAGC AD_COLD attack
 * note that if SHEER_COLD were to be MCF_INDIRECT, it could be cast at range,
 * but would deal 0 damage, because of the castmu code that sets damage to 0 if
 * !foundyou, which it is when castmu is called from monmove. This might be
 * possible to kludge around. */
MONSPELL(SHEER_COLD,  30, MCF_HOSTILE|MCF_SIGHT),
MONSPELL(BLIGHT,      30, MCF_HOSTILE),
MONSPELL(DISENCHANT,  30, MCF_HOSTILE),

#undef MONSPELL
