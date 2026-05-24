## xNetHack 10.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 9.0, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack 10.0 is df06fc3. Note
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
made in previous xNetHack versions, xNetHack 10.0 contains the following
changes:

### Gameplay changes

- Randomly generated amulets other than the amulet of reflection now get
  assigned a material from an amulet-specific list rather than using the same
  material list as shields of reflection. This list has a more even distribution
  of possible materials, and also includes gemstone, stone, and bone.
  - Amulets of reflection still use the shield of reflection material list.
- Human Rangers can now be played as Lawful.
- Rangers are 4 times as good as other roles at finding pits and spiked pits.
- Add the giant scorpion.
  - It is basically a scaled-up, faster version of the regular scorpion.
  - Unlike the smaller scorpion, it has a grapple attack.
  - Regular scorpions can grow up into them.
  - They appear on the Ranger quest.
  - They are orange to be distinguishable from regular scorpions.
- Allow large s-class monsters (giant spider and giant scorpion) to be saddled
  and ridden.
- Add a new material: hard light, which is a magic construct that is
  lightweight and fairly strong.
- Rename the Longbow of Diana to the Longbow of Orion.
- Add a new object: arrow of light. It is a weightless arrow made of hard light
  that deals exceptionally high damage but disintegrates after hitting
  something or falling to the floor. Monsters it kills in one shot will leave
  no corpse, and monsters it fails to kill will be blinded and stunned.
  - The arrow of light deals half its normal damage if you are not a Ranger who
    has killed Scorpius.
- The Longbow of Orion's invoke effect now creates arrows of light, granting
  either 3 of them, or if that would make there be more than 5 arrows of light
  in the game, as many as can be created to bring the total to 5.
  - If you are not a Ranger, only one arrow of light will be created.
  - If there are already 5 in the game, it creates ordinary arrows enchanted
    equal to the Longbow's enchantment.
  - These ordinary arrows will be elven arrows if you are an elf and orcish
    arrows if you are an orc.
- Orion is replaced as the Ranger quest leader with Cedalion, his lieutenant
  and guide.
- Wielding the Longbow of Orion puts a floor on the amount of multishot you get
  from arrows fired from it. This floor is 1 (no multishot) if you wouldn't be
  able to multishoot with it, or 2 if you can only shoot a maximum of 2 arrows,
  but otherwise it's 3.
- The Ranger quest has received an overhaul:
  - The home level is lightly redesigned (from when it was redone in xNetHack
    3.0); monsters will no longer spawn in the sacred grove.
  - The upper filler level is no longer a cavern fill with trees as the
    background but is still somewhat similar, consisting of randomized trails
    through a forest leading into one of two caves.
  - The story has not been severely changed, but more closely tracks the
    mythological conflict between Orion and Scorpius. The largest change is
    that Orion is already dead and the mission is one of vengeance.
  - You receive the Longbow of Orion from Cedalion as soon as you get assigned
    the quest. However, it will not create arrows of light until Scorpius is
    killed, though it can still create normal arrows and its passive extrinsics
    still apply.
  - Scorpions and other s-class monsters generate more frequently on the quest,
    with centaurs generating less frequently.
  - The wumpus on the locate level is much stronger than ordinary wumpuses. It
    has much more HP, is significantly faster, and its bite does a huge amount
    of damage, enough to kill most characters.
  - The locate level is changed to be more cave-like and less
    artificial-feeling while still being a series of 20 chambers arranged like
    the vertices of a dodecahedron.
    - Passages between the chambers are now corridor terrain rather than room
      terrain.
    - The upstairs and downstairs cavern are now randomized instead of fixed.
    - The wumpus is waiting on the downstairs rather than being asleep.
    - The two random "bottomless pit" chambers are open air terrain. Falling
      into these is an instadeath. (The spiked pit traps are removed to avoid
      confusion.)
    - The centaurs spawn in two groups in random chambers rather than being
      spread out.
  - The lower filler level(s) are similar to the upper one, but the map is
    dark with some random clouds, there is no cave, and there are many more
    scorpions than centaurs.
  - The goal level is another forest traverse, but with more fog and a river.
    Scorpius, a few centaurs, and a lot of scorpions inhabit a clearing across
    the river.
  - Scorpius is buffed:
    - He has a higher base level, giving him more HP.
    - His AC is now 0 instead of 10, in line with most other nemeses.
    - His base MR is now 50.
    - He is faster at speed 18 instead of 12.
    - One of his claw attacks is now a grabbing attack.
    - He is now huge, though his attacks do *not* cause knockback.
    - If he stings you a second time while you are already ill and you have
      intrinsic poison resistance, you will lose that intrinsic.
    - When below half health, he can summon scorpions and giant scorpions.
    - On the other hand, he will no longer covetous warp.
  - All levels of the quest have barren trees that contain no fruit or bees.
- Ice devils and cold traps no longer strip temporary cold resistance.

### Interface changes

- An extended achievement (which does not correspond to any bits in the xlogfile
  achieve field) is awarded for solving the Wizard's Puzzle, which is encoded in
  achieveX as "solved_wiz_puzzle".
- Monster lookup shows whether or not a monster can be saddled.

### Architectural changes

- Add a make_spanning_tree function in the Lua nhlib file, which constructs a
  general spanning tree from a given unweighted graph.
- Change the semantics of the "arboreal" level flag.
  - Instead of meaning "rock should appear and behave as trees", it now means
    "trees should not contain fruit or bees by default".
  - It is still possible for a level designer to create individual trees that
    do contain fruit or bees on an arboreal level by using des.terrain.
- Add a makemon flag MM_NOITEM which suppresses the creation of a random item
  for a snake or scorpion to hide underneath.
