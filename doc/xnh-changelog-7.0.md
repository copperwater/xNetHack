## xNetHack 7.0 Changelog

This is a major version of xNetHack. It is based directly on xNetHack 6.0, and
is a fork off the vanilla NetHack 3.7.0 development version release.

The most recent vanilla commit incorporated into xNetHack is d6d42f5. Note that
because 3.7.0 is still in development status, xNetHack may contain major changes
including new monsters, new objects, themed rooms, and other things *not*
documented in this file or other xNetHack changelogs. See doc/fixes37.0 for the
DevTeam's changes.

The xNetHack page at the NetHackWiki, https://nethackwiki.com/wiki/XNetHack,
attempts to describe these changes in a way that's better formatted and more
friendly to players. However, the wiki page might be out of date; in case of
conflicting information, this changelog and others in this directory are more
up-to-date than the wiki page, and the commit messages are more up-to-date than
this changelog.

On top of any changes made by the NetHack devteam on 3.7, and any changes
made in previous xNetHack versions, xNetHack 7.0 contains the following
changes:

### Gameplay changes

- 5% of humanoid non-lord A monsters (Angels and Aleaxes) generate with a harp,
  which is magic 20% of the time.
- 1/40 of dwarves in the Mines and 1/8 of dwarves outside the Mines generate
  with d3 booze potions.
- French fried and deep fried tins give more nutrition (80 and 100
  respectively.)
- New random tin flavor "stale", giving only 25 nutrition.
- Skeletons rarely drop bone skeleton keys.
- Blowing up a bag of holding gives some experience points, identifies the bag
  of holding, and identifies the wand of cancellation if that is the cause of
  the explosion.
- Ghosts will not make themselves uninvisible in an attempt to frighten you if
  you are already paralyzed.
- Elven ring mail, in its default copper form, provides 2 AC, and provides 4 AC
  if made of mithril.

### Interface changes

- Upon first entering the bottom Sokoban level, you get a message about grooves
  on the floor.
- Shopkeepers address certain players with a role-specific title, such as
  Knights or high-level Monks or Healers.
- The "You hear someone cursing shoplifters" message uses a shopkeeper's real
  name if you have visited their shop previously.
- Monks will stop seeing "You feel guilty" messages after eating meat enough
  times.
- The levels with the stairs to the Mines, Sokoban, and Vlad's Tower have
  special ambient level sounds. The Sokoban and Vlad ones only persist until you
  have finished the first level of Sokoban and killed Vlad, respectively.
- Swap the chaotic and unaligned altar colors to be in line with vanilla:
  chaotic is now black and unaligned red.

### Architectural changes

- Add a function sokoban_solved() which determines whether a given Sokoban level
  is solved.
