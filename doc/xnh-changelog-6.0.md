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
- 21 new themed rooms:
  - A cluster of four square interconnected 3x3 rooms.
  - A barbell-shaped room connecting two 3x3 rooms with a doored, walled
    corridor (vertical and horizontal).
  - A room containing a bunch of graffiti.
  - A room filled with small 1x1 pillars. Sometimes, these pillars will be
    trees instead, and then the room will contain wood nymphs, and possibly
    statues and a figurine of wood nymphs.
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

### Architectural changes

- tipcontainer() is generalized to work for spilling objects out of a container
  on some space other than the player' other than the player's.
- A bunch of enhancements, fixes, and refactors of the level parser, initially
  implemented as patches versus vanilla. In particular, themed room subrooms
  and special rooms can now be filled.
- Monsters created in a bones file to represent a dead player (ghost, mummy,
  etc) are tracked with a new "ebones" mextra struct.
- A new body part, TORSO, is added.
- New function gear_simple_name, which collects all the various foo_simple_name
  functions for gear pieces into one function.
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
