## xNetHack 9.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 8.0, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack 9.0 is df06fc3. Note
that because 3.7.0 is still in development status, xNetHack contains major
changes including new monsters, new objects, themed rooms, and other things
*not* documented in this file or other xNetHack changelogs. See doc/fixes37.0
for the DevTeam's changes.

The xNetHack page at the NetHackWiki, https://nethackwiki.com/wiki/XNetHack,
attempts to describe these changes in a way that's better formatted and more
friendly to players. However, the wiki page might be out of date; in case of
conflicting information, this changelog and others in this directory are more
up-to-date than the wiki page, and the commit messages are more up-to-date than
this changelog.

On top of any changes made by the NetHack devteam on 3.7, and any changes
made in previous xNetHack versions, xNetHack 9.0 contains the following
changes:

### Gameplay changes

- The blessed scroll of gold detection also detects gems across the level.
- Every lock in the game may be unopenable by using a credit card. Boxes have a
  40% chance, and doors have a chance that starts at 10% and increases with
  depth.
- Also, if the card is cursed or you are fumbling, the card may slip through the
  lock, ending up inside a box or on the other side of a door.
- Cockatrice nests may contain chickatrices and cockatrice eggs.
- Eating troll meat provides 2d6 turns of intrinsic regneneration.
- Drinking from a magic fountain when either HP or Pw is below 60% may give a
  healing spring effect that increases both maximums by 1 and fully restores
  both, instead of the usual gain ability effect. The fountain still becomes
  nonmagical afterward.
- Monsters will consider if their regular melee attacks might deal a lot more
  damage to the hero than their offensive items would, and may subsequently
  decide to attack in melee rather than using that offensive item.
- Rolling boulders, eating carrots, and occasionally trying to read a dusty
  spellbook (via the dust making you sneeze) causes enough noise to wake nearby
  monsters.
- Monsters stepping on a squeaky board near you can wake you up.
- Leprechauns have a one-sided grudge against gold golems and gold dragons.
- Rock moles can eat gems and rocks (and other objects made of stone, like
  marble wands). Dilithium crystals grant them extra speed. The player can eat
  boulders and (empty) statues when polymorphed into one, but other rock moles
  won't.
- Gain energy potions are guaranteed to restore more energy. A blessed one
  restores at least 40% of your energy maximum and an uncursed one restores at
  least 25%.
- The potion of gain energy can be alchemized by combining the potions of full
  healing and gain ability.
- Sitting down can partly wipe engravings on your space.
- The chance of finding a secret door or passage via searching is no longer
  dependent on Luck.
- Zombies cannot open closed doors.
- Zombies additionally are immune to being scared by any source, including the
  scroll of scare monster.
- Orcs, barbarians, and cavemen have a Wisdom-dependent chance of avoiding the
  urge to take a bath in a fountain.
- Scrolls of earth work again in all levels besides the non-Earth Planes.
- Yeenoghu no longer carries a wand of wishing; instead there is a buried chest
  on his level that may contain it.
- You will only abuse Wisdom when trying to clumsily throw weapons without the
  proper launcher when you actually perceive you're throwing them at a monster,
  but the Wisdom penalty now happens 100% of the time instead of 20%.
- It is no longer possible to levelport downwards or teleport within the same
  level in the Quest before the nemesis is killed.
- Being adjacent to a hezrou may nauseate you, with the odds increasing if there
  are multiple hezrous.
- Hill giants are now the weakest giant, taking over the statblock of the stone
  giant (equivalent to the plain giant) with its 2d8 weapon attack.
- Stone giants are slightly stronger, taking over the statblock of the hill
  giant with its 2d10 weapon attack. They are now able to rip boulders out of
  the floor when they don't already have one, which creates a pit on that
  square.
- Fire giants have a 2d4 fire touch attack.
- Frost giants have a 3d4 cold touch attack and their own encyclopedia entry.
- Storm giants have a 4d4 ranged lightning bolt attack.
- Hrymr's sword on the Valkyrie quest is now named Ice. (Its stats are otherwise
  unchanged, and it is not an artifact.)
- Levels that have been changed following some event, such as the Valkyrie
  locate level, are ineligible to leave bones.
- Luck plays a reduced factor in to-hit calculations, now adding +1 to hit for
  every 3 points of luck, rounded to the next greatest increment: +1 for 1 to 3
  luck, +2 for 4 to 6 luck, -1 for -1 to -3 luck, etc.
- Critical hits on martial arts attacks made at Expert or higher skill may
  disarm an enemy of its weapon and knock it to an adjacent space, with higher
  chance for higher levels of skill.
- Being Skilled or better in martial arts or bare-handed combat now confers an
  bonus to AC when not wearing any body armor or a shield.
- New artifact The Amulet of Storms: a chaotic amulet of flying that grants
  shock resistance when worn and allows you to chat to hostile vortices, air
  elementals, and storm giants to pacify them. It also prevents you from being
  paralyzed by lightning on the Plane of Air.
- The bigroom variant which contains patches of light and darkness has a new
  variation with randomly generated "spotlights" of lit area.
- Martial arts users with at least Basic skill (all monks and samurai) no longer
  take damage or wound their legs when kicking inadvisable things.
- Lawful minions' weapons can no longer rarely be very highly enchanted; swords
  they get will be +0 to +3, and maces +3 to +6.
- Attacking with no weapon is now handled the same as vanilla, meaning multiple
  attacks will hit with alternating hands, and you will always lead with your
  dominant hand. This mainly has implications for if you are hitting monsters
  with rings made of materials they hate.
- Object merging now follows vanilla rules - there are a few cases where objects
  won't immediately merge such as being blind or hallucinating and not fully
  knowing one of the objects.
- Pauper characters get their normal race-based skill caps (but not any actual
  skill in them, same as everyone else).
- Monsters break ammo less often - 25% of the time assuming it isn't influenced
  by beatitude or enchantment.
- Zapping oneself with a wand of make invisible confers 30+d15 turns of
  invisibility, down from 50+d50.
- A vampire reforming from fog cloud form on a wooden door's space will always
  smash it; on an iron door's space, they will never smash it unless the door
  was trapped to explode.
- Vanilla artifact gift rebalance is mostly unchanged, except that it still only
  considers the number of gifts you have already received and artifacts randomly
  generating won't harm it.
  - Mirror Brand will be +1 if gifted and requires a sacrifice of difficulty 5+.
  - The Apple of Discord requires a sacrifice of difficulty 7+.
  - The Amulet of Storms requires a sacrifice of difficulty 2+.
- Bone items now have a +1 damage bonus against ghosts and shades.
- Saving grace will not alleviate death from falling into open air and
  splattering on the ground below.
- Grappling hooks can no longer generate as plastic or glass.
- Glass weapons as well as armor are subject to the 3-stages-of-cracking system,
  instead of randomly shattering. Like armor, a glass weapon taking a light
  impact has a 10% chance of cracking (unless it's tempered).
- Heavy objects that hit non-tempered glass armor (boulders, iron balls, etc)
  will unconditionally crack it.
- Glass piercers that fall on a monster wearing a non-tempered glass helm will
  still shatter the helm in a single hit.
- Falling rocks from traps or from zapping at the ceiling can crack non-tempered
  glass helmets; this is considered a light impact so it only happens 10% of the
  time.
- Enchanting a mundane harp, flute, whistle, or drum will identify the magical
  counterpart it turns into. (Not so for tooled horns or sacks, which could turn
  into different items and are ambiguous).
- Dipping for Excalibur can only be done at level 5 if the hero is a Knight who
  has never abused their alignment. Otherwise, the minimum level is 10.
- The Monk quest has received an overhaul:
  - Note: the home and locate level and some parts of the quest text were
    modified in previous xNetHack versions. Those largely haven't changed in
    this version.
  - The Monk quest leader is now named the Chan-Sune Lama.
  - The Monk lower filler levels are no longer standard room-and-corridor: they
    are an interconnected set of rooms with various types of things blocking the
    ways between them.
  - The Monk goal level is a chamber, smaller, walled in, and with several
    wings. It contains mostly the same enemies as the former goal level, but
    does not contain any lava.
  - The Monk quest text is now fully unique and does not contain any more
    copy-pasted lines from the Priest quest text.
  - The Monk quest is only counted as completed when you return to the quest
    leader having killed Master Kaen, not merely by returning the Eyes of the
    Overworld.
  - The 5 random comestibles in a chest on the Monk quest start level are
    replaced with 4 food rations.
  - Master Kaen can phase through walls and boulders, and casts summon
    elementals instead of summon insects.
- One giant on the Valkyrie quest goal level may have a fire horn.
- You can now play a Barbarian as a dwarf. Barbarians can consequently be
  lawful.
- You can now play a Tourist as a gnome.
- You can now play a Rogue as an elf.
- Several xNetHack themed rooms have been converted into themed room fills
  so that they can appear in rooms of varying shape, not just rectangles:
  - "Graffiti room"
  - "Scummy moldy room"
  - "Gas spore den"
  - "Water temple"
  - "Meadow"
- The xNetHack "Garden" themed room is removed (but the vanilla "Garden" themed
  room fill is still present).
- The "Garden" themed room, which contains wood nymphs, now only generates a
  couple levels after wood nymphs can start appearing, and it may contain some
  gnome statues.
- 3 new themed room fills:
  - "Minesweeper": contains buried land mines and engravings which denote how
    many mines there are adjacent to that spot.
  - "Monster sauna": steam vortices, fog clouds, clouds of steam, towels, and a
    few random monsters
  - "Scattered gems": contains a number of gems (but no rocks or gray stones)
    strewn about
- 7 new themed rooms:
  - "The Casque of Amontillado": contains a walled-off closet in which an
    unfortunate has been imprisoned until their death
  - "Triple Rhombus V2": three rhombi connected in a horizontal row. Eligible
    for random fills.
  - "Spikes": three upward pointing connected triangles. Eligible for random
    fills.
  - "Pennants": three downward pointing connected triangles. Eligible for random
    fills.
  - "Wizard study": a disconnected 3x3 vault-like room containing some
    spellbooks, scrolls, other magic items, and a teleportation trap.
  - "Ring, medium": a circular 1-space-wide ring. Eligible for random fills.
  - "Ring, large": a larger version of the above circular ring.
- The "Mini maze" themed room has a slightly larger maze.
- The "Dragon hall" themed room can now appear when the level difficulty is 21,
  rather than 23.

### Interface changes

- Wishing for "leather armor" will give the player a leather light armor (its
  equivalent from vanilla).
- Pets are referred to with their correct gender pronoun if they have one,
  rather than "it".
- When you #chat and are next to only one observable monster, you will
  automatically attempt to chat to that monster without being prompted for a
  direction to chat in.
- When you are a few turns from temporary invisibility expiring, you receive a
  message warning you that you're becoming visible again.
- When farlooking a closed and unlocked door you are next to, it will show up as
  "unlocked door" instead of the generic "closed door".
- Produce a message when a nymph teleports away after a successful theft.
- When polymorphed into a monster with an explosion attack, you can #monster to
  intentionally explode. (This was already possible by attacking a monster or
  force-fighting thin air.)
- Escaping out of a polymorph prompt no longer causes a random polymorph; it
  instead gives instructions on how to explicitly request that and prompts
  again. (However, escaping or failing to enter a valid form 5 times still
  causes a random polymorph.)
- The legacy text at the start of the game uses different role-based opening
  lines that replace the standard "It is written in the Book of [deity]:"
- YAFMs:
  - Chatting to a troll (even though you can't see it's a troll) while
    hallucinating.
  - Being life-saved while hallucinating.
  - Monster drinking a potion where you can't see while hallucinating.
- There is now a pager message for the bridge in the Valkyrie locate level being
  restored.
- There is no longer a S_engraving character. Instead, there are the S_engroom
  and S_engrcorr ones from vanilla, which represent engravings in rooms (or
  other non-corridor terrain) and corridors respectively. However, the default
  ASCII glyphs for both of these have been changed to the former default glyph
  for S_engraving, "~", instead of vanilla's "`". They can still be mapped to
  a symbol of the player's preference via the SYMBOLS option.
- Paranoid swim and paranoid trap are reverted to vanilla behavior, meaning if
  you don't have paranoid trap set, you aren't prompted to confirm before moving
  into a not-obviously-harmless trap, and if you do have it set, you only need
  to type "y" to move in instead of "yes" (unless paranoid_confirmation:Confirm
  is also set).
- Secret door detection wands no longer produce a "You find many hidden bugs on
  the floor" message since they always unambiguously identify themselves.
- Erodeproof glass objects are now referred to as "tempered" rather than
  "shatterproof", though shatterproof will still be treated as valid for wishes.
  "Indestructible" is still used for glass items which have been made immune to
  shock damage from erodeproofing - i.e. rings and wands.
- Inventory weights are formatted as "(x aum)" instead of "{x}".
- Archfiends (besides Juiblex) now render as regular magenta &, the same as
  vanilla. The Riders are still bright magenta.
- Vrocks are now green, the same as vanilla.
- Erinyes are now orange.
- Print a message "You crash into the ground" when taking damage falling down a
  hole or a trap door.
- Nonmagic flutes and harps display as "mundane flute" and "mundane harp" when
  fully identified.
- Attribute display from enlightenment shows whether and how much you ever
  abused your alignment (in wizard mode, you are also shown the exact amount).
- Iron doors have distinct defsyms and tiles from ordinary doors.
  - The new symbols are S_voidoor (vertical open iron), S_vcidoor (vertical
    closed iron), S_hoidoor (horizontal open iron), and S_hcidoor (horizontal
    closed iron).
  - Like all other symbols, these can be redefined with SYMBOLS in the
    configuration options.
  - The new default tiles have the same shape as regular doors, but use the iron
    color palette instead of brown.

### Architectural changes

- The monster ID of the quest nemesis is now tracked similar to the quest
  leader's.
- current_holidays() now lives in calendar.c.
- Hash functions now live in rnd.c.
- Some (but not all) xNetHack-added functions are annotated with the NONNULL*
  attributes.
