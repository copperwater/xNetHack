## xNetHack 6.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 5.1, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack is d564fa8. Note that
this contains some major changes including new monsters, new objects, themed
rooms, and other things *not* documented in this file. See doc/fixes37.0 for
the devteam's changes.

The xNetHack page at the NetHackWiki, https://nethackwiki.com/wiki/XNetHack,
attempts to describe these changes in a way that's better formatted and more
friendly to players. However, the wiki page might be out of date; in case of
conflicting information, this changelog and others in this directory are more
up-to-date than the wiki page, and the commit messages are more up-to-date than
this changelog.

On top of any changes made by the NetHack devteam on 3.7, and any changes
made in previous xNetHack versions, xNetHack 6.0 contains the following
changes:

### Gameplay changes

- The geyser spell will rust armor and instakill you if you are an iron golem.
- Vault guards grudge leprechauns.
- Vampires grudge werewolves and vice versa.
- Potions of acid no longer freeze when hit with cold damage.
- Add two new random wand appearances "gold" and "gilded". Both are made out of
  gold and can therefore be stolen by leprechauns.
- All nymphs are now herbivorous (mainly meaning that pet nymphs will eat veggy
  food and polymorphing into one does not make you inediate).
- Salamanders, all Kops, and all corpse-leaving i-class monsters (imps, tengu,
  quasits, homunculi, and leprechauns) are now carnivorous. This mainly means
  that they are capable of eating meat when tamed and prefer it over veggy
  food, and polymorphing into one does not make you inediate.
- Ghosts cannot be renamed.
- Reading a scroll of identify while confused will "identify yourself" and give
  enlightenment.
- Port L's Cursed Wand Backfire Patch: zapping a cursed directional wand will
  cause it to backfire and zap you 10% of the time. This is independent of the
  1% chance that a cursed wand will blow up upon use.
- Martial arts users, sasquatches, and players wearing kicking boots can no
  longer miss a monster completely with a clumsy kick.
- Statues hit by a wand of opening will drop their contents without breaking.
- Watch captains generate with a key.
- The Gnomish Sewer now contains a couple figurines of jellies.
- The Gnomish Sewer's land spaces are now exclusively room spaces ("."); all
  corridor spaces ("#") have been replaced with them. The land/water/stone
  layout of the map is not changed.
- Genetic engineers (added in vanilla 3.7.0) are in the U monster class like
  quantum mechanics are.
- Attempting to use a crystal ball while hallucinating will paralyze you for d2
  turns if the ball is uncharged or d4 turns if it is charged. (This is the
  only part of the vanilla 3.7.0 crystal ball overhaul that has been included
  in xNetHack.)
- Monsters prefer to wear an amulet of life saving or reflection rather than an
  amulet of ESP or guarding.
- Metal armor won't provide less than 1 AC (not counting enchantments) if it
  has a decreased AC from being a different material. For instance, copper
  gauntlets of power will provide 1 AC instead of 0.
- Some themed rooms have minimum and maximum level difficulty cutoffs. Notably,
  the spider room will not appear above level 10.
- The "Fake Delphi" themed room now has one fountain in its center.
- 27 new themed rooms:
  - A cluster of four square interconnected 3x3 rooms.
  - A barbell-shaped room connecting two 3x3 rooms with a doored, walled
    corridor (vertical and horizontal).
  - A room containing a bunch of graffiti.
  - A room filled with small 1x1 pillars. Sometimes, these pillars will be
    trees instead, and then the room will contain wood nymphs, and possibly
    statues and a figurine of wood nymphs. This only happens at depths at or
    above wood nymphs' difficulty.
  - A boomerang-shaped room (left and right rotations).
  - A 1-wide rectangular corridor (surrounding a chunk of normal stone).
  - A room featuring an O, M, Z, or T monster in a cage of iron bars.
    It can also be a D below level 12. From EvilHack.
  - A room split horizontally or vertically into two subrooms. From EvilHack.
  - A room containing a 2x2 subroom in any corner. From EvilHack.
  - A vault containing 1-2 chests with random contents, not connected to the
    rest of the dungeon like a gold vault.
  - A room containing an X of water connecting its corners.
  - A miniature 7x7 maze.
  - A horizontal corridor with a bunch of closets along the top and bottom,
    some of them containing monsters or objects.
  - A wide room containing 1 to 3 2x2 beehives.
  - A large honeycomb-shaped room with randomly connected cells, which is a
    beehive.
  - A room where each square becomes cloud, ice, grass, lava, pool, or tree
    with a 30% chance. From UnNetHack.
  - A room containing a ring of floor tiles surrounding a "swimming pool"
    filled with water, which contains a few random sea monsters and possibly a
    chest with gold and gems. (Unlike the Gehennom submerged special room,
    there is no magic lamp.)
  - A long, narrow (height only 1 or 2) room. From UnNetHack.
  - A room populated with several random F, b, j, P and gas spore monsters.
    From UnNetHack.
  - Ozymandias' Tomb, a 7x7 room with a throne, some traps, a couple statue
    traps and an engraving. Only appears deep in the dungeon. From UnNetHack.
  - A room filled mostly or entirely with gas spores, which should prove
    extremely fun to blow up.
  - A room imitating the Pool challenge from the /dev/null/nethack tournament,
    a 9x9 room with six holes for "pockets" and nine numbered boulders. (It
    does not contain a way to launch the boulders or bounce them off each
    other.)
  - A room containing two 3x3 shops, one weapons and one armor, with an aisle
    around them.
  - A four-way circle-and-cross room, possibly containing a central tree or
    fountain.
  - Four square 3x3 rooms, but all directly connected as part of one larger
    room instead of separated by corridors.
  - A prison cell in the corner of a room, possibly containing a prisoner and
    rat.
  - A large room containing some type of obstacle terrain in a vertically
    symmetric shape.
- If you ride a steed into a polymorph trap, unchanging will not protect the
  steed (but magic resistance still will).
- Canceled or beheaded zombies or trolls don't rise from the dead.
- You can opt to cast the non-advanced version of fireball or cone of cold if
  you are Skilled or above in attack spells. If you do cast the non-advanced
  version, the energy cost is reduced by up to half.
- You can no longer be permanently expelled from the quest. Doing the things
  that got you permanently expelled still expel you, but temporarily, and the
  leader will get mad if you return.
- You can enter the quest if your quest leader is dead.
- When your quest leader is dead, your Luck will time out to -4 (regardless of
  Friday the 13th or a full moon), good luck always times out for you, and bad
  luck never times out for you, regardless of any luck items you are carrying.
  Luckstones continue to be able to provide +3 or -3 luck.
- Directly killing your quest leader sets your Luck to its minimum and
  increments god anger by 7 in addition to its huge alignment penalty.
- You only gain an alignment bonus for killing your quest nemesis if your quest
  leader is still alive.
- Directly killing a quest guardian decreases your Luck by 4 and increments god
  anger by 1 in addition to its sizeable alignment penalty.
- Monsters will ignore Elbereth and scare monster scrolls if you are generating
  conflict.
- A cursed amulet of life saving only works 50% of the time.
- Leather items can no longer generate as paper.
- Cloth weapon and armor items and towels can no longer generate as paper.
- More tools are made eligible for material randomization; in particular, bags
  can now be leather, plastic, or paper. Leashes, saddles, and blindfolds can
  also now get different materials.
- Orcish items can no longer generate as plastic or glass, but can now generate
  as stone.
- Nazgul are ungenocideable.
- In wizard mode, you can choose to teleport into locations that you can't
  teleport into in normal play (e.g. into a wall or across no-teleport zones).
- In wizard mode, a message prints when you teleport on a non-teleport level.
- Grass is restored to the Barbarian quest goal level.
- Undead iron-haters do not recoil from the touch of iron.
- Wood and paper golems instakilled by Fire Brand do not drop items.
- A lot of the doors in Fort Ludios are now iron.
- All A-class monsters are now immune to death magic.
- Couatls and ki-rin are now shock, sleep, and poison resistant. Ki-rin are
  additionally cold resistant.
- Mindless monsters and werewolves are now sleep resistant.
- Randomly named weapons can now use names from the top 10 list.
- Creating Excalibur via fountain will set its enchantment to +0 if it was
  negative, and obtaining it via crowning will set its enchantment to +3 if it
  was lower than that.
- Tame V-class monsters won't shapeshift from vampire form into something else.
- Potion vapor effects are buffed:
  - Full healing vapors restore 13 HP, extra healing 3, and healing 1.
  - Booze vapors no longer do anything except cause you to smell alcohol.
  - Confusion confuses you for 5+d10 turns.
  - Sleeping makes you fall asleep for 5+d5 turns.
  - Speed gives you fast speed for d10 turns.
  - Blindness and hallucination make you blind/hallucinating for 20+d20 turns.
- Monsters can now generate with a potion of hallucination as an offensive
  item, and will throw it at the player. They generate with a potion of
  sleeping a little less often.
- Monsters can displace peacefuls out of their way like the player can, unless
  the peaceful is larger than them.
- Having unchanging while turning to slime only pauses the slime timer; it does
  not cancel the slime effect entirely. Unchanging conferred by dying and
  corrupting or polyinit mode does not pause sliming at all.
- You can determine the species of a non-stale egg by listening to it with a
  blessed stethoscope.
- Monsters will not wear objects made of a material they hate.
- Monsters will not equip gear when they think the player is close by.
- Monsters can no longer start equipping gear in zero turns after doing other
  actions.
- Cancelled thiefstones can be reactivated by dipping them in a non-cursed
  potion of restore ability. This will key them to the level and space where
  the player is currently standing.
- Monkeys get a short, one-time burst of speed after stealing an item.
- 1/40 of randomly generated erodable items will have some amount of erosion.
- Lieutenants count as lords, preventing them from getting bad gear and
  increasing their multishot by 1.
- Captains count as princes, giving them slightly better gear and increasing
  their multishot by 2.
- The chance of a falling piercer missing its target is now a Dex% chance for
  the player, and monster speed% chance for monsters.
- Monsters fighting the player in melee with weapon attacks will use ranged
  weapons if they have no melee weapon available.
- Candles, lamps, and other ignitable things can be ignited by sources of fire
  (typically in situations when other burnables also catch fire).
- Quasits are buffed:
  - Speed raised from 15 to 18.
  - Their bite attacks can drain 1d4 points of Dexterity (up from 1d2).
  - They generate in small groups.
  - 25% of them generate invisible.
  - They can see invisible.
  - Difficulty is raised from 7 to 9.
- The amount of backstab bonus Rogues can get is capped based on their skill
  level in the weapon they attack with, but also gains a flat bonus based on
  skill level.
- Rogues get backstab bonus on trapped, frozen, unmoving, sleeping, stunned,
  confused, and blinded monsters in addition to fleeing ones.
- Orcish Town's general store shopkeeper is replaced with Izchak.
- Dropping a ring of polymorph control into a sink causes it to change into
  another sink, which allows a new pudding and foocubus to be generated from it.
- Cleaver does not hit enemies in an arc if there is a known peaceful or tame
  monster at either of the two additional spots it hits.
- Grimtooth confers poison resistance when wielded.
- Piranha now have two 2d6 bite attacks instead of one. Their speed is raised
  from 12 to 18.
- The Big Room now appears (if it appears at all) on levels 13-15 instead of
  10-12.
- Kicking open secret doors and corridors is easier, and never causes you to
  hurt your legs.
- A tiny fraction of erodable items will generate greased.
- Dwarvish characters can always reach Skilled in pick-axe skill.
- Gnomish characters can always reach Basic in club and crossbow skill.
- Elvish characters can always reach Basic in enchantment spell skill.
- Orcish characters can always reach Skilled in scimitar skill.
- Flint stones and hard gems break 50% less often when used as projectiles due
  to hardness.
- Hobbits that generate with a sling will also generate with a stack of 4 + 1d6
  flint stones (75% of the time) or rocks (25% of the time).
- The caveperson role no longer starts with rocks. Instead, they now start with
  20 + 1d10 flint stones (formerly 10 + 1d10).
- Flint stones weigh 2 (up from previous weight of 1, but still lower than the
  10 they weigh in vanilla NetHack).
- The melee strength bonus is applied to ammunition thrown from a sling.
- Skilled and above slingers can instakill any H monster on a critical hit.
- Statues may generate near cockatrices that generate at level creation time
  and not in a special room.
- Tool shops now stock touchstones as 2% of items rather than 7%.
- Tool shops stock potions of oil as 3% of items.
- Yellow lights and fire elementals emit light in a radius of 2.
- Fruits generate with holiday-themed names for a number of different holidays.
- Mimics have additional special-cased things to mimic in special rooms.
- The scroll of stinking cloud can be centered at a little over 7 spaces away
  instead of slightly over 5. Strictly speaking, the radius is sqrt(50).
- Gas clouds block line-of-sight.
- Hallucination prevents petrification (because you are already stoned).
- Mordor orcs and Uruk-hai can generate with orcish spears.
- Any monster that generates with a spear can generate with a stack of them.
- If a monster dies to damage from a trap on the player's turn, the player is
  considered responsible.
- Pit fiends' hug attack now creates a pit underneath their target, followed by
  them hurling the target down into the pit. Sometimes a pit may be enlarged
  into a hole. Levitation and flying prevent getting trapped in the pit, but
  do not prevent damage dealt by being hurled down to the bottom.
- Anything that polymorphs into a snaky form that is nonhumanoid and not
  Gigantic will not break its armor. Instead, it will slither out of its armor.
- Chaotics no longer incur a -1 alignment record penalty for angering peaceful
  monsters.
- Chaotics no longer incur a -1 alignment record penalty for casting healing
  spells at their pets (but they still do for healing peacefuls).
- There are no alignment record adjustments at all for casting healing spells
  at a monster which is already at maximum HP.
- Summon nasties scales based on the summoner's level, not the player's, and
  always creates at least 3 monsters.
- Add L's Wounds patch, which allows healers to receive messages showing how
  wounded a monster is, and obtain the same information from farlook.
- When disarming a shooting trap, its ammo is placed directly into your
  inventory.
- Spheres' explosions are now real explosions, hitting the 8 squares around
  them and items on the floor in addition to their target. Note: real
  explosions can destroy items!
- When the player explodes as a sphere, their carried gear is safe from any
  elemental damage the explosion would normally cause.
- Flaming and freezing spheres are difficulty 9 (up from 8), and shocking
  spheres are difficulty 10 (up from 8).
- Digging out of an engulfer's stomach cuts its HP in half rather than setting
  it to 1. If the engulfer is amorphous, its HP is only cut by 25%. There's no
  longer a special case for unique engulfers (Juiblex counts under the
  amorphous rule).
- Getting lifesaved lowers current and maximum Con by 2, to a minimum of 3.
- Mold grows on corpses roughly 5% of the time rather than 50%.
- When a container is polymorphed, its contents are spilled out rather than
  silently deleted.
- Anaraxis the Black wears a cloak of magic resistance.
- Sokoban levels are not eligible to be flipped.
- Magic missile damage is no longer completely negated by magic resistance.
  Instead, it halves the damage.
- Half spell damage reduces the damage taken by magic missiles by half. This
  stacks with the halving from magic resistance.
- Half physical damage no longer applies when you zap yourself with a wand
  unless the wand is striking.
- Alchemic blasts deal twice as much damage ((# mixing potions + d9) * 2), but
  acid resistance halves this (bringing it back to its vanilla level).
- Vorpal Blade now deals +1d8 bonus damage instead of +1.
- When you kill the Wizard or perform the Invocation, the upper limit on which
  difficulty monsters can randomly generate is removed.
- The 3.6.1 change in which your Int+Cha is capped at 32 for the purposes of
  foocubus encounters is removed; there is no longer a cap.
- The denominator of the seduction success formula is no longer a constant 35;
  it is now 25 + 5 times the seducer's level, plus 20 if in Gehennom.
- Secret corridors never randomly generate except as part of some closets.
- Wood nymphs are slower than an unhasted player (10 speed).
- Water nymphs have a slightly increased base level (4) and difficulty (6).
- Mountain nymphs have an increased base level (5), speed (15), and difficulty
  (7).
- Being inside an abattoir can inflict you with nausea, with a Con-dependent
  chance. Entering an abattoir for the first time also triggers this with a
  higher chance.
- The items inside Vlad's Tower are no longer guaranteed (except the candles,
  which there will still be at least 8 of).
- If you pray while Weak, not carrying much food, and at critically low HP,
  your god will fix your HP rather than your hunger. (If you're Fainting and
  don't have enough food, they'll still fix your hunger first.)
- When evaluating if you're carrying enough food, your god will ignore any food
  you're carrying if you have maintained the foodless conduct.
- The Amulet now blocks teleport control instead of all intra-level teleports.
  (It still blocks 1/3 of intra-level teleports, as in vanilla.)
- Paper and straw golems (in addition to wood and leather golems) will rot and
  instantly die when hit with a decay attack.
- A wished-for thiefstone will be keyed to the spot the player is currently
  standing on.
- Blessed gain ability potions now increase a single attribute by 1-2 points
  (the player chooses which) instead of raising all by 1 point.
- Gain ability (uncursed or blessed) trying to raise an attribute randomly will
  always succeed provided there is something raiseable. "Innate improvement"
  attributes, e.g. strength while wearing gauntlets of power but when the base
  value isn't maxed yet, now count as raiseable.
- New Big Room "Tea Party", containing a number of obstacles.
- 8 polearms are removed: ranseur, spetum, bardiche, voulge, fauchard,
  guisarme, bill-guisarme, lucern hammer.
- The bec de corbin's damage stats are now those of the lucern hammer
  (2d4 vs small / d6 vs large) instead of its former d8/d6.
- Wielded rings can be enchanted with a scroll of enchant weapon, which has
  identical effects to a charging scroll of the same beatitude.
- Certain tools that have corresponding magical counterparts can be enchanted
  with a scroll of enchant weapon while wielded, turning them into their
  counterparts.
- The "dtsund-DSM" proposal is implemented, which replaces dragon scale mail
  with dragon-scaled armor of other types.
  - All dragon scale mail is removed and no longer exists as objects. Wishing
    for dragon scale mail now gives the corresponding set of dragon scales.
  - Dragon scales are now worn in the cloak slot. Their stats are unchanged:
    3 AC, 0 MC, providing the same extrinsics.
  - Reading enchant armor with scales equipped will always target the scales.
  - Enchanting scales with body armor underneath will meld the scales into the
    body armor, causing that piece of armor to provide the dragon extrinsic
    associated with the scales as well as gaining 3 AC.
    - The armor does not become erodeproof, but will become uneroded and
      blessed if the scroll is blessed.
    - This process does not change the enchantment of the armor.
    - If the scroll is cursed, the resulting scaled armor proceeds to lose a
      point of enchantment and become cursed, like normal for a cursed enchant
      armor scroll.
    - If you are confused, the scales will meld and then you will polymorph
      into the associated type of dragon (see below). This skips the cursed
      behavior.
    - Melding scales into armor that already has scales will replace the old
      color of scales with the new one.
  - Enchanting scales without body armor underneath causes a polymorph into the
    type of dragon matching the scales. Wearing a shirt does not affect this,
    but polymorph control allows you to choose whether you wish to become a
    dragon. A blessed scroll gives you a longer duration as a dragon and a
    cursed scroll gives you a shorter duration.
  - Reading a scroll of enchant armor while wearing a set of scales will always
    try to do something with that set of scales; it will not choose an armor
    piece randomly.
  - Merging with scaled armor while polymorphing does not unfuse the scales
    from the armor or cause any other sort of reversion; changing back will
    restore the armor as it was.
  - Scales' enchantment does not get transferred onto armor. It should not be
    possible to obtain a set of scales with some enchantment other than +0,
    apart from wishing for one.
  - Player monsters that would receive dragon scale mail now instead receive a
    standard body armor suit that is scaled with a random color of dragon
    scales.
  - Scaled armor cannot be wished for, it must be crafted.
- Worn armor weighs 75% of its listed weight.
- You can use #loot on adjacent pets to give them items and take items from
  them. They will reevaluate their gear choices if you give or take anything.
  Equipped items are valid selections; trying to take one will get the pet to
  take it off. In wizard mode, this can be done to anyone, not just pets.
- You can #rub a thiefstone on your hand as a gold golem in order to teleport
  yourself to your stash with it.
- Thiefstones no longer teleport themselves along with a gold golem that they
  steal away, unless you are a gold golem rubbing it on your own hand, in which
  case it stays with you.
- Thiefstones now can be used to teleport any gold item to their stash, not just
  coins.
- Revert some of version 3.0's monster letter and color changes:
  - Sea monsters moved back to ';'
  - Zruties reintroduced as 'z'
  - Lizards moved back to ':'
  - Leprechauns moved back to 'l'
  - Kop Sergeants/Lieutenants/Kaptains returned to blue/cyan/magenta
- Mummy wrappings are no longer eligible for object materials and are always
  cloth.

### Interface changes

- Light armor, studded armor, jackets, plain cloaks, and identified plain
  gloves will display their default leather material when made out of leather.
- Engravings now render as ~ (tilde) instead of " (quotation mark) by default.
- The Vibrating Square is colored yellow instead of purple.
- When water walking boots are identified by walking on liquid with them, a
  message is printed.
- The code is brought to parity with the interface improvements in the
  NHTangles/NetHack36 github repository, branch 3.7-hdf.
- Killing "player remnant" monsters (e.g. ghosts) from bones files is
  livelogged.
- Player remnant monsters cannot be renamed.
- The 'checkpoint' option now defaults to true.
- When you wear a cursed piece of gear, it will print a message about welding
  to your body and immediately become identified as cursed.
- Dying on a corridor or grass space now leaves a gravestone in the bones file.
- You can no longer push a boulder in the course of running or traveling.
- Monster and object lookup now print a heading which displays the monster or
  object that it is printing details for.
- Viewing an encyclopedia entry now prints the string that was used to find an
  entry in the encyclopedia and the string in data.base that matched it.
- The livelog for life saving prints the would-be cause of death.
- The #conduct command (and thus the dumplog) shows how many items you have
  polymorphed during your game, outside of wizard mode.
- Add encyclopedia entry for Ozymandias.
- If you have nothing quivered but are wielding a weapon that can return to
  your hand when thrown, the f (fire) command will choose that to throw (unless
  autoquiver is on and it finds something else to quiver).
- Riders now have unique messages for when they resurrect.
- A couple more major Oracle consultations.
- Monks now have role-specific hello/goodbye messages like several other roles.
- An offscreen pet death will make you feel "woebegone" if hallucinating.
- If you had a travel destination saved that you hadn't yet reached, a
  controlled teleport will suggest that destination at its prompt.
- Wielded weapons you are not at least Basic in are treated as non-weapons for
  the purpose of "You begin bashing" messages.
- When a monster wakes up, a message is printed indicating that, unless there
  is a more specific message already being shown.
- When you #loot while standing over a single container, and there is no
  adjacent monster for you to loot, it will automatically loot that container
  without prompting.
- Monster lookup additionally shows the base level of the monster.
- The xlogfile now contains a "polyinit=" field for all games, showing the
  monster species the player polyinitted as, or "none" if the game was not in
  polyinit mode.
- New SERVERSEED sysconf option, which is intended for server operators to
  obfuscate the values returned by the game's deterministic hash functions so
  players can't compute them.
- Conduct flags in the xlogfile are moved around (xNetHack flags now start at
  the highest bit and count down) and conductX is extended to include xNetHack
  conducts.
- Bonesless conduct is determined by not loading any bones files, rather than
  simply having the bones option off.
- The xlogfile "flags" field is rearranged (xNetHack flags now start at the
  highest bit and count down, and it no longer stores the number of bones).
- You can now set monster colors to recolor a monster species: either via
  "MONSTERCOLOR=species:color" in the configuration file, or via in-game
  options. They do not persist when the game is saved and restored, but
  configuration file changes will be reloaded upon restore.
- More hallucinatory monsters, shirts, epitaphs, random engravings, rumors,
  hallucinatory gods, hallucinatory blasts, tin labels, weapon names, and
  taunts.
- A number of new encyclopedia entries are added, mostly for things that vanilla
  did not have an entry for.

### Architectural changes

- tipcontainer() is generalized to work for spilling objects out of a container
  on some space other than the player' other than the player's.
- A bunch of enhancements, fixes, and refactors of the level parser, initially
  implemented as patches versus vanilla. In particular, themed room subrooms
  and special rooms can now be filled.
- Monsters created in a bones file to represent a dead player (ghost, mummy,
  etc) are tracked with a new "ebones" mextra struct.
- A new body part, TORSO, is added.
- Overhaul special_dmgval(); it now checks all possible sources of special
  damage and returns the one that had the highest damage roll.
- New function attack_contact_slots, which returns the worn gear slots which
  will come in contact with a monster being attacked.
- searmsg(), special_dmgval(), and mon_hates_material() are all
  xhity-compatible.
- polymon() now takes a second argument of a bitmask of flags, rather than a
  boolean, for finer-grained control of which types of messages it will print.
- New function invalid_obj_material(), used only by other object material
  functions to check for a specific otyp-to-material invalidity.
  valid_obj_material remains the main function accessible outside mkobj.c.
- New macro livelog_mon_nam(), which is a wrapper around x_monnam()
  specifically suppressing hallucination so that livelogs don't print a
  hallucinatory monster.
- New monster flag alias golem_destroyed, which is actually mrevived and
  indicates that the monster was completely wiped out and should drop no items
  from its body. Only should be used on monsters for which mrevived is never
  set.
- New function tt_name(), which returns a random name from the top 10 list.
- artifact_hit now returns a bitmask of two flags: one for having printed any
  special message (and the caller doesn't need to print a normal hit message),
  and one for specifically printing an instakill message (for the caller to
  suppress more messages in certain circumstances).
- Rearrange the logic in mm_displacement() into a series of if statements
  rather than one big if.
- New function autoreturning_wep(), an xhity-compatible function that
  determines if a thrown weapon should return to its user.
- The its_dead() function now takes a fourth argument, the stethoscope being
  used to listen to floor objects.
- New function can_slime_with_unchanging(), which determines whether you have a
  certain form of unchanging that should not impede turning to slime (such as
  polyinit mode).
- New function faulty_lifesaver(), which unifies the logic for deciding when an
  amulet of life saving will fail.
- New function check_gear_next_turn(), which sets a flag on a monster that it
  should reassess its worn equipment for anything better at the beginning of its
  next turn.
- New function piercer_hit(), an xhity-compatible function that handles what
  happens when a piercer attacks something by falling.
- The thrwmu function now returns boolean instead of void, used to determine
  whether the monster actually took any action.
- New function ignite_items, which makes items in an object chain (candles,
  lamps, etc) light on fire.
- New function adj_monnam(), which contains the logic from Adjmonnam but
  doesn't force the first character to be uppercase. Adjmonnam now calls this.
- New function should_cleave(), which determines whether there would be
  peaceful monsters in the path of Cleaver.
- Move will_weld() macro up to obj.h so that it can be used outside wield.c.
- Gas clouds are now capped at 150 squares (nothing that creates a cloud
  currently uses this much area).
- Convert the emits_light macro into a function in mondata.c.
- New function weekday(), which returns the day of the week in a 0-6 range.
- New function days_since_epoch(), which returns the number of days since
  1/1/0000.
- Externify the inside_region function.
- Externify the cant_wield_corpse function.
- New function create_pit_under(), an xhity-compatible function that has an
  aggressor create a pit beneath a target monster and throw them down into it.
- New function is_fleshy(), which determines if a monster has flesh or not.
- New function dump_container(), which contains the logic extracted from
  tipcontainer() that deals with actually dumping the items out on the floor.
  Various item-spillage bits of code now call this.
- Add nh.mon_difficulty() as a Lua function which returns the difficulty of a
  given monster species.
- Add door_into_nonjoined() which checks if a potential door location would
  connect to an area marked as non-joined.
- Unify the probabilities for random door generation between regular dungeon
  levels and other levels: how often they generate closed, trapped, iron, etc.
  into a new function random_door_mask().
- New function clear_nonsense_doortraps() to delete traps on doors where the
  trap type doesn't make any sense.
- Remove the rnddoor() function.
- retouch_object() now takes a third argument which indicates the caller thinks
  the object isn't in contact with your body and there should be no damage due
  to hating the material.
- New function armor_bonus(), which is converted from the ARM_BONUS macro.
- New function dragon_scales_color(), which returns just the color string from
  a set of dragon scales or dragon-scaled armor.
- armor_to_dragon() is externified and takes a monst instead of an otyp. It
  unifies player and monster logic of figuring out what dragon a creature
  should polymorph into based on what it's wearing.
- New function armor_provides_extrinsic(), which finds the extrinsic property a
  piece of dragon-scaled armor should provide.
- Define constants FIRST_DRAGON, LAST_DRAGON, FIRST_DRAGON_SCALES and
  LAST_DRAGON_SCALES for better semantics where they are used in the code.
- The Is_dragon_mail and Dragon_to_scales macros are removed.
- The Dragon_scales_to_pm and Dragon_mail_to_pm macros are replaced by
  Dragon_armor_to_pm, which unifies them.
- New macros Is_dragon_scaled_armor, Dragon_armor_to_scales,
  Dragon_armor_to_pm, and mndx_to_dragon_scales.
- An argument of 4 to polyself() indicates a polymorph from trying to enchant
  dragon scales onto oneself (so that polymorph control only allows the player
  to decline to turn into a dragon rather than being able to become anything).
