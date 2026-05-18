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

### Interface changes

- An extended achievement (which does not correspond to any bits in the xlogfile
  achieve field) is awarded for solving the Wizard's Puzzle, which is encoded in
  achieveX as "solved_wiz_puzzle".
- Monster lookup shows whether or not a monster can be saddled.

### Architectural changes

- Add a make_spanning_tree function in the Lua nhlib file, which constructs a
  general spanning tree from a given unweighted graph.
